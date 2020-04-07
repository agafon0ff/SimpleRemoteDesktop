#ifndef PROXYUNITINGCLASS_H
#define PROXYUNITINGCLASS_H

#include <QObject>
#include "serverhttp.h"
#include "websockettransfer.h"

class ProxyUnitingClass : public QObject
{
    Q_OBJECT
public:
    explicit ProxyUnitingClass(QObject *parent = Q_NULLPTR);

private:

    ServerHttp *m_serverHttp;
    WebSocketTransfer *m_clientsSocketTransfer;
    WebSocketTransfer *m_desktopSocketTransfer;

private slots:
    void loadSettings();
    void startHttpServer(quint16 port, const QString &filesPath);
    void startClientsWebSocketTransfer(quint16 port, const QString &login, const QString &pass);
    void startDesktopWebSocketTransfer(quint16 port, const QString &login, const QString &pass);
    void moveWebSocketTransferToThread(WebSocketTransfer *webSocketTransfer);
};

#endif // PROXYUNITINGCLASS_H
