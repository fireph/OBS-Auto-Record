#ifndef OBSAUTORECORD_H
#define OBSAUTORECORD_H

#include <string>
#include <QJsonObject>
#include <QtCore/QObject>
#include <QTimer>
#include "ObsWebSocket.h"
#include "ObsUtils.h"

class ObsAutoRecord : public QObject
{
    Q_OBJECT
public:
    explicit ObsAutoRecord(const QUrl &url, const int interval, bool debug = false, QObject *parent = nullptr);
    void setAddress(const QUrl &url);
    void setInterval(const int interval);

private slots:
    void pingStatus();
    void startRecording(); 
    void setFilenameFormatting(QString appName);
    void changeFolderBack();
    void onStatus(QJsonObject msg);

private:
    ObsWebSocket m_obsWebSocket;
    ObsUtils m_obsUtils;
    QUrl m_url;
    bool m_debug;
    int m_msgid = 0;
    QTimer *timer;
    std::string defaultFilenameFormatting = "%CCYY-%MM-%DD %hh-%mm-%ss";
    std::set<std::string> appsToLookFor;
};

#endif // OBSAUTORECORD_H
