#include "serverweb.h"

ServerWeb::ServerWeb(QObject *parent) : QObject(parent)
{
    m_webSocketServer = new QWebSocketServer("Web Socket Server",QWebSocketServer::NonSecureMode,this);
}

ServerWeb::~ServerWeb()
{
    m_webSocketServer->close();
    m_webClients.clear();
}

bool ServerWeb::initServer(quint16 port)
{
    bool result = false;
    if(m_webSocketServer->listen(QHostAddress::Any, port))
    {
        qDebug()<<"OK: Web-Server is started on port: "<<port;
        connect(m_webSocketServer,SIGNAL(newConnection()),this,SLOT(newSocketConnection()));
        result = true;
    }
    else qDebug()<<"ERROR: Web-Server is not started on port:"<<port;

    return result;
}

void ServerWeb::sendText(const QString &text)
{
    for(int i=0;i<m_webClients.size();++i)
        m_webClients.at(i)->sendTextMessage(text);
}

void ServerWeb::sendData(const QByteArray &data)
{
    for(int i=0;i<m_webClients.size();++i)
        m_webClients.at(i)->sendBinaryMessage(data);
}


void ServerWeb::newSocketConnection()
{
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();

    qDebug()<<"Web: New connection.";

    connect(socket,SIGNAL(binaryMessageReceived(QByteArray)),this,SLOT(binData(QByteArray)));
    connect(socket,SIGNAL(textMessageReceived(QString)),this,SLOT(textData(QString)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnected()));

    m_webClients.append(socket);
}

void ServerWeb::socketDisconnected()
{
    QWebSocket *socket = static_cast<QWebSocket*>(sender());

    if(socket)
    {
        qDebug()<<"Web: One disconnected";
        m_webClients.removeOne(socket);
        socket->deleteLater();

        if(m_webClients.size() == 0)
            emit disconnectedAll();
    }
}

void ServerWeb::textData(const QString &text)
{
    qDebug()<<"ServerWeb::textData"<<text;
    emit textFromSocket(text);
}

void ServerWeb::binData(const QByteArray &buf)
{
    qDebug()<<"ServerWeb::binData"<<buf;
    emit dataFromSocket(buf);
}
