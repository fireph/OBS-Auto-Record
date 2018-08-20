#ifndef OBSAUTORECORD_H
#define OBSAUTORECORD_H

#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QtCore/QObject>
#include <QTimer>

#include "ObsAutoRecordState.hpp"
#include "ObsUtils.hpp"
#include "ObsWebSocket.hpp"

class ObsAutoRecord : public QObject
{
    Q_OBJECT
public:
    explicit ObsAutoRecord(
        const QUrl &url,
        const int &interval,
        const QString &folder,
        QHash<QString, QString> &appsToWatch,
        bool debug = false,
        QObject *parent = nullptr);
    void setAddress(const QUrl &url);
    void setInterval(const int &interval);
    void setFolder(const QString &folder);
    void toggleIsPaused();

signals:
    void onStateUpdate(ObsAutoRecordState state);

private slots:
    void pingStatus();
    void startRecording(); 
    void setFilenameFormatting(QString appName, int msgid);
    void changeFolderBack();
    void onStatus(QJsonObject msg);
    void setIsConnected(bool isConnected);

private:
    void pingStatusOverrideTimer();
    void internalUpdateState();
    ObsWebSocket m_obsWebSocket;
    QUrl m_url;
    int m_interval;
    QString m_folder;
    QHash<QString, QString> &m_appsToWatch;
    bool m_debug;
    int m_msgid = 0;
    QTimer *timer;
    QString defaultFilenameFormatting = "%CCYY-%MM-%DD %hh-%mm-%ss";
    QSet<int> idsWaitToRecord;
    bool m_isConnected = false;
    bool m_isPaused = false;
};

#endif // OBSAUTORECORD_H
