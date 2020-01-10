#ifndef GRABERCLASS_H
#define GRABERCLASS_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QList>

class GraberClass : public QObject
{
    Q_OBJECT
public:
    explicit GraberClass(QObject *parent = Q_NULLPTR);

private:

    struct TileStruct
    {
        int x;
        int y;
        QImage image;

        TileStruct(int posX, int posY, const QImage &image) :
        x(posX), y(posY), image(image){}

        TileStruct(){}
    };

    QTimer *m_grabTimer;
    int m_grabInterval;
    int m_rectSize;
    int m_screenNumber;
    QImage m_lastImage;

    QVector<int> m_meanCounter;

signals:
    void finished();
    void imageParameters(const QSize &imageSize, int rectWidth);
    void imageTile(quint16 posX, quint16 posY, const QImage &image);
    void lastTileSended();
    void screenPositionChanged(const QPoint &pos);

public slots:
    void start();
    void stop();
    void setInterval(int msec){m_grabInterval = msec;}
    void setRectSize(int size){m_rectSize = size;}
    void changeScreenNum();

    void startSending();
    void stopSending();
    void updateImage();

private slots:
    void calculateSendInterval(int dataSize);
};

#endif // GRABERCLASS_H
