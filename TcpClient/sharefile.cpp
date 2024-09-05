#include "sharefile.h"
#include <QCheckBox>
#include <QDebug>
#include <tcpclient.h>
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAllPB=new QPushButton("All select ");
    m_pCancelSelectPB=new QPushButton("cancel select");

    m_pOKPB=new QPushButton("ok");
    m_pCancelPB=new QPushButton("cancel");
    m_pSA=new QScrollArea;
    m_pFriendw=new QWidget;
    m_pFriendWVBL=new QVBoxLayout(m_pFriendw);

    m_pButtonGroup=new QButtonGroup(m_pFriendw);
    m_pButtonGroup->setExclusive(false);

    QHBoxLayout *pTopHBL=new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL=new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);
    connect(m_pCancelSelectPB,SIGNAL(clicked(bool))
            ,this,SLOT(cancelSelect()));
    connect(m_pSelectAllPB,SIGNAL(clicked(bool))
            ,this,SLOT(selectAll()));
    connect(m_pOKPB,SIGNAL(clicked(bool))
            ,this,SLOT(okShare()));
    connect(m_pCancelPB,SIGNAL(clicked(bool))
            ,this,SLOT(cancel()));

}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if(NULL==pFriendList){
        return;
    }
    QAbstractButton *tem=NULL;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    for(int i=0;i<preFriendList.size();i++){
        tem=preFriendList[i];
        m_pFriendWVBL->removeWidget(tem);
        m_pButtonGroup->removeButton(tem);
        preFriendList.removeOne(tem);
        delete tem;
        tem=NULL;
    }
    QCheckBox *pCB=NULL;
    for(int i=0;i<pFriendList->count();i++){
        pCB=new QCheckBox(pFriendList->item(i)->text());
        qDebug()<<"+++"<<pFriendList->item(i)->text();
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendw);


}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i=0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i=0;i<cbList.size();i++){
        if(!cbList[i]->isChecked()){
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()
{
    QString strName=TcpClient::getInstance().loginName();
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strShareFileName=OpeWidget::getInstance().getBook()->getShareFileName();
    QString strPath = strCurPath+"/"+strShareFileName;
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int num=0;
    for(int i=0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            num++;
        }
    }
    PDU *pdu = mkPDU(32*num+strPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num);
    int count=0;
    for(int i=0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            memcpy((char*)(pdu->caMsg)+count*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().size());
            count++;
        }
    }
    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(),strPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void ShareFile::cancel()
{

}
