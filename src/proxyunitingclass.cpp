#include "proxyunitingclass.h"
#include <QCoreApplication>
#include <QSettings>
#include <QThread>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ProxyUnitingClass u;
    return a.exec();
}

ProxyUnitingClass::ProxyUnitingClass(QObject *parent) : QObject(parent),
    m_serverHttp(new ServerHttp(this)),
    m_clientsSocketTransfer(Q_NULLPTR),
    m_desktopSocketTransfer(Q_NULLPTR)
{
    qDebug()<<"Create(ProxyUnitingClass)";
    loadSettings();
}

void ProxyUnitingClass::loadSettings()
{
    QSettings settings("config.ini",QSettings::IniFormat);
    settings.beginGroup("REMOTE_DESKTOP_PROXY");

    int portHttp = settings.value("portHttp",0).toInt();
    if(portHttp == 0)
    {
        portHttp = 8080;
        settings.setValue("portHttp",portHttp);
    }

    QString filesPath = settings.value("filesPath").toString();
    if(filesPath.isEmpty())
    {
        filesPath = ":/res/";
        settings.setValue("filesPath",filesPath);
    }

    int portWebClient = settings.value("portWebClient",0).toInt();
    if(portWebClient == 0)
    {
        portWebClient = 8081;
        settings.setValue("portWebClient",portWebClient);
    }

    int portWebDesktop = settings.value("portWebDesktop",0).toInt();
    if(portWebDesktop == 0)
    {
        portWebDesktop = 8082;
        settings.setValue("portWebDesktop",portWebDesktop);
    }

    QString login = settings.value("login").toString();
    if(login.isEmpty())
    {
        login = "login";
        settings.setValue("login",login);
    }

    QString pass = settings.value("pass").toString();
    if(pass.isEmpty())
    {
        pass = "pass";
        settings.setValue("pass",pass);
    }

    settings.endGroup();
    settings.sync();

    startHttpServer(static_cast<quint16>(portHttp),filesPath);
    startClientsWebSocketTransfer(static_cast<quint16>(portWebClient),login,pass);
    startDesktopWebSocketTransfer(static_cast<quint16>(portWebDesktop),login,pass);
}

void ProxyUnitingClass::startHttpServer(quint16 port, const QString &filesPath)
{
    m_serverHttp->setPort(static_cast<quint16>(port));
    m_serverHttp->setPath(filesPath);
    m_serverHttp->start();
}

void ProxyUnitingClass::startClientsWebSocketTransfer(quint16 port, const QString &login, const QString &pass)
{
    m_clientsSocketTransfer = new WebSocketTransfer;
    m_clientsSocketTransfer->setType(WebSocketTransfer::TransferProxyClients);
    m_clientsSocketTransfer->setPort(port);
    m_clientsSocketTransfer->setLoginPass(login, pass);

    connect(m_clientsSocketTransfer, &WebSocketTransfer::newSocketConnected,
            this, &ProxyUnitingClass::createClientWebSocketConnection);

    moveWebSocketTransferToThread(m_clientsSocketTransfer);
}

void ProxyUnitingClass::startDesktopWebSocketTransfer(quint16 port, const QString &login, const QString &pass)
{
    m_desktopSocketTransfer = new WebSocketTransfer;
    m_desktopSocketTransfer->setType(WebSocketTransfer::TransferDesktops);
    m_desktopSocketTransfer->setPort(port);
    m_desktopSocketTransfer->setLoginPass(login, pass);

    connect(m_desktopSocketTransfer, &WebSocketTransfer::newSocketConnected,
            this, &ProxyUnitingClass::createDesktopWebSocketConnection);

    moveWebSocketTransferToThread(m_desktopSocketTransfer);
}

void ProxyUnitingClass::moveWebSocketTransferToThread(WebSocketTransfer *webSocketTransfer)
{
    QThread *thread = new QThread;

    connect(thread, &QThread::started, webSocketTransfer, &WebSocketTransfer::start);
    connect(webSocketTransfer, &WebSocketTransfer::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, webSocketTransfer, &WebSocketTransfer::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    webSocketTransfer->moveToThread(thread);
    thread->start();
}

void ProxyUnitingClass::createClientWebSocketConnection(WebSocketHandler *webSocket)
{
    if(!webSocket)
        return;

    connect(webSocket, &WebSocketHandler::remoteAuthenticationRequest,
            m_desktopSocketTransfer, &WebSocketTransfer::checkRemoteAuthentication);

    connect(webSocket, &WebSocketHandler::newProxyConnection,
            m_desktopSocketTransfer, &WebSocketTransfer::createProxyConnection);


}

void ProxyUnitingClass::createDesktopWebSocketConnection(WebSocketHandler *webSocket)
{
    if(!webSocket)
        return;

    connect(webSocket, &WebSocketHandler::remoteAuthenticationResponse,
            m_clientsSocketTransfer, &WebSocketTransfer::setRemoteAuthenticationResponse);
}
