#pragma once

#include <QSystemTrayIcon>
#include <QObject>
#include <QMenu>

#include "serverhttp.h"
#include "infowidget.h"
#include "websockettransfer.h"
#include "graberclass.h"
#include "inputsimulator.h"

class RemoteDesktopUniting : public QObject
{
    Q_OBJECT
public:
    explicit RemoteDesktopUniting(QObject *parent = Q_NULLPTR);

private:

    WebSocketTransfer *m_webSocketTransfer;
    WebSocketHandler *m_webSocketHandler;
    ServerHttp *m_serverHttp;
    GraberClass *m_graberClass;
    InputSimulator *m_inputSimulator;
    QMenu *m_trayMenu;
    InfoWidget *m_infoWidget;
    QSystemTrayIcon *m_trayIcon;

    QString m_title;
    int m_currentPort;
    bool m_isConnectedToProxy;

    QList<QByteArray> m_remoteClientsList;

signals:
    void closeSignal();
    void stopGrabing();

public slots:

private slots:
    void showInfoWidget();
    void startHttpServer(quint16 port, const QString &filesPath);
    void startGraberClass();
    void startWebSocketTransfer(quint16 port, const QString &login, const QString &pass, const QString &name);
    void startWebSocketHandler(const QString &host, const QString &name, const QString &login,
                               const QString &pass, const QString &proxyLogin, const QString &proxyPass);
    void createConnectionToHandler(WebSocketHandler *webSocketHandler);
    void finishedWebSockeTransfer();
    void finishedWebSockeHandler();
    void remoteClientConnected(const QByteArray &uuid);
    void remoteClientDisconnected(const QByteArray &uuid);
    void connectedToProxyServer(bool state);
};

