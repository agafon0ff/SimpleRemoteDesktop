#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <QObject>
#include <QSize>
#include <QTimer>
#include <QMap>

class DataParser : public QObject
{
    Q_OBJECT
public:
    explicit DataParser(QObject *parent = nullptr);

private:

    struct SocketStruct
    {
        QByteArray uuid;
        QByteArray nonce;
        bool isAuthenticated = false;

        SocketStruct(const QByteArray &uuid, const QByteArray &nonce):
        uuid(uuid), nonce(nonce){}

        SocketStruct(){}
    };

    QString m_login;
    QString m_pass;
    QByteArray m_dataTmp;

    QMap<QByteArray,SocketStruct> m_socketsMap;

signals:
    void message(const QByteArray &data);
    void messageToSocket(const QByteArray &uuid, const QByteArray &data);
    void startGraber();
    void stopGraber();
    void receivedTileNum(quint16 num);
    void changeDisplayNum();
    void setKeyPressed(quint16 keyCode, bool state);
    void setMousePressed(quint16 keyCode, bool state);
    void setWheelChanged(bool deltaPos);
    void setMouseMove(quint16 posX, quint16 posY);
    void setMouseDelta(qint16 deltaX, qint16 deltaY);

public slots:
    void setLoginPass(const QString &login, const QString &pass);
    void setNewSocket(const QByteArray &uuid);
    void removeSocket(const QByteArray &uuid);

    void setData(const QByteArray &uuid, const QByteArray &data);

    void sendImageParameters(const QSize &imageSize, int rectWidth);
    void sendImageTile(quint16 posX, quint16 posY, const QByteArray &imageData, quint16 tileNum);

private slots:
    void newData(const QByteArray &uuid, const QByteArray &command, const QByteArray &data);
    void checkAuthentication(const QByteArray &uuid, const QByteArray &request);
    bool isSocketAuthenticated(const QByteArray &uuid);

    void debugHexData(const QByteArray &data);

public:
    static QByteArray arrayFromUint16(quint16 number);
    static quint16 uint16FromArray(const QByteArray &buf);
};

#endif // DATAPARSER_H
