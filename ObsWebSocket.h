#ifndef OBSWEBSOCKET_H
#define OBSWEBSOCKET_H

#include <QJsonObject>
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

class ObsWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit ObsWebSocket(const QUrl &url, bool debug = false, QObject *parent = nullptr);
    void sendRequest(QString requestType);
    void sendRequest(QString requestType, QJsonObject data);

private Q_SLOTS:
    void onConnected();
    void onClosed();
    void onTextMessageReceived(QString message);
    void startWebsocket();
    QString jsonToString(QJsonObject json);

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;
    int m_msgid = 0;
};

#endif // OBSWEBSOCKET_H
