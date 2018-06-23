# OBS-Auto-Record
Controls OBS to auto record specific games when they are running (works on Windows/Mac)

## How to install
1. Install [OBS Studio](https://obsproject.com/download)
2. Install [obs-websocket](https://github.com/Palakis/obs-websocket/releases) (must be version 4.3.1 or newer)
- You'll probably have to reinstall OBS studio again after installing obs-socket (trust me, it works)
3. Download the latest [release](https://github.com/DungFu/OBS-Auto-Record/releases) of OBS Auto Record
4. Run OBS Auto Record!

## How to compile
### Windows
Install [Microsoft Visual Studio Code](https://code.visualstudio.com/download)
Install [Qt Creator](https://www.qt.io/download) (with corresponding msvsc version in newest Qt version)
Press the run button!

And if you want to move dependencies:
```
windeployqt obsautorecord.exe
```
You can also use UPX to compress the DLL files is desired.
### Mac
Install [Qt Creator](https://www.qt.io/download) (with MacOS support in newest Qt version)
```
qmake
make
open -a obsautorecord.app
```
