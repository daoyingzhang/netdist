#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include "privatechat.h"
#include <QInputDialog>

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE=new QTextEdit;
    m_pFirendListwidget=new QListWidget;
    m_pInputMsgLE=new QLineEdit;
    m_pDelFriendPB=new QPushButton("delete friend");
    m_pFlushFriendPB=new QPushButton("flush friend");
    m_pShowOnlineUsrPB=new QPushButton("show online friend");
    m_pSearchUsrPB=new QPushButton("search friend");
    m_pMsgSendPB=new QPushButton("message send");
    m_pPrivateChatPB=new QPushButton("private chat");
    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFirendListwidget);
    pTopHBL->addLayout(pRightPBVBL);
    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;
    QVBoxLayout*pMain= new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();//先将这个界面隐藏。
    setLayout(pMain);
    connect(m_pShowOnlineUsrPB,SIGNAL(clicked(bool))
            ,this,SLOT(showOnline()));
    connect(m_pSearchUsrPB,SIGNAL(clicked(bool))
            ,this,SLOT(searchUsr()));
    connect(m_pFlushFriendPB,SIGNAL(clicked(bool))
            ,this,SLOT(flushFriend()));
    connect(m_pDelFriendPB,SIGNAL(clicked(bool))
            ,this,SLOT(delFriend()));
    connect(m_pPrivateChatPB,SIGNAL(clicked(bool))
            ,this,SLOT(privateChat()));
    connect(m_pMsgSendPB,SIGNAL(clicked(bool))
            ,this,SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if(NULL==pdu){
        return;
    }
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(NULL==pdu){
        return;
    }
    m_pFirendListwidget->clear();//先将原来的清除掉
    uint uiSize=pdu->uiMsgLen/32;
    char caName[32]={'\0'};
    for(uint i=0;i<uiSize;i++){
        memcpy(caName,(char*)pdu->caMsg+i*32,32);
        m_pFirendListwidget->addItem(caName);
    }
}

void Friend::updateGroupMsg(PDU *pdu)
{
    QString strMsg=QString("%1 says: %2").arg(pdu->caData).arg((char*)pdu->caMsg);//这个位置要考虑一下
    m_pShowMsgTE->append(strMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFirendListwidget;
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden()){
        m_pOnline->show();
        PDU *pdu =mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;

    }else{
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    m_strSearchName=QInputDialog ::getText(this,"search","name");
    if(!m_strSearchName.isEmpty()){
        PDU *pdu=mkPDU(0);
        memcpy(pdu->caData,m_strSearchName.toStdString().c_str(),m_strSearchName.size());
        pdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Friend::flushFriend()
{
    QString strName =TcpClient::getInstance().loginName();
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Friend::delFriend()
{
    if(m_pFirendListwidget->currentItem()!=NULL){
        QString strFriendName = m_pFirendListwidget->currentItem()->text();
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        QString strSelfName=TcpClient::getInstance().loginName();
        memcpy(pdu->caData,strSelfName.toStdString().c_str(),strSelfName.size());
        memcpy(pdu->caData+32,strFriendName.toStdString().c_str(),strFriendName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }

}

void Friend::privateChat()
{
    if(m_pFirendListwidget->currentItem()!=NULL){

        PrivateChat::getInstance().setChatName(m_pFirendListwidget->currentItem()->text());
        if(PrivateChat::getInstance().isHidden()){
            PrivateChat::getInstance().show();
        }
    }else{
        QMessageBox::warning(this,"private","place chose the private people");
    }
}

void Friend::groupChat()
{
    QString strMsg= m_pInputMsgLE->text();
    m_pInputMsgLE->clear();
    if(!strMsg.isEmpty()){
        PDU *pdu =mkPDU(strMsg.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        strncpy(pdu->caData,TcpClient::getInstance().loginName().toStdString().c_str()
                ,TcpClient::getInstance().loginName().size());
        strncpy((char*)pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);

    }else{
        QMessageBox::warning(this,"group","group message is empty");
    }
}
