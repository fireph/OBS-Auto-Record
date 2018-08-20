# OBS-Auto-Record

| OSX | Windows | Latest  | Downloads |
|-----|---------|---------|-----------|
|[![Travis Build Status](https://travis-ci.org/DungFu/OBS-Auto-Record.svg?branch=master)](https://travis-ci.org/DungFu/OBS-Auto-Record)|[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/xxe0gbovybndeeyv/branch/master?svg=true)](https://ci.appveyor.com/project/DungFu/obs-auto-record)|[![Latest GitHub Release](https://img.shields.io/github/release/DungFu/OBS-Auto-Record.svg)](https://github.com/DungFu/OBS-Auto-Record/releases/latest)|[![Total Downloads](https://img.shields.io/github/downloads/DungFu/OBS-Auto-Record/total.svg)](https://github.com/DungFu/OBS-Auto-Record/releases/latest)|

Controls OBS to auto record specific games when they are running (works on Windows/Mac)

![OBS Auto Record Screenshot](https://i.imgur.com/Ist6StX.png)

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
```
qpm install
```
Press the run button!

And if you want to move dependencies:
```
windeployqt obsautorecord.exe
```
You can also use UPX to compress the DLL files is desired.
### Mac
Install [Qt Creator](https://www.qt.io/download) (with MacOS support in newest Qt version)

```
qpm install
qmake -config release
make
macdeployqt "release/OBS Auto Record.app" -dmg
```
