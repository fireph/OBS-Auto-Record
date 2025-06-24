use std::path::PathBuf;
use std::collections::HashMap;
use anyhow::Result;
use crate::youtube::{YouTubeAuth, LiveStreamInfo};

#[derive(Debug, Clone)]
pub enum Message {
    AddGame,
    GameSelected(PathBuf),
    RemoveGame(usize),
    EditGameName(usize, String),
    ToggleGameMode(usize),
    ToggleDarkMode,
    UpdateObsUrl(String),
    TestObsConnection,
    ObsConnectionResult(Result<String, String>),
    ObsConnectionUpdated(Result<(), String>),
    ProcessUpdate(HashMap<String, bool>),
    StartedRecording(String),
    StoppedRecording(String),
    StartedStreaming(String),
    StoppedStreaming(String),
    Error(String),
    
    // YouTube integration messages
    AuthenticateYouTube,
    YouTubeAuthResult(Result<YouTubeAuth, String>),
    CheckYouTubeAuth,
    YouTubeAuthStatus(YouTubeAuth),
    DisconnectYouTube,
    YouTubeDisconnected(Result<(), String>),
    SetClientSecret,
    ClientSecretSet(Result<(), String>),
    CreateLiveStream(String), // game name
    LiveStreamCreated(Result<LiveStreamInfo, String>),
    ShowClientSecretDialog,
    HideClientSecretDialog,
    ClientSecretInput(String),
}