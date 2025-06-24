use iced::{Task, Subscription, Theme, time};
use sysinfo::{System, ProcessesToUpdate};
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::Mutex;

use crate::config::Config;
use crate::obs::ObsManager;
use crate::process_monitor::ProcessMonitor;
use crate::messages::Message;
use crate::game::GameEntry;

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
    pub fn new() -> (Self, Task<Message>) {
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
                crate::obs::test_connection(app.obs_url.clone()),
                Message::ObsConnectionResult,
            )
        } else {
            Task::none()
        };

        (app, connect_task)
    }

    pub fn title(&self) -> String {
        "OBS Auto Record".to_string()
    }

    pub fn theme(&self) -> Theme {
        if self.dark_mode {
            Theme::Dark
        } else {
            Theme::Light
        }
    }

    pub fn update(&mut self, message: Message) -> Task<Message> {
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
                        crate::obs::test_connection(self.obs_url.clone()),
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
                        crate::obs::test_connection(self.obs_url.clone()),
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

    pub fn subscription(&self) -> Subscription<Message> {
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

    pub fn view(&self) -> iced::Element<Message> {
        crate::ui::view(self)
    }

    fn save_config(&mut self) {
        self.config.games = self.games.clone();
        self.config.obs_url = self.obs_url.clone();
        self.config.dark_mode = self.dark_mode;
        
        if let Err(e) = self.config.save() {
            tracing::error!("Failed to save config: {}", e);
        }
    }

    // Getters for UI
    pub fn games(&self) -> &[GameEntry] {
        &self.games
    }

    pub fn obs_url(&self) -> &str {
        &self.obs_url
    }

    pub fn dark_mode(&self) -> bool {
        self.dark_mode
    }

    pub fn obs_connection_status(&self) -> &str {
        &self.obs_connection_status
    }
}

async fn pick_game_file() -> std::path::PathBuf {
    let file = rfd::AsyncFileDialog::new()
        .add_filter("Executable", &["exe"])
        .set_title("Select Game Executable")
        .pick_file()
        .await;

    file.map(|f| f.path().to_path_buf())
        .unwrap_or_default()
}