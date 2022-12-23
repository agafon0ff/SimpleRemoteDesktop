#include "websockethandler.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QUuid>

static const char* KEY_GET_IMAGE = "GIMG";
static const char* KEY_IMAGE_PARAM = "IMGP";
static const char* KEY_IMAGE_TILE = "IMGT";
static const char* KEY_SET_KEY_STATE = "SKST";
static const char* KEY_SET_CURSOR_POS = "SCUP";
static const char* KEY_SET_CURSOR_DELTA = "SCUD";
static const char* KEY_SET_MOUSE_KEY = "SMKS";
static const char* KEY_SET_MOUSE_WHEEL = "SMWH";
static const char* KEY_CHANGE_DISPLAY = "CHDP";
static const char* KEY_TILE_RECEIVED = "TLRD";
static const char* KEY_SET_NONCE = "STNC";
static const char* KEY_SET_AUTH_REQUEST = "SARQ";
static const char* KEY_SET_AUTH_RESPONSE = "SARP";
static const char* KEY_CHECK_AUTH_REQUEST = "CARQ";
static const char* KEY_CHECK_AUTH_RESPONSE = "CARP";
static const char* KEY_SET_NAME = "STNM";
static const char* KEY_CONNECT_UUID = "CTUU";
static const char* KEY_CONNECTED_PROXY_CLIENT = "CNPC";
static const char* KEY_DISCONNECTED_PROXY_CLIENT = "DNPC";
static const char* KEY_PING_REQUEST = "PINQ";
static const char* KEY_PING_RESPONSE = "PINS";

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
    m_command.resize(COMMAD_SIZE);
}

void WebSocketHandler::createSocket()
{
    if (m_webSocket)
        return;

    m_webSocket = new QWebSocket("",QWebSocketProtocol::VersionLatest,this);
    connect(m_webSocket, &QWebSocket::stateChanged, this, &WebSocketHandler::socketStateChanged);
    connect(m_webSocket, &QWebSocket::textMessageReceived,this, &WebSocketHandler::textMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived,this, &WebSocketHandler::binaryMessageReceived);

    if (!m_timerReconnect)
    {
        m_timerReconnect = new QTimer(this);
        connect(m_timerReconnect, &QTimer::timeout, this, &WebSocketHandler::timerReconnectTick);
    }

    m_webSocket->open(QUrl(m_url));
}

void WebSocketHandler::removeSocket()
{
    if (m_timerReconnect)
    {
        if (m_timerReconnect->isActive())
            m_timerReconnect->stop();

        m_timerReconnect->deleteLater();
        m_timerReconnect = Q_NULLPTR;
    }

    if (m_timerWaitResponse)
    {
        if (m_timerWaitResponse->isActive())
            m_timerWaitResponse->stop();

        m_timerWaitResponse->deleteLater();
        m_timerWaitResponse = Q_NULLPTR;
    }

    if (m_webSocket)
    {
        if (m_webSocket->state() != QAbstractSocket::UnconnectedState)
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
    m_name = name.toUtf8();
}

QString WebSocketHandler::getName()
{
    return QString::fromUtf8(m_name);
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
    if (!webSocket)
        return;

    m_webSocket = webSocket;
    connect(m_webSocket, &QWebSocket::textMessageReceived,this, &WebSocketHandler::textMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived,this, &WebSocketHandler::binaryMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected,this, &WebSocketHandler::socketDisconnected);

    m_uuid = QUuid::createUuid().toRfc4122();
    m_nonce = QUuid::createUuid().toRfc4122();

    qDebug()<<"WebSocketHandler::setSocket"<<m_type<<m_uuid.toBase64();

    m_dataToSend.clear();
    m_dataToSend.append(KEY_SET_NONCE);
    appendUint16(m_dataToSend, static_cast<quint16>(m_nonce.size()));
    m_dataToSend.append(m_nonce);

    sendBinaryMessage(m_dataToSend);
}

QWebSocket *WebSocketHandler::getSocket()
{
    return m_webSocket;
}

void WebSocketHandler::sendImageParameters(const QSize &imageSize, int rectWidth)
{
    if (!m_isAuthenticated)
        return;

    m_dataToSend.clear();
    m_dataToSend.append(KEY_IMAGE_PARAM);
    appendUint16(m_dataToSend, 6);
    appendUint16(m_dataToSend, static_cast<quint16>(imageSize.width()));
    appendUint16(m_dataToSend, static_cast<quint16>(imageSize.height()));
    appendUint16(m_dataToSend, static_cast<quint16>(rectWidth));

    sendBinaryMessage(m_dataToSend);
}

void WebSocketHandler::sendImageTile(quint16 posX, quint16 posY, const QByteArray &imageData, quint16 tileNum)
{
    if (!m_isAuthenticated)
        return;

    m_dataToSend.clear();
    m_dataToSend.append(KEY_IMAGE_TILE);
    appendUint16(m_dataToSend, static_cast<quint16>(imageData.size() + 6));
    appendUint16(m_dataToSend, posX);
    appendUint16(m_dataToSend, posY);
    appendUint16(m_dataToSend, tileNum);
    m_dataToSend.append(imageData);

    sendBinaryMessage(m_dataToSend);
}

void WebSocketHandler::sendName(const QByteArray &name)
{
    if (!m_isAuthenticated)
        return;

    m_dataToSend.clear();
    m_dataToSend.append(KEY_SET_NAME);
    appendUint16(m_dataToSend, static_cast<quint16>(name.size()));
    m_dataToSend.append(name);

    sendBinaryMessage(m_dataToSend);

    qDebug() << "WebSocketHandler::sendName" << name << m_dataToSend;
}

void WebSocketHandler::checkRemoteAuthentication(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request)
{
    if (!m_isAuthenticated)
        return;

    qDebug()<<"WebSocketHandler::checkRemoteAuthentication"<<m_type<<uuid.toBase64();

    if (uuid.size() != SIZE_UUID ||
       nonce.size() != SIZE_UUID ||
       request.size() != SIZE_UUID)
        return;

    m_dataToSend.clear();
    m_dataToSend.append(KEY_CHECK_AUTH_REQUEST);
    appendUint16(m_dataToSend, static_cast<quint16>(SIZE_UUID * 3));
    m_dataToSend.append(uuid);
    m_dataToSend.append(nonce);
    m_dataToSend.append(request);

    sendBinaryMessage(m_dataToSend);
}

void WebSocketHandler::setRemoteAuthenticationResponse(const QByteArray &uuid, const QByteArray &name)
{
    qDebug() << "WebSocketHandler::setRemoteAuthenticationResponse" << m_type << uuid.toBase64() << name;

    stopWaitResponseTimer();

    m_dataToSend.clear();
    m_dataToSend.append(KEY_CHECK_AUTH_RESPONSE);
    appendUint16(m_dataToSend, static_cast<quint16>(SIZE_UUID + name.size()));
    m_dataToSend.append(uuid);
    m_dataToSend.append(name);

    sendBinaryMessage(m_dataToSend);
}

void WebSocketHandler::createProxyConnection(WebSocketHandler *handler, const QByteArray &uuid)
{
    disconnect(handler->getSocket(), &QWebSocket::binaryMessageReceived,handler, &WebSocketHandler::binaryMessageReceived);

    connect(m_webSocket, &QWebSocket::binaryMessageReceived, handler->getSocket(), &QWebSocket::sendBinaryMessage);
    connect(handler->getSocket(), &QWebSocket::binaryMessageReceived, m_webSocket, &QWebSocket::sendBinaryMessage);
    connect(handler, &WebSocketHandler::disconnectedUuid, this, &WebSocketHandler::proxyHandlerDisconnected);

    connect(this, &WebSocketHandler::proxyConnectionCreated, handler, &WebSocketHandler::sendAuthenticationResponse);
    emit proxyConnectionCreated(true);

    m_dataToSend.clear();
    m_dataToSend.append(KEY_CONNECTED_PROXY_CLIENT);
    appendUint16(m_dataToSend, static_cast<quint16>(uuid.size()));
    m_dataToSend.append(uuid);
    sendBinaryMessage(m_dataToSend);
}

void WebSocketHandler::proxyHandlerDisconnected(const QByteArray &uuid)
{
    m_dataToSend.clear();
    m_dataToSend.append(KEY_DISCONNECTED_PROXY_CLIENT);
    appendUint16(m_dataToSend, static_cast<quint16>(uuid.size()));
    m_dataToSend.append(uuid);
    sendBinaryMessage(m_dataToSend);
}

void WebSocketHandler::newData(const QByteArray &command, const QByteArray &data)
{
//    qDebug() << "DataParser::newData" << command << data;

    if (!m_isAuthenticated)
    {
        if (command == KEY_SET_AUTH_REQUEST)
        {
            if (m_type == HandlerWebClient || m_type == HandlerDesktop)
            {
                if (data.toBase64() == getHashSum(m_nonce, m_login, m_pass))
                {
                    if (!m_isAuthenticated)
                        m_isAuthenticated = true;
                }
                else m_isAuthenticated = false;

                sendAuthenticationResponse(m_isAuthenticated);
                qDebug()<<"Authentication attempt: "<<m_isAuthenticated;
            }
            else if (m_type == HandlerProxyClient)
            {
                emit remoteAuthenticationRequest(m_uuid, m_nonce, data);
                startWaitResponseTimer(5000,WaitTypeRemoteAuth);
            }

            return;
        }
        else if (command == KEY_SET_NONCE)
        {
            if (m_type == HandlerSingleClient)
            {
                m_nonce = data;
                sendAuthenticationRequestToProxy();
            }

            return;
        }
        else if (command == KEY_SET_AUTH_RESPONSE)
        {
            bool authState = uint16FromArray(data.data());

            if (m_type == HandlerSingleClient && authState)
            {
                m_isAuthenticated = true;
                sendName(m_name);
            }
            else
            {
                m_isAuthenticated = false;
            }

            emit authenticatedStatus(m_isAuthenticated);

            return;
        }
        else if (command == KEY_CONNECT_UUID)
        {
            emit newProxyConnection(this, m_uuid, data);
        }
        else
        {
            qDebug() << "DataParser::newData" << command << data;
            debugHexData(data);
        }

        return;
    }

    if (command == KEY_IMAGE_TILE)
    {
        return;
    }
    else if (command == KEY_GET_IMAGE)
    {
        sendName(m_name);
        emit getDesktop();
    }
    else if (command == KEY_TILE_RECEIVED)
    {
        quint16 tileNum = uint16FromArray(data.data());
        emit receivedTileNum(tileNum);
    }
    else if (command == KEY_PING_REQUEST)
    {
        sendBinaryMessage(QByteArray(KEY_PING_RESPONSE));
    }
    else if (command == KEY_CHANGE_DISPLAY)
    {
        emit changeDisplayNum();
    }
    else if (command == KEY_SET_CURSOR_POS)
    {
        if (data.size() >= 4)
        {
            quint16 posX = uint16FromArray(data.data());
            quint16 posY = uint16FromArray(data.data() + 2);
            emit setMouseMove(posX, posY);
        }
    }
    else if (command == KEY_SET_CURSOR_DELTA)
    {
        if (data.size() >= 4)
        {
            qint16 deltaX = static_cast<qint16>(uint16FromArray(data.data()));
            qint16 deltaY = static_cast<qint16>(uint16FromArray(data.data() + 2));
            emit setMouseDelta(deltaX, deltaY);
        }
    }
    else if (command == KEY_SET_KEY_STATE)
    {
        if (data.size() >= 4)
        {
            quint16 keyCode = uint16FromArray(data.data());
            quint16 keyState = uint16FromArray(data.data() + 2);
            emit setKeyPressed(keyCode,static_cast<bool>(keyState));
        }
    }
    else if (command == KEY_SET_MOUSE_KEY)
    {
        if (data.size() >= 4)
        {
            quint16 keyCode = uint16FromArray(data.data());
            quint16 keyState = uint16FromArray(data.data() + 2);
            emit setMousePressed(keyCode,static_cast<bool>(keyState));
        }
    }
    else if (command == KEY_SET_MOUSE_WHEEL)
    {
        if (data.size() >= 4)
        {
            quint16 keyState = uint16FromArray(data.data() + 2);
            emit setWheelChanged(static_cast<bool>(keyState));
        }
    }
    else if (command == KEY_SET_NAME)
    {
        m_name = data;
        qDebug() << "New desktop connected:" << m_name;
    }
    else if (command == KEY_CHECK_AUTH_REQUEST)
    {
        if (data.size() == SIZE_UUID * 3)
        {
            QByteArray uuid = data.mid(0, SIZE_UUID);
            QByteArray nonce = data.mid(SIZE_UUID, SIZE_UUID);
            QByteArray requset = data.mid(SIZE_UUID * 2, SIZE_UUID);

            sendRemoteAuthenticationResponse(uuid, nonce, requset);
        }
    }
    else if (command == KEY_CHECK_AUTH_RESPONSE)
    {
        if (data.size() == SIZE_UUID + 2)
        {
            QByteArray uuid = data.mid(0, SIZE_UUID);
            quint16 authResponse = uint16FromArray(data.data() + SIZE_UUID);
            emit remoteAuthenticationResponse(uuid, m_uuid, m_name, static_cast<bool>(authResponse));
        }
    }
    else if (command == KEY_CONNECTED_PROXY_CLIENT)
    {
        emit connectedProxyClient(data);
    }
    else if (command == KEY_DISCONNECTED_PROXY_CLIENT)
    {
        emit disconnectedProxyClient(data);
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
    m_dataToSend.clear();
    m_dataToSend.append(KEY_SET_AUTH_REQUEST);
    appendUint16(m_dataToSend, static_cast<quint16>(hashSum.size()));
    m_dataToSend.append(hashSum);
    sendBinaryMessage(m_dataToSend);
}

void WebSocketHandler::sendAuthenticationResponse(bool state)
{
    m_dataToSend.clear();
    m_dataToSend.append(KEY_SET_AUTH_RESPONSE);
    appendUint16(m_dataToSend, 2);
    appendUint16(m_dataToSend, static_cast<quint16>(state));

    sendBinaryMessage(m_dataToSend);

    WebSocketHandler *handler = static_cast<WebSocketHandler*>(sender());
    disconnect(handler, &WebSocketHandler::proxyConnectionCreated, this, &WebSocketHandler::sendAuthenticationResponse);
}

void WebSocketHandler::sendRemoteAuthenticationResponse(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request)
{
    bool result = request.toBase64() == getHashSum(nonce, m_login, m_pass);

    qDebug() << "WebSocketHandler::sendRemoteAuthenticationResponse" << m_type << uuid.toBase64();

    if (!result)
        return;

    if (uuid.size() != SIZE_UUID ||
       nonce.size() != SIZE_UUID ||
       request.size() != SIZE_UUID)
        return;

    m_dataToSend.clear();
    m_dataToSend.append(KEY_CHECK_AUTH_RESPONSE);
    appendUint16(m_dataToSend, static_cast<quint16>(SIZE_UUID + 2));
    m_dataToSend.append(uuid);
    appendUint16(m_dataToSend, static_cast<quint16>(result));

    sendBinaryMessage(m_dataToSend);
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
            if (m_timerReconnect)
                if (!m_timerReconnect->isActive())
                    m_timerReconnect->start(5000);
            break;
        }
        case QAbstractSocket::HostLookupState:
        {
            break;
        }
        case QAbstractSocket::ConnectingState:
        {
            if (m_timerReconnect)
                if (!m_timerReconnect->isActive())
                    m_timerReconnect->start(5000);
            break;
        }
        case QAbstractSocket::ConnectedState:
        {
            qDebug()<<"SocketWeb::socketStateChanged: Connected to server.";
            emit connectedStatus(true);
            break;
        }
        case QAbstractSocket::ClosingState:
        {
            qDebug()<<"SocketWeb::socketStateChanged: Disconnected from server.";
            emit connectedStatus(false);
            emit authenticatedStatus(false);
            m_isAuthenticated = false;

            if (m_timerReconnect)
                if (!m_timerReconnect->isActive())
                    m_timerReconnect->start(5000);

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
    emit disconnectedUuid(m_uuid);
    emit disconnected(this);
}

void WebSocketHandler::sendBinaryMessage(const QByteArray &data)
{
    if (m_webSocket)
        if (m_webSocket->state() == QAbstractSocket::ConnectedState)
            m_webSocket->sendBinaryMessage(data);
}

void WebSocketHandler::textMessageReceived(const QString &message)
{
    qDebug() << "WebSocketHandler::textMessageReceived:" << message;
}

void WebSocketHandler::binaryMessageReceived(const QByteArray &data)
{
    m_dataReceived = m_dataTmp;
    m_dataReceived.append(data);
    m_dataTmp.clear();
    m_payload.clear();

    int size = m_dataReceived.size();

    if (size == COMMAD_SIZE)
    {
        newData(data, m_payload);
        return;
    }

    if (size < REQUEST_MIN_SIZE)
        return;

    int dataStep = 0;

    for(int i=0;i<size;++i)
    {
        m_command.setRawData(m_dataReceived.data(), COMMAD_SIZE);
        quint16 dataSize = uint16FromArray(m_dataReceived.data() + COMMAD_SIZE);

        if (size >= (dataStep + COMMAD_SIZE + dataSize))
        {
            m_payload.resize(dataSize);
            m_payload.setRawData(m_dataReceived.data() + dataStep + COMMAD_SIZE + 2, dataSize);
            dataStep += COMMAD_SIZE + 2 + dataSize;
            newData(m_command, m_payload);

            i = dataStep;
        }
        else
        {
            debugHexData(m_dataReceived);

            if (size - dataStep < 2000) {
                m_dataTmp.resize(size - dataStep);
                m_dataTmp.setRawData(m_dataReceived.data() + dataStep, m_dataTmp.size());
            }

            break;
        }
    }
}

void WebSocketHandler::timerReconnectTick()
{
    if (m_webSocket->state() == QAbstractSocket::ConnectedState)
    {
        m_timerReconnect->stop();
        return;
    }

    if (m_webSocket->state() == QAbstractSocket::ConnectingState)
        m_webSocket->abort();

    m_webSocket->open(QUrl(m_url));
}

void WebSocketHandler::startWaitResponseTimer(int msec, int type)
{
    if (!m_timerWaitResponse)
    {
        m_timerWaitResponse = new QTimer(this);
        connect(m_timerWaitResponse, &QTimer::timeout, this, &WebSocketHandler::timerWaitResponseTick);
    }

    m_timerWaitResponse->start(msec);
    m_waitType = type;
}

void WebSocketHandler::stopWaitResponseTimer()
{
    if (m_timerWaitResponse)
        if (m_timerWaitResponse->isActive())
            m_timerWaitResponse->stop();

    m_waitType = WaitTypeUnknown;
}

void WebSocketHandler::timerWaitResponseTick()
{
    m_timerWaitResponse->stop();

    if (m_waitType == WaitTypeRemoteAuth)
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

        if (i < dataSize-1)
            textHex.append("|");
    }

    qDebug() << "DataParser::debugHexData:"<< textHex << data;
}

void WebSocketHandler::appendUint16(QByteArray &data, quint16 number)
{
    int size = data.size();
    data.resize(size + 2);
    data[size] = static_cast<char>(number);
    data[size + 1] = static_cast<char>(number >> 8);
}

quint16 WebSocketHandler::uint16FromArray(const char* buf)
{
    quint16 m_number;
    m_number = static_cast<quint16>(static_cast<quint8>(buf[0]) |
            static_cast<quint8>(buf[1]) << 8);
    return m_number;
}
