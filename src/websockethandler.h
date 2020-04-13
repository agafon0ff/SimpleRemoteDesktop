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

    enum WaitResponseType
    {
        WaitTypeUnknown,
        WaitTypeRemoteAuth
    };

private:
    QWebSocket *m_webSocket;
    QTimer *m_timerReconnect;
    QTimer *m_timerWaitResponse;
    int m_type;
    int m_waitType;
    bool m_isAuthenticated;
    QString m_url;
    QString m_name;

    QString m_login;
    QString m_pass;

    QString m_proxyLogin;
    QString m_proxyPass;

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
    void remoteAuthenticationRequest(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request);
    void remoteAuthenticationResponse(const QByteArray &uuidDst, const QByteArray &uuidSrc,
                                      const QByteArray &name, bool state);
    void newProxyConnection(WebSocketHandler *handler, const QByteArray &uuid);
    void proxyConnectionCreated(bool state);

public slots:
    void createSocket();
    void removeSocket();
    void setUrl(const QString &url);
    void setType(int type);

    void setName(const QString &name);
    QString getName();

    QByteArray getUuid();

    void setLoginPass(const QString &login, const QString &pass);
    void setProxyLoginPass(const QString &login, const QString &pass);

    void setSocket(QWebSocket *webSocket);
    QWebSocket *getSocket();


    void sendImageParameters(const QSize &imageSize, int rectWidth);
    void sendImageTile(quint16 posX, quint16 posY,
                       const QByteArray &imageData, quint16 tileNum);
    void sendName(const QString &name);
    void checkRemoteAuthentication(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request);
    void setRemoteAuthenticationResponse(const QByteArray &uuid, const QByteArray &name);

    void createProxyConnection(WebSocketHandler *handler);
    void createNormalConnection();

private slots:
    void newData(const QByteArray &command, const QByteArray &data);
    void sendAuthenticationRequestToProxy();
    void sendAuthenticationResponse(bool state);
    void sendRemoteAuthenticationResponse(const QByteArray &uuid, const QByteArray &nonce, const QByteArray &request);
    QByteArray getHashSum(const QByteArray &nonce, const QString &login, const QString &pass);
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketDisconnected();
    void sendBinaryMessage(const QByteArray &data);
    void textMessageReceived(const QString &message);
    void binaryMessageReceived(const QByteArray &data);
    void timerReconnectTick();

    void startWaitResponseTimer(int msec, int type);
    void stopWaitResponseTimer();
    void timerWaitResponseTick();

    void debugHexData(const QByteArray &data);

public:
    static QByteArray arrayFromUint16(quint16 number);
    static quint16 uint16FromArray(const QByteArray &buf);
};

#endif // WEBSOCKETHANDLER_H
