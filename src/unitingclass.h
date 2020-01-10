#ifndef UNITINGCLASS_H
#define UNITINGCLASS_H

#include <QObject>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "serverhttp.h"
#include "serverweb.h"
#include "dataparser.h"
#include "graberclass.h"
#include "inputsimulator.h"

class UnitingClass : public QObject
{
    Q_OBJECT
public:
    explicit UnitingClass(QObject *parent = Q_NULLPTR);

private:

    ServerHttp *m_serverHttp;
    ServerWeb *m_serverWeb;
    DataParser *m_dataParser;
    GraberClass *m_graberClass;
    InputSimulator *m_inputSimulator;

    QMenu *m_trayMenu;
    QSystemTrayIcon *m_trayIcon;

    QString m_currentIp;
    int m_currentPort;

signals:

public slots:

private slots:
    void actionTriggered(QAction* action);
    void showInfoMessage();
    void updateCurrentIp();
    void loadSettings();
};

#endif // UNITINGCLASS_H
