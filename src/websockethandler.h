#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QtWebSockets/qwebsocket.h>
#include <QSize>
#include <QMap>

class WebSocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketHandler(QObject *parent = Q_NULLPTR);

    enum HandlerType
    {
        HandlerWebClient,
        HandlerProxyClient,
        HandlerDesktop,
        HandlerSingleClient
    };

private:
    QWebSocket *m_webSocket;
    QTimer *m_timerReconnect;
    int m_type;
    bool m_isAuthenticated;
    QString m_url;
    QString m_name;
    QString m_login;
    QString m_pass;
    QByteArray m_dataTmp;
    QByteArray m_uuid;
    QByteArray m_nonce;

signals:
    void finished();
    void getDesktop();
    void receivedTileNum(quint16 num);
    void changeDisplayNum();
    void setKeyPressed(quint16 keyCode, bool state);
    void setMousePressed(quint16 keyCode, bool state);
    void setWheelChanged(bool deltaPos);
    void setMouseMove(quint16 posX, quint16 posY);
    void setMouseDelta(qint16 deltaX, qint16 deltaY);
    void disconnected(WebSocketHandler *pointer);
    void findAuthentication(const QByteArray &nonce, const QByteArray &request);

public slots:
    void createSocket();
    void removeSocket();
    void setUrl(const QString &url);
    void setType(int type);

    void setName(const QString &name);
    QString getName();

    void setLoginPass(const QString &login, const QString &pass);
    void setSocket(QWebSocket *webSocket);
    void sendImageParameters(const QSize &imageSize, int rectWidth);
    void sendImageTile(quint16 posX, quint16 posY,
                       const QByteArray &imageData, quint16 tileNum);
    void sendName(const QString &name);

private slots:
    void newData(const QByteArray &command, const QByteArray &data);
    void checkAuthentication(const QByteArray &request);
    void sendAuthenticationRequest();
    QByteArray getHashSum();
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketDisconnected();
    void sendBinaryMessage(const QByteArray &data);
    void textMessageReceived(const QString &message);
    void binaryMessageReceived(const QByteArray &data);
    void reconnectTimerTick();
    void debugHexData(const QByteArray &data);

public:
    static QByteArray arrayFromUint16(quint16 number);
    static quint16 uint16FromArray(const QByteArray &buf);
};

#endif // WEBSOCKETHANDLER_H
