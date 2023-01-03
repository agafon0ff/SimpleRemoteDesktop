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
    setWindowIcon(QIcon(":/res/favicon.ico"));
    setAttribute(Qt::WA_QuitOnClose, false);
    setMinimumWidth(400);

    QWidget *centralWidget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(centralWidget);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(centralWidget);

    loadSettings();
    createInfoWidget();
    createSettingsWidget();

    QMenuBar *menuBar = new QMenuBar(this);
    menuBar->addAction("Info", this, [=]{ m_settingsWidget->hide(); m_infoWidget->show(); });
    menuBar->addAction("Settings", this, [=]{ m_infoWidget->hide(); m_settingsWidget->show(); });

    QCommonStyle style;
    QMenu *menuAbout = menuBar->addMenu("About");
    menuAbout->addAction(QIcon(":/res/favicon.ico"), "About SRD", this, &InfoWidget::showAboutSRDMessage);
    menuAbout->addAction(QIcon(style.standardPixmap(QStyle::SP_TitleBarMenuButton)),
                         "About Qt", this, &InfoWidget::showAboutQtMessage);

    setMenuBar(menuBar);
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
    if (m_infoWidget)
        return;

    m_infoWidget = new QWidget(centralWidget());
    QGridLayout *gridLayout = new QGridLayout(m_infoWidget);
    gridLayout->setSpacing(9);
    gridLayout->setContentsMargins(9, 9, 9, 9);

    QFont font = QApplication::font();
    font.setPointSize(font.pointSize() * 1.5);

    QLabel *labelFavicon = new QLabel(m_infoWidget);
    labelFavicon->setPixmap(QPixmap(":/res/favicon.ico"));
    labelFavicon->setScaledContents(true);
    labelFavicon->setMaximumSize(50, 50);
    gridLayout->addWidget(labelFavicon, 0, 0);

    QLabel *infoLabel = new QLabel(m_infoWidget);
    infoLabel->setText("To manage this computer\nfollow the link below:");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    infoLabel->setFont(font);
    gridLayout->addWidget(infoLabel, 0, 1);


    QLabel *addrLabel = new QLabel(m_infoWidget);
    addrLabel->setText(getLocalAddress());
    addrLabel->setAlignment(Qt::AlignCenter);
    addrLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    addrLabel->setFont(font);
    gridLayout->addWidget(addrLabel, 1, 0, 1, 2);

    centralWidget()->layout()->addWidget(m_infoWidget);
}

void InfoWidget::createSettingsWidget()
{
    if (m_settingsWidget)
        return;

    m_settingsWidget = new QWidget(centralWidget());
    QGridLayout *gridLayout = new QGridLayout(m_settingsWidget);
    gridLayout->setSpacing(9);
    gridLayout->setContentsMargins(9, 9, 9, 9);

    int row = 0;

    QLabel *labelName = new QLabel("Name:", m_settingsWidget);
    gridLayout->addWidget(labelName, ++row, 0);

    QLineEdit *lineEditName = new QLineEdit(m_name, m_settingsWidget);
    gridLayout->addWidget(lineEditName, row, 1);

    QLabel *labelPortHttp = new QLabel("Http port:", m_settingsWidget);
    gridLayout->addWidget(labelPortHttp, ++row, 0);

    QLineEdit *lineEditPortHttp = new QLineEdit(QString::number(m_portHttp), m_settingsWidget);
    lineEditPortHttp->setValidator(new QIntValidator(0, 65535, this));
    gridLayout->addWidget(lineEditPortHttp, row, 1);

    QLabel *labelPortWeb = new QLabel("WebSocket port:", m_settingsWidget);
    gridLayout->addWidget(labelPortWeb, ++row, 0);

    QLineEdit *lineEditPortWeb = new QLineEdit(QString::number(m_portWeb), m_settingsWidget);
    lineEditPortWeb->setValidator(lineEditPortHttp->validator());
    gridLayout->addWidget(lineEditPortWeb, row, 1);

    QLabel *labelLogin = new QLabel("Login:", m_settingsWidget);
    gridLayout->addWidget(labelLogin, ++row, 0);

    QLineEdit *lineEditLogin = new QLineEdit(m_login, m_settingsWidget);
    gridLayout->addWidget(lineEditLogin, row, 1);

    QLabel *labelPass = new QLabel("Password:", m_settingsWidget);
    gridLayout->addWidget(labelPass, ++row, 0);

    QLineEdit *lineEditPass = new QLineEdit(m_pass, m_settingsWidget);
    lineEditPass->setEchoMode(QLineEdit::Password);
    gridLayout->addWidget(lineEditPass, row, 1);

    QLabel *labelProxyAddr = new QLabel("Proxy address:", m_settingsWidget);
    gridLayout->addWidget(labelProxyAddr, ++row, 0);

    QLineEdit *lineEditProxyAddr = new QLineEdit(m_proxyHost, m_settingsWidget);
    gridLayout->addWidget(lineEditProxyAddr, row, 1);

    QLabel *labelProxyLogin = new QLabel("Proxy login:", m_settingsWidget);
    gridLayout->addWidget(labelProxyLogin, ++row, 0);

    QLineEdit *lineEditProxyLogin = new QLineEdit(m_proxyLogin, m_settingsWidget);
    gridLayout->addWidget(lineEditProxyLogin, row, 1);

    QLabel *labelProxyPass = new QLabel("Proxy password:", m_settingsWidget);
    gridLayout->addWidget(labelProxyPass, ++row, 0);

    QLineEdit *lineEditProxyPass = new QLineEdit(m_proxyPass, m_settingsWidget);
    lineEditProxyPass->setEchoMode(QLineEdit::Password);
    gridLayout->addWidget(lineEditProxyPass, row, 1);

    centralWidget()->layout()->addWidget(m_settingsWidget);
    m_settingsWidget->hide();
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
