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
    void sendRequest(QString requestType, int msgId);
    void sendRequest(QString requestType, int msgId, QJsonObject data);
    QString jsonToString(const QJsonObject& json);
    QJsonObject stringToJson(const QString& in);
    bool isConnected();

signals:
    void onResponse(QJsonObject json);

private slots:
    void onConnected();
    void onClosed();
    void onMessageReceived(QString message);
    void startWebsocket();

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;
    bool m_isConnected;
};

#endif // OBSWEBSOCKET_H
