#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QTcpSocket> //这个是用来链接服务器的包。
#include <QHostAddress>
#include "protocol.h"
#include "opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    static TcpClient &getInstance();//添加一个静态的单例函数，使得全局都可以访问
    QTcpSocket &getTcpSocket();
    QString loginName();
    QString curPath();
    void setCurPath(QString strCurPath);

public slots:
    void showConnect();//用来判断连接服务器是否成功，属于槽函数。
    void recvMsg();

private slots:

    // void on_send_pb_clicked();

    void on_login_pb_clicked();

    void on_registe_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;
    QTcpSocket m_tcpSocket;//链接服务器用来进行数据交互。
    QString m_strLoginName;

    QString m_strCurPath;
    QFile m_file;
};
#endif // TCPCLIENT_H
