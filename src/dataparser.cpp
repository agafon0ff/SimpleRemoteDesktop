#include "dataparser.h"
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

DataParser::DataParser(QObject *parent) : QObject(parent)
{

}

void DataParser::setLoginPass(const QString &login, const QString &pass)
{
    m_login = login;
    m_pass = pass;
}

void DataParser::setNewSocket(const QByteArray &uuid)
{
    SocketStruct sStruct;
    sStruct.uuid = uuid;
    sStruct.nonce = QUuid::createUuid().toRfc4122();
    m_socketsMap.insert(uuid, sStruct);

    QByteArray data;
    data.append(KEY_SET_NONCE);
    data.append(arrayFromUint16(static_cast<quint16>(sStruct.nonce.size())));
    data.append(sStruct.nonce);

    emit messageToSocket(uuid, data);
}

void DataParser::removeSocket(const QByteArray &uuid)
{
    m_socketsMap.remove(uuid);

    bool isStopGraber = true;
    foreach(SocketStruct socket, m_socketsMap.values())
    {
        if(socket.isAuthenticated)
        {
            isStopGraber = false;
            break;
        }
    }

    if(isStopGraber)
        emit stopGraber();
}

void DataParser::setData(const QByteArray &uuid, const QByteArray &data)
{
    Q_UNUSED(uuid);

    QByteArray activeBuf = m_dataTmp;
    activeBuf.append(data);
    m_dataTmp.clear();

    int size = activeBuf.size();

    if(size == COMMAD_SIZE)
    {
        newData(uuid, data, QByteArray());
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
            newData(uuid, command, payload);

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

void DataParser::sendImageParameters(const QSize &imageSize, int rectWidth)
{
    QByteArray data;
    data.append(KEY_IMAGE_PARAM);
    data.append(arrayFromUint16(6));
    data.append(arrayFromUint16(static_cast<quint16>(imageSize.width())));
    data.append(arrayFromUint16(static_cast<quint16>(imageSize.height())));
    data.append(arrayFromUint16(static_cast<quint16>(rectWidth)));

    foreach(SocketStruct sStruct, m_socketsMap.values())
        if(sStruct.isAuthenticated)
            emit messageToSocket(sStruct.uuid,data);
}

void DataParser::sendImageTile(quint16 posX, quint16 posY, const QByteArray &imageData, quint16 tileNum)
{
    QByteArray data;
    data.append(KEY_IMAGE_TILE);
    data.append(arrayFromUint16(static_cast<quint16>(imageData.size() + 6)));
    data.append(arrayFromUint16(posX));
    data.append(arrayFromUint16(posY));
    data.append(arrayFromUint16(tileNum));
    data.append(imageData);

    foreach(SocketStruct sStruct, m_socketsMap.values())
        if(sStruct.isAuthenticated)
            emit messageToSocket(sStruct.uuid,data);
}

void DataParser::newData(const QByteArray &uuid, const QByteArray &command, const QByteArray &data)
{
//    qDebug()<<"DataParser::newData"<<command<<data;

    if(command == KEY_SET_AUTH_REQUEST)
    {
        checkAuthentication(uuid,data);
        return;
    }
    else
    {
        if(!isSocketAuthenticated(uuid))
            return;
    }

    if(command == KEY_GET_IMAGE)
    {
        emit startGraber();
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

void DataParser::checkAuthentication(const QByteArray &uuid, const QByteArray &request)
{
    if(!m_socketsMap.contains(uuid))
        return;

    SocketStruct sStruct = m_socketsMap.value(uuid);
    QString sum = m_login + m_pass;
    QByteArray concatFirst = QCryptographicHash::hash(sum.toUtf8(),QCryptographicHash::Md5).toBase64();
    concatFirst.append(sStruct.nonce.toBase64());
    QByteArray concatSecond = QCryptographicHash::hash(concatFirst,QCryptographicHash::Md5).toBase64();
    bool isAuthenticated = false;

    if(request.toBase64() == concatSecond)
    {
        if(!sStruct.isAuthenticated)
            m_socketsMap[uuid].isAuthenticated = true;

        isAuthenticated = true;
    }

    QByteArray data;
    data.append(KEY_SET_AUTH_RESPONSE);
    data.append(arrayFromUint16(2));

    if(isAuthenticated)
        data.append(arrayFromUint16(1));
    else data.append(arrayFromUint16(0));

    emit messageToSocket(uuid, data);
}

bool DataParser::isSocketAuthenticated(const QByteArray &uuid)
{
    bool result = false;

    if(m_socketsMap.contains(uuid))
        result = m_socketsMap.value(uuid).isAuthenticated;

    return result;
}

void DataParser::debugHexData(const QByteArray &data)
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

QByteArray DataParser::arrayFromUint16(quint16 number)
{
    QByteArray buf;
    buf.resize(2);
    buf[0] = static_cast<char>(number);
    buf[1] = static_cast<char>(number >> 8);
    return buf;
}

quint16 DataParser::uint16FromArray(const QByteArray &buf)
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
