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
    QObject::connect(&m_obsWebSocket, SIGNAL(onResponse(QJsonObject)),
                     this, SLOT(onStatus(QJsonObject)));
    QTimer *timer = new QTimer;
    timer->setInterval(5000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(pingStatus()));
}

void ObsAutoRecord::pingStatus()
{
    if (m_obsWebSocket.isConnected()) {
        m_msgid++;
        m_obsWebSocket.sendRequest("GetStreamingStatus", m_msgid);
    }
}

void ObsAutoRecord::startRecording()
{
    m_msgid++;
    m_obsWebSocket.sendRequest("StartRecording", m_msgid);
}
        
void ObsAutoRecord::setFilenameFormatting(QString appName)
{
    QString filenameFormatting =
        QString::fromStdString(
            appName.toStdString() + " - " + defaultFilenameFormatting);
    QJsonObject object
    {
        {"filename-formatting", filenameFormatting}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetFilenameFormatting", m_msgid, object);
}

void ObsAutoRecord::changeFolderBack()
{
    std::string folder = "Test folder";
    QJsonObject object
    {
        {"rec-folder", QString::fromStdString(folder)}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetRecordingFolder", m_msgid, object);
    QJsonObject object2
    {
        {"filename-formatting", QString::fromStdString(defaultFilenameFormatting)}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetFilenameFormatting", m_msgid, object2);
}

void ObsAutoRecord::onStatus(QJsonObject msg)
{
    if (msg.contains("recording")) {
        bool recording = msg.value("recording").toBool();
        std::set<std::string> appsToLookFor;
        appsToLookFor.insert("chrome.exe");
        std::string openApp = m_obsUtils.getOpenApp(appsToLookFor);
        // At this point, titles if fully populated and could be displayed, e.g.:
        if (!openApp.empty()) {
            if (m_debug) {
                qDebug() << "App found: " << QString::fromStdString(openApp);
            }
        }
        // open_app = ""
        // std::string folder = "Test folder"
        // if (!recording && open_app is not None) {
        //     rec_folder = os.path.join(folder, open_app).replace('\\', '/');
        //     QJsonObject object
        //     {
        //         {"rec-folder", rec_folder}
        //     };
        //     m_obsWebSocket.sendRequest("SetRecordingFolder", object);
        //     setFilenameFormatting(open_app);
        // } else if (recording && open_app is None) {
        //     m_obsWebSocket.send("StopRecording");
        //     changeFolderBack();
        // }
    }
}
