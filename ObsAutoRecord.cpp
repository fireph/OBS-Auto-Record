#include "ObsAutoRecord.h"
#include <QJsonDocument>
#include <QtCore/QDebug>
#include <QTimer>

QT_USE_NAMESPACE

ObsAutoRecord::ObsAutoRecord(const QUrl &url, bool debug, QObject *parent) :
    QObject(parent),
    m_obsWebSocket(url, debug, this),
    m_url(url),
    m_debug(debug)
{
    QTimer *timer = new QTimer;
    timer->setInterval(5000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(pingStatus()));
}

void ObsAutoRecord::pingStatus()
{
    if (m_obsWebSocket.isConnected()) {
        m_msgid++;
        QObject::connect(&m_obsWebSocket, SIGNAL(onResponse(QJsonObject)),
                         this, SLOT(onStatus(QJsonObject)));
        m_obsWebSocket.sendRequest("GetStreamingStatus", m_msgid);
    }
}

void ObsAutoRecord::startRecording()
{
    m_obsWebSocket.sendRequest("StartRecording");
}
        
void ObsAutoRecord::setFilenameFormatting(QString appName)
{
    m_obsWebSocket.sendRequest(
        "SetFilenameFormatting",
        data={'filename-formatting': app_name + ' - ' + self.default_filename_formatting},
        success_callback=lambda msg: start_recording(),
        error_callback=lambda msg: start_recording())
}

void ObsAutoRecord::changeFolderBack()
{
    std::string folder = "Test folder";
    QJsonObject object
    {
        {"rec-folder", folder}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetRecordingFolder", m_msgid, object);
    QJsonObject object2
    {
        {"filename-formatting", defaultFilenameFormatting}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetFilenameFormatting", object2);
}

void ObsAutoRecord::onStatus(QJsonObject msg)
{
    if (msg.contains("recording")) {
        bool recording = msg.value("recording").toBool();
        open_app = ""
        std::string folder = "Test folder"
        if (!recording && open_app is not None) {
            rec_folder = os.path.join(folder, open_app).replace('\\', '/');
            QJsonObject object
            {
                {"rec-folder", rec_folder}
            };
            m_obsWebSocket.sendRequest("SetRecordingFolder", object);
            setFilenameFormatting(open_app);
        } else if (recording && open_app is None) {
            m_obsWebSocket.send("StopRecording");
            changeFolderBack();
        }
    }
}
