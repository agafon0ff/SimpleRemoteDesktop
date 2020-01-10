#ifndef INPUTSIMULATOR_H
#define INPUTSIMULATOR_H

#include <QObject>
#include <QMap>
#include <QPoint>

class InputSimulator : public QObject
{
    Q_OBJECT
public:
    explicit InputSimulator(QObject *parent = nullptr);

private:

    QMap<quint16,quint16> m_keysMap;
    QPoint m_screenPosition;

signals:

public slots:
    void simulateKeyboard(quint16 keyCode, bool state);
    void simulateMouseKeys(quint16 keyCode, bool state);
    void simulateMouseMove(quint16 posX, quint16 posY);
    void simulateWheelEvent(bool deltaPos);
    void setMouseDelta(qint16 deltaX, qint16 deltaY);
    void setScreenPosition(const QPoint &pos){m_screenPosition = pos;}

private slots:
    void createKeysMap();
};

#endif // INPUTSIMULATOR_H
