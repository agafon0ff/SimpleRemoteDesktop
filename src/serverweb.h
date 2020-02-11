#ifndef SERVERWEB_H
#define SERVERWEB_H

#include <QObject>
#include <QDebug>
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebSockets/qwebsocket.h>
#include <QSettings>

class ServerWeb : public QObject
{
    Q_OBJECT
public:
    explicit ServerWeb(QObject *parent = Q_NULLPTR);
    ~ServerWeb();

private:

    QWebSocketServer *m_webSocketServer;
    QList<QWebSocket*> m_webClients;
    QByteArray m_dataTmp;

signals:
    void textFromSocket(const QByteArray &uuid, const QString &text);
    void dataFromSocket(const QByteArray &uuid, const QByteArray &data);
    void disconnectedAll();

public slots:
    bool initServer(quint16 port);
    void sendText(const QByteArray &uuid, const QString &text);
    void sendData(const QByteArray &uuid, const QByteArray &data);

    void sendTextToAll(const QString &text);
    void sendDataToAll(const QByteArray &data);

private slots:
    void newSocketConnection();
    void socketDisconnected();
    void textData(const QString &text);
    void binData(const QByteArray &buf);
};

#endif // SERVERWEB_H
