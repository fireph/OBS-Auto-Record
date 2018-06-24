#include "ObsWebSocket.hpp"
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

void ObsWebSocket::setAddress(const QUrl &url)
{
    if (m_url != url) {
        m_url = url;
        m_webSocket.close();
    }
}

void ObsWebSocket::sendRequest(QString requestType, int msgId)
{
    QJsonObject data {};
    sendRequest(requestType, msgId, data);
}

void ObsWebSocket::sendRequest(QString requestType, int msgId, QJsonObject data)
{
    QString messageId = QString::number(msgId);
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

QString ObsWebSocket::jsonToString(const QJsonObject& json)
{
    QJsonDocument Doc(json);
    QByteArray ba = Doc.toJson();
    return QString(ba);
}

QJsonObject ObsWebSocket::stringToJson(const QString& in)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();        
        }
        else
        {
            if (m_debug)
                qDebug() << "Document is not an object" << endl;
        }
    }
    else
    {
        if (m_debug)
            qDebug() << "Invalid JSON..." << endl;
    }
    return obj;
}

bool ObsWebSocket::isConnected()
{
    return m_isConnected;
}

void ObsWebSocket::onConnected()
{
    if (m_debug)
        qDebug() << "WebSocket connected:" << m_url;
    m_isConnected = true;
    emit connected(m_isConnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &ObsWebSocket::onMessageReceived);
}

void ObsWebSocket::onClosed()
{
    m_isConnected = false;
    emit connected(m_isConnected);
    disconnect(&m_webSocket, &QWebSocket::connected,
               this, &ObsWebSocket::onConnected);
    disconnect(&m_webSocket, &QWebSocket::disconnected,
               this, &ObsWebSocket::onClosed);
    disconnect(&m_webSocket, &QWebSocket::textMessageReceived,
               this, &ObsWebSocket::onMessageReceived);
    startWebsocket();
}

void ObsWebSocket::onMessageReceived(QString message)
{
    emit onResponse(stringToJson(message));
    if (m_debug)
        qDebug().noquote() << "Message received:" << message;
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
