use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use anyhow::Result;
use crate::game::GameEntry;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    pub games: Vec<GameEntry>,
    pub obs_url: String,
    pub dark_mode: bool,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            games: Vec::new(),
            obs_url: "ws://localhost:4455".to_string(),
            dark_mode: false,
        }
    }
}

impl Config {
    pub fn load() -> Result<Self> {
        let config_path = Self::config_path()?;
        
        if config_path.exists() {
            let content = std::fs::read_to_string(&config_path)?;
            let config: Config = serde_json::from_str(&content)?;
            Ok(config)
        } else {
            Ok(Self::default())
        }
    }

    pub fn save(&self) -> Result<()> {
        let config_path = Self::config_path()?;
        
        // Create config directory if it doesn't exist
        if let Some(parent) = config_path.parent() {
            std::fs::create_dir_all(parent)?;
        }
        
        let content = serde_json::to_string_pretty(self)?;
        std::fs::write(&config_path, content)?;
        
        Ok(())
    }

    fn config_path() -> Result<PathBuf> {
        let config_dir = dirs::config_dir()
            .ok_or_else(|| anyhow::anyhow!("Could not find config directory"))?;
        
        Ok(config_dir.join("obs_auto_record").join("config.json"))
    }
}