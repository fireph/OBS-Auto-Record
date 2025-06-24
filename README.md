# 🎮 OBS Auto Record

<div align="center">

**A lightweight, intelligent game recording and streaming automation tool for OBS Studio**

[![Rust](https://img.shields.io/badge/rust-2024-orange.svg)](https://www.rust-lang.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)](https://github.com/your-repo/obs-auto-record)

*Never miss a gaming moment again! Automatically start and stop OBS recordings or streams when your favorite games launch.*

![OBS Auto Record Screenshot](https://via.placeholder.com/800x500/2d3748/ffffff?text=OBS+Auto+Record+%26+Stream+Screenshot)

</div>

---

## ✨ Features

### 🎯 **Smart Game Detection**
- **Automatic monitoring** of your selected game executables
- **Real-time status updates** with color-coded indicators
- **Custom game names** - rename games to whatever you want

### 🎬 **Dual Mode Support**
- **Recording Mode** - Auto-start/stop recording when games launch
- **Streaming Mode** - Auto-start/stop streaming when games launch
- **Per-game configuration** - Set each game to record or stream individually
- **Smart cleanup** - removes games automatically stop their assigned action

### 🔴 **Seamless Recording & Streaming**
- **Auto-start recording/streaming** when games launch
- **Auto-stop recording/streaming** when games close
- **OBS WebSocket integration** for reliable control
- **Mode switching** - easily toggle between recording and streaming per game

### 🎨 **Beautiful Interface**
- **Modern, clean UI** built with Iced framework
- **Light/Dark mode** toggle for your preference
- **Responsive design** that works on any screen size
- **Intuitive controls** - add, remove, and manage games easily
- **Visual mode indicators** - clearly see which games record vs stream

### ⚡ **Optimized Performance**
- **Ultra-lightweight** - only ~20MB RAM usage
- **Tiny binary size** - optimized for minimal footprint
- **CPU efficient** - smart polling every 5 seconds
- **No console window** in release mode

## 🚀 Quick Start

### Prerequisites

1. **OBS Studio** with WebSocket plugin enabled
   ```
   OBS Studio → Tools → WebSocket Server Settings
   ✅ Enable WebSocket server
   📝 Default: ws://localhost:4455
   ```

2. **Windows/macOS/Linux** - Cross-platform support

### Installation

#### Option 1: Download Release (Recommended)
1. Download the latest release from [Releases](https://github.com/your-repo/obs-auto-record/releases)
2. Extract and run `obs_auto_record.exe`

#### Option 2: Build from Source
```bash
# Clone the repository
git clone https://github.com/your-repo/obs-auto-record.git
cd obs-auto-record

# Build optimized release
cargo build --release

# Run the application
./target/release/obs_auto_record
```

## 📖 Usage Guide

### 🔧 Initial Setup

1. **Configure OBS WebSocket**
   - Enter your OBS WebSocket URL (default: `ws://localhost:4455`)
   - Click **Test** to verify connection
   - You should see "✅ Connected to OBS Studio [version]"

2. **Add Your Games**
   - Click **Add Game** 
   - Select your game's executable file (`.exe`)
   - Game name auto-detects but can be customized
   - Choose **Recording** or **Streaming** mode for each game
   - Repeat for all games you want to monitor

3. **Start Gaming!**
   - Launch any monitored game
   - Recording or streaming starts automatically based on game mode
   - Stop the game to end recording/streaming

### 🎮 Game Status Indicators

| Status | Indicator | Meaning |
|--------|-----------|---------|
| **● Recording** | 🔴 Red | Game is running and recording |
| **● Streaming** | 🟣 Purple | Game is running and streaming |
| **● Running** | 🟢 Green | Game is running but not recording/streaming |
| **○ Stopped** | ⚫ Gray | Game is not running |

### 🔄 Mode Management

- **Recording Mode**: Games will automatically start/stop OBS recording
- **Streaming Mode**: Games will automatically start/stop OBS streaming
- **Mode Toggle**: Click the mode button to switch between Recording/Streaming
- **Smart Restrictions**: Mode can only be changed when game is not active

### ⚙️ Advanced Features

- **Theme Toggle**: Switch between light and dark modes
- **Live Editing**: Rename games while they're running
- **Smart Cleanup**: Removing an active game automatically stops recording/streaming
- **Persistent Settings**: All configurations saved between sessions
- **Mixed Modes**: Some games can record while others stream

## 🏗️ Technical Details

### Architecture
- **Frontend**: Iced GUI framework with tiny-skia renderer
- **Backend**: Tokio async runtime
- **OBS Integration**: obws WebSocket client with full recording/streaming support
- **Process Monitoring**: sysinfo system interface
- **Configuration**: JSON-based persistent storage

### System Requirements
- **RAM**: ~20MB (extremely lightweight!)
- **CPU**: Minimal - 5-second polling interval
- **Storage**: <10MB binary size
- **Network**: Local WebSocket to OBS only

### Performance Optimizations
- **Async architecture** for non-blocking operations
- **Minimal system calls** - only process monitoring
- **Smart caching** - reduces redundant operations
- **Compiler optimizations** - size and speed focused

## 📁 Configuration

Settings are automatically saved to:

| Platform | Location |
|----------|----------|
| **Windows** | `%APPDATA%\obs_auto_record\config.json` |
| **macOS** | `~/Library/Application Support/obs_auto_record/config.json` |
| **Linux** | `~/.config/obs_auto_record/config.json` |

### Sample Configuration
```json
{
  "games": [
    {
      "path": "C:\\Games\\Rocket League\\RocketLeague.exe",
      "name": "Rocket League",
      "is_running": false,
      "is_recording": false,
      "is_streaming": false,
      "mode": "Recording"
    },
    {
      "path": "C:\\Games\\Valorant\\VALORANT.exe",
      "name": "Valorant Streams",
      "is_running": false,
      "is_recording": false,
      "is_streaming": false,
      "mode": "Streaming"
    }
  ],
  "obs_url": "ws://localhost:4455",
  "dark_mode": true
}
```

## 🔧 Troubleshooting

### Common Issues

#### 🔴 **OBS Connection Failed**
- ✅ Ensure OBS Studio is running
- ✅ Check WebSocket server is enabled in OBS
- ✅ Verify correct port (usually 4455)
- ✅ Try restarting OBS Studio

#### 🔴 **Game Not Detected**
- ✅ Ensure you selected the correct main executable
- ✅ Some games have multiple .exe files - choose the launcher
- ✅ Check game process name matches executable name
- ✅ Run as administrator if needed

#### 🔴 **Recording/Streaming Not Starting**
- ✅ Verify OBS connection is active (green status)
- ✅ Check OBS recording/streaming settings are configured
- ✅ Ensure OBS has write permissions to output folder
- ✅ For streaming: verify stream key and server settings in OBS
- ✅ Try manually starting recording/streaming in OBS first

#### 🔴 **Mode Toggle Not Working**
- ✅ Mode can only be changed when game is not running
- ✅ Stop the game first, then change mode
- ✅ Active games (recording/streaming) cannot change modes

### Debug Mode

For development or troubleshooting, run with debug logging:
```bash
# Debug build with console output
cargo run

# Release build (no console)
cargo build --release
```

## 🛠️ Development

### Building from Source

```bash
# Prerequisites
rustup install stable
rustup default stable

# Clone and build
git clone https://github.com/your-repo/obs-auto-record.git
cd obs-auto-record
cargo build --release

# For development
cargo run
```

### Project Structure
```
obs-auto-record/
├── src/
│   ├── main.rs          # Main application entry point
│   ├── app.rs           # App state and update logic
│   ├── ui.rs            # User interface components
│   ├── messages.rs      # Message types for UI events
│   ├── game.rs          # Game entry and mode definitions
│   ├── obs.rs           # OBS WebSocket integration
│   ├── config.rs        # Configuration management
│   └── process_monitor.rs # System process monitoring
├── Cargo.toml           # Dependencies and build config
└── README.md           # This file
```

### Dependencies
- **iced** - Modern GUI framework
- **obws** - OBS WebSocket client
- **sysinfo** - System information
- **tokio** - Async runtime
- **serde** - Serialization
- **rfd** - File dialogs

## 🤝 Contributing

We welcome contributions! Here's how to get started:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Development Guidelines
- Follow Rust best practices
- Add tests for new features
- Update documentation
- Ensure cross-platform compatibility

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **OBS Studio** team for the amazing streaming software
- **Iced** framework for the beautiful GUI toolkit
- **Rust community** for the excellent ecosystem
- **Contributors** who help make this project better

---

<div align="center">

**Made with ❤️ and Rust 🦀**

[⭐ Star this repo](https://github.com/your-repo/obs-auto-record) • [🐛 Report Bug](https://github.com/your-repo/obs-auto-record/issues) • [💡 Request Feature](https://github.com/your-repo/obs-auto-record/issues)

</div>