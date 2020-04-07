#include "websockethandler.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QUuid>

static const QByteArray KEY_GET_IMAGE = "GIMG";
static const QByteArray KEY_IMAGE_PARAM = "IMGP";
static const QByteArray KEY_IMAGE_TILE = "IMGT";
static const QByteArray KEY_SET_KEY_STATE = "SKST";
static const QByteArray KEY_SET_CURSOR_POS = "SCUP";
static const QByteArray KEY_SET_CURSOR_DELTA = "SCUD";
static const QByteArray KEY_SET_MOUSE_KEY = "SMKS";
static const QByteArray KEY_SET_MOUSE_WHEEL = "SMWH";
static const QByteArray KEY_CHANGE_DISPLAY = "CHDP";
static const QByteArray KEY_TILE_RECEIVED = "TLRD";
static const QByteArray KEY_SET_NONCE = "STNC";
static const QByteArray KEY_SET_AUTH_REQUEST = "SARQ";
static const QByteArray KEY_SET_AUTH_RESPONSE = "SARP";

const int COMMAD_SIZE = 4;
const int REQUEST_MIN_SIZE = 6;

WebSocketHandler::WebSocketHandler(QObject *parent) : QObject(parent),
    m_webSocket(Q_NULLPTR),
    m_isAuthenticated(false)
{

}

void WebSocketHandler::crateSocket()
{
    if(m_webSocket)
        return;

    m_webSocket = new QWebSocket("",QWebSocketProtocol::VersionLatest,this);
    connect(m_webSocket, &QWebSocket::stateChanged, this, &WebSocketHandler::socketStateChanged);
    connect(m_webSocket, &QWebSocket::textMessageReceived,this, &WebSocketHandler::textMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived,this, &WebSocketHandler::binaryMessageReceived);

    m_timerReconnect = new QTimer(this);
    connect(m_timerReconnect,SIGNAL(timeout()),this,SLOT(reconnectTimerTick()));

    m_webSocket->open(QUrl(m_url));
}

void WebSocketHandler::removeSocket()
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

    emit finished();
}

void WebSocketHandler::setUrl(const QString &url)
{
    m_url = url;
    qDebug()<<"SocketWeb::setUrl"<<url<<QUrl(url).isValid();
}

void WebSocketHandler::setLoginPass(const QString &login, const QString &pass)
{
    m_login = login;
    m_pass = pass;
}

void WebSocketHandler::setSocket(QWebSocket *webSocket)
{
    if(!webSocket)
        return;

    m_webSocket = webSocket;
    connect(m_webSocket, &QWebSocket::textMessageReceived,this, &WebSocketHandler::textMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived,this, &WebSocketHandler::binaryMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,this, &WebSocketHandler::socketDisconnected);

    m_uuid = QUuid::createUuid().toRfc4122();
    m_nonce = QUuid::createUuid().toRfc4122();

    QByteArray data;
    data.append(KEY_SET_NONCE);
    data.append(arrayFromUint16(static_cast<quint16>(m_nonce.size())));
    data.append(m_nonce);

    sendBinaryMessage(data);
}

void WebSocketHandler::sendImageParameters(const QSize &imageSize, int rectWidth)
{
    if(!m_isAuthenticated)
        return;

    QByteArray data;
    data.append(KEY_IMAGE_PARAM);
    data.append(arrayFromUint16(6));
    data.append(arrayFromUint16(static_cast<quint16>(imageSize.width())));
    data.append(arrayFromUint16(static_cast<quint16>(imageSize.height())));
    data.append(arrayFromUint16(static_cast<quint16>(rectWidth)));

    sendBinaryMessage(data);
}

void WebSocketHandler::sendImageTile(quint16 posX, quint16 posY, const QByteArray &imageData, quint16 tileNum)
{
    if(!m_isAuthenticated)
        return;

    QByteArray data;
    data.append(KEY_IMAGE_TILE);
    data.append(arrayFromUint16(static_cast<quint16>(imageData.size() + 6)));
    data.append(arrayFromUint16(posX));
    data.append(arrayFromUint16(posY));
    data.append(arrayFromUint16(tileNum));
    data.append(imageData);

    sendBinaryMessage(data);
}

void WebSocketHandler::newData(const QByteArray &command, const QByteArray &data)
{
//    qDebug()<<"DataParser::newData"<<command<<data;

    if(command == KEY_SET_AUTH_REQUEST)
    {
        checkAuthentication(data);
        return;
    }
    else
    {
        if(!m_isAuthenticated)
            return;
    }

    if(command == KEY_GET_IMAGE)
    {
        emit getDesktop();
    }
    else if(command == KEY_TILE_RECEIVED)
    {
        quint16 tileNum = uint16FromArray(data.mid(0,2));
        emit receivedTileNum(tileNum);
    }
    else if(command == KEY_CHANGE_DISPLAY)
    {
        emit changeDisplayNum();
    }
    else if(command == KEY_SET_CURSOR_POS)
    {
        if(data.size() >= 4)
        {
            quint16 posX = uint16FromArray(data.mid(0,2));
            quint16 posY = uint16FromArray(data.mid(2,2));
            emit setMouseMove(posX, posY);
        }
    }
    else if(command == KEY_SET_CURSOR_DELTA)
    {
        if(data.size() >= 4)
        {
            qint16 deltaX = static_cast<qint16>(uint16FromArray(data.mid(0,2)));
            qint16 deltaY = static_cast<qint16>(uint16FromArray(data.mid(2,2)));
            emit setMouseDelta(deltaX, deltaY);
        }
    }
    else if(command == KEY_SET_KEY_STATE)
    {
        if(data.size() >= 4)
        {
            quint16 keyCode = uint16FromArray(data.mid(0,2));
            quint16 keyState = uint16FromArray(data.mid(2,4));
            emit setKeyPressed(keyCode,static_cast<bool>(keyState));
        }
    }
    else if(command == KEY_SET_MOUSE_KEY)
    {
        if(data.size() >= 4)
        {
            quint16 keyCode = uint16FromArray(data.mid(0,2));
            quint16 keyState = uint16FromArray(data.mid(2,4));
            emit setMousePressed(keyCode,static_cast<bool>(keyState));
        }
    }
    else if(command == KEY_SET_MOUSE_WHEEL)
    {
        if(data.size() >= 4)
        {
            quint16 keyState = uint16FromArray(data.mid(2,4));
            emit setWheelChanged(static_cast<bool>(keyState));
        }
    }
    else
    {
        qDebug()<<"DataParser::newData"<<command<<data;
        debugHexData(data);
    }
}

void WebSocketHandler::checkAuthentication(const QByteArray &request)
{
    QString sum = m_login + m_pass;
    QByteArray concatFirst = QCryptographicHash::hash(sum.toUtf8(),QCryptographicHash::Md5).toBase64();
    concatFirst.append(m_nonce.toBase64());
    QByteArray concatSecond = QCryptographicHash::hash(concatFirst,QCryptographicHash::Md5).toBase64();
    bool isAuthenticated = false;

    if(request.toBase64() == concatSecond)
    {
        if(!m_isAuthenticated)
            m_isAuthenticated = true;

        isAuthenticated = true;
    }

    QByteArray data;
    data.append(KEY_SET_AUTH_RESPONSE);
    data.append(arrayFromUint16(2));

    if(isAuthenticated)
        data.append(arrayFromUint16(1));
    else data.append(arrayFromUint16(0));

    sendBinaryMessage(data);
}

void WebSocketHandler::socketStateChanged(QAbstractSocket::SocketState state)
{
    switch(state)
    {
        case QAbstractSocket::UnconnectedState:
        {
            if(m_timerReconnect)
                if(m_timerReconnect->isActive())
                    m_timerReconnect->start(1000);
            break;
        }
        case QAbstractSocket::HostLookupState:
        {
            break;
        }
        case QAbstractSocket::ConnectingState:
        {
            if(m_timerReconnect)
                if(m_timerReconnect->isActive())
                    m_timerReconnect->start(1000);
            break;
        }
        case QAbstractSocket::ConnectedState:
        {
            qDebug()<<"SocketWeb::socketStateChanged: Connected to server.";
            break;
        }
        case QAbstractSocket::ClosingState:
        {
            qDebug()<<"SocketWeb::socketStateChanged: Disconnected from server.";
            break;
        }
        default:
        {
            break;
        }
    }
}

void WebSocketHandler::socketDisconnected()
{
    emit disconnected(this);
}

void WebSocketHandler::sendBinaryMessage(const QByteArray &data)
{
    if(m_webSocket)
        if(m_webSocket->state() == QAbstractSocket::ConnectedState)
            m_webSocket->sendBinaryMessage(data);
}

void WebSocketHandler::textMessageReceived(const QString &message)
{
    qDebug() << "WebSocketHandler::textMessageReceived:" << message;
}

void WebSocketHandler::binaryMessageReceived(const QByteArray &data)
{
    QByteArray activeBuf = m_dataTmp;
    activeBuf.append(data);
    m_dataTmp.clear();

    int size = activeBuf.size();

    if(size == COMMAD_SIZE)
    {
        newData(data, QByteArray());
        return;
    }

    if(size < REQUEST_MIN_SIZE)
        return;

    int dataStep = 0;

    for(int i=0;i<size;++i)
    {
        QByteArray command = activeBuf.mid(dataStep, COMMAD_SIZE);
        quint16 dataSize = uint16FromArray(activeBuf.mid(dataStep + COMMAD_SIZE, 2));

        if(size >= (dataStep + COMMAD_SIZE + dataSize))
        {
            QByteArray payload = activeBuf.mid(dataStep + COMMAD_SIZE + 2, dataSize);
            dataStep += COMMAD_SIZE + 2 + dataSize;
            newData(command, payload);

            i = dataStep;
        }
        else
        {
            debugHexData(activeBuf);

            m_dataTmp = activeBuf.mid(dataStep, size - dataStep);

            if(m_dataTmp.size() > 2000)
                m_dataTmp.clear();

            break;
        }
    }
}

void WebSocketHandler::reconnectTimerTick()
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

void WebSocketHandler::debugHexData(const QByteArray &data)
{
    QString textHex;
    int dataSize = data.size();
    for(int i=0;i<dataSize;++i)
    {
        quint8 oneByte = static_cast<quint8>(data.at(i));
        textHex.append(QString::number(oneByte,16));

        if(i < dataSize-1)
            textHex.append("|");
    }

    qDebug()<<"DataParser::debugHexData:"<<textHex<<data;
}

QByteArray WebSocketHandler::arrayFromUint16(quint16 number)
{
    QByteArray buf;
    buf.resize(2);
    buf[0] = static_cast<char>(number);
    buf[1] = static_cast<char>(number >> 8);
    return buf;
}

quint16 WebSocketHandler::uint16FromArray(const QByteArray &buf)
{
    if(buf.count() == 2)
    {
        quint16 m_number;
        m_number = static_cast<quint16>(static_cast<quint8>(buf[0]) |
                static_cast<quint8>(buf[1]) << 8);
        return m_number;
    }
    else return 0x0000;
}
