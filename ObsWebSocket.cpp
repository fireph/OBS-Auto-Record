#include "ObsWebSocket.h"
#include <QJsonDocument>
#include <QtCore/QDebug>

QT_USE_NAMESPACE

ObsWebSocket::ObsWebSocket(const QUrl &url, bool debug, QObject *parent) :
    QObject(parent),
    m_url(url),
    m_debug(debug)
{
    startWebsocket();
}

void ObsWebSocket::sendRequest(QString requestType, std::function<void(QJsonObject)>&& callback)
{
    QJsonObject data {};
    sendRequest(requestType, data, callback);
}

void ObsWebSocket::sendRequest(QString requestType, QJsonObject data, std::function<void(QJsonObject)>&& callback)
{
    m_msgid++;
    QString messageId = QString::number(m_msgid);
    QJsonObject object
    {
        {"request-type", requestType},
        {"message-id", messageId}
    };
    foreach(const QString& key, data.keys()) {
        QJsonValue value = data.value(key);
        object[key] = value;
    }
    m_webSocket.sendTextMessage(jsonToString(object));
}

void ObsWebSocket::onConnected()
{
    if (m_debug)
        qDebug() << "WebSocket connected:" << m_url;
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &ObsWebSocket::onTextMessageReceived);
    sendRequest("GetStreamingStatus");
}

void ObsWebSocket::onClosed()
{
    disconnect(&m_webSocket, &QWebSocket::connected,
               this, &ObsWebSocket::onConnected);
    disconnect(&m_webSocket, &QWebSocket::disconnected,
               this, &ObsWebSocket::onClosed);
    disconnect(&m_webSocket, &QWebSocket::textMessageReceived,
               this, &ObsWebSocket::onTextMessageReceived);
    startWebsocket();
}

void ObsWebSocket::onTextMessageReceived(QString message)
{
    if (m_debug)
        qDebug() << "Message received:" << message;
}

void ObsWebSocket::startWebsocket()
{
    if (m_debug)
        qDebug() << "Connecting to WebSocket server:" << m_url;
    connect(&m_webSocket, &QWebSocket::connected,
            this, &ObsWebSocket::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected,
            this, &ObsWebSocket::onClosed);
    m_webSocket.open(QUrl(m_url));
}

QString ObsWebSocket::jsonToString(QJsonObject json)
{
    QJsonDocument Doc(json);
    QByteArray ba = Doc.toJson();
    return QString(ba);
}
