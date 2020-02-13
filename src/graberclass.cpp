#include "graberclass.h"

#include <QPixmap>
#include <QScreen>
#include <QApplication>
#include <QWindow>
#include <QDesktopWidget>
#include <QDebug>

GraberClass::GraberClass(QObject *parent) : QObject(parent),
    m_grabTimer(Q_NULLPTR),
    m_grabInterval(40),
    m_rectSize(50),
    m_screenNumber(0),
    m_currentTileNum(0),
    m_receivedTileNum(0)
{
    m_meanCounter.resize(4);
    m_meanCounter.fill(1);
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

    if(screens.size() > m_screenNumber+1)
        ++m_screenNumber;
    else m_screenNumber = 0;

    QScreen* screen = screens.at(m_screenNumber);
    emit screenPositionChanged(QPoint(screen->geometry().x(),screen->geometry().y()));

    startSending();
}

void GraberClass::startSending()
{
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
    if(m_grabTimer)
        if(m_grabTimer->isActive())
            m_grabTimer->stop();
}

void GraberClass::updateImage()
{
    if(!isSendTilePermit())
        return;

    QScreen *screen = QApplication::screens().at(m_screenNumber);
    QImage currentImage = screen->grabWindow(0).toImage().convertToFormat(QImage::Format_RGB444);

    int columnCount = currentImage.width() / m_rectSize;
    int rowCount = currentImage.height() / m_rectSize;

    if(currentImage.width() % m_rectSize > 0)
        ++columnCount;

    if(currentImage.height() % m_rectSize > 0)
        ++rowCount;

    if(m_lastImage.isNull())
    {
        m_lastImage = QImage(currentImage.size(),currentImage.format());
        m_lastImage.fill(QColor(Qt::black));

        emit imageParameters(currentImage.size(), m_rectSize);
    }

    quint16 tileNum = 0;

    for(int i=0;i<columnCount;++i)
    {
        for(int j=0;j<rowCount;++j)
        {
            QImage image = currentImage.copy(i*m_rectSize, j*m_rectSize, m_rectSize, m_rectSize);
            QImage lastImage = m_lastImage.copy(i*m_rectSize, j*m_rectSize, m_rectSize, m_rectSize);

            if(lastImage != image)
            {
                emit imageTile(static_cast<quint16>(i),static_cast<quint16>(j),image,tileNum);

                m_currentTileNum = tileNum;
                ++tileNum;
            }
        }
    }

    m_lastImage = currentImage;
}

void GraberClass::setReceivedTileNum(quint16 num)
{
    m_permitCounter = 0;
    m_receivedTileNum = num;
}

bool GraberClass::isSendTilePermit()
{
    bool result = false;

    if(m_currentTileNum <= (m_receivedTileNum))
        result = true;

    if(!result)
    {
        ++m_permitCounter;

        if(m_permitCounter > 30)
            result = true;
    }

    return result;
}
