#include "remotedesktopuniting.h"
#include <QApplication>
#include <QCommonStyle>
#include <QMessageBox>
#include <QSettings>
#include <QHostInfo>
#include <QThread>
#include <QAction>
#include <QDebug>
#include <QUuid>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RemoteDesktopUniting u;
    return a.exec();
}

RemoteDesktopUniting::RemoteDesktopUniting(QObject *parent) : QObject(parent),
    m_webSocketTransfer(Q_NULLPTR),
    m_webSocketHandler(Q_NULLPTR),
    m_serverHttp(new ServerHttp(this)),
    m_graberClass(nullptr),
    m_inputSimulator(new InputSimulator(this)),
    m_trayMenu(new QMenu),
    m_infoWidget(new InfoWidget),
    m_trayIcon(new QSystemTrayIcon(this)),
    m_title("SimpleRemoteDesktop v1.1"),
    m_currentPort(8080),
    m_isConnectedToProxy(false)
{
    m_infoWidget->setWindowTitle(m_title);
    m_trayMenu->addAction(QIcon(":/res/info.ico"), "Info", this, &RemoteDesktopUniting::showInfoWidget);
    m_trayMenu->addAction(QIcon(":/res/close.ico"), "Exit", this, &RemoteDesktopUniting::closeSignal);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setIcon(QIcon(":/res/favicon.ico"));
    m_trayIcon->setToolTip(m_title);
    m_trayIcon->show();

    startGraberClass();
    startHttpServer(static_cast<quint16>(m_infoWidget->portHttp()), m_infoWidget->filesPath());

    startWebSocketTransfer(static_cast<quint16>(m_infoWidget->portWeb()), m_infoWidget->login(),
                           m_infoWidget->pass(), m_infoWidget->name());

    startWebSocketHandler(m_infoWidget->proxyHost(), m_infoWidget->name(), m_infoWidget->login(),
                          m_infoWidget->pass(), m_infoWidget->proxyLogin(), m_infoWidget->proxyPass());
}

void RemoteDesktopUniting::showInfoWidget()
{
    m_infoWidget->showNormal();
    m_infoWidget->raise();
}

void RemoteDesktopUniting::startHttpServer(quint16 port, const QString &filesPath)
{
    m_serverHttp->setPort(static_cast<quint16>(port));
    m_serverHttp->setPath(filesPath);
    m_currentPort = port;

    if(m_serverHttp->start())
    {
        showInfoWidget();
    }
    else
    {
        m_trayIcon->showMessage(m_title, "Failed to start on port: " +
                                QString::number(port) + "!", QSystemTrayIcon::Critical, 5000);
    }
}

void RemoteDesktopUniting::startGraberClass()
{
    QThread *thread = new QThread;
    m_graberClass = new GraberClass;

    connect(thread, &QThread::started, m_graberClass, &GraberClass::start);
    connect(this, &RemoteDesktopUniting::closeSignal, m_graberClass, &GraberClass::stop);
    connect(m_graberClass, &GraberClass::finished, this, &RemoteDesktopUniting::finishedWebSockeTransfer);
    connect(m_graberClass, &GraberClass::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, m_graberClass, &WebSocketTransfer::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    connect(this, &RemoteDesktopUniting::stopGrabing, m_graberClass, &GraberClass::stopSending);

    m_graberClass->moveToThread(thread);
    thread->start();
}

void RemoteDesktopUniting::startWebSocketTransfer(quint16 port, const QString &login, const QString &pass, const QString &name)
{
    QThread *thread = new QThread;
    m_webSocketTransfer = new WebSocketTransfer;
    m_webSocketTransfer->setType(WebSocketTransfer::TransferWebClients);
    m_webSocketTransfer->setPort(port);
    m_webSocketTransfer->setName(name);
    m_webSocketTransfer->setLoginPass(login, pass);

    connect(thread, &QThread::started, m_webSocketTransfer, &WebSocketTransfer::start);
    connect(this, &RemoteDesktopUniting::closeSignal, m_webSocketTransfer, &WebSocketTransfer::stop);
    connect(m_webSocketTransfer, &WebSocketTransfer::finished, this, &RemoteDesktopUniting::finishedWebSockeTransfer);
    connect(m_webSocketTransfer, &WebSocketTransfer::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, m_webSocketTransfer, &WebSocketTransfer::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    connect(m_webSocketTransfer, &WebSocketTransfer::newSocketConnected, this, &RemoteDesktopUniting::createConnectionToHandler);
    connect(m_webSocketTransfer, &WebSocketTransfer::connectedSocketUuid, this, &RemoteDesktopUniting::remoteClientConnected);
    connect(m_webSocketTransfer, &WebSocketTransfer::disconnectedSocketUuid, this, &RemoteDesktopUniting::remoteClientDisconnected);

    m_webSocketTransfer->moveToThread(thread);
    thread->start();
}

void RemoteDesktopUniting::startWebSocketHandler(const QString &host, const QString &name, const QString &login,
                                                 const QString &pass, const QString &proxyLogin, const QString &proxyPass)
{
    QThread *thread = new QThread;
    m_webSocketHandler = new WebSocketHandler;
    m_webSocketHandler->setType(WebSocketHandler::HandlerSingleClient);
    m_webSocketHandler->setUrl(host);
    m_webSocketHandler->setName(name);
    m_webSocketHandler->setLoginPass(login, pass);
    m_webSocketHandler->setProxyLoginPass(proxyLogin, proxyPass);

    connect(thread, &QThread::started, m_webSocketHandler, &WebSocketHandler::createSocket);
    connect(this, &RemoteDesktopUniting::closeSignal, m_webSocketHandler, &WebSocketHandler::removeSocket);
    connect(m_webSocketHandler, &WebSocketHandler::finished, this, &RemoteDesktopUniting::finishedWebSockeHandler);
    connect(m_webSocketHandler, &WebSocketHandler::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, m_webSocketHandler, &WebSocketHandler::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    connect(m_webSocketHandler, &WebSocketHandler::connectedProxyClient, this, &RemoteDesktopUniting::remoteClientConnected);
    connect(m_webSocketHandler, &WebSocketHandler::disconnectedProxyClient, this, &RemoteDesktopUniting::remoteClientDisconnected);
    connect(m_webSocketHandler, &WebSocketHandler::authenticatedStatus, this, &RemoteDesktopUniting::connectedToProxyServer);

    createConnectionToHandler(m_webSocketHandler);

    m_webSocketHandler->moveToThread(thread);
    thread->start();
}

void RemoteDesktopUniting::createConnectionToHandler(WebSocketHandler *webSocketHandler)
{
    if(!webSocketHandler)
        return;

    connect(m_graberClass, &GraberClass::imageParameters, webSocketHandler, &WebSocketHandler::sendImageParameters);
    connect(m_graberClass, &GraberClass::imageTile, webSocketHandler, &WebSocketHandler::sendImageTile);
    connect(m_graberClass, &GraberClass::screenPositionChanged, m_inputSimulator, &InputSimulator::setScreenPosition);

    connect(webSocketHandler, &WebSocketHandler::getDesktop, m_graberClass, &GraberClass::startSending);
    connect(webSocketHandler, &WebSocketHandler::changeDisplayNum, m_graberClass, &GraberClass::changeScreenNum);
    connect(webSocketHandler, &WebSocketHandler::receivedTileNum, m_graberClass, &GraberClass::setReceivedTileNum);

    connect(webSocketHandler, &WebSocketHandler::setKeyPressed, m_inputSimulator, &InputSimulator::simulateKeyboard);
    connect(webSocketHandler, &WebSocketHandler::setMousePressed, m_inputSimulator, &InputSimulator::simulateMouseKeys);
    connect(webSocketHandler, &WebSocketHandler::setMouseMove, m_inputSimulator, &InputSimulator::simulateMouseMove);
    connect(webSocketHandler, &WebSocketHandler::setWheelChanged, m_inputSimulator, &InputSimulator::simulateWheelEvent);
    connect(webSocketHandler, &WebSocketHandler::setMouseDelta, m_inputSimulator, &InputSimulator::setMouseDelta);
}

void RemoteDesktopUniting::finishedWebSockeTransfer()
{
    m_webSocketTransfer = Q_NULLPTR;

    if(!m_webSocketHandler)
        QApplication::quit();
}

void RemoteDesktopUniting::finishedWebSockeHandler()
{
    m_webSocketHandler = Q_NULLPTR;

    if(!m_webSocketTransfer)
        QApplication::quit();
}

void RemoteDesktopUniting::remoteClientConnected(const QByteArray &uuid)
{
    if(!m_remoteClientsList.contains(uuid))
        m_remoteClientsList.append(uuid);

    qDebug()<<"RemoteDesktopUniting remote client count:" << m_remoteClientsList.size();
}

void RemoteDesktopUniting::remoteClientDisconnected(const QByteArray &uuid)
{
    if(m_remoteClientsList.contains(uuid))
    {
        m_remoteClientsList.removeOne(uuid);

        if(m_remoteClientsList.size() == 0)
            emit stopGrabing();
    }

    qDebug()<<"RemoteDesktopUniting remoteClientDisconnected, count:" << m_remoteClientsList.size();
}

void RemoteDesktopUniting::connectedToProxyServer(bool state)
{
    if(m_isConnectedToProxy != state)
    {
        m_isConnectedToProxy = state;

        if(state)
            showInfoWidget();
    }
}
