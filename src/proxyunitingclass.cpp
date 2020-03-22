#include "proxyunitingclass.h"
#include <QSettings>
#include <QDebug>

ProxyUnitingClass::ProxyUnitingClass(QObject *parent) : QObject(parent),
    m_serverHttp(new ServerHttp(this)),
    m_serverWebClients(new ServerWeb(this)),
    m_serverWebDesktops(new ServerWeb(this))
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

    m_serverHttp->setPort(static_cast<quint16>(portHttp));
    m_serverHttp->setPath(filesPath);
    m_serverHttp->start();

    m_serverWebClients->initServer(static_cast<quint16>(portWebClient));
    m_serverWebDesktops->initServer(static_cast<quint16>(portWebDesktop));
}
