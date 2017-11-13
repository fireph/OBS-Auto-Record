import configparser
import fnmatch
import json
import platform
import psutil
import threading
import time
import uuid
import websocket

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
        self.ws = None
        self.wst = None
        self.timer = None
        self.interval = 15
        self.should_restart = True
        self.state = False
        self.on_state_change = None
        self.apps_to_record = get_apps_to_record();
        self.start()

    def set_on_state_change(self, on_state_change):
        self.on_state_change = on_state_change
        self.on_state_change(self.state)

    def start(self):
        self.should_restart = True
        self.ws = websocket.WebSocketApp("ws://" + get_address(),
            on_message = self.on_message,
            on_error = self.on_error,
            on_close = self.on_close)
        self.ws.on_open = self.on_open
        self.wst = threading.Thread(target=self.ws.run_forever)
        self.wst.daemon = True
        self.wst.start()

    def on_message(self, ws, message):
        json_msg = json.loads(message)
        if 'recording' in json_msg:
            is_app_open = self.is_app_open()
            if not json_msg['recording'] and is_app_open:
                self.send_message("StartRecording")
            elif json_msg['recording'] and not is_app_open:
                self.send_message("StopRecording")

    def on_error(self, ws, error):
        print(error)

    def on_close(self, ws):
        self.set_state(False)
        if self.timer is not None:
            self.timer.cancel()
        if self.should_restart:
            time.sleep(self.interval)
            self.start()

    def on_open(self, ws):
        self.set_state(True)
        self.ping_status()

    def ping_status(self):
        self.send_message("GetStreamingStatus")
        self.timer = threading.Timer(self.interval, self.ping_status)
        self.timer.start()

    def is_app_open(self):
        for proc in psutil.process_iter(attrs=['name']):
            for app in self.apps_to_record:
                if fnmatch.fnmatchcase(proc.name(), app):
                    return True
        return False

    def send_message(self, requestType):
        if self.ws is not None:
            self.ws.send(json.dumps({'request-type': requestType, 'message-id': str(uuid.uuid4())}))

    def set_state(self, state):
        self.state = state
        if self.on_state_change is not None:
            self.on_state_change(self.state)

    def close(self):
        self.should_restart = False
        if self.timer is not None:
            self.timer.cancel()
