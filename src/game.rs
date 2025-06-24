use serde::{Deserialize, Serialize};
use std::path::PathBuf;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameEntry {
    pub path: PathBuf,
    pub name: String,
    pub is_running: bool,
    pub is_recording: bool,
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
        }
    }
}