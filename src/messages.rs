use std::path::PathBuf;
use std::collections::HashMap;
use anyhow::Result;

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