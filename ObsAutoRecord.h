#ifndef OBSAUTORECORD_H
#define OBSAUTORECORD_H

#include <string>
#include <set>
#include <QJsonObject>
#include <QtCore/QObject>
#include "ObsWebSocket.h"
#include "ObsUtils.h"

class ObsAutoRecord : public QObject
{
    Q_OBJECT
public:
    explicit ObsAutoRecord(const QUrl &url, bool debug = false, QObject *parent = nullptr);

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
    std::string defaultFilenameFormatting = "%CCYY-%MM-%DD %hh-%mm-%ss";
};

#endif // OBSAUTORECORD_H
