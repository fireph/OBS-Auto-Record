use obws::Client as ObsClient;
use anyhow::{Result, anyhow};
use std::time::Duration;

pub struct ObsManager {
    client: Option<ObsClient>,
}

impl std::fmt::Debug for ObsManager {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ObsManager")
            .field("client", &self.client.is_some())
            .finish()
    }
}

impl ObsManager {
    pub fn new() -> Self {
        Self { client: None }
    }

    pub async fn connect(&mut self, url: &str) -> Result<()> {
        // Parse host and port from URL
        let (host, port) = if url.starts_with("ws://") {
            let url_part = &url[5..]; // Remove "ws://"
            if let Some(colon_pos) = url_part.find(':') {
                let host = &url_part[..colon_pos];
                let port_str = &url_part[colon_pos + 1..];
                let port = port_str.parse::<u16>().unwrap_or(4455);
                (host.to_string(), port)
            } else {
                (url_part.to_string(), 4455u16)
            }
        } else if url.contains(':') {
            let parts: Vec<&str> = url.split(':').collect();
            let host = parts[0].to_string();
            let port = parts.get(1).and_then(|p| p.parse().ok()).unwrap_or(4455u16);
            (host, port)
        } else {
            (url.to_string(), 4455u16)
        };

        // Create client
        let client = ObsClient::connect(host, port, None::<String>).await
            .map_err(|e| anyhow!("Failed to connect to OBS: {}", e))?;

        self.client = Some(client);
        Ok(())
    }

    pub async fn start_recording(&mut self) -> Result<()> {
        if let Some(client) = &self.client {
            // Check if already recording
            let recording_status = client.recording().status().await?;
            if recording_status.active {
                return Ok(()); // Already recording
            }

            client.recording().start().await
                .map_err(|e| anyhow!("Failed to start recording: {}", e))?;
            
            Ok(())
        } else {
            Err(anyhow!("Not connected to OBS"))
        }
    }

    pub async fn stop_recording(&mut self) -> Result<()> {
        if let Some(client) = &self.client {
            // Check if actually recording
            let recording_status = client.recording().status().await?;
            if !recording_status.active {
                return Ok(()); // Not recording
            }

            client.recording().stop().await
                .map_err(|e| anyhow!("Failed to stop recording: {}", e))?;
            
            Ok(())
        } else {
            Err(anyhow!("Not connected to OBS"))
        }
    }

    pub async fn start_streaming(&mut self) -> Result<()> {
        if let Some(client) = &self.client {
            // Check if already streaming
            let streaming_status = client.streaming().status().await?;
            if streaming_status.active {
                return Ok(()); // Already streaming
            }

            client.streaming().start().await
                .map_err(|e| anyhow!("Failed to start streaming: {}", e))?;
            
            Ok(())
        } else {
            Err(anyhow!("Not connected to OBS"))
        }
    }

    pub async fn stop_streaming(&mut self) -> Result<()> {
        if let Some(client) = &self.client {
            // Check if actually streaming
            let streaming_status = client.streaming().status().await?;
            if !streaming_status.active {
                return Ok(()); // Not streaming
            }

            client.streaming().stop().await
                .map_err(|e| anyhow!("Failed to stop streaming: {}", e))?;
            
            Ok(())
        } else {
            Err(anyhow!("Not connected to OBS"))
        }
    }
}

pub async fn test_connection(url: String) -> Result<String, String> {
    // Parse host and port from URL
    let (host, port) = if url.starts_with("ws://") {
        let url_part = &url[5..]; // Remove "ws://"
        if let Some(colon_pos) = url_part.find(':') {
            let host = &url_part[..colon_pos];
            let port_str = &url_part[colon_pos + 1..];
            let port = port_str.parse::<u16>().unwrap_or(4455);
            (host.to_string(), port)
        } else {
            (url_part.to_string(), 4455u16)
        }
    } else if url.contains(':') {
        let parts: Vec<&str> = url.split(':').collect();
        let host = parts[0].to_string();
        let port = parts.get(1).and_then(|p| p.parse().ok()).unwrap_or(4455u16);
        (host, port)
    } else {
        (url.to_string(), 4455u16)
    };

    // Try to connect with a timeout
    let connect_future = ObsClient::connect(host, port, None::<String>);
    let timeout_duration = Duration::from_secs(5);

    match tokio::time::timeout(timeout_duration, connect_future).await {
        Ok(Ok(client)) => {
            // Try to get version info to verify connection
            match client.general().version().await {
                Ok(version) => {
                    Ok(format!("Connected to OBS Studio {}", version.obs_version))
                }
                Err(e) => {
                    Err(format!("Connected but failed to get version: {}", e))
                }
            }
        }
        Ok(Err(e)) => {
            Err(format!("Connection failed: {}", e))
        }
        Err(_) => {
            Err("Connection timed out".to_string())
        }
    }
}