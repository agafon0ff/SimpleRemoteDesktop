#include "serverhttp.h"
#include <QSettings>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QDirIterator>

ServerHttp::ServerHttp(QObject *parent) : QObject(parent),
    m_tcpServer(Q_NULLPTR),
    m_port(8080),
    m_path(":/res/")
{

}

bool ServerHttp::start()
{
    bool result = false;

    if(!m_tcpServer)
    {
        m_tcpServer = new QTcpServer(this);
        connect(m_tcpServer,SIGNAL(newConnection()),this,SLOT(newSocketConnected()));

        if(!m_tcpServer->listen(QHostAddress::Any, m_port))
            qDebug()<<"ERROR: HTTP-Server is not started on port:"<<m_port;
        else
        {
            result = true;
            qDebug()<<"OK: HTTP-Server is started on port:"<<m_port;
            updateFilesList();
        }
    }

    return result;
}

void ServerHttp::stop()
{
    if(m_tcpServer->isListening())
        m_tcpServer->close();
}

void ServerHttp::setPort(quint16 port)
{
    m_port = port;
}

void ServerHttp::setPath(const QString &path)
{
    if(path.size() < 2)
        return;

    m_path = path;

    if(m_path.at(m_path.size() - 1) != '/')
        m_path.append("/");
}

void ServerHttp::sendResponse(QTcpSocket *socket, const QString &path, const QByteArray &data)
{
    if(!socket)
        return;

    QByteArray response = createHeader(path,data.size());
    response.append(data);

    socket->write(response);
    socket->waitForBytesWritten(100);
    socket->disconnectFromHost();
}

void ServerHttp::newSocketConnected()
{
    QTcpSocket* socket = m_tcpServer->nextPendingConnection();
//    QString address = QHostAddress(socket->peerAddress().toIPv4Address()).toString();
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconneted()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(readDataFromSocket()));

    m_tcpSockets.append(socket);
}

void ServerHttp::socketDisconneted()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
//    QString address = QHostAddress(socket->peerAddress().toIPv4Address()).toString();
    m_tcpSockets.removeOne(socket);
}

void ServerHttp::readDataFromSocket()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    QByteArray buf = socket->readAll();
    QStringList list = QString(buf).split("\r\n");
    QString method;
    QString path;
    QByteArray requestData;
    QMap<QString,QString> cookies;
    QRegularExpression re;

    if(list.last().size() > 0)
        requestData.append(list.last());

    foreach (QString line, list)
    {
        if(line.contains("HTTP"))
        {
            re = QRegularExpression("(\\S*)\\s*(\\S*)\\s*HTTP");
            QRegularExpressionMatchIterator it = re.globalMatch(line);

            while(it.hasNext())
            {
                QRegularExpressionMatch match = it.next();
                method = match.captured(1);
                path = match.captured(2);
            }
        }
        else if(line.contains("Cookie:"))
        {
            re = QRegularExpression("\\s(.*?)=(.*?)($|;|:)");
            QRegularExpressionMatchIterator it = re.globalMatch(line);

            while(it.hasNext())
            {
                QRegularExpressionMatch match = it.next();
                cookies.insert(match.captured(1),match.captured(2));
            }
        }
    }

    requestHandler(socket,method,path,cookies,requestData);
}

void ServerHttp::requestHandler(QTcpSocket *socket, const QString &method, const QString &path, const QMap<QString,QString> &cookies, const QByteArray &requestData)
{
    Q_UNUSED(cookies);

    qDebug()<<"ServerHttp::requestHandler"<<socket<<method<<path<<requestData;

    if(method == "GET")
    {
        sendResponse(socket,path,getData(path));
    }
    else if(method == "POST")
    {
        if(receivers(SIGNAL(postRequest(QTcpSocket*,QString,QByteArray))) == 0)
            sendResponse(socket,path,getData(path));
        else emit postRequest(socket,path,requestData);
    }
}

QByteArray ServerHttp::getData(const QString &name)
{
    QByteArray result;
    QString fileName;

    if(name.size() < 3)
        fileName = m_path+"index.html";
    else
    {
        fileName = name;

        if(fileName.at(0) == '/')
            fileName.remove(0,1);

        fileName.prepend(m_path);
    }

    if(!m_filesList.contains(fileName))
        return result;

    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly))
    {
        result = file.readAll();
        file.close();
    }
    else
    {
        qDebug()<<"ERROR: File is not open: "+fileName+"!";
    }

    return result;
}

QByteArray ServerHttp::createHeader(const QString &path, int dataSize)
{
    QByteArray code = "200 OK\r\n";

    if(dataSize == 0)
        code = "404 Not Found\r\n";

    QByteArray type("Content-Type: ");

    if(path.contains(".ico"))
        type += "image/ico";
    else if(path.contains(".css"))
        type += "text/css";
    else if(path.contains(".html"))
        type += "text/html";
    else if(path.contains(".js"))
        type += "text/javascript";
    else if(path.contains(".png"))
        type += "image/png";

    type += "; charset=utf-8\r\n";

    QByteArray length = "Content-Length: ";
    length.append(QString::number(dataSize));
    length.append("\r\n");

    QByteArray date = "Date: ";
    QDateTime dt =  QDateTime::currentDateTime();
    QLocale locale {QLocale(QLocale::English)};
    date.append(locale.toString(dt, "ddd, dd MMM yyyy hh:mm:ss"));
    date.append(" GMT\r\n");

    QByteArray result;
    result.append("HTTP/1.1 ");
    result.append(code);
    result.append(type);
    result.append(length);
    result.append("Vary: Accept-Encoding\r\n");
    result.append("Connection: keep-alive\r\n");
//    result.append("Set-Cookie: firstCookie=hello; Max-Age=600; HttpOnly; Version=1\r\n");
//    result.append("Set-Cookie: secondCookie=world; Max-Age=600; HttpOnly; Version=1\r\n");
    result.append(date);
    result.append("\r\n");

    return result;
}

void ServerHttp::updateFilesList()
{
    m_filesList.clear();

    QDir dir(m_path);
    if(!dir.exists())
    {
        qDebug()<<"ERROR: Dir is not exists:"<<m_path;
        return;
    }

    qDebug()<<"OK: Dir with files:"<<m_path;

    QDirIterator dirIterator(m_path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    QStringList fileList;
    fileList = QDir(m_path).entryList(QDir::Files);

    for(int i=0;i<fileList.count();++i)
        m_filesList.append(m_path + fileList.at(i));

    while(dirIterator.hasNext())
    {
        dirIterator.next();

        QString filePath = dirIterator.filePath();
        fileList = QDir(filePath).entryList(QDir::Files);

        for(int i=0;i<fileList.count();++i)
            m_filesList.append(filePath + "/" + fileList.at(i));

        m_filesList.prepend(filePath);

        if(m_filesList.size() > 3000)
            return;
    }
}
