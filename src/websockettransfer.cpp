#include "websockettransfer.h"

WebSocketTransfer::WebSocketTransfer(QObject *parent) : QObject(parent),
    m_webSocketServer(Q_NULLPTR),
    m_type(TransferWebClients),
    m_port(8081)
{

}

void WebSocketTransfer::start()
{
    if(m_webSocketServer)
        return;

    m_webSocketServer = new QWebSocketServer("Web Socket Server",QWebSocketServer::NonSecureMode,this);

    if(m_webSocketServer->listen(QHostAddress::Any, m_port))
    {
        qDebug()<<"OK: Web-Server is started on port: "<<m_port;
        connect(m_webSocketServer,&QWebSocketServer::newConnection,this,&WebSocketTransfer::setSocketConnected);
    }
    else qDebug()<<"ERROR: Web-Server is not started on port:"<<m_port;
}

void WebSocketTransfer::stop()
{
    if(m_webSocketServer)
    {
        if(m_webSocketServer->isListening())
            m_webSocketServer->close();
    }

    emit finished();
}

void WebSocketTransfer::setPort(quint16 port)
{
    m_port = port;
}

void WebSocketTransfer::setLoginPass(const QString &login, const QString &pass)
{
    m_login = login;
    m_pass = pass;
}

void WebSocketTransfer::setType(int type)
{
    m_type = type;
}

void WebSocketTransfer::checkRemoteAuthentication(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request)
{
    foreach(WebSocketHandler *socketHandler, m_sockets)
        socketHandler->checkRemoteAuthentication(uuid,nonce,request);
}

void WebSocketTransfer::setRemoteAuthenticationResponse(const QByteArray &uuidDst, const QByteArray &uuidSrc, const QByteArray &nameSrc)
{
    foreach(WebSocketHandler *socketHandler, m_sockets)
    {
        if(socketHandler->getUuid() == uuidDst)
        {
            socketHandler->setRemoteAuthenticationResponse(uuidSrc, nameSrc);
            break;
        }
    }
}

void WebSocketTransfer::createProxyConnection(WebSocketHandler *handler, const QByteArray &uuid)
{
    foreach(WebSocketHandler *socketHandler, m_sockets)
    {
        if(socketHandler->getUuid() == uuid)
        {
            socketHandler->createProxyConnection(handler);
            break;
        }
    }
}

void WebSocketTransfer::setSocketConnected()
{
    QWebSocket *webSocket = m_webSocketServer->nextPendingConnection();

    WebSocketHandler *socketHandler = new WebSocketHandler(this);
    connect(socketHandler, &WebSocketHandler::disconnected, this, &WebSocketTransfer::socketDisconnected);

    if(m_type == TransferDesktops)
        socketHandler->setType(WebSocketHandler::HandlerDesktop);
    else if(m_type == TransferWebClients)
        socketHandler->setType(WebSocketHandler::HandlerWebClient);
    else if(m_type == TransferProxyClients)
        socketHandler->setType(WebSocketHandler::HandlerProxyClient);

    socketHandler->setLoginPass(m_login, m_pass);
    socketHandler->setSocket(webSocket);
    m_sockets.append(socketHandler);

    emit newSocketConnected(socketHandler);
}

void WebSocketTransfer::socketDisconnected(WebSocketHandler *pointer)
{
    bool isDisconnectedAll = false;

    if(m_sockets.contains(pointer))
    {
        m_sockets.removeOne(pointer);

        if(m_sockets.size() == 0)
            isDisconnectedAll = true;
    }

    if(pointer)
    {
        qDebug()<<"Disconnected one:"<<pointer->getName();

        QWebSocket *socket = pointer->getSocket();
        socket->disconnect();
        socket->deleteLater();

        pointer->disconnect();
        pointer->deleteLater();
    }

    if(isDisconnectedAll)
        emit disconnectedAll();
}
