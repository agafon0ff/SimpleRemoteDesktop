#ifndef SOCKETWEB_H
#define SOCKETWEB_H

#include <QObject>
#include <QTimer>
#include <QWebSocket>

class SocketWeb : public QObject
{
    Q_OBJECT
public:
    explicit SocketWeb(QObject *parent = Q_NULLPTR);

private:
    QWebSocket *m_webSocket;
    QTimer *m_timerReconnect;
    QString m_url;

signals:
    void finished();
    void connectedToHost();
    void disconnectedFromHost();

public slots:
    void start();
    void stop();
    void setUrl(const QString &url);

private slots:
    void socketStateChanged(QAbstractSocket::SocketState state);
    void textMessageReceived(const QString &message);
    void reconnectTimerTick();
    void deleteSocket();

};

#endif // SOCKETWEB_H
