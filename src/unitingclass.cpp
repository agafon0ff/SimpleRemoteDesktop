#include "unitingclass.h"
#include <QApplication>
#include <QCommonStyle>
#include <QNetworkInterface>
#include <QSettings>
#include <QDebug>

UnitingClass::UnitingClass(QObject *parent) : QObject(parent),
    m_serverHttp(new ServerHttp(this)),
    m_serverWeb(new ServerWeb(this)),
    m_socketWeb(new SocketWeb(this)),
    m_dataParser(new DataParser(this)),
    m_graberClass(new GraberClass(this)),
    m_inputSimulator(new InputSimulator(this)),
    m_trayMenu(new QMenu),
    m_trayIcon(new QSystemTrayIcon(this)),
    m_currentIp("127.0.0.1"),
    m_currentPort(8080)
{
    QCommonStyle style;
    m_trayMenu->addAction(QIcon(style.standardPixmap(QStyle::SP_MessageBoxInformation)),"Help");
    m_trayMenu->addAction(QIcon(style.standardPixmap(QStyle::SP_DialogCancelButton)),"Exit");

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setIcon(QIcon(":/res/favicon.ico"));
    m_trayIcon->setToolTip("SimpleRemoteDesktop");
    m_trayIcon->show();

    connect(m_trayMenu,SIGNAL(triggered(QAction*)),this,SLOT(actionTriggered(QAction*)));

    updateCurrentIp();
    loadSettings();

    connect(m_serverWeb,SIGNAL(dataFromSocket(QByteArray,QByteArray)),m_dataParser,SLOT(setData(QByteArray,QByteArray)));
    connect(m_serverWeb,SIGNAL(socketConnected(QByteArray)),m_dataParser,SLOT(setNewSocket(QByteArray)));
    connect(m_serverWeb,SIGNAL(socketDisconnected(QByteArray)),m_dataParser,SLOT(removeSocket(QByteArray)));

    connect(m_dataParser,SIGNAL(message(QByteArray)),m_serverWeb,SLOT(sendDataToAll(QByteArray)));
    connect(m_dataParser,SIGNAL(messageToSocket(QByteArray,QByteArray)),m_serverWeb,SLOT(sendData(QByteArray,QByteArray)));

    connect(m_dataParser,SIGNAL(startGraber()),m_graberClass,SLOT(startSending()));
    connect(m_serverWeb,SIGNAL(disconnectedAll()),m_graberClass,SLOT(stopSending()));

    connect(m_graberClass,SIGNAL(imageParameters(QSize,int)),m_dataParser,SLOT(sendImageParameters(QSize,int)));
    connect(m_graberClass,SIGNAL(imageTile(quint16,quint16,QImage,quint16)),m_dataParser,SLOT(sendImageTile(quint16,quint16,QImage,quint16)));
    connect(m_graberClass,SIGNAL(screenPositionChanged(QPoint)),m_inputSimulator,SLOT(setScreenPosition(QPoint)));

    connect(m_dataParser,SIGNAL(setKeyPressed(quint16,bool)),m_inputSimulator,SLOT(simulateKeyboard(quint16,bool)));
    connect(m_dataParser,SIGNAL(setMousePressed(quint16,bool)),m_inputSimulator,SLOT(simulateMouseKeys(quint16,bool)));
    connect(m_dataParser,SIGNAL(setMouseMove(quint16,quint16)),m_inputSimulator,SLOT(simulateMouseMove(quint16,quint16)));
    connect(m_dataParser,SIGNAL(setWheelChanged(bool)),m_inputSimulator,SLOT(simulateWheelEvent(bool)));
    connect(m_dataParser,SIGNAL(changeDisplayNum()),m_graberClass,SLOT(changeScreenNum()));
    connect(m_dataParser,SIGNAL(receivedTileNum(quint16)),m_graberClass,SLOT(setReceivedTileNum(quint16)));
    connect(m_dataParser,SIGNAL(setMouseDelta(qint16,qint16)),m_inputSimulator,SLOT(setMouseDelta(qint16,qint16)));

    m_graberClass->start();

//    QString url = "ws://127.0.0.1:8765";
//    m_socketWeb->setUrl(url);
//    m_socketWeb->start();
}

void UnitingClass::actionTriggered(QAction *action)
{
    if(action->text() == "Help")
        showInfoMessage();
    else if(action->text() == "Exit")
        QApplication::quit();
}

void UnitingClass::showInfoMessage()
{
    m_trayIcon->showMessage("SimpleRemoteDesktop",
                            "To connect, enter in browser:\n" +
                            m_currentIp + ":" + QString::number(m_currentPort),
                            QSystemTrayIcon::Information);
}

void UnitingClass::updateCurrentIp()
{
    QList<QHostAddress> my_addr_list;
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();

    for(int i=0;i<addresses.count();++i)
    {
        if(addresses.at(i).protocol() == QAbstractSocket::IPv4Protocol &&
                !addresses.at(i).toString().contains("127") &&
                !addresses.at(i).toString().contains("172"))
        {
            my_addr_list.append(addresses.at(i));
        }
    }

    if(my_addr_list.count() > 0)
        m_currentIp = my_addr_list.at(0).toString();
}

void UnitingClass::loadSettings()
{
    QSettings settings("config.ini",QSettings::IniFormat);
    settings.beginGroup("REMOTE_DESKTOP");

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

    int portWeb = settings.value("portWeb",0).toInt();
    if(portWeb == 0)
    {
        portWeb = 8081;
        settings.setValue("portWeb",portWeb);
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

    m_dataParser->setLoginPass(login,pass);

    m_serverHttp->setPort(static_cast<quint16>(portHttp));
    m_serverHttp->setPath(filesPath);
    m_currentPort = portHttp;

    if(m_serverHttp->start())
    {
        m_serverWeb->initServer(static_cast<quint16>(portWeb));
        showInfoMessage();
    }
    else
    {
        m_trayIcon->showMessage("SimpleRemoteDesktop","Failed to start on port: " +
                                QString::number(portHttp) + "!",QSystemTrayIcon::Critical,5000);
    }
}
