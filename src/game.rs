use serde::{Deserialize, Serialize};
use std::path::PathBuf;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum GameMode {
    Recording,
    Streaming,
}

impl Default for GameMode {
    fn default() -> Self {
        GameMode::Recording
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameEntry {
    pub path: PathBuf,
    pub name: String,
    pub is_running: bool,
    pub is_recording: bool,
    pub is_streaming: bool,
    pub mode: GameMode,
}

impl GameEntry {
    pub fn new(path: PathBuf) -> Self {
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
            is_streaming: false,
            mode: GameMode::Recording,
        }
    }

    pub fn is_active(&self) -> bool {
        match self.mode {
            GameMode::Recording => self.is_recording,
            GameMode::Streaming => self.is_streaming,
        }
    }

    pub fn status_text(&self) -> &'static str {
        if self.is_recording {
            "● Recording"
        } else if self.is_streaming {
            "● Streaming"
        } else if self.is_running {
            "● Running"
        } else {
            "○ Stopped"
        }
    }

    pub fn status_color(&self) -> iced::Color {
        if self.is_recording {
            iced::Color::from_rgb(0.8, 0.2, 0.2) // Red for recording
        } else if self.is_streaming {
            iced::Color::from_rgb(0.6, 0.2, 0.8) // Purple for streaming
        } else if self.is_running {
            iced::Color::from_rgb(0.2, 0.8, 0.2) // Green for running
        } else {
            iced::Color::from_rgb(0.6, 0.6, 0.6) // Gray for stopped
        }
    }
}