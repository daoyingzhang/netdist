#include "tcpserver.h"
#include "ui_tcpsever.h"
#include "mytcpserver.h"

TcpSever::TcpSever(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpSever)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
}

TcpSever::~TcpSever()
{
    delete ui;
}

void TcpSever::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODevice::ReadOnly)){
        QByteArray baData=file.readAll();
        QString strData=baData.toStdString().c_str(); //这里的c_str是取首地址。
        qDebug()<<strData;
        file.close();
        strData.replace("\r\n"," "); //将数据进行替换，以便拆分
        QStringList strList = strData.split(" ");//进行拆分
        m_strIP=strList[0];
        m_usPort=strList[1].toUShort();
    }else{
        //这个地方讨个懒。
        qDebug()<<"dsds";
    }
}
