#ifndef WEBSOCKETTRANSFER_H
#define WEBSOCKETTRANSFER_H

#include <QObject>
#include <QDebug>
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebSockets/qwebsocket.h>
#include "websockethandler.h"

class WebSocketTransfer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketTransfer(QObject *parent = Q_NULLPTR);

    enum TransferType
    {
        TransferDesktops,
        TransferWebClients,
        TransferProxyClients
    };

private:
    QWebSocketServer *m_webSocketServer;
    int m_type;
    quint16 m_port;
    QString m_login;
    QString m_pass;

    QList<WebSocketHandler*> m_sockets;

signals:
    void finished();
    void newSocketConnected(WebSocketHandler *webSocket);
    void disconnectedAll();

public slots:
    void start();
    void stop();
    void setPort(quint16 port);
    void setLoginPass(const QString &login, const QString &pass);
    void setType(int type);

private slots:
    void setSocketConnected();
    void socketDisconnected(WebSocketHandler *pointer);
};

#endif // WEBSOCKETTRANSFER_H
