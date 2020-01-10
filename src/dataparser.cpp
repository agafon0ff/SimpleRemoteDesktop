#include "dataparser.h"
#include <QBuffer>
#include <QtDebug>

static const QByteArray KEY_GET_IMAGE = "GIMG";
static const QByteArray KEY_IMAGE_PARAM = "IMGP";
static const QByteArray KEY_IMAGE_TILE = "IMGT";
static const QByteArray KEY_GET_NEXT_TILE = "GNXT";
static const QByteArray KEY_SET_LAST_TILE = "SLST";
static const QByteArray KEY_SET_KEY_STATE = "SKST";
static const QByteArray KEY_SET_CURSOR_POS = "SCUP";
static const QByteArray KEY_SET_CURSOR_DELTA = "SCUD";
static const QByteArray KEY_SET_MOUSE_KEY = "SMKS";
static const QByteArray KEY_SET_MOUSE_WHEEL = "SMWH";
static const QByteArray KEY_CHANGE_DISPLAY = "CHDP";

const int COMMAD_SIZE = 4;
const int REQUEST_MIN_SIZE = 6;

DataParser::DataParser(QObject *parent) : QObject(parent),
    m_timerClearTmp(new QTimer(this))
{
    connect(m_timerClearTmp,SIGNAL(timeout()),this,SLOT(timerClearTmpTick()));
}

void DataParser::setData(const QByteArray &data)
{
    QByteArray activeBuf = m_dataTmp;
    activeBuf.append(data);
    m_dataTmp.clear();

    int size = activeBuf.size();

    if(size == COMMAD_SIZE)
    {
        newData(data,QByteArray());
        return;
    }

    if(size < REQUEST_MIN_SIZE)
        return;

    int dataStep = 0;

    for(int i=0;i<size;++i)
    {
        QByteArray command = activeBuf.mid(dataStep, COMMAD_SIZE);
        quint16 dataSize = uint16FromArray(activeBuf.mid(dataStep + COMMAD_SIZE, 2));

        if(size >= (dataStep + COMMAD_SIZE + 2 + dataSize))
        {
            QByteArray payload = activeBuf.mid(dataStep + COMMAD_SIZE + 2, dataSize);
            dataStep += COMMAD_SIZE + 2 + dataSize;

            newData(command,payload);

            i = dataStep;
        }
        else
        {
            debugHexData(activeBuf);

            m_dataTmp = activeBuf.mid(dataStep, size - dataStep);

            if(m_dataTmp.size() > 300)
                m_dataTmp.clear();

            m_timerClearTmp->start(500);
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

    emit messgage(data);
}

void DataParser::sendImageTile(quint16 posX, quint16 posY, const QImage &image)
{
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    QByteArray data;
    data.append(KEY_IMAGE_TILE);
    data.append(arrayFromUint16(static_cast<quint16>(bArray.size() + 4)));
    data.append(arrayFromUint16(posX));
    data.append(arrayFromUint16(posY));
    data.append(bArray);

    emit messgage(data);
}

void DataParser::sendLastTile()
{
    QByteArray data;
    data.append(KEY_SET_LAST_TILE);
    data.append(arrayFromUint16(0));

    emit messgage(data);
}

void DataParser::newData(const QByteArray &command, const QByteArray &data)
{
//    qDebug()<<"DataParser::newData"<<command<<data;

    if(command == KEY_GET_IMAGE)
    {
        emit startGraber();
    }
    else if(command == KEY_GET_NEXT_TILE)
    {
        emit getNextTile();
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
        if(data.size() >= 3)
        {
            quint16 keyCode = uint16FromArray(data.mid(0,2));
            bool state = static_cast<bool>(data.at(2));
            emit setKeyPressed(keyCode,state);
        }
    }
    else if(command == KEY_SET_MOUSE_KEY)
    {
        if(data.size() >= 3)
        {
            quint16 keyCode = uint16FromArray(data.mid(0,2));
            bool state = static_cast<bool>(data.at(2));
            emit setMousePressed(keyCode,state);
        }
    }
    else if(command == KEY_SET_MOUSE_WHEEL)
    {
        if(data.size() >= 3)
        {
            bool state = static_cast<bool>(data.at(2));
            emit setWheelChanged(state);
        }
    }
    else
    {
        qDebug()<<"DataParser::newData"<<command<<data;
        debugHexData(data);
    }
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

void DataParser::timerClearTmpTick()
{
    m_dataTmp.clear();
    m_timerClearTmp->stop();
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
