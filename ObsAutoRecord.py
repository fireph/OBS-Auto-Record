import configparser
import fnmatch
from ObsWebSocket import ObsWebSocket
import platform
import psutil
import threading

def get_apps_to_record():
    config = configparser.ConfigParser()
    config.read('settings.ini')
    default_apps = ''
    if platform.system() == 'Windows':
        default_apps = ', '.join(['destiny2.exe', 'HeroesOfTheStorm*.exe', 'Wow*.exe'])
    elif platform.system() == 'Darwin':
        default_apps = ', '.join(['Heroes of the Storm', 'Hearthstone'])
    if 'DEFAULT' not in config:
        config['DEFAULT'] = {'apps_to_record': default_apps}
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    if 'apps_to_record' not in config['DEFAULT']:
        config['DEFAULT']['apps_to_record'] = default_apps
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    apps = config['DEFAULT']['apps_to_record'].split(',')
    return [app.strip() for app in apps]

def get_address():
    config = configparser.ConfigParser()
    config.read('settings.ini')
    default_address = 'localhost:4444'
    if 'DEFAULT' not in config:
        config['DEFAULT'] = {'address': default_address}
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    if not ("address" in config['DEFAULT']):
        config['DEFAULT']['address'] = default_address
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
    return config['DEFAULT']['address']

class ObsAutoRecord():
    def __init__(self):
        self.apps_to_record = get_apps_to_record();
        self.interval = 15
        self.obs_web_socket = None
        self.on_state_change = None
        self.timer = None
        self.start()

    def start(self):
        self.obs_web_socket = ObsWebSocket(
            get_address(),
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
        def on_status(msg):
            if 'recording' in msg:
                is_app_open = self.is_app_open()
                if not msg['recording'] and is_app_open:
                    self.obs_web_socket.send("StartRecording")
                elif msg['recording'] and not is_app_open:
                    self.obs_web_socket.send("StopRecording")
        self.obs_web_socket.send("GetStreamingStatus", success_callback=on_status)
        self.timer = threading.Timer(self.interval, self.ping_status)
        self.timer.start()

    def is_app_open(self):
        for proc in psutil.process_iter(attrs=['name']):
            for app in self.apps_to_record:
                if fnmatch.fnmatchcase(proc.name(), app):
                    return True
        return False

    def _on_state_change(self):
        if self.on_state_change is not None:
            self.on_state_change(self.obs_web_socket.is_connected())
