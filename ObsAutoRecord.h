#ifndef OBSAUTORECORD_H
#define OBSAUTORECORD_H

#include <string>
#include <QJsonObject>
#include <QtCore/QObject>
#include <QTimer>
#include "ObsWebSocket.h"
#include "ObsUtils.h"
#include "ObsAutoRecordState.h"

class ObsAutoRecord : public QObject
{
    Q_OBJECT
public:
    explicit ObsAutoRecord(
        const QUrl &url,
        const int interval,
        const QString &folder,
        std::set<std::string> appsToWatch,
        bool debug = false,
        QObject *parent = nullptr);
    void setAddress(const QUrl &url);
    void setInterval(const int interval);
    void setFolder(const QString &folder);
    void setAppsToWatch(std::set<std::string> appsToWatch);

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
    ObsWebSocket m_obsWebSocket;
    ObsUtils m_obsUtils;
    QUrl m_url;
    QString m_folder;
    std::set<std::string> m_appsToWatch;
    bool m_debug;
    int m_msgid = 0;
    QTimer *timer;
    QString defaultFilenameFormatting = "%CCYY-%MM-%DD %hh-%mm-%ss";
    std::set<int> idsWaitToRecord;
};

#endif // OBSAUTORECORD_H
