#include "ObsAutoRecord.h"
#include <QJsonDocument>
#include <QtCore/QDebug>

QT_USE_NAMESPACE

ObsAutoRecord::ObsAutoRecord(const QUrl &url, bool debug, QObject *parent) :
    QObject(parent),
    m_url(url),
    m_debug(debug)
{
    m_obsWebSocket = ObsWebSocket(QUrl(QStringLiteral("ws://localhost:4444")), true);
}

void ObsAutoRecord::pingStatus()
{
    if self.pause_end_time > 0 and self.is_paused:
        if time.time() >= self.pause_end_time:
            self.on_pause_key()
        else:
            self._on_state_change()
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
}
