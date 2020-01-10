#include "graberclass.h"

#include <QPixmap>
#include <QScreen>
#include <QApplication>
#include <QWindow>
#include <QDesktopWidget>
#include <QDebug>

GraberClass::GraberClass(QObject *parent) : QObject(parent),
    m_grabTimer(Q_NULLPTR),
    m_grabInterval(1000),
    m_rectSize(50),
    m_screenNumber(0)
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

    int dataSize = 0;

    for(int i=0;i<columnCount;++i)
    {
        for(int j=0;j<rowCount;++j)
        {
            QImage image = currentImage.copy(i*m_rectSize, j*m_rectSize, m_rectSize, m_rectSize);
            QImage lastImage = m_lastImage.copy(i*m_rectSize, j*m_rectSize, m_rectSize, m_rectSize);

            if(lastImage != image)
            {
                ++dataSize;
                emit imageTile(static_cast<quint16>(i),static_cast<quint16>(j),image);
            }
        }
    }

    m_lastImage = currentImage;
    calculateSendInterval(dataSize);
}

void GraberClass::calculateSendInterval(int dataSize)
{
    int vSize = m_meanCounter.size();
    int sum = dataSize;
    for(int i=0;i<vSize;++i)
        sum += m_meanCounter.at(i);

    sum = sum/(vSize+1);

    m_meanCounter[0] = dataSize;
    vSize = m_meanCounter.size()-1;
    for(int j=vSize;j>0;--j)
        m_meanCounter[j] = m_meanCounter.at(j-1);

    int interval = 40 + sum*4;

    if(interval > 1000)
        interval = 1000;

    m_grabTimer->setInterval(interval);
}

