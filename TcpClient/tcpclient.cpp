#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QMessageBox>
#include "privatechat.h"
#include "book.h"
TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    resize(500,300);
    loadConfig();
    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));
    connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    //与recvMsg()进行关联，一旦有readyRead()消息，则执行recvMsg()。
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);//连接服务器。
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
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
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath=strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"connect server","success connect the server");
}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getInstance().getBook()->getDownloadStatus()){


        qDebug()<<m_tcpSocket.bytesAvailable()<<"rerer";
        uint uiPDULen=0;
        m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));
        uint uiMsgLen=uiPDULen-sizeof(PDU);
        PDU *pdu=mkPDU(uiMsgLen);
        m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
        qDebug()<<pdu->caData;
        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if(0==strcmp(pdu->caData,REGIST_OK)){
                QMessageBox::information(this,"regist",REGIST_OK);
            }else if(0==strcmp(pdu->caData,REGIST_FAILED)){
                QMessageBox::warning(this,"regist",REGIST_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if(0==strcmp(pdu->caData,LOGIN_OK)){
                m_strCurPath =QString("./%1").arg(m_strLoginName);
                QMessageBox::information(this,"login",LOGIN_OK);
                OpeWidget::getInstance().show();//好友界面展示。
                this->hide();
            }else if(0==strcmp(pdu->caData,LOGIN_FAILED)){
                QMessageBox::warning(this,"login",LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
        {
            qDebug()<<OpeWidget::getInstance().getFriend()->m_strSearchName;
            if(0==strcmp(SEARCH_USR_NO,pdu->caData)){
                QMessageBox::information(this,"search",QString("%1: not exit").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }else if(0==strcmp(SEARCH_USR_Online,pdu->caData)){
                QMessageBox::information(this,"search",QString("%1: Online").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }else if(0==strcmp(SEARCH_USR_Offline,pdu->caData)){
                QMessageBox::information(this,"search",QString("%1: Offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST://这个视频里没有讲全，需要自己在考虑一下以后
        {

            char caName[32]={'\0'};
            strncpy(caName,pdu->caData+32,32);
            int ret=QMessageBox::information(this,"add friend"
                                     ,QString("%1 want to add you.").arg(caName)
                                     ,QMessageBox::Yes,QMessageBox::No);
            PDU *respdu=mkPDU(0);
            memcpy(respdu->caData,pdu->caData,32);
            memcpy(respdu->caData+32,caName,32);
            if(ret==QMessageBox::Yes){
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            }else {
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            }
            m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            QMessageBox::QMessageBox::information(this,"add friend",pdu->caData);;

            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE://回复我：同意
        {
            char friendName[32]={'\0'};
            strncpy(friendName,pdu->caData,32);
            QMessageBox::information(this,"add friend",QString("%1 agree you ").arg(friendName));
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE://回复我：拒绝
        {
            char friendName[32]={'\0'};
            strncpy(friendName,pdu->caData,32);
            QMessageBox::information(this,"add friend",QString("%1 refuse you").arg(friendName));
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            OpeWidget::getInstance().getFriend()->updateFriendList(pdu);

            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caName[32]={'\0'};
            memcpy(caName,pdu->caData,32);
            QMessageBox::information(this, "delete friend",QString("%1 delete you.").arg(caName));
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            QMessageBox::information(this,"delete friend","delete friend success");
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            if(PrivateChat::getInstance().isHidden()){
                PrivateChat::getInstance().show();
            }
            char caSendName[32]={'\0'};
            memcpy(caSendName,pdu->caData,32);
            QString strSendName=caSendName;
            PrivateChat::getInstance().setChatName(strSendName);
            PrivateChat::getInstance().updateMsg(pdu);


            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            QMessageBox::information(this,"create file",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            OpeWidget::getInstance().getBook()->updateFileList(pdu);

            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
        {
            QMessageBox::information(this,"delete dir",pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            QMessageBox::information(this,"rename",pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            if(strcmp(pdu->caData,ENTER_DIR_FAILURED)==0){
                QMessageBox::information(this,"enter dir",pdu->caData);
                OpeWidget::getInstance().getBook()->clearEnterDir();
                break;
            }else{
                OpeWidget::getInstance().getBook()->updateFileList(pdu);
                QString strEnterDir=OpeWidget::getInstance().getBook()->enterDir();
                if(!strEnterDir.isEmpty()){//这个有bug所以不加了
                    m_strCurPath=m_strCurPath+"/"+strEnterDir;
                    qDebug()<<"enter dir: "<<m_strCurPath;
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            QMessageBox::information(this,"upload file",pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND:
        {
            QMessageBox::information(this,"delete dir",pdu->caData);
            OpeWidget::getInstance().getBook()->flushFile();
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            qDebug()<<pdu->caData;
            char caFileName[32]={'\0'};
            sscanf(pdu->caData,"%s %11d",caFileName,&(OpeWidget::getInstance().getBook()->m_iTotal));
            if(strlen(caFileName)>0 && OpeWidget::getInstance().getBook()->m_iTotal>0)
            {
                OpeWidget::getInstance().getBook()->setDownloadStatus(true);
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                if(!m_file.open(QIODevice::WriteOnly)){
                    QMessageBox::warning(this,"download file","get path failured");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
        {
            QMessageBox::information(this,"share file",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
        {
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            char *pos = strrchr(pPath,'/');
            if(NULL != pos){
                QString strNote = QString("%1 share file->%2 \n Do you accept?").arg(pdu->caData).arg(pos);
                int ret = QMessageBox::question(this,"share file",strNote);
                if(QMessageBox::Yes==ret){
                    PDU *respdu =mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg,pdu->caMsg,pdu->uiMsgLen);
                    QString strName=TcpClient::getInstance().loginName();
                    strcpy(respdu->caData,strName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
        {
            QMessageBox::information(this,"move file",pdu->caData);
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu=NULL;
    }
    else{
        QByteArray buffer = m_tcpSocket.readAll();
        m_file.write(buffer);
        Book *pBook=OpeWidget::getInstance().getBook();
        pBook->m_iReceive+=buffer.size();
        if(pBook->m_iTotal==pBook->m_iReceive){
            m_file.close();
            pBook->m_iTotal=0;
            pBook->m_iReceive=0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this,"download file","download success");
        }else if(pBook->m_iTotal<pBook->m_iReceive){
            QMessageBox::critical(this,"download file","download failured");
        }
    }

}

#if 0
void TcpClient::on_send_pb_clicked()
{
    QString strMsg=ui->lineEdit->text();
    if(!strMsg.isEmpty()){
        PDU *pdu=mkPDU(strMsg.size()+1);
        pdu->uiMsgType=8888;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        qDebug()<<(char*)(pdu->caMsg);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::warning(this,"send message","not empty");
    }
}
#endif

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        m_strLoginName=strName;//保存用户名，以便后续使用。
        PDU *pdu=mkPDU(0);//注册的时候消息部分写零。
        pdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//通过socket发送出去。
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::critical(this,"login","login faile: name or password is empty");
    }
}


void TcpClient::on_registe_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        PDU *pdu=mkPDU(0);//注册的时候消息部分写零。
        pdu->uiMsgType=ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//通过socket发送出去。
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::critical(this,"regist","regist faile: name or password is empty");
    }
}


void TcpClient::on_cancel_pb_clicked()
{

}

