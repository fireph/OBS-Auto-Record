#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use iced::{
    widget::{
        button, column, container, row, scrollable, text, text_input, Space,
        horizontal_rule
    },
    Task, Element, Length, Subscription, Theme,
    time,
};
use sysinfo::{System, ProcessesToUpdate};
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::Arc;
use tokio::sync::Mutex;
use serde::{Deserialize, Serialize};
use anyhow::Result;

mod config;
mod obs;
mod process_monitor;

use config::Config;
use obs::ObsManager;
use process_monitor::ProcessMonitor;

#[derive(Debug, Clone)]
pub enum Message {
    AddGame,
    GameSelected(PathBuf),
    RemoveGame(usize),
    EditGameName(usize, String),
    ToggleDarkMode,
    UpdateObsUrl(String),
    TestObsConnection,
    ObsConnectionResult(Result<String, String>),
    ObsConnectionUpdated(Result<(), String>),
    ProcessUpdate(HashMap<String, bool>),
    StartedRecording(String),
    StoppedRecording(String),
    Error(String),
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameEntry {
    pub path: PathBuf,
    pub name: String,
    pub is_running: bool,
    pub is_recording: bool,
}

impl GameEntry {
    fn new(path: PathBuf) -> Self {
        let name = path
            .file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or("Unknown Game")
            .to_string();
        
        Self {
            path,
            name,
            is_running: false,
            is_recording: false,
        }
    }
}

#[derive(Debug)]
pub struct App {
    games: Vec<GameEntry>,
    obs_url: String,
    dark_mode: bool,
    obs_connection_status: String,
    obs_manager: Arc<Mutex<ObsManager>>,
    process_monitor: ProcessMonitor,
    config: Config,
}

impl App {
    fn new() -> (Self, Task<Message>) {
        let config = Config::load().unwrap_or_default();
        let obs_manager = Arc::new(Mutex::new(ObsManager::new()));
        let process_monitor = ProcessMonitor::new();

        let app = Self {
            games: config.games.clone(),
            obs_url: config.obs_url.clone(),
            dark_mode: config.dark_mode,
            obs_connection_status: "Not connected".to_string(),
            obs_manager,
            process_monitor,
            config,
        };

        // Try to connect to OBS on startup
        let connect_task = if !app.obs_url.is_empty() {
            Task::perform(
                obs::test_connection(app.obs_url.clone()),
                Message::ObsConnectionResult,
            )
        } else {
            Task::none()
        };

        (app, connect_task)
    }

    fn title(&self) -> String {
        "OBS Game Recorder".to_string()
    }

    fn theme(&self) -> Theme {
        if self.dark_mode {
            Theme::Dark
        } else {
            Theme::Light
        }
    }

    fn update(&mut self, message: Message) -> Task<Message> {
        match message {
            Message::AddGame => {
                Task::perform(pick_game_file(), Message::GameSelected)
            }
            Message::GameSelected(path) => {
                if path.exists() {
                    let game = GameEntry::new(path);
                    self.games.push(game);
                    self.save_config();
                }
                Task::none()
            }
            Message::RemoveGame(index) => {
                if index < self.games.len() {
                    let game = &self.games[index];
                    
                    // If the game being removed is currently recording, stop recording
                    if game.is_recording {
                        let obs_manager = self.obs_manager.clone();
                        let game_name = game.name.clone();
                        
                        // Remove the game first
                        self.games.remove(index);
                        self.save_config();
                        
                        // Then stop recording
                        Task::perform(
                            async move {
                                let mut manager = obs_manager.lock().await;
                                match manager.stop_recording().await {
                                    Ok(_) => Message::StoppedRecording(game_name),
                                    Err(e) => Message::Error(format!("Failed to stop recording when removing game: {}", e)),
                                }
                            },
                            |msg| msg,
                        )
                    } else {
                        // Just remove the game if not recording
                        self.games.remove(index);
                        self.save_config();
                        Task::none()
                    }
                } else {
                    Task::none()
                }
            }
            Message::EditGameName(index, name) => {
                if let Some(game) = self.games.get_mut(index) {
                    game.name = name;
                    self.save_config();
                }
                Task::none()
            }
            Message::ToggleDarkMode => {
                self.dark_mode = !self.dark_mode;
                self.save_config();
                Task::none()
            }
            Message::UpdateObsUrl(url) => {
                self.obs_url = url;
                self.save_config();
                if !self.obs_url.is_empty() {
                    Task::perform(
                        obs::test_connection(self.obs_url.clone()),
                        Message::ObsConnectionResult,
                    )
                } else {
                    self.obs_connection_status = "Not connected".to_string();
                    Task::none()
                }
            }
            Message::TestObsConnection => {
                if !self.obs_url.is_empty() {
                    Task::perform(
                        obs::test_connection(self.obs_url.clone()),
                        Message::ObsConnectionResult,
                    )
                } else {
                    Task::none()
                }
            }
            Message::ObsConnectionResult(result) => {
                match result {
                    Ok(status) => {
                        self.obs_connection_status = status;
                        // Update OBS manager with new URL
                        let obs_manager = self.obs_manager.clone();
                        let url = self.obs_url.clone();
                        Task::perform(
                            async move {
                                let mut manager = obs_manager.lock().await;
                                match manager.connect(&url).await {
                                    Ok(_) => Message::ObsConnectionUpdated(Ok(())),
                                    Err(e) => Message::ObsConnectionUpdated(Err(e.to_string())),
                                }
                            },
                            |msg| msg,
                        )
                    }
                    Err(error) => {
                        self.obs_connection_status = format!("Error: {}", error);
                        Task::none()
                    }
                }
            }
            Message::ObsConnectionUpdated(result) => {
                match result {
                    Ok(_) => {
                        // Connection updated successfully, no need to log anything
                        Task::none()
                    }
                    Err(error) => {
                        // Only log actual errors
                        tracing::error!("Failed to update OBS manager connection: {}", error);
                        self.obs_connection_status = format!("Connection error: {}", error);
                        Task::none()
                    }
                }
            }
            Message::ProcessUpdate(running_processes) => {
                let mut commands = Vec::new();
                
                for game in &mut self.games {
                    let exe_name = game.path
                        .file_name()
                        .and_then(|n| n.to_str())
                        .unwrap_or("")
                        .to_string();
                    
                    let is_running = running_processes.get(&exe_name).copied().unwrap_or(false);
                    let was_running = game.is_running;
                    
                    game.is_running = is_running;
                    
                    // Game started
                    if is_running && !was_running && !game.is_recording {
                        let obs_manager = self.obs_manager.clone();
                        let game_name = game.name.clone();
                        commands.push(Task::perform(
                            async move {
                                let mut manager = obs_manager.lock().await;
                                match manager.start_recording().await {
                                    Ok(_) => Message::StartedRecording(game_name),
                                    Err(e) => Message::Error(format!("Failed to start recording: {}", e)),
                                }
                            },
                            |msg| msg,
                        ));
                    }
                    // Game stopped
                    else if !is_running && was_running && game.is_recording {
                        let obs_manager = self.obs_manager.clone();
                        let game_name = game.name.clone();
                        commands.push(Task::perform(
                            async move {
                                let mut manager = obs_manager.lock().await;
                                match manager.stop_recording().await {
                                    Ok(_) => Message::StoppedRecording(game_name),
                                    Err(e) => Message::Error(format!("Failed to stop recording: {}", e)),
                                }
                            },
                            |msg| msg,
                        ));
                    }
                }
                
                Task::batch(commands)
            }
            Message::StartedRecording(game_name) => {
                if let Some(game) = self.games.iter_mut().find(|g| g.name == game_name) {
                    game.is_recording = true;
                }
                Task::none()
            }
            Message::StoppedRecording(game_name) => {
                if let Some(game) = self.games.iter_mut().find(|g| g.name == game_name) {
                    game.is_recording = false;
                }
                Task::none()
            }
            Message::Error(error) => {
                tracing::error!("Application error: {}", error);
                Task::none()
            }
        }
    }

    fn subscription(&self) -> Subscription<Message> {
        // Monitor processes every 5 seconds to reduce CPU/memory usage
        time::every(std::time::Duration::from_secs(5))
            .map(|_| {
                let mut system = System::new();
                system.refresh_processes(ProcessesToUpdate::All, false);
                
                let mut running_processes = HashMap::new();
                for process in system.processes().values() {
                    let name = process.name().to_string_lossy().to_string();
                    running_processes.insert(name, true);
                }
                
                Message::ProcessUpdate(running_processes)
            })
    }

    fn view(&self) -> Element<Message> {
        let header = row![
            text("OBS Game Recorder")
                .size(24)
                .width(Length::Fill),
            button(if self.dark_mode { "Light" } else { "Dark" })
                .on_press(Message::ToggleDarkMode)
                .padding(8),
        ]
        .align_y(iced::Alignment::Center)
        .spacing(10);

        let obs_section = column![
            text("OBS WebSocket Settings").size(18),
            row![
                text_input("ws://localhost:4455", &self.obs_url)
                    .on_input(Message::UpdateObsUrl)
                    .width(Length::Fill),
                button("Test")
                    .on_press(Message::TestObsConnection)
                    .padding([5, 10]),
            ]
            .spacing(10),
            text(&self.obs_connection_status)
                .color(if self.obs_connection_status.starts_with("Connected") {
                    iced::Color::from_rgb(0.0, 0.8, 0.0)
                } else if self.obs_connection_status.starts_with("Error") {
                    iced::Color::from_rgb(0.8, 0.0, 0.0)
                } else {
                    iced::Color::BLACK
                }),
        ]
        .spacing(10);

        let games_header = row![
            text("Games to Monitor").size(18),
            Space::with_width(Length::Fill),
            button("Add Game")
                .on_press(Message::AddGame)
                .padding([5, 10]),
        ]
        .align_y(iced::Alignment::Center);

        let games_list: Element<Message> = if self.games.is_empty() {
            container(
                text("No games added yet. Click 'Add Game' to get started!")
                    .color(iced::Color::from_rgb(0.5, 0.5, 0.5))
            )
            .padding(20)
            .center_x(Length::Fill)
            .into()
        } else {
            scrollable(
                column(
                    self.games
                        .iter()
                        .enumerate()
                        .map(|(i, game)| {
                            let status_text = if game.is_recording {
                                "● Recording"
                            } else if game.is_running {
                                "● Running" 
                            } else {
                                "○ Stopped"
                            };

                            container(
                                row![
                                    column![
                                        text_input("Game name", &game.name)
                                            .on_input(move |name| Message::EditGameName(i, name))
                                            .width(Length::Fill),
                                        text(format!("Path: {}", game.path.display()))
                                            .size(12)
                                            .color(iced::Color::from_rgb(0.6, 0.6, 0.6)),
                                    ]
                                    .width(Length::Fill)
                                    .spacing(5),
                                    column![
                                        text(status_text).size(14)
                                            .color(if game.is_recording {
                                                iced::Color::from_rgb(0.8, 0.2, 0.2) // Red for recording
                                            } else if game.is_running {
                                                iced::Color::from_rgb(0.2, 0.8, 0.2) // Green for running
                                            } else {
                                                iced::Color::from_rgb(0.6, 0.6, 0.6) // Gray for stopped
                                            }),
                                        button("Remove")
                                            .on_press(Message::RemoveGame(i))
                                            .padding([3, 8]),
                                    ]
                                    .align_x(iced::Alignment::End)
                                    .spacing(5),
                                ]
                                .align_y(iced::Alignment::Center)
                                .spacing(15)
                            )
                            .padding(15)
                            .into()
                        })
                        .collect::<Vec<_>>()
                )
                .spacing(10)
            )
            .height(Length::Fill)
            .into()
        };

        container(
            column![
                header,
                horizontal_rule(2),
                Space::with_height(10),
                obs_section,
                Space::with_height(20),
                horizontal_rule(1),
                Space::with_height(10),
                games_header,
                Space::with_height(10),
                games_list,
            ]
            .spacing(10)
            .max_width(800)
        )
        .padding(20)
        .center_x(Length::Fill)
        .into()
    }
}

impl App {
    fn save_config(&mut self) {
        self.config.games = self.games.clone();
        self.config.obs_url = self.obs_url.clone();
        self.config.dark_mode = self.dark_mode;
        
        if let Err(e) = self.config.save() {
            tracing::error!("Failed to save config: {}", e);
        }
    }
}

async fn pick_game_file() -> PathBuf {
    let file = rfd::AsyncFileDialog::new()
        .add_filter("Executable", &["exe"])
        .set_title("Select Game Executable")
        .pick_file()
        .await;

    file.map(|f| f.path().to_path_buf())
        .unwrap_or_default()
}

fn main() -> iced::Result {
    // Only initialize logging in debug mode
    #[cfg(debug_assertions)]
    tracing_subscriber::fmt()
        .with_max_level(tracing::Level::DEBUG)
        .init();

    // In release mode, use minimal error-only logging
    #[cfg(not(debug_assertions))]
    tracing_subscriber::fmt()
        .with_max_level(tracing::Level::ERROR)
        .with_writer(std::io::sink) // Discard all output in release
        .init();

    iced::application("OBS Game Recorder", App::update, App::view)
        .subscription(App::subscription)
        .theme(App::theme)
        .window_size(iced::Size::new(900.0, 700.0))
        .run_with(App::new)
}