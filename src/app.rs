use iced::{Task, Subscription, Theme, time};
use std::sync::Arc;
use tokio::sync::Mutex;

use crate::config::Config;
use crate::obs::ObsManager;
use crate::process_monitor::ProcessMonitor;
use crate::messages::Message;
use crate::game::{GameEntry, GameMode};
use crate::youtube::{YouTubeManager, YouTubeAuth};

#[derive(Debug)]
pub struct App {
    games: Vec<GameEntry>,
    obs_url: String,
    dark_mode: bool,
    obs_connection_status: String,
    obs_manager: Arc<Mutex<ObsManager>>,
    youtube_manager: Arc<Mutex<YouTubeManager>>,
    youtube_auth: YouTubeAuth,
    youtube_integration_enabled: bool,
    config: Config,
    
    // UI state for client secret dialog
    show_client_secret_dialog: bool,
    client_secret_input: String,
}

impl App {
    pub fn new() -> (Self, Task<Message>) {
        let config = Config::load().unwrap_or_default();
        let obs_manager = Arc::new(Mutex::new(ObsManager::new()));
        let youtube_manager = Arc::new(Mutex::new(YouTubeManager::new()));

        let app = Self {
            games: config.games.clone(),
            obs_url: config.obs_url.clone(),
            dark_mode: config.dark_mode,
            obs_connection_status: "Not connected".to_string(),
            obs_manager,
            youtube_manager,
            youtube_auth: config.youtube_auth.clone(),
            youtube_integration_enabled: config.youtube_integration_enabled,
            config,
            show_client_secret_dialog: false,
            client_secret_input: String::new(),
        };

        // Create startup tasks
        let mut tasks = vec![];

        // Try to connect to OBS on startup
        if !app.obs_url.is_empty() {
            tasks.push(Task::perform(
                crate::obs::test_connection(app.obs_url.clone()),
                Message::ObsConnectionResult,
            ));
        }

        // Check YouTube authentication status on startup
        tasks.push(Task::perform(
            async { Message::CheckYouTubeAuth },
            |msg| msg,
        ));

        (app, Task::batch(tasks))
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
                    
                    // If the game being removed is currently active, stop the appropriate action
                    if game.is_recording || game.is_streaming {
                        let obs_manager = self.obs_manager.clone();
                        let game_name = game.name.clone();
                        let is_recording = game.is_recording;
                        
                        // Remove the game first
                        self.games.remove(index);
                        self.save_config();
                        
                        // Then stop recording or streaming
                        Task::perform(
                            async move {
                                let mut manager = obs_manager.lock().await;
                                if is_recording {
                                    match manager.stop_recording().await {
                                        Ok(_) => Message::StoppedRecording(game_name),
                                        Err(e) => Message::Error(format!("Failed to stop recording when removing game: {}", e)),
                                    }
                                } else {
                                    match manager.stop_streaming().await {
                                        Ok(_) => Message::StoppedStreaming(game_name),
                                        Err(e) => Message::Error(format!("Failed to stop streaming when removing game: {}", e)),
                                    }
                                }
                            },
                            |msg| msg,
                        )
                    } else {
                        // Just remove the game if not active
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
            Message::ToggleGameMode(index) => {
                if let Some(game) = self.games.get_mut(index) {
                    // Only allow mode change if game is not currently active
                    if !game.is_recording && !game.is_streaming {
                        game.mode = match game.mode {
                            GameMode::Recording => GameMode::Streaming,
                            GameMode::Streaming => GameMode::Recording,
                        };
                        self.save_config();
                    }
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
                    if is_running && !was_running && !game.is_active() {
                        let obs_manager = self.obs_manager.clone();
                        let youtube_manager = self.youtube_manager.clone();
                        let game_name = game.name.clone();
                        let mode = game.mode.clone();
                        let youtube_enabled = self.youtube_integration_enabled;
                        
                        commands.push(Task::perform(
                            async move {
                                match mode {
                                    GameMode::Recording => {
                                        let mut manager = obs_manager.lock().await;
                                        match manager.start_recording().await {
                                            Ok(_) => Message::StartedRecording(game_name),
                                            Err(e) => Message::Error(format!("Failed to start recording: {}", e)),
                                        }
                                    }
                                    GameMode::Streaming => {
                                        // If YouTube integration is enabled, create a live stream first
                                        if youtube_enabled {
                                            let mut yt_manager = youtube_manager.lock().await;
                                            match yt_manager.create_live_stream(&game_name).await {
                                                Ok(_stream_info) => {
                                                    // TODO: Configure OBS with the stream key
                                                    // For now, just start streaming with existing settings
                                                    drop(yt_manager); // Release the lock
                                                    let mut manager = obs_manager.lock().await;
                                                    match manager.start_streaming().await {
                                                        Ok(_) => Message::StartedStreaming(game_name),
                                                        Err(e) => Message::Error(format!("Failed to start streaming: {}", e)),
                                                    }
                                                }
                                                Err(e) => Message::Error(format!("Failed to create YouTube live stream: {}", e)),
                                            }
                                        } else {
                                            // Just start streaming without YouTube integration
                                            let mut manager = obs_manager.lock().await;
                                            match manager.start_streaming().await {
                                                Ok(_) => Message::StartedStreaming(game_name),
                                                Err(e) => Message::Error(format!("Failed to start streaming: {}", e)),
                                            }
                                        }
                                    }
                                }
                            },
                            |msg| msg,
                        ));
                    }
                    // Game stopped
                    else if !is_running && was_running && game.is_active() {
                        let obs_manager = self.obs_manager.clone();
                        let game_name = game.name.clone();
                        let is_recording = game.is_recording;
                        
                        commands.push(Task::perform(
                            async move {
                                let mut manager = obs_manager.lock().await;
                                if is_recording {
                                    match manager.stop_recording().await {
                                        Ok(_) => Message::StoppedRecording(game_name),
                                        Err(e) => Message::Error(format!("Failed to stop recording: {}", e)),
                                    }
                                } else {
                                    match manager.stop_streaming().await {
                                        Ok(_) => Message::StoppedStreaming(game_name),
                                        Err(e) => Message::Error(format!("Failed to stop streaming: {}", e)),
                                    }
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
            Message::StartedStreaming(game_name) => {
                if let Some(game) = self.games.iter_mut().find(|g| g.name == game_name) {
                    game.is_streaming = true;
                }
                Task::none()
            }
            Message::StoppedStreaming(game_name) => {
                if let Some(game) = self.games.iter_mut().find(|g| g.name == game_name) {
                    game.is_streaming = false;
                }
                Task::none()
            }
            Message::Error(error) => {
                tracing::error!("Application error: {}", error);
                Task::none()
            }

            // YouTube integration messages
            Message::AuthenticateYouTube => {
                let youtube_manager = self.youtube_manager.clone();
                Task::perform(
                    async move {
                        let mut manager = youtube_manager.lock().await;
                        match manager.authenticate().await {
                            Ok(auth) => Message::YouTubeAuthResult(Ok(auth)),
                            Err(e) => Message::YouTubeAuthResult(Err(e.to_string())),
                        }
                    },
                    |msg| msg,
                )
            }
            Message::YouTubeAuthResult(result) => {
                match result {
                    Ok(auth) => {
                        self.youtube_auth = auth;
                        self.youtube_integration_enabled = true;
                        self.save_config();
                        Task::none()
                    }
                    Err(error) => {
                        tracing::error!("YouTube authentication failed: {}", error);
                        Task::none()
                    }
                }
            }
            Message::CheckYouTubeAuth => {
                let youtube_manager = self.youtube_manager.clone();
                Task::perform(
                    async move {
                        let mut manager = youtube_manager.lock().await;
                        match manager.check_auth_status().await {
                            Ok(auth) => Message::YouTubeAuthStatus(auth),
                            Err(_) => Message::YouTubeAuthStatus(YouTubeAuth::default()),
                        }
                    },
                    |msg| msg,
                )
            }
            Message::YouTubeAuthStatus(auth) => {
                self.youtube_auth = auth;
                if self.youtube_auth.authenticated {
                    self.youtube_integration_enabled = true;
                } else {
                    self.youtube_integration_enabled = false;
                }
                self.save_config();
                Task::none()
            }
            Message::DisconnectYouTube => {
                let youtube_manager = self.youtube_manager.clone();
                Task::perform(
                    async move {
                        let mut manager = youtube_manager.lock().await;
                        match manager.disconnect().await {
                            Ok(_) => Message::YouTubeDisconnected(Ok(())),
                            Err(e) => Message::YouTubeDisconnected(Err(e.to_string())),
                        }
                    },
                    |msg| msg,
                )
            }
            Message::YouTubeDisconnected(result) => {
                match result {
                    Ok(_) => {
                        self.youtube_auth = YouTubeAuth::default();
                        self.youtube_integration_enabled = false;
                        self.save_config();
                        Task::none()
                    }
                    Err(error) => {
                        tracing::error!("Failed to disconnect from YouTube: {}", error);
                        Task::none()
                    }
                }
            }
            Message::ShowClientSecretDialog => {
                self.show_client_secret_dialog = true;
                self.client_secret_input.clear();
                Task::none()
            }
            Message::HideClientSecretDialog => {
                self.show_client_secret_dialog = false;
                self.client_secret_input.clear();
                Task::none()
            }
            Message::ClientSecretInput(input) => {
                self.client_secret_input = input;
                Task::none()
            }
            Message::SetClientSecret => {
                let client_secret = self.client_secret_input.clone();
                self.show_client_secret_dialog = false;
                self.client_secret_input.clear();
                
                Task::perform(
                    async move {
                        match crate::youtube::YouTubeManager::save_client_secret(&client_secret).await {
                            Ok(_) => Message::ClientSecretSet(Ok(())),
                            Err(e) => Message::ClientSecretSet(Err(e.to_string())),
                        }
                    },
                    |msg| msg,
                )
            }
            Message::ClientSecretSet(result) => {
                match result {
                    Ok(_) => {
                        // Client secret saved successfully, now we can authenticate
                        Task::perform(
                            async { Message::AuthenticateYouTube },
                            |msg| msg,
                        )
                    }
                    Err(error) => {
                        tracing::error!("Failed to save client secret: {}", error);
                        Task::none()
                    }
                }
            }
            Message::CreateLiveStream(game_name) => {
                let youtube_manager = self.youtube_manager.clone();
                Task::perform(
                    async move {
                        let mut manager = youtube_manager.lock().await;
                        match manager.create_live_stream(&game_name).await {
                            Ok(stream_info) => Message::LiveStreamCreated(Ok(stream_info)),
                            Err(e) => Message::LiveStreamCreated(Err(e.to_string())),
                        }
                    },
                    |msg| msg,
                )
            }
            Message::LiveStreamCreated(result) => {
                match result {
                    Ok(_stream_info) => {
                        // TODO: We could store the stream info and use it to configure OBS
                        // For now, just log success
                        tracing::info!("Live stream created successfully");
                        Task::none()
                    }
                    Err(error) => {
                        tracing::error!("Failed to create live stream: {}", error);
                        Task::none()
                    }
                }
            }
        }
    }

    pub fn subscription(&self) -> Subscription<Message> {
        // Monitor processes every 5 seconds to reduce CPU/memory usage
        time::every(std::time::Duration::from_secs(5))
            .map(|_| {
                let running_processes = ProcessMonitor::get_running_processes();
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
        self.config.youtube_auth = self.youtube_auth.clone();
        self.config.youtube_integration_enabled = self.youtube_integration_enabled;
        
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

    pub fn youtube_auth(&self) -> &YouTubeAuth {
        &self.youtube_auth
    }

    pub fn youtube_integration_enabled(&self) -> bool {
        self.youtube_integration_enabled
    }

    pub fn show_client_secret_dialog(&self) -> bool {
        self.show_client_secret_dialog
    }

    pub fn client_secret_input(&self) -> &str {
        &self.client_secret_input
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