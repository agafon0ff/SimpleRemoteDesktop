#include "graberclass.h"

#include <QPixmap>
#include <QScreen>
#include <QApplication>
#include <QWindow>
#include <QDesktopWidget>
#include <QDebug>
#include <QBuffer>

GraberClass::GraberClass(QObject *parent) : QObject(parent),
    m_grabTimer(Q_NULLPTR),
    m_grabInterval(40),
    m_rectSize(60),
    m_screenNumber(0),
    m_currentTileNum(0),
    m_receivedTileNum(0),
    m_permitCounter(0),
    m_tileCurrentImage(m_rectSize, m_rectSize, QImage::Format_RGB444)
{
}

void GraberClass::start()
{
    if(!m_grabTimer)
    {
        m_grabTimer = new QTimer(this);
        connect(m_grabTimer,SIGNAL(timeout()),this,SLOT(updateImage()));
    }
}

void GraberClass::stop()
{
    stopSending();
    emit finished();
}

void GraberClass::changeScreenNum()
{
    QList<QScreen *> screens = QApplication::screens();

    if(screens.size() > m_screenNumber + 1)
        ++m_screenNumber;
    else m_screenNumber = 0;

    QScreen* screen = screens.at(m_screenNumber);
    emit screenPositionChanged(QPoint(screen->geometry().x(),screen->geometry().y()));

    startSending();
}

void GraberClass::startSending()
{
    qDebug()<<"GraberClass::startSending";

    if(m_grabTimer)
        if(!m_grabTimer->isActive())
            m_grabTimer->start(m_grabInterval);

    m_permitCounter = 0;
    m_receivedTileNum = 0;
    m_currentTileNum = 0;

    m_lastImage = QImage();
    updateImage();
}

void GraberClass::stopSending()
{
    qDebug()<<"GraberClass::stopSending";

    if(m_grabTimer)
        if(m_grabTimer->isActive())
            m_grabTimer->stop();
}

void GraberClass::updateImage()
{
    if(!isSendTilePermit())
        return;

    QScreen *screen = QApplication::screens().at(m_screenNumber);
    m_currentImage = screen->grabWindow(0).toImage().convertToFormat(QImage::Format_RGB444);

    int columnCount = m_currentImage.width() / m_rectSize;
    int rowCount = m_currentImage.height() / m_rectSize;

    if(m_currentImage.width() % m_rectSize > 0)
        ++columnCount;

    if(m_currentImage.height() % m_rectSize > 0)
        ++rowCount;

    bool sendWithoutCompare = false;

    if(m_lastImage.isNull())
    {
        m_lastImage = QImage(m_currentImage.size(), m_currentImage.format());
        m_lastImage.fill(QColor(Qt::black));

        emit imageParameters(m_currentImage.size(), m_rectSize);
        sendWithoutCompare = true;
    }

    QRect tileRect;
    quint16 tileNum = 0;
    m_currentTileNum = 0;

    for (int j=0; j<rowCount; ++j)
    {
        for (int i=0; i<columnCount; ++i)
        {
            tileRect.setRect(i * m_rectSize, j * m_rectSize, m_rectSize, m_rectSize);
            if(sendWithoutCompare || !compareImagesRects(m_currentImage, m_lastImage, tileRect))
            {
                sendImage(i, j, tileNum, m_currentImage.copy(tileRect));
                m_currentTileNum = tileNum;
                ++tileNum;
                sendWithoutCompare = tileNum > columnCount;
            }
        }
    }

    m_lastImage = m_currentImage;
}

void GraberClass::setReceivedTileNum(quint16 num)
{
    m_permitCounter = 0;
    m_receivedTileNum = num;
}

void GraberClass::sendImage(int posX, int posY, int tileNum, const QImage &image)
{
    QBuffer buffer(&m_dataToSend);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    emit imageTile(static_cast<quint16>(posX), static_cast<quint16>(posY), m_dataToSend, static_cast<quint16>(tileNum));
}

bool GraberClass::isSendTilePermit()
{
    if (m_receivedTileNum >= m_currentTileNum)
        return true;

    if (++m_permitCounter > 20)
    {
        m_permitCounter = 0;
        return true;
    }

    return false;
}

bool GraberClass::compareImagesRects(const QImage &img1, const QImage &img2, const QRect &r)
{
    if (img1.depth() != img2.depth())
        return false;

    if (img1.width() != img2.width())
        return false;

    int depth = img1.depth() / 8;
    int bytesPerLine = img1.width() * depth;
    int lineBegin = r.x() * depth;
    int size = r.width() * depth;

    for (int i=r.y(); i<(r.y() + r.height()); ++i) {
        int shift = bytesPerLine * i + lineBegin;
        if (memcmp(img1.constBits() + shift, img2.constBits() + shift, size) != 0) {
            return false;
        }
    }

    return true;
}
