#include "ObsAutoRecord.hpp"

#include <QJsonDocument>
#include <QtCore/QDebug>

QT_USE_NAMESPACE

ObsAutoRecord::ObsAutoRecord(
    const QUrl &url,
    const int &interval,
    const QString &folder,
    QHash<QString, QString> &appsToWatch,
    bool debug,
    QObject *parent) :
        QObject(parent),
        m_obsWebSocket(url, debug, this),
        m_url(url),
        m_interval(interval * 1000),
        m_folder(folder),
        m_appsToWatch(appsToWatch),
        m_debug(debug)
{
    QObject::connect(&m_obsWebSocket, SIGNAL(onResponse(QJsonObject)),
                     this, SLOT(onStatus(QJsonObject)));
    QObject::connect(&m_obsWebSocket, SIGNAL(connected(bool)),
                     this, SLOT(setIsConnected(bool)));
    timer = new QTimer;
    timer->setInterval(m_interval);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(pingStatus()));
}

void ObsAutoRecord::setAddress(const QUrl &url)
{
    m_url = url;
    m_obsWebSocket.setAddress(m_url);
}

void ObsAutoRecord::setInterval(const int &interval)
{
    m_interval = interval * 1000;
    timer->setInterval(m_interval);
}

void ObsAutoRecord::setFolder(const QString &folder)
{
    m_folder = folder;
}

void ObsAutoRecord::toggleIsPaused()
{
    m_isPaused = !m_isPaused;
    internalUpdateState();
    pingStatusOverrideTimer();
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
        
void ObsAutoRecord::setFilenameFormatting(QString appName, int msgid)
{
    QString filenameFormatting = appName + " - " + defaultFilenameFormatting;
    QJsonObject object
    {
        {"filename-formatting", filenameFormatting}
    };
    m_obsWebSocket.sendRequest("SetFilenameFormatting", msgid, object);
}

void ObsAutoRecord::changeFolderBack()
{
    if (!m_folder.isEmpty()) {
        QJsonObject object
        {
            {"rec-folder", m_folder}
        };
        m_msgid++;
        m_obsWebSocket.sendRequest("SetRecordingFolder", m_msgid, object);
    }
    QJsonObject object2
    {
        {"filename-formatting", defaultFilenameFormatting}
    };
    m_msgid++;
    m_obsWebSocket.sendRequest("SetFilenameFormatting", m_msgid, object2);
}

void ObsAutoRecord::onStatus(QJsonObject msg)
{
    if (msg.contains("message-id")) {
        int msgid = msg.value("message-id").toString().toInt();
        if (idsWaitToRecord.contains(msgid)) {
            idsWaitToRecord.remove(msgid);
            if (idsWaitToRecord.isEmpty()) {
                startRecording();
            }
        }
    }
    if (msg.contains("recording") && idsWaitToRecord.isEmpty()) {
        bool recording = msg.value("recording").toBool();
        QString openApp = ObsUtils::getOpenApp(m_appsToWatch);
        if (!recording && !openApp.isEmpty() && !m_isPaused) {
            if (m_debug) {
                qDebug() << "App found: " << openApp;
            }
            if (!m_folder.isEmpty()) {
                QString recFolder = m_folder;
                if (m_folder.contains("\\")) {
                    // windows paths
                    if (m_folder.endsWith("\\")) {
                        recFolder.append(openApp);
                    } else {
                        recFolder.append("\\" + openApp);
                    }
                } else if (m_folder.contains("/")) {
                    // unix paths
                    if (m_folder.endsWith("/")) {
                        recFolder.append(openApp);
                    } else {
                        recFolder.append("/" + openApp);
                    }
                } else {
                    // not a valid path
                }
                QJsonObject object
                {
                    {"rec-folder", recFolder}
                };
                m_msgid++;
                idsWaitToRecord.insert(m_msgid);
                m_obsWebSocket.sendRequest("SetRecordingFolder", m_msgid, object);
            }
            m_msgid++;
            idsWaitToRecord.insert(m_msgid);
            setFilenameFormatting(openApp, m_msgid);
        } else if (recording && (openApp.isEmpty() || m_isPaused)) {
            m_msgid++;
            m_obsWebSocket.sendRequest("StopRecording", m_msgid);
            changeFolderBack();
        }
    }
}

void ObsAutoRecord::setIsConnected(bool isConnected)
{
    m_isConnected = isConnected;
    internalUpdateState();
    pingStatusOverrideTimer();
}

void ObsAutoRecord::pingStatusOverrideTimer()
{
    if (m_isConnected) {
        timer->setInterval(m_interval);
        pingStatus();
    }
}

void ObsAutoRecord::internalUpdateState()
{
    if (m_isPaused) {
        emit onStateUpdate(ObsAutoRecordState::PAUSED);
    } else {
        if (m_isConnected) {
            emit onStateUpdate(ObsAutoRecordState::CONNECTED);
        } else {
            idsWaitToRecord.clear();
            emit onStateUpdate(ObsAutoRecordState::DISCONNECTED);
        }
    }
}
