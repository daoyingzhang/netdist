#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include "mytcpsocket.h"
#include <QList>
class MyTcpServer : public QTcpServer
{
    Q_OBJECT //要想要支持信号槽，必须加这个。
public:
    MyTcpServer();
    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);
    void resend(const char *pername,PDU *pdu);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
public slots:
    void deleteSocket(MyTcpSocket *mysocket);
};

#endif // MYTCPSERVER_H
