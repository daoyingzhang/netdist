#include "mytcpserver.h"

#include <mytcpsocket.h>
MyTcpServer::MyTcpServer() {

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug()<<"new client connected";
    MyTcpSocket *pTcpSocket=new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*))
            ,this,SLOT(deleteSocket(MyTcpSocket*)));
    //关联槽函数，
}

void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    if(NULL==pername || NULL==pdu){
        return;
    }
    QString strName=pername;
    for(int i=0;i<m_tcpSocketList.length();i++){
        if(strName==m_tcpSocketList.at(i)->getName()){
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            break;
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket *mySocket)
{
    //遍历list，然后删除
    QList<MyTcpSocket*>::iterator iter=m_tcpSocketList.begin();
    for(;iter!=m_tcpSocketList.end();++iter)
    {
        if(mySocket==*iter)
        {
            (*iter) -> deleteLater(); //new的对象删除,
            *iter=NULL;
            m_tcpSocketList.erase(iter);//指针删除
            break;
        }
    }
    for(int i=0;i<m_tcpSocketList.size();++i)
    {
        qDebug()<<m_tcpSocketList.at(i)->getName();
    }
/*    来自csdn上的解释
 *    如果线程中没有运行着的事件循环，线程中的对象调用了deleteLater()，当线程结束后对象才被销毁。
 *    当第一次QDeferredDeleteEvent传递给事件循环后，对象的任何待处理事件(pending events)都从事件队列中移除。

      Qt中不建议手动delete掉QObject对象。
      原因一：不注意父子关系会导致某个对象析构两次，一次是手动析构，还有一次是parent析构，后者可能会出现delete堆上的对象。

      原因二：删除一个pending events等待传递的QObject会导致崩溃，所以不能直接跨线程删除对象，而QObject析构函数会断开所有信号和槽，
       因此用deleteLater代替比较好，它会让所有事件都发送完一切处理好后马上清除这片内存，而且就算调用多次的deletelater也是安全的

很多人应该用过 QPointer.吧， deleteLater()跟它类似，就是在删除一个对像时，依赖它的子对象，子对象的子对象都会关联地删除。
    当我们使用父对象来创建一个对象的时候 ，父对象会把这个对象添加到自己的子对象列表中。
    当这个父对象被删除的时候，它会遍历它的子对象类表并且删除每一个子对象，然后子对象们自己再删除它们自己的子对象，
    这样递归调用直到所有对象都被删除。 这种父子对象机制会在很大程度上简化我们的内存管理工作，减少内存泄露的风险。
    所以，使用deleteLater主要作用还是减少内存泄露的风险。
*/



}
