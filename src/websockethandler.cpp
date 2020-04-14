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
static const QByteArray KEY_CHECK_AUTH_REQUEST = "CARQ";
static const QByteArray KEY_CHECK_AUTH_RESPONSE = "CARP";
static const QByteArray KEY_SET_NAME = "STNM";
static const QByteArray KEY_CONNECT_UUID = "CTUU";

const int COMMAD_SIZE = 4;
const int REQUEST_MIN_SIZE = 6;
const int SIZE_UUID = 16;

WebSocketHandler::WebSocketHandler(QObject *parent) : QObject(parent),
    m_webSocket(Q_NULLPTR),
    m_timerReconnect(Q_NULLPTR),
    m_timerWaitResponse(Q_NULLPTR),
    m_type(HandlerWebClient),
    m_waitType(WaitTypeUnknown),
    m_isAuthenticated(false)
{

}

void WebSocketHandler::createSocket()
{
    if(m_webSocket)
        return;

    m_webSocket = new QWebSocket("",QWebSocketProtocol::VersionLatest,this);
    connect(m_webSocket, &QWebSocket::stateChanged, this, &WebSocketHandler::socketStateChanged);
    connect(m_webSocket, &QWebSocket::textMessageReceived,this, &WebSocketHandler::textMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived,this, &WebSocketHandler::binaryMessageReceived);

    if(!m_timerReconnect)
    {
        m_timerReconnect = new QTimer(this);
        connect(m_timerReconnect, &QTimer::timeout, this, &WebSocketHandler::timerReconnectTick);
    }

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

    if(m_timerWaitResponse)
    {
        if(m_timerWaitResponse->isActive())
            m_timerWaitResponse->stop();

        m_timerWaitResponse->deleteLater();
        m_timerWaitResponse = Q_NULLPTR;
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

void WebSocketHandler::setType(int type)
{
    m_type = type;
}

void WebSocketHandler::setName(const QString &name)
{
    m_name = name;
}

QString WebSocketHandler::getName()
{
    return m_name;
}

QByteArray WebSocketHandler::getUuid()
{
    return m_uuid;
}

void WebSocketHandler::setLoginPass(const QString &login, const QString &pass)
{
    m_login = login;
    m_pass = pass;
}

void WebSocketHandler::setProxyLoginPass(const QString &login, const QString &pass)
{
    m_proxyLogin = login;
    m_proxyPass = pass;
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

QWebSocket *WebSocketHandler::getSocket()
{
    return m_webSocket;
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

void WebSocketHandler::sendName(const QString &name)
{
    if(!m_isAuthenticated)
        return;

    QByteArray nameArray = name.toUtf8();
    QByteArray data;
    data.append(KEY_SET_NAME);
    data.append(arrayFromUint16(static_cast<quint16>(nameArray.size())));
    data.append(nameArray);

    sendBinaryMessage(data);
}

void WebSocketHandler::checkRemoteAuthentication(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request)
{
    if(!m_isAuthenticated)
        return;

    if(uuid.size() != SIZE_UUID ||
       nonce.size() != SIZE_UUID ||
       request.size() != SIZE_UUID)
        return;

    QByteArray data;
    data.append(KEY_CHECK_AUTH_REQUEST);
    data.append(arrayFromUint16(static_cast<quint16>(SIZE_UUID * 3)));
    data.append(uuid);
    data.append(nonce);
    data.append(request);

    sendBinaryMessage(data);
}

void WebSocketHandler::setRemoteAuthenticationResponse(const QByteArray &uuid, const QByteArray &name)
{
    qDebug()<<"WebSocketHandler::setRemoteAuthenticationResponse"<<uuid<<name;
    stopWaitResponseTimer();

    QByteArray data;
    data.append(KEY_CHECK_AUTH_RESPONSE);
    data.append(arrayFromUint16(static_cast<quint16>(SIZE_UUID + name.size())));
    data.append(uuid);
    data.append(name);

    sendBinaryMessage(data);
}

void WebSocketHandler::createProxyConnection(WebSocketHandler *handler)
{
    disconnect(m_webSocket, &QWebSocket::binaryMessageReceived,this, &WebSocketHandler::binaryMessageReceived);
    disconnect(handler->getSocket(), &QWebSocket::binaryMessageReceived,handler, &WebSocketHandler::binaryMessageReceived);
    disconnect(handler->getSocket(), &QWebSocket::disconnected,this, &WebSocketHandler::socketDisconnected);

    connect(m_webSocket, &QWebSocket::binaryMessageReceived, handler->getSocket(), &QWebSocket::sendBinaryMessage);
    connect(handler->getSocket(), &QWebSocket::binaryMessageReceived, m_webSocket, &QWebSocket::sendBinaryMessage);
    connect(handler->getSocket(), &QWebSocket::disconnected, this, &WebSocketHandler::createNormalConnection);

    connect(this, &WebSocketHandler::proxyConnectionCreated, handler, &WebSocketHandler::sendAuthenticationResponse);
    emit proxyConnectionCreated(true);
}

void WebSocketHandler::createNormalConnection()
{
    connect(m_webSocket, &QWebSocket::binaryMessageReceived,this, &WebSocketHandler::binaryMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,this, &WebSocketHandler::socketDisconnected);
}

void WebSocketHandler::newData(const QByteArray &command, const QByteArray &data)
{
//    qDebug()<<"DataParser::newData"<<command<<data;

    if(!m_isAuthenticated)
    {
        if(command == KEY_SET_AUTH_REQUEST)
        {
            if(m_type == HandlerWebClient || m_type == HandlerDesktop)
            {
                if(data.toBase64() == getHashSum(m_nonce, m_login, m_pass))
                {
                    if(!m_isAuthenticated)
                        m_isAuthenticated = true;
                }
                else m_isAuthenticated = false;

                sendAuthenticationResponse(m_isAuthenticated);
            }
            else if(m_type == HandlerProxyClient)
            {
                emit remoteAuthenticationRequest(m_uuid, m_nonce, data);
                startWaitResponseTimer(5000,WaitTypeRemoteAuth);
            }

            return;
        }
        else if(command == KEY_SET_NONCE)
        {
            if(m_type == HandlerSingleClient)
            {
                m_nonce = data;
                sendAuthenticationRequestToProxy();
            }

            return;
        }
        else if(command == KEY_SET_AUTH_RESPONSE)
        {
            bool authState = uint16FromArray(data);

            if(m_type == HandlerSingleClient && authState)
            {
                m_isAuthenticated = true;
                sendName(m_name);
            }
            else m_isAuthenticated = false;

            return;
        }
        else if(command == KEY_CONNECT_UUID)
        {
            emit newProxyConnection(this, data);
        }

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
            quint16 keyState = uint16FromArray(data.mid(2,2));
            emit setKeyPressed(keyCode,static_cast<bool>(keyState));
        }
    }
    else if(command == KEY_SET_MOUSE_KEY)
    {
        if(data.size() >= 4)
        {
            quint16 keyCode = uint16FromArray(data.mid(0,2));
            quint16 keyState = uint16FromArray(data.mid(2,2));
            emit setMousePressed(keyCode,static_cast<bool>(keyState));
        }
    }
    else if(command == KEY_SET_MOUSE_WHEEL)
    {
        if(data.size() >= 4)
        {
            quint16 keyState = uint16FromArray(data.mid(2,2));
            emit setWheelChanged(static_cast<bool>(keyState));
        }
    }
    else if(command == KEY_SET_NAME)
    {
        m_name = QString::fromUtf8(data);
        qDebug()<<"New desktop connected:"<<m_name;
    }
    else if(command == KEY_CHECK_AUTH_REQUEST)
    {
        if(data.size() == SIZE_UUID * 3)
        {
            QByteArray uuid = data.mid(0, SIZE_UUID);
            QByteArray nonce = data.mid(SIZE_UUID, SIZE_UUID);
            QByteArray requset = data.mid(SIZE_UUID * 2, SIZE_UUID);

            sendRemoteAuthenticationResponse(uuid, nonce, requset);
        }
    }
    else if(command == KEY_CHECK_AUTH_RESPONSE)
    {
        if(data.size() == SIZE_UUID + 2)
        {
            QByteArray uuid = data.mid(0, SIZE_UUID);
            quint16 authResponse = uint16FromArray(data.mid(SIZE_UUID, 2));
            QByteArray nameUtf8 = m_name.toUtf8();
            emit remoteAuthenticationResponse(uuid, m_uuid, nameUtf8, static_cast<bool>(authResponse));
        }
    }
    else if(command == KEY_IMAGE_TILE)
    {

    }
    else
    {
        qDebug()<<"DataParser::newData"<<command<<data;
        debugHexData(data);
    }
}

void WebSocketHandler::sendAuthenticationRequestToProxy()
{
    QByteArray hashSum = QByteArray::fromBase64(getHashSum(m_nonce, m_proxyLogin, m_proxyPass));
    QByteArray data;
    data.append(KEY_SET_AUTH_REQUEST);
    data.append(arrayFromUint16(static_cast<quint16>(hashSum.size())));
    data.append(hashSum);
    sendBinaryMessage(data);
}

void WebSocketHandler::sendAuthenticationResponse(bool state)
{
    QByteArray data;
    data.append(KEY_SET_AUTH_RESPONSE);
    data.append(arrayFromUint16(2));
    data.append(arrayFromUint16(static_cast<quint16>(state)));

    sendBinaryMessage(data);
}

void WebSocketHandler::sendRemoteAuthenticationResponse(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request)
{
    bool result = request.toBase64() == getHashSum(nonce, m_login, m_pass);

    if(!result)
        return;

    if(uuid.size() != SIZE_UUID ||
       nonce.size() != SIZE_UUID ||
       request.size() != SIZE_UUID)
        return;

    QByteArray data;
    data.append(KEY_CHECK_AUTH_RESPONSE);
    data.append(arrayFromUint16(static_cast<quint16>(SIZE_UUID + 2)));
    data.append(uuid);
    data.append(arrayFromUint16(static_cast<quint16>(result)));

    sendBinaryMessage(data);
}

QByteArray WebSocketHandler::getHashSum(const QByteArray &nonce, const QString &login, const QString &pass)
{
    QString sum = login + pass;
    QByteArray concatFirst = QCryptographicHash::hash(sum.toUtf8(),QCryptographicHash::Md5).toBase64();
    concatFirst.append(nonce.toBase64());
    QByteArray result = QCryptographicHash::hash(concatFirst,QCryptographicHash::Md5).toBase64();
    return result;
}

void WebSocketHandler::socketStateChanged(QAbstractSocket::SocketState state)
{
    switch(state)
    {
        case QAbstractSocket::UnconnectedState:
        {
            if(m_timerReconnect)
                if(!m_timerReconnect->isActive())
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
                if(!m_timerReconnect->isActive())
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

            m_isAuthenticated = false;

            if(m_timerReconnect)
                if(!m_timerReconnect->isActive())
                    m_timerReconnect->start(1000);

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

void WebSocketHandler::timerReconnectTick()
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

void WebSocketHandler::startWaitResponseTimer(int msec, int type)
{
    if(!m_timerWaitResponse)
    {
        m_timerWaitResponse = new QTimer(this);
        connect(m_timerWaitResponse, &QTimer::timeout, this, &WebSocketHandler::timerWaitResponseTick);
    }

    m_timerWaitResponse->start(msec);
    m_waitType = type;
}

void WebSocketHandler::stopWaitResponseTimer()
{
    if(m_timerWaitResponse)
        if(m_timerWaitResponse->isActive())
            m_timerWaitResponse->stop();

    m_waitType = WaitTypeUnknown;
}

void WebSocketHandler::timerWaitResponseTick()
{
    m_timerWaitResponse->stop();

    if(m_waitType == WaitTypeRemoteAuth)
        sendAuthenticationResponse(false);

    m_waitType = WaitTypeUnknown;
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
