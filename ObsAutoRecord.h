#ifndef OBSWEBSOCKET_H
#define OBSWEBSOCKET_H

#include <QJsonObject>
#include <QtCore/QObject>

class ObsAutoRecord : public QObject
{
    Q_OBJECT
public:
    explicit ObsAutoRecord(const QUrl &url, bool debug = false, QObject *parent = nullptr);

private Q_SLOTS:
    void onConnected();
    void onClosed();
    void onTextMessageReceived(QString message);
    void startWebsocket();
    QString jsonToString(QJsonObject json);

private:
    ObsWebSocket m_obsWebSocket;
    QUrl m_url;
    bool m_debug;
    int m_msgid = 0;
};

#endif // OBSWEBSOCKET_H
