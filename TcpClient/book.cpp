#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QModelIndex>
#include<QFileDialog>
#include "sharefile.h"
Book::Book(QWidget *parent)
    : QWidget{parent}
{

    m_strEnterDir.clear();
    m_bDownload=false;
    m_pTimer=new QTimer;
    m_pBookListW=new QListWidget;
    m_pReturnPB=new QPushButton("return");
    m_pCreateDirPB=new QPushButton("create dir");
    m_pDelDirPB=new QPushButton("delete dir");
    m_pRenamePB=new QPushButton("rename");
    m_pFlushFilePB=new QPushButton("flush file");

    QVBoxLayout *pDirVBL =new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);



    m_pUploadPB=new QPushButton("upload");
    m_pDownLoadPB=new QPushButton("download");
    m_pDelFilePB=new QPushButton("delete file");
    m_pShareFilePB=new QPushButton("share file");
    m_pMoveFilePB=new QPushButton("move file");
    m_pSelectDirPB =new QPushButton("target file");
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pFileVBL =new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);
    QHBoxLayout *pMain =new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);
    setLayout(pMain);

    connect(m_pCreateDirPB,SIGNAL(clicked(bool))
            ,this,SLOT(creatDir()));
    connect(m_pFlushFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(flushFile()));
    connect(m_pDelDirPB,SIGNAL(clicked(bool))
            ,this,SLOT(delDir()));
    connect(m_pDelFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(delFile()));
    connect(m_pRenamePB,SIGNAL(clicked(bool))
            ,this,SLOT(renameFile()));
    connect(m_pBookListW,SIGNAL(doubleClicked(QModelIndex))
            ,this,SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB,SIGNAL(clicked(bool))
            ,this,SLOT(returnPre()));
    connect(m_pUploadPB,SIGNAL(clicked(bool))
            ,this,SLOT(uploadFile()));
    connect(m_pTimer,SIGNAL(timeout())
            ,this,SLOT(uploadFileData()));

    connect(m_pDownLoadPB,SIGNAL(clicked(bool))
            ,this,SLOT(downloadFile()));
    connect(m_pShareFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(shareFile()));
    connect(m_pMoveFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(moveFile()));
    connect(m_pSelectDirPB,SIGNAL(clicked(bool))
            ,this,SLOT(selectDestDir()));
}

void Book::updateFileList(const PDU *pdu)
{
    if(NULL==pdu){
        return ;
    }
    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    m_pBookListW->clear();
    for(int i=0;i<iCount;i++){
        pFileInfo=(FileInfo*)(pdu->caMsg)+i;
        // qDebug()<<pFileInfo->caFileName<<pFileInfo;
        QListWidgetItem *pItem=new QListWidgetItem;
        if(0==pFileInfo->iFileType){
            pItem->setIcon(QIcon(QPixmap(":/images/dir.png")));
        }else if(1==pFileInfo->iFileType){
            pItem->setIcon(QIcon(QPixmap(":/images/file.jpg")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

void Book::clearEnterDir()
{

}

QString Book::enterDir()
{
    return m_strEnterDir;
}

void Book::setDownloadStatus(bool status)
{
    m_bDownload=status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::creatDir()
{
    QString strNewDir = QInputDialog::getText(this,"new dir","the new dir");
    if(!strNewDir.isEmpty()){
        if(strNewDir.size()>32){
            QMessageBox::warning(this,"new Dir","the name of dir is over 32");
        }else{
            QString strName = TcpClient::getInstance().loginName();
            QString strCurPath=TcpClient::getInstance().curPath();
            PDU *pdu =mkPDU(strCurPath.size()+1);
            pdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());
            strncpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=NULL;
        }
    }else{
        QMessageBox::warning(this,"new Dir","the name of dir is empty");
    }

}

void Book::flushFile()
{
    QString strCurPath =TcpClient::getInstance().curPath();
    PDU *pdu=mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::delDir()//删除常规文件还没有写
{
    QString strCurPath =TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL==pItem){
        QMessageBox::warning(this,"delete dir","place chose dir to delete");
    }else{
        QString strDelName = pItem->text();
        PDU *pdu=mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Book::delFile()
{
    QString strCurPath =TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL==pItem){
        QMessageBox::warning(this,"delete file","place chose file to delete");
    }else{
        QString strDelName = pItem->text();
        PDU *pdu=mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Book::renameFile()
{
    QString strCurPath =TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL==pItem){
        QMessageBox::warning(this,"rename file","place chose file to rename");
    }else{
        QString strOldName=pItem->text();
        QString strNewName=QInputDialog::getText(this,"rename","place input the new name");
        if(!strNewName.isEmpty()){
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType=ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData,strOldName.toStdString().c_str(),strOldName.size());
            strncpy(pdu->caData+32,strNewName.toStdString().c_str(),strNewName.size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=NULL;
        }else{
            QMessageBox::warning(this,"rename","is empty");
        }
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    m_strEnterDir=strDirName;
    QString strCurPath=TcpClient::getInstance().curPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData,strDirName.toStdString().c_str(),strDirName.size());
    memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::returnPre()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    QString strRootPath = "./"+TcpClient::getInstance().loginName();
    if(strCurPath==strRootPath){
        QMessageBox::warning(this,"return pre level","failured: the Top");
    }else{
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index,strCurPath.size()-index);
        qDebug()<<strCurPath;
        TcpClient::getInstance().setCurPath(strCurPath);
        flushFile();
    }
}

void Book::uploadFile()
{

    m_strUploadFilePath=QFileDialog::getOpenFileName();//返回的是点击的文件的绝对路径
    qDebug()<<m_strUploadFilePath;
    if(m_strUploadFilePath.isEmpty()){
        QMessageBox::warning(this,"upload file","the name is empty.");
    }else{
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName=m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
        QFile file(m_strUploadFilePath);
        qint64 fileSize=file.size();//文件大小
        QString strCurPath =TcpClient::getInstance().curPath();
        PDU *pdu=mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        sprintf(pdu->caData,"%s %11d",strFileName.toStdString().c_str(),fileSize);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;

        m_pTimer->start(1000);
    }
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"upload file","open the file failured");
        return ;
    }
    char *pBuffer =new char[4096];//4096的时候效率比较高
    qint64 ret=0;
    while(true){
        ret=file.read(pBuffer,4096);
        if(ret>0 && ret<=4096){
            TcpClient::getInstance().getTcpSocket().write(pBuffer,ret);
        }else if(ret==0){
            break;
        }else{
            QMessageBox::warning(this,"upload file","upload file faulures: read failured");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer=NULL;
}

void Book::downloadFile()
{

    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL==pItem){
        QMessageBox::warning(this,"download file","place chose file to download");
    }else{
        QString strSaveFilePath = QFileDialog::getSaveFileName();//这个会弹出一个窗口供选择。
        if(strSaveFilePath.isEmpty()){
            QMessageBox::warning(this,"download file","place chose the loaction to save");
            m_strSaveFilePath.clear();
        }else{
            m_strSaveFilePath = strSaveFilePath;

        }

        QString strCurPath =TcpClient::getInstance().curPath();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName = pItem->text();
        strcpy(pdu->caData,strFileName.toStdString().c_str());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;

    }
}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if(NULL==pItem){
        QMessageBox::warning(this,"share file","place chose file to share");
        return ;
    }else{
        m_strShareFileName = pItem->text();
    }
    Friend *pFriend = OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList = pFriend->getFriendList();
    ShareFile::getInstance().updateFriend(pFriendList);
    if(ShareFile::getInstance().isHidden()){
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(NULL!=pCurItem){
        m_strMoveFileName = pCurItem->text();
        QString strCutPath = TcpClient::getInstance().curPath();
        m_strMoveFilePath = strCutPath+"/"+m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);
    }else{
        QMessageBox::warning(this,"move file","place chose the file to move");
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(NULL!=pCurItem){
        QString strDestDir = pCurItem->text();
        QString strCutPath = TcpClient::getInstance().curPath();
        m_strDestDir = strCutPath+"/"+strDestDir;
        int srcLen = m_strMoveFilePath.size();
        int destLen =m_strDestDir.size();
        PDU *pdu= mkPDU(srcLen+destLen+2);
        pdu->uiMsgType=ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str());
        memcpy(pdu->caMsg,m_strMoveFilePath.toStdString().c_str(),m_strMoveFilePath.size());
        memcpy((char*)(pdu->caMsg)+srcLen+1,m_strDestDir.toStdString().c_str(),destLen);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::warning(this,"move dir","place chose the dir to move");
    }
    m_pSelectDirPB->setEnabled(false);
}


