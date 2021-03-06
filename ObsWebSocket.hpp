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
    void setAddress(const QUrl &url);
    void sendRequest(const QString &requestType, int msgId);
    void sendRequest(const QString &requestType, int msgId, const QJsonObject &data);
    QString jsonToString(const QJsonObject &json);
    QJsonObject stringToJson(const QString &in);
    bool isConnected();

signals:
    void onResponse(QJsonObject json);
    void connected(bool isConnected);

private slots:
    void onConnected();
    void onClosed();
    void onMessageReceived(const QString &message);
    void startWebsocket();

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;
    bool m_isConnected = false;
};

#endif // OBSWEBSOCKET_H
