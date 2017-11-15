import json
import threading
import time
import uuid
import websocket

class ObsWebSocket():
    def __init__(self, address, on_open=None, on_close=None):
        self.address = address
        self.connected = False
        self.on_open = on_open
        self.on_close = on_close
        self.ws = None
        self.wst = None
        self.callbacks_by_id = {}
        self.callbacks_by_type = {}
        self.start()

    def start(self):
        self.ws = websocket.WebSocketApp("ws://" + self.address,
            on_message = self._on_message,
            on_error = self._on_error,
            on_close = self._on_close)
        self.ws.on_open = self._on_open
        self.wst = threading.Thread(target=self.ws.run_forever)
        self.wst.daemon = True
        self.wst.start()

    def is_connected(self):
        return self.ws.sock is not None and self.ws.sock.connected

    def on(self, update_type, callback):
        if update_type not in self.callbacks_by_type:
            self.callbacks_by_type[update_type] = []
        self.callbacks_by_type[update_type].append(callback)

    def send(self, request_type, data={}, success_callback=None, error_callback=None):
        if self.is_connected():
            msg_id = str(uuid.uuid4())
            data_obj = {'request-type': request_type, 'message-id': msg_id}
            data_obj.update(data)
            if success_callback is not None or error_callback is not None:
                self.callbacks_by_id[msg_id] = {
                    'success_callback': success_callback,
                    'error_callback': error_callback
                }
            self.ws.send(json.dumps(data_obj))

    def _on_message(self, ws, message):
        msg_obj = json.loads(message)
        if 'status' in msg_obj and msg_obj['status'] == 'error':
            if 'message-id' in msg_obj:
                msg_id = msg_obj['message-id']
                if msg_id in self.callbacks_by_id:
                    if self.callbacks_by_id[msg_id]['error_callback'] is not None:
                        self.callbacks_by_id[msg_id]['error_callback'](message)
                    del self.callbacks_by_id[msg_id]
        else:
            if 'message-id' in msg_obj:
                msg_id = msg_obj['message-id']
                if msg_id in self.callbacks_by_id:
                    if self.callbacks_by_id[msg_id]['success_callback'] is not None:
                        self.callbacks_by_id[msg_id]['success_callback'](msg_obj)
                    del self.callbacks_by_id[msg_id]
            if 'update-type' in msg_obj:
                update_type = msg_obj['update-type']
                if update_type in self.callbacks_by_type:
                    for type_callback in self.callbacks_by_type[update_type]:
                        type_callback(msg_obj)

    def _on_error(self, ws, error):
        print(error)

    def _on_open(self, ws):
        if self.on_open is not None:
            self.on_open()

    def _on_close(self, ws):
        if self.on_close is not None:
            self.on_close()
        time.sleep(5)
        self.start()
