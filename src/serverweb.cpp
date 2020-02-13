#include "serverweb.h"
#include <QUuid>

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
        connect(m_webSocketServer,SIGNAL(newConnection()),this,SLOT(setSocketConnected()));
        result = true;
    }
    else qDebug()<<"ERROR: Web-Server is not started on port:"<<port;

    return result;
}

void ServerWeb::sendText(const QByteArray &uuid, const QString &text)
{
    for(int i=0;i<m_webClients.size();++i)
        if(m_webClients.at(i)->property("uuid").toByteArray() == uuid)
            m_webClients.at(i)->sendTextMessage(text);
}

void ServerWeb::sendData(const QByteArray &uuid, const QByteArray &data)
{
    for(int i=0;i<m_webClients.size();++i)
        if(m_webClients.at(i)->property("uuid").toByteArray() == uuid)
            m_webClients.at(i)->sendBinaryMessage(data);
}

void ServerWeb::sendTextToAll(const QString &text)
{
    for(int i=0;i<m_webClients.size();++i)
        m_webClients.at(i)->sendTextMessage(text);
}

void ServerWeb::sendDataToAll(const QByteArray &data)
{
    for(int i=0;i<m_webClients.size();++i)
        m_webClients.at(i)->sendBinaryMessage(data);
}

void ServerWeb::setSocketConnected()
{
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();

    QByteArray uuidArr = QUuid::createUuid().toRfc4122();
    socket->setProperty("uuid",uuidArr);

    qDebug()<<"Web: New connection."<<QUuid::fromRfc4122(uuidArr);

    connect(socket,SIGNAL(binaryMessageReceived(QByteArray)),this,SLOT(binData(QByteArray)));
    connect(socket,SIGNAL(textMessageReceived(QString)),this,SLOT(textData(QString)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(setSocketDisconnected()));

    m_webClients.append(socket);

    emit socketConnected(uuidArr);
}

void ServerWeb::setSocketDisconnected()
{
    QWebSocket *socket = static_cast<QWebSocket*>(sender());

    QByteArray uuidArr = socket->property("uuid").toByteArray();

    qDebug()<<"Web: One disconnected"<<QUuid::fromRfc4122(uuidArr);

    m_webClients.removeOne(socket);
    socket->deleteLater();

    emit socketDisconnected(uuidArr);

    if(m_webClients.size() == 0)
        emit disconnectedAll();
}

void ServerWeb::textData(const QString &text)
{
    qDebug()<<"ServerWeb::textData"<<text;

    QWebSocket *socket = static_cast<QWebSocket*>(sender());
    QByteArray uuidArr = socket->property("uuid").toByteArray();

    emit textFromSocket(uuidArr, text);
}

void ServerWeb::binData(const QByteArray &buf)
{
//    qDebug()<<"ServerWeb::binData"<<buf;

    QWebSocket *socket = static_cast<QWebSocket*>(sender());
    QByteArray uuidArr = socket->property("uuid").toByteArray();

    emit dataFromSocket(uuidArr, buf);
}
