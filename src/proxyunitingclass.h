#ifndef PROXYUNITINGCLASS_H
#define PROXYUNITINGCLASS_H

#include <QObject>
#include "serverhttp.h"
#include "serverweb.h"
#include "dataparser.h"

class ProxyUnitingClass : public QObject
{
    Q_OBJECT
public:
    explicit ProxyUnitingClass(QObject *parent = Q_NULLPTR);

private:

    ServerHttp *m_serverHttp;
    ServerWeb *m_serverWebClients;
    ServerWeb *m_serverWebDesktops;
    DataParser *m_dataParser;

private slots:
    void loadSettings();

};

#endif // PROXYUNITINGCLASS_H
