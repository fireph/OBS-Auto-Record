# OBS-Auto-Record
Controls OBS to auto record specific games when they are running (works on Windows/Mac)

## How to install
1. Install [OBS Studio](https://obsproject.com/download)
2. Install [obs-websocket](https://github.com/Palakis/obs-websocket/releases)
  - Until obs-websocket officially works on Mac, use [this](https://obs-websocket-osx-builds.s3-eu-central-1.amazonaws.com/obs-websocket-latest-master.pkg) build.
3. Download the latest [release](https://github.com/DungFu/OBS-Auto-Record/releases) of OBS Auto Record
4. Run OBS Auto Record and configure the settings.ini file that gets created next to executable
  - On Mac, the settings.ini file lives in `ObsAutoRecord.app/Contents/Resources`

## How to compile
### Windows
Install [pywin32](https://github.com/mhammond/pywin32)
```
pip3 install psutil pyinstaller websocket-client
pyinstaller .\ObsAutoRecordWin.py --onefile --noconsole --icon=record_red.ico --name=ObsAutoRecord
```
and put the ico files in the same folder as the executable.
### Mac
```
pip3 install psutil py2app rumps websocket-client
python ObsAutoRecordMacSetup.py py2app
```
