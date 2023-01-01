#include <QNetworkInterface>
#include <QIntValidator>
#include <QCommonStyle>
#include <QApplication>
#include <QMessageBox>
#include <QGridLayout>
#include <QHostInfo>
#include <QSettings>
#include <QLineEdit>
#include <QMenuBar>
#include <QAction>
#include <QStyle>
#include <QLabel>
#include <QMenu>

#include "infowidget.h"

InfoWidget::InfoWidget(QWidget *parent)
    : QMainWindow{parent}
{
    loadSettings();
    setWindowIcon(QIcon(":/res/favicon.ico"));
    setAttribute(Qt::WA_QuitOnClose, false);
    setMinimumWidth(400);

    QMenuBar *menuBar = new QMenuBar(this);
    menuBar->addAction("Settings");

    QCommonStyle style;
    QMenu *menuAbout = menuBar->addMenu("About");
    menuAbout->addAction(QIcon(":/res/favicon.ico"), "About SRD", this, &InfoWidget::showAboutSRDMessage);
    menuAbout->addAction(QIcon(style.standardPixmap(QStyle::SP_TitleBarMenuButton)),
                         "About Qt", this, &InfoWidget::showAboutQtMessage);

    setMenuBar(menuBar);

    QWidget *centralWidget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(centralWidget);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(centralWidget);

    createInfoWidget();
    createSettingsWidget();
}

void InfoWidget::loadSettings()
{
    QSettings settings("config.ini",QSettings::IniFormat);
    settings.beginGroup("REMOTE_DESKTOP");

    int portHttp = settings.value("portHttp", 0).toInt();
    if (portHttp != 0) m_portHttp = portHttp;
    else settings.setValue("portHttp", m_portHttp);

    QString filesPath(settings.value("filesPath").toString());
    if (!filesPath.isEmpty()) m_filesPath = std::move(filesPath);
    else settings.setValue("filesPath", m_filesPath);

    int portWeb = settings.value("portWeb", 0).toInt();
    if (portWeb != 0) m_portWeb = portWeb;
    else settings.setValue("portWeb",portWeb);

    QString login(settings.value("login").toString());
    if (!login.isEmpty()) m_login = std::move(login);
    else settings.setValue("login", m_login);

    QString pass(settings.value("pass").toString());
    if (!pass.isEmpty()) m_pass = std::move(pass);
    else settings.setValue("pass", m_pass);

    m_name = settings.value("name").toString();
    if (m_name.isEmpty())
    {
        m_name = QHostInfo::localHostName();
        settings.setValue("name", m_name);
    }

    QString proxyHost(settings.value("proxyHost").toString());
    if (!proxyHost.isEmpty()) m_proxyHost = std::move(proxyHost);
    else settings.setValue("proxyHost", m_proxyHost);

    QString proxyLogin(settings.value("proxyLogin").toString());
    if (!proxyLogin.isEmpty()) m_proxyLogin = std::move(proxyLogin);
    else settings.setValue("proxyLogin", m_proxyLogin);

    QString proxyPass(settings.value("proxyPass").toString());
    if(!proxyPass.isEmpty()) m_proxyPass = std::move(proxyPass);
    else settings.setValue("proxyPass",proxyPass);

    settings.endGroup();
    settings.sync();
}

void InfoWidget::showAboutSRDMessage()
{
    QString text = tr("<h3>Simple Remote Desktop</h3>\n\n"
                       "<p>This software is distributed under the GPL-3.0 license.</p>"
                       "<p>Remote desktop management from a web browser, based on Qt5.</p>");

    QString contacts = tr("<p>Contacts:</p><p>Email:  agafon0ff@mail.ru</p>"
                       "<p>Github: <a href=\"https://%1/\">%1</a></p>"
                       "<p>Current version: <a href=\"https://%2/\">SimpleRemoteDesktop/releases</a></p>").
            arg(QStringLiteral("github.com/agafon0ff"),
                QStringLiteral("github.com/agafon0ff/SimpleRemoteDesktop/releases/"));

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("About Simple Remote Desktop"));
    msgBox->setText(text);
    msgBox->setInformativeText(contacts);
    msgBox->setAttribute(Qt::WA_QuitOnClose, false);

    msgBox->setIconPixmap(QPixmap(":/res/favicon.ico"));
    msgBox->exec();
    delete msgBox;
}

void InfoWidget::showAboutQtMessage()
{
    QMessageBox::aboutQt(this, "About Qt Libraries");
}

void InfoWidget::createInfoWidget()
{
    QWidget *infoWidget = new QWidget(centralWidget());
    QGridLayout *gridLayout = new QGridLayout(infoWidget);
    gridLayout->setSpacing(21);
    gridLayout->setContentsMargins(21, 21, 21, 21);

    QFont font = QApplication::font();
    font.setPointSize(font.pointSize() * 2);

    QLabel *infoLabel = new QLabel(infoWidget);
    infoLabel->setText("To manage this computer\nfollow the link below:");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    infoLabel->setFont(font);
    gridLayout->addWidget(infoLabel);


    QLabel *addrLabel = new QLabel(infoWidget);
    addrLabel->setText(getLocalAddress());
    addrLabel->setAlignment(Qt::AlignCenter);
    addrLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    addrLabel->setFont(font);
    gridLayout->addWidget(addrLabel);

    centralWidget()->layout()->addWidget(infoWidget);
    infoWidget->hide();
}

void InfoWidget::createSettingsWidget()
{
    QWidget *infoWidget = new QWidget(centralWidget());
    QGridLayout *gridLayout = new QGridLayout(infoWidget);
    gridLayout->setSpacing(9);
    gridLayout->setContentsMargins(6, 6, 6, 6);

    QLabel *labelName = new QLabel("Name:", infoWidget);
    gridLayout->addWidget(labelName, 0, 0);

    QLineEdit *lineEditName = new QLineEdit(m_name, infoWidget);
    gridLayout->addWidget(lineEditName, 0, 1);

    QLabel *labelPortHttp = new QLabel("Http port:", infoWidget);
    gridLayout->addWidget(labelPortHttp, 1, 0);

    QLineEdit *lineEditPortHttp = new QLineEdit(QString::number(m_portHttp), infoWidget);
    lineEditPortHttp->setValidator(new QIntValidator(0, 65535, this));
    gridLayout->addWidget(lineEditPortHttp, 1, 1);

    QLabel *labelPortWeb = new QLabel("WebSocket port:", infoWidget);
    gridLayout->addWidget(labelPortWeb, 2, 0);

    QLineEdit *lineEditPortWeb = new QLineEdit(QString::number(m_portWeb), infoWidget);
    lineEditPortWeb->setValidator(lineEditPortHttp->validator());
    gridLayout->addWidget(lineEditPortWeb, 2, 1);

    QLabel *labelLogin = new QLabel("Login:", infoWidget);
    gridLayout->addWidget(labelLogin, 3, 0);

    QLineEdit *lineEditLogin = new QLineEdit(m_login, infoWidget);
    gridLayout->addWidget(lineEditLogin, 3, 1);

    QLabel *labelPass = new QLabel("Password:", infoWidget);
    gridLayout->addWidget(labelPass, 4, 0);

    QLineEdit *lineEditPass = new QLineEdit(m_pass, infoWidget);
    lineEditPass->setEchoMode(QLineEdit::Password);
    gridLayout->addWidget(lineEditPass, 4, 1);

    QLabel *labelProxyAddr = new QLabel("Proxy address:", infoWidget);
    gridLayout->addWidget(labelProxyAddr, 5, 0);

    QLineEdit *lineEditProxyAddr = new QLineEdit(m_proxyHost, infoWidget);
    gridLayout->addWidget(lineEditProxyAddr, 5, 1);

    QLabel *labelProxyLogin = new QLabel("Proxy login:", infoWidget);
    gridLayout->addWidget(labelProxyLogin, 6, 0);

    QLineEdit *lineEditProxyLogin = new QLineEdit(m_proxyLogin, infoWidget);
    gridLayout->addWidget(lineEditProxyLogin, 6, 1);

    QLabel *labelProxyPass = new QLabel("Proxy password:", infoWidget);
    gridLayout->addWidget(labelProxyPass, 7, 0);

    QLineEdit *lineEditProxyPass = new QLineEdit(m_proxyPass, infoWidget);
    lineEditProxyPass->setEchoMode(QLineEdit::Password);
    gridLayout->addWidget(lineEditProxyPass, 7, 1);

    centralWidget()->layout()->addWidget(infoWidget);
}

QString InfoWidget::getLocalAddress()
{
    QString result;
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

    if(my_addr_list.count() == 0)
        return result;

    result.append("http://");
    result.append(my_addr_list.at(0).toString());
    result.append(":");
    result.append(QString::number(portHttp()));

    return result;
}
