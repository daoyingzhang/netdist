#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QTcpSocket> //这个是用来链接服务器的包。
#include <QHostAddress>

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpSever;
}
QT_END_NAMESPACE

class TcpSever : public QWidget
{
    Q_OBJECT

public:
    TcpSever(QWidget *parent = nullptr);
    ~TcpSever();
    void loadConfig();

private:
    Ui::TcpSever *ui;
    QString m_strIP;
    quint16 m_usPort;
};
#endif // TCPSERVER_H
