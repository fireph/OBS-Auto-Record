# OBS-Auto-Record
Controls OBS to auto record specific games when they are running (works on Windows/Mac)

## How to install
1. Install [OBS Studio](https://obsproject.com/download)
2. Install [obs-websocket](https://github.com/Palakis/obs-websocket/releases) (must be version 4.3.1 or newer)
- obs-websocket seems to crash on latest MacOS :(
3. Download the latest [release](https://github.com/DungFu/OBS-Auto-Record/releases) of OBS Auto Record
4. Run OBS Auto Record and configure the settings.ini file that gets created next to executable
  - On Mac, the settings.ini file lives in `ObsAutoRecord.app/Contents/Resources`

## How to compile
### Windows
Install [pywin32](https://github.com/mhammond/pywin32)
```
pip3 install psutil pyinstaller websocket-client keyboard
set PYTHONOPTIMIZE=2
pyinstaller .\ObsAutoRecordWin.py --onefile --noconsole --icon=record_red.ico --name=ObsAutoRecord --add-data "*.ico;."
```
### Mac
```
pip3 install psutil py2app rumps websocket-client keyboard
python3 ObsAutoRecordMacSetup.py py2app
```
