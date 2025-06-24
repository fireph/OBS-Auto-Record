use anyhow::{Result, anyhow};
use google_youtube3 as youtube3;
use youtube3::{YouTube, hyper_rustls, hyper_util, yup_oauth2};
use youtube3::hyper::body::Bytes;
use youtube3::hyper::Error;
use http_body_util::combinators::BoxBody;
use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use tokio::fs;
use chrono::Utc;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct YouTubeAuth {
    pub authenticated: bool,
    pub channel_name: Option<String>,
    pub channel_id: Option<String>,
}

impl Default for YouTubeAuth {
    fn default() -> Self {
        Self {
            authenticated: false,
            channel_name: None,
            channel_id: None,
        }
    }
}

#[derive(Debug, Clone)]
pub struct LiveStreamInfo {
    pub stream_key: String,
    pub stream_url: String,
    pub broadcast_id: String,
    pub stream_id: String,
}

pub struct YouTubeManager {
    hub: Option<YouTube<hyper_rustls::HttpsConnector<hyper_util::client::legacy::connect::HttpConnector>>>,
}

impl std::fmt::Debug for YouTubeManager {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("YouTubeManager")
            .field("hub", &self.hub.is_some())
            .finish()
    }
}

impl YouTubeManager {
    pub fn new() -> Self {
        Self { hub: None }
    }

    /// Get the path where we store the client secret
    fn client_secret_path() -> Result<PathBuf> {
        let config_dir = dirs::config_dir()
            .ok_or_else(|| anyhow!("Could not find config directory"))?;
        Ok(config_dir.join("obs_auto_record").join("client_secret.json"))
    }

    /// Get the path where we store OAuth tokens
    fn token_storage_path() -> Result<PathBuf> {
        let config_dir = dirs::config_dir()
            .ok_or_else(|| anyhow!("Could not find config directory"))?;
        Ok(config_dir.join("obs_auto_record").join("youtube_tokens.json"))
    }

    /// Check if client secret exists
    pub async fn has_client_secret() -> bool {
        if let Ok(path) = Self::client_secret_path() {
            path.exists()
        } else {
            false
        }
    }

    /// Save the client secret to secure location
    pub async fn save_client_secret(client_secret_json: &str) -> Result<()> {
        let path = Self::client_secret_path()?;
        
        // Create directory if it doesn't exist
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).await?;
        }
        
        fs::write(&path, client_secret_json).await?;
        
        // Set secure permissions (Unix only)
        #[cfg(unix)]
        {
            use std::os::unix::fs::PermissionsExt;
            let mut perms = fs::metadata(&path).await?.permissions();
            perms.set_mode(0o600); // Read/write for owner only
            fs::set_permissions(&path, perms).await?;
        }
        
        Ok(())
    }

    /// Create HTTP client with the exact type YouTube expects
    fn create_http_client() -> hyper_util::client::legacy::Client<hyper_rustls::HttpsConnector<hyper_util::client::legacy::connect::HttpConnector>, BoxBody<Bytes, Error>> {
        hyper_util::client::legacy::Client::builder(
            hyper_util::rt::TokioExecutor::new()
        )
        .build(
            hyper_rustls::HttpsConnectorBuilder::new()
                .with_native_roots()
                .unwrap()
                .https_or_http()
                .enable_http1()
                .build()
        )
    }

    /// Authenticate with YouTube using the exact pattern from the example
    pub async fn authenticate(&mut self) -> Result<YouTubeAuth> {
        let client_secret_path = Self::client_secret_path()?;
        if !client_secret_path.exists() {
            return Err(anyhow!("Client secret not found. Please set up your Google Cloud Console credentials first."));
        }

        // Read the client secret
        let secret = yup_oauth2::read_application_secret(&client_secret_path).await
            .map_err(|e| anyhow!("Failed to read client secret: {}", e))?;

        let token_storage_path = Self::token_storage_path()?;
        
        // Create directory for token storage
        if let Some(parent) = token_storage_path.parent() {
            fs::create_dir_all(parent).await?;
        }

        // Create the authenticator exactly like the example
        let auth = yup_oauth2::InstalledFlowAuthenticator::builder(
            secret,
            yup_oauth2::InstalledFlowReturnMethod::HTTPRedirect,
        )
        .persist_tokens_to_disk(&token_storage_path)
        .build()
        .await
        .map_err(|e| anyhow!("Failed to create authenticator: {}", e))?;

        // Create the HTTP client using the exact pattern from the example
        let client = Self::create_http_client();
        
        // Create the YouTube hub
        let hub = YouTube::new(client, auth);

        // Test the authentication by getting channel info
        let channels_result = hub
            .channels()
            .list(&vec!["snippet".to_string()])
            .mine(true)
            .doit()
            .await;

        match channels_result {
            Ok((_, channel_list)) => {
                let channel_info = if let Some(items) = channel_list.items {
                    if let Some(channel) = items.first() {
                        let channel_name = channel.snippet.as_ref()
                            .and_then(|s| s.title.as_ref())
                            .map(|t| t.clone());
                        let channel_id = channel.id.clone();
                        
                        (channel_name, channel_id)
                    } else {
                        (None, None)
                    }
                } else {
                    (None, None)
                };

                self.hub = Some(hub);

                Ok(YouTubeAuth {
                    authenticated: true,
                    channel_name: channel_info.0,
                    channel_id: channel_info.1,
                })
            }
            Err(e) => {
                Err(anyhow!("Authentication failed: {}", e))
            }
        }
    }

    /// Check if we're currently authenticated
    pub async fn check_auth_status(&mut self) -> Result<YouTubeAuth> {
        // First, check if we have a hub and it works
        if let Some(hub) = &self.hub {
            match hub
                .channels()
                .list(&vec!["snippet".to_string()])
                .mine(true)
                .doit()
                .await
            {
                Ok((_, channel_list)) => {
                    let channel_info = if let Some(items) = channel_list.items {
                        if let Some(channel) = items.first() {
                            let channel_name = channel.snippet.as_ref()
                                .and_then(|s| s.title.as_ref())
                                .map(|t| t.clone());
                            let channel_id = channel.id.clone();
                            
                            (channel_name, channel_id)
                        } else {
                            (None, None)
                        }
                    } else {
                        (None, None)
                    };

                    return Ok(YouTubeAuth {
                        authenticated: true,
                        channel_name: channel_info.0,
                        channel_id: channel_info.1,
                    });
                }
                Err(_) => {
                    // Current hub is not working, clear it
                    self.hub = None;
                }
            }
        }

        // If we don't have a hub or it's not working, try to load from stored tokens
        let client_secret_path = Self::client_secret_path()?;
        if !client_secret_path.exists() {
            return Ok(YouTubeAuth::default());
        }

        let token_storage_path = Self::token_storage_path()?;
        if !token_storage_path.exists() {
            return Ok(YouTubeAuth::default());
        }

        // Try to create a new hub with stored credentials
        match yup_oauth2::read_application_secret(&client_secret_path).await {
            Ok(secret) => {
                match yup_oauth2::InstalledFlowAuthenticator::builder(
                    secret,
                    yup_oauth2::InstalledFlowReturnMethod::HTTPRedirect,
                )
                .persist_tokens_to_disk(&token_storage_path)
                .build()
                .await {
                    Ok(auth) => {
                        let client = Self::create_http_client();
                        let hub = YouTube::new(client, auth);

                        // Test if this hub actually works
                        match hub
                            .channels()
                            .list(&vec!["snippet".to_string()])
                            .mine(true)
                            .doit()
                            .await
                        {
                            Ok((_, channel_list)) => {
                                let channel_info = if let Some(items) = channel_list.items {
                                    if let Some(channel) = items.first() {
                                        let channel_name = channel.snippet.as_ref()
                                            .and_then(|s| s.title.as_ref())
                                            .map(|t| t.clone());
                                        let channel_id = channel.id.clone();
                                        
                                        (channel_name, channel_id)
                                    } else {
                                        (None, None)
                                    }
                                } else {
                                    (None, None)
                                };

                                // Only store the hub if authentication actually works
                                self.hub = Some(hub);

                                Ok(YouTubeAuth {
                                    authenticated: true,
                                    channel_name: channel_info.0,
                                    channel_id: channel_info.1,
                                })
                            }
                            Err(_) => {
                                // Authentication failed, don't store the hub
                                Ok(YouTubeAuth::default())
                            }
                        }
                    }
                    Err(_) => Ok(YouTubeAuth::default()),
                }
            }
            Err(_) => Ok(YouTubeAuth::default()),
        }
    }

    /// Create a new live stream for a game
    pub async fn create_live_stream(&mut self, game_name: &str) -> Result<LiveStreamInfo> {
        let hub = self.hub.as_ref()
            .ok_or_else(|| anyhow!("Not authenticated with YouTube"))?;

        // Step 1: Create a live stream
        let stream_title = format!("{} - Live Stream", game_name);
        
        let live_stream = youtube3::api::LiveStream {
            snippet: Some(youtube3::api::LiveStreamSnippet {
                title: Some(stream_title.clone()),
                description: Some(format!("Live streaming {} via OBS Auto Record", game_name)),
                ..Default::default()
            }),
            cdn: Some(youtube3::api::CdnSettings {
                format: Some("1080p".to_string()),
                ingestion_type: Some("rtmp".to_string()),
                ..Default::default()
            }),
            ..Default::default()
        };

        let (_, created_stream) = hub
            .live_streams()
            .insert(live_stream)
            .add_part("snippet")
            .add_part("cdn")
            .doit()
            .await
            .map_err(|e| anyhow!("Failed to create live stream: {}", e))?;

        // Step 2: Create a live broadcast
        let broadcast_title = format!("{} - Live", game_name);
        
        let live_broadcast = youtube3::api::LiveBroadcast {
            snippet: Some(youtube3::api::LiveBroadcastSnippet {
                title: Some(broadcast_title),
                description: Some(format!("Live streaming {} via OBS Auto Record", game_name)),
                scheduled_start_time: Some(Utc::now()),
                ..Default::default()
            }),
            status: Some(youtube3::api::LiveBroadcastStatus {
                privacy_status: Some("unlisted".to_string()),
                ..Default::default()
            }),
            ..Default::default()
        };

        let (_, created_broadcast) = hub
            .live_broadcasts()
            .insert(live_broadcast)
            .add_part("snippet")
            .add_part("status")
            .doit()
            .await
            .map_err(|e| anyhow!("Failed to create live broadcast: {}", e))?;

        // Step 3: Bind the stream to the broadcast
        let broadcast_id = created_broadcast.id
            .ok_or_else(|| anyhow!("No broadcast ID returned"))?;
        let stream_id = created_stream.id
            .ok_or_else(|| anyhow!("No stream ID returned"))?;

        hub
            .live_broadcasts()
            .bind(&broadcast_id, &vec!["id".to_string()])
            .stream_id(&stream_id)
            .doit()
            .await
            .map_err(|e| anyhow!("Failed to bind stream to broadcast: {}", e))?;

        // Extract stream key and URL
        let stream_key = created_stream.cdn.as_ref()
            .and_then(|cdn| cdn.ingestion_info.as_ref())
            .and_then(|info| info.stream_name.as_ref())
            .ok_or_else(|| anyhow!("No stream key returned"))?
            .clone();

        let stream_url = created_stream.cdn.as_ref()
            .and_then(|cdn| cdn.ingestion_info.as_ref())
            .and_then(|info| info.ingestion_address.as_ref())
            .cloned()
            .unwrap_or_else(|| "rtmp://a.rtmp.youtube.com/live2".to_string());

        Ok(LiveStreamInfo {
            stream_key,
            stream_url,
            broadcast_id,
            stream_id,
        })
    }

    /// Disconnect/logout from YouTube
    pub async fn disconnect(&mut self) -> Result<()> {
        self.hub = None;

        // Remove stored tokens file
        let token_storage_path = Self::token_storage_path()?;
        if token_storage_path.exists() {
            fs::remove_file(&token_storage_path).await
                .map_err(|e| anyhow!("Failed to remove stored tokens file: {}", e))?;
        }
        // If the file doesn't exist, that's fine - we're already "disconnected"

        Ok(())
    }
}

/// Helper function to get client secret setup instructions
pub fn get_client_secret_setup_instructions() -> String {
    r#"To use YouTube integration, you need to set up Google Cloud Console credentials:

🔧 SETUP STEPS:

1. Go to Google Cloud Console (https://console.cloud.google.com/)
2. Create a new project or select: obs-auto-record-463901
3. Enable the YouTube Data API v3:
   • Go to APIs & Services > Library
   • Search for "YouTube Data API v3"
   • Click "Enable"

4. Create OAuth 2.0 credentials:
   • Go to APIs & Services > Credentials
   • Click "Create Credentials" > "OAuth client ID"
   • Choose "Desktop Application"
   • Name it "OBS Auto Record"
   • Download the JSON file

5. Copy the entire JSON content from the downloaded file and paste it below.

🔒 SECURITY: Your credentials are stored securely on your computer and never shared.

📋 The JSON should look like:
{"installed":{"client_id":"...","client_secret":"...","auth_uri":"..."}}"#.to_string()
}