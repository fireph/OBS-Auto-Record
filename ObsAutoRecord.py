import fnmatch
from ObsWebSocket import ObsWebSocket
import os
import psutil
import threading
import ObsUtils

class ObsAutoRecord():
    def __init__(self):
        self.address = ObsUtils.get_address()
        self.apps_to_record = ObsUtils.get_apps_to_record();
        self.interval = 15
        self.obs_web_socket = None
        self.on_state_change = None
        self.timer = None
        self.start()

    def start(self):
        self.obs_web_socket = ObsWebSocket(
            self.address,
            on_open=self.on_open,
            on_close=self.on_close)

    def set_on_state_change(self, on_state_change):
        self.on_state_change = on_state_change
        self._on_state_change()

    def on_open(self):
        self._on_state_change()
        self.ping_status()

    def on_close(self):
        self._on_state_change()

    def ping_status(self):
        def start_recording(msg):
            self.obs_web_socket.send("StartRecording")
        def change_folder_back(msg):
            folder = ObsUtils.get_folder()
            self.obs_web_socket.send("SetRecordingFolder", data={'rec-folder': folder})
        def on_status(msg):
            if 'recording' in msg:
                open_app = self.get_open_app()
                folder = ObsUtils.get_folder()
                if not msg['recording'] and open_app is not None:
                    if folder is not None:
                        rec_folder = os.path.join(folder, open_app).replace('\\', '/')
                        if "localhost" in self.address or "127.0.0.1" in self.address:
                            ObsUtils.assure_path_exists(rec_folder)
                        self.obs_web_socket.send(
                            "SetRecordingFolder",
                            data={'rec-folder': rec_folder},
                            success_callback=start_recording,
                            error_callback=start_recording)
                    else:
                        self.obs_web_socket.send("StartRecording")
                elif msg['recording'] and open_app is None:
                    self.obs_web_socket.send(
                        "StopRecording",
                        success_callback=change_folder_back if folder is not None else None)
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
            self.on_state_change(self.obs_web_socket.is_connected())
