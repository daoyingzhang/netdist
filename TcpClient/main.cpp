#include "tcpclient.h"
#include <QMessageBox>
#include <QApplication>
#include "sharefile.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // ShareFile w;
    // w.show();
    // TcpClient w;
    // w.show();
    TcpClient::getInstance().show();//这个调用产生的那个单例，也就是全局只有一个对象。
    return a.exec();

}
