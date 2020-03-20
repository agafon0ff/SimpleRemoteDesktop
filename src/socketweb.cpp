#include "socketweb.h"
#include <QDebug>

SocketWeb::SocketWeb(QObject *parent) : QObject(parent),
    m_webSocket(Q_NULLPTR),
    m_timerReconnect(Q_NULLPTR)
{

}

void SocketWeb::start()
{
    deleteSocket();

    m_webSocket = new QWebSocket("",QWebSocketProtocol::VersionLatest,this);
    connect(m_webSocket, &QWebSocket::stateChanged, this, &SocketWeb::socketStateChanged);
    connect(m_webSocket, &QWebSocket::textMessageReceived,this, &SocketWeb::textMessageReceived);

    m_timerReconnect = new QTimer(this);
    connect(m_timerReconnect,SIGNAL(timeout()),this,SLOT(reconnectTimerTick()));

    m_webSocket->open(QUrl(m_url));
}

void SocketWeb::stop()
{
    deleteSocket();
    emit finished();
}

void SocketWeb::setUrl(const QString &url)
{
    m_url = url;
    qDebug()<<"SocketWeb::setUrl"<<url<<QUrl(url).isValid();
}

void SocketWeb::socketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug()<< "SocketWeb::socketStateChanged" << state;

    switch(state) {
    case QAbstractSocket::UnconnectedState:
        m_timerReconnect->start(1000);
        break;

    case QAbstractSocket::HostLookupState:
        break;

    case QAbstractSocket::ConnectingState:
        m_timerReconnect->start(1000);
        break;

    case QAbstractSocket::ConnectedState:
        qDebug()<<"SocketWeb::socketStateChanged: Connected to server.";
        m_webSocket->sendTextMessage("Hello, world!");
        break;

    case QAbstractSocket::ClosingState:
        qDebug()<<"SocketWeb::socketStateChanged: Disconnected from server.";
        break;

    default:
        break;
    }
}

void SocketWeb::textMessageReceived(const QString &message)
{
    qDebug() << "Message received:" << message;
}

void SocketWeb::reconnectTimerTick()
{
    if(m_webSocket->state() == QAbstractSocket::ConnectedState)
    {
        m_timerReconnect->stop();
        return;
    }

    if(m_webSocket->state() == QAbstractSocket::ConnectingState)
        m_webSocket->abort();

    m_webSocket->open(QUrl(m_url));
}

void SocketWeb::deleteSocket()
{
    if(m_timerReconnect)
    {
        if(m_timerReconnect->isActive())
            m_timerReconnect->stop();

        m_timerReconnect->deleteLater();
        m_timerReconnect = Q_NULLPTR;
    }

    if(m_webSocket)
    {
        if(m_webSocket->state() != QAbstractSocket::UnconnectedState)
            m_webSocket->close();

        m_webSocket->disconnect();
        m_webSocket->deleteLater();
        m_webSocket = Q_NULLPTR;
    }
}
