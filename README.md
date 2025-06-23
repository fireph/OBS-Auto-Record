# OBS Auto Record

A lightweight Rust application that automatically starts and stops OBS recording when specific games are running. Built with Iced for a beautiful cross-platform GUI.

## Features

- **Automatic Recording**: Monitors specified game executables and automatically starts/stops OBS recording
- **Beautiful UI**: Clean, modern interface with light/dark mode support
- **Persistent Settings**: All configurations are saved between launches
- **OBS WebSocket Integration**: Connects to OBS Studio via WebSocket for reliable control
- **Editable Game Names**: Customize display names for your games
- **Real-time Status**: Shows which games are running and recording status
- **Minimal Resource Usage**: Optimized for low memory and CPU usage

## Prerequisites

1. **OBS Studio** with WebSocket plugin enabled
   - In OBS, go to Tools → WebSocket Server Settings
   - Enable WebSocket server (usually runs on `ws://localhost:4455`)
   - Note the port and password if set

2. **Rust Toolchain** (for building from source)
   - Install from [rustup.rs](https://rustup.rs/)

## Building

```bash
# Clone the repository
git clone <repository-url>
cd OBS-Auto-Record

# Build in release mode for optimal size and performance
cargo build --release

# The executable will be in target/release/obs_auto_record
```

## Usage

1. **Launch the application**
   - Run the executable or use `cargo run`

2. **Configure OBS WebSocket**
   - Enter your OBS WebSocket URL (default: `ws://localhost:4455`)
   - Click "Test" to verify connection
   - You should see "Connected to OBS Studio [version]"

3. **Add Games**
   - Click "Add Game" button
   - Select the game's executable file (.exe)
   - The game name will be automatically detected but can be edited
   - Repeat for all games you want to monitor

4. **Automatic Recording**
   - The app monitors your selected games every 2 seconds
   - When a game starts running, OBS recording automatically begins
   - When the game stops, recording automatically stops
   - Status indicators show: ⚫ Stopped, 🟢 Running, 🔴 Recording

## Configuration

Settings are automatically saved to:
- **Windows**: `%APPDATA%\obs_auto_record\config.json`
- **macOS**: `~/Library/Application Support/obs_auto_record/config.json`
- **Linux**: `~/.config/obs_auto_record/config.json`

## Optimization Features

The application is optimized for minimal resource usage:

- **Small Binary Size**: Optimized compilation flags reduce executable size
- **Low Memory Usage**: Efficient data structures and minimal allocations
- **Low CPU Usage**: Process monitoring runs only every 2 seconds
- **Async Architecture**: Non-blocking I/O for OBS communication

## Troubleshooting

### OBS Connection Issues
- Ensure OBS Studio is running
- Check that WebSocket server is enabled in OBS
- Verify the correct port (usually 4455)
- Try restarting OBS if connection fails

### Game Not Detected
- Ensure you've selected the correct .exe file
- Some games may have multiple executables - select the main one
- Check that the game process name matches the executable name

### Recording Not Starting/Stopping
- Verify OBS connection is established
- Check OBS recording settings and output path
- Ensure OBS has proper permissions to write to the output folder

## Dependencies

- **iced**: Modern GUI framework
- **obws**: OBS WebSocket client
- **sysinfo**: System and process information
- **serde**: Serialization for configuration
- **tokio**: Async runtime
- **rfd**: File picker dialogs

## Building for Distribution

For the smallest possible binary:

```bash
# Install cargo-bloat to analyze binary size
cargo install cargo-bloat

# Build with maximum optimizations
cargo build --release

# Analyze what's taking up space
cargo bloat --release --crates

# Strip debug symbols (if not already done)
strip target/release/obs_auto_record
```

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]