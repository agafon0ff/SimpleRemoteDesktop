#pragma once

#include <QMainWindow>

class InfoWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit InfoWidget(QWidget *parent = nullptr);

signals:

public:
    void loadSettings();

    int portHttp() const { return m_portHttp; }
    int portWeb() const { return m_portWeb; }
    QString login() const { return m_login; }
    QString pass() const { return m_pass; }
    QString name() const { return m_name; }
    QString filesPath() const { return m_filesPath; }
    QString proxyHost() const { return m_proxyHost; }
    QString proxyLogin() const { return m_proxyLogin; }
    QString proxyPass() const { return m_proxyPass; }

private slots:
    void showAboutSRDMessage();
    void showAboutQtMessage();
    void createInfoWidget();
    void createSettingsWidget();
    QString getLocalAddress();

private:
    int m_portHttp = 8080;
    int m_portWeb = 8081;
    QString m_login = "login";
    QString m_pass = "pass";
    QString m_name = "DESK1";
    QString m_filesPath = ":/res/";
    QString m_proxyHost = "ws://<your.proxy.address>:8082";
    QString m_proxyLogin = "sysadmin";
    QString m_proxyPass = "12345678";
    QWidget *m_infoWidget = nullptr;
    QWidget *m_settingsWidget = nullptr;
};
