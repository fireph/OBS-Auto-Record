import fnmatch
import os
import threading

import keyboard
import psutil

import ObsUtils
from ObsAutoRecordState import ObsAutoRecordState
from ObsWebSocket import ObsWebSocket

class ObsAutoRecord():
    def __init__(self):
        self.address = ObsUtils.get_address()
        self.apps_to_record = ObsUtils.get_apps_to_record();
        self.default_filename_formatting = '%CCYY-%MM-%DD %hh-%mm-%ss'
        self.interval = 15
        self.obs_web_socket = None
        self.on_state_change = None
        self.timer = None
        self.is_paused = False
        keyboard.add_hotkey('ctrl+alt+end', self.on_pause_key)
        self.start()

    def start(self):
        self.obs_web_socket = ObsWebSocket(
            self.address,
            on_open=self.on_open,
            on_close=self.on_close)

    def on_pause_key(self):
        self.is_paused = not self.is_paused
        self._on_state_change()
        if self.timer is not None:
            self.ping_status()

    def set_on_state_change(self, on_state_change):
        self.on_state_change = on_state_change
        self._on_state_change()

    def on_open(self):
        self._on_state_change()
        self.ping_status()

    def on_close(self):
        self._on_state_change()

    def ping_status(self):
        def start_recording():
            self.obs_web_socket.send("StartRecording")
        def set_filename_formatting(app_name):
            self.obs_web_socket.send(
                "SetFilenameFormatting",
                data={'filename-formatting': app_name + ' - ' + self.default_filename_formatting},
                success_callback=lambda msg: start_recording(),
                error_callback=lambda msg: start_recording())
        def change_folder_back():
            folder = ObsUtils.get_folder()
            if folder is not None:
                self.obs_web_socket.send("SetRecordingFolder", data={'rec-folder': folder})
            self.obs_web_socket.send("SetFilenameFormatting", data={'filename-formatting': self.default_filename_formatting})
        def on_status(msg):
            if 'recording' in msg:
                open_app = self.get_open_app()
                folder = ObsUtils.get_folder()
                if not msg['recording'] and open_app is not None and not self.is_paused:
                    if folder is not None:
                        rec_folder = os.path.join(folder, open_app).replace('\\', '/')
                        self.obs_web_socket.send(
                            "SetRecordingFolder",
                            data={'rec-folder': rec_folder},
                            success_callback=lambda msg: set_filename_formatting(open_app),
                            error_callback=lambda msg: set_filename_formatting(open_app))
                    else:
                        set_filename_formatting(open_app)
                elif msg['recording'] and (open_app is None or self.is_paused):
                    self.obs_web_socket.send(
                        "StopRecording",
                        success_callback=lambda msg: change_folder_back())
        self.obs_web_socket.send("GetStreamingStatus", success_callback=on_status)
        self.timer = threading.Timer(self.interval, self.ping_status)
        self.timer.start()

    def get_open_app(self):
        for proc in psutil.process_iter(attrs=['name']):
            for app in self.apps_to_record:
                if fnmatch.fnmatchcase(proc.name(), app):
                    return ObsUtils.get_app_name_from_process(proc)
        return None

    def _on_state_change(self):
        if self.on_state_change is not None:
            self.on_state_change(self._get_state())

    def _get_state(self):
        if self.is_paused:
            return ObsAutoRecordState.PAUSED
        elif self.obs_web_socket.is_connected():
            return ObsAutoRecordState.CONNECTED
        else:
            return ObsAutoRecordState.DISCONNECTED
