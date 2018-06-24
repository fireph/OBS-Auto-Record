#ifndef OBSAUTORECORD_H
#define OBSAUTORECORD_H

#include <string>
#include <QJsonObject>
#include <QtCore/QObject>
#include <QTimer>
#include "ObsUtils.hpp"
#include "ObsWebSocket.hpp"
#include "ObsAutoRecordState.hpp"

class ObsAutoRecord : public QObject
{
    Q_OBJECT
public:
    explicit ObsAutoRecord(
        const QUrl &url,
        const int interval,
        const QString &folder,
        std::unordered_map<std::string, std::string> &appsToWatch,
        bool debug = false,
        QObject *parent = nullptr);
    void setAddress(const QUrl &url);
    void setInterval(const int interval);
    void setFolder(const QString &folder);
    void setAppsToWatch(std::unordered_map<std::string, std::string> &appsToWatch);

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
    QUrl m_url;
    QString m_folder;
    std::unordered_map<std::string, std::string> m_appsToWatch;
    bool m_debug;
    int m_msgid = 0;
    QTimer *timer;
    QString defaultFilenameFormatting = "%CCYY-%MM-%DD %hh-%mm-%ss";
    std::set<int> idsWaitToRecord;
};

#endif // OBSAUTORECORD_H
