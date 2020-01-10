#ifndef SERVERHTTP_H
#define SERVERHTTP_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class ServerHttp : public QObject
{
    Q_OBJECT
public:
    explicit ServerHttp(QObject *parent = Q_NULLPTR);

private:

    QTcpServer *m_tcpServer;
    QList<QTcpSocket*> m_tcpSockets;

    quint16 m_port;
    QString m_path;
    QStringList m_filesList;

signals:
    void postRequest(QTcpSocket* socket, const QString &path, const QByteArray &data);

public slots:
    bool start();
    void stop();
    void setPort(quint16 port);
    void setPath(const QString &path);
    void sendResponse(QTcpSocket* socket, const QString &path, const QByteArray &data);

private slots:
    void newSocketConnected();
    void socketDisconneted();
    void readDataFromSocket();
    void requestHandler(QTcpSocket* socket, const QString &method, const QString &path, const QMap<QString,QString> &cookies, const QByteArray &requestData);
    QByteArray getData(const QString &name);
    QByteArray createHeader(const QString &path, int dataSize);
    void updateFilesList();
};
#endif // SERVERHTTP_H
