#include "mytcpsocket.h"
// #include <QDebug>
#include "opedb.h"
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>



MyTcpSocket::MyTcpSocket()
{
    connect(this,SIGNAL(readyRead())
            ,this,SLOT(recvMsg()));
    connect(this,SIGNAL(disconnected())
            ,this,SLOT(clientOffline()));
    m_bUpload=false;
    m_pTimer = new QTimer;

    connect(m_pTimer,SIGNAL(timeout())
            ,this,SLOT(sendFileToClient()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);
    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();
    QString srcTmp;
    QString destTmp;
    for(int i=0;i<fileInfoList.size();i++){
        if(fileInfoList[i].isFile()){

            srcTmp = strSrcDir+"/"+fileInfoList[i].fileName();
            destTmp = strDestDir+"/"+fileInfoList[i].fileName();
            QFile::copy(srcTmp,destTmp);
        }else if(fileInfoList[i].isDir()){
            if(fileInfoList[i].fileName()==QString("..") || fileInfoList[i].fileName()==QString(".")) continue;
            srcTmp = strSrcDir+"/"+fileInfoList[i].fileName();
            destTmp = strDestDir+"/"+fileInfoList[i].fileName();
            copyDir(srcTmp,destTmp);
        }
    }
}

void MyTcpSocket::recvMsg()
{
    if(!m_bUpload){
        uint uiPDULen=0;
        this->read((char*)&uiPDULen,sizeof(uint));
        uint uiMsgLen=uiPDULen-sizeof(PDU);
        PDU *pdu=mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32]={'\0'};
            char caPwd[32]={'\0'};
            strncpy(caName,pdu->caData,32);
            strncpy(caPwd,pdu->caData+32,32);
            bool ret=OpeDB::getInstance().handleRegist(caName,caPwd);
            PDU *respdu=mkPDU(0);//64个字节足够了,重写创建一个pdu是为了发送返回消息。
            respdu->uiMsgType=ENUM_MSG_TYPE_REGIST_RESPOND;
            if(ret){
                //当返回值为真，需要回复客户端这个成功放入数据库。
                strcpy(respdu->caData,REGIST_OK);
                QDir dir;
                qDebug()<<dir.mkdir(QString("./%1").arg(caName));
            }else{
                strcpy(respdu->caData,REGIST_FAILED);
            }
            //这一步写完就可以发送回客户端。
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            //这个有点bug，就是没法区别，是数据库不存在还是数据库的这个用户已经登录了，返回客户端无法区分。
            char caName[32]={'\0'};
            char caPwd[32]={'\0'};
            strncpy(caName,pdu->caData,32);
            strncpy(caPwd,pdu->caData+32,32);
            bool ret = OpeDB::getInstance().handleLogin(caName,caPwd);

            PDU *respdu=mkPDU(0);//64个字节足够了,重写创建一个pdu是为了发送返回消息。
            respdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_RESPOND;
            if(ret){
                //当返回值为真，需要回复客户端这个成功放入数据库。
                strcpy(respdu->caData,LOGIN_OK);
                m_strName=caName;
            }else{
                strcpy(respdu->caData,LOGIN_FAILED);
            }
            //这一步写完就可以发送回客户端。
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList ret=OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen=ret.size()*32;
            PDU *respdu=mkPDU(uiMsgLen);
            respdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i=0;i<ret.size();i++){
                memcpy((char*)respdu->caMsg+i*32
                       ,ret.at(i).toStdString().c_str()
                       ,ret.at(i).size());
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            int ret=OpeDB::getInstance().handleSearchUsr(pdu->caData);
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if(-1==ret){
                strcpy(respdu->caData,SEARCH_USR_NO);

            }else if(1==ret){
                strcpy(respdu->caData,SEARCH_USR_Online);
            }else if(0==ret){
                strcpy(respdu->caData,SEARCH_USR_Offline);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caPerName[32]={'\0'};
            char caName[32]={'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
            int ret =OpeDB::getInstance().handleAddFriend(caPerName,caName);
            PDU *respdu=NULL;
            if(-1==ret){
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,UNKNOW_ERROR);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }else if(0==ret){
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,EXITED_FRIEND);
                write((char*)respdu,respdu->uiPDULen);
                qDebug()<<caPerName;
                free(respdu);
                respdu=NULL;
            }else if(1==ret){
                MyTcpServer::getInstance().resend(caPerName,pdu);
            }else if(2==ret){
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }else if(3==ret){
                respdu=mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caData,ADD_FRIEND_NO_EXIST);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE://同意了添加好友的请求
        {
            char caPerName[32]={'\0'};
            char caName[32]={'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
            //修改数据库
            //向发送方反馈消息
            OpeDB::getInstance().handleAddRelationship(caPerName,caName);
            MyTcpServer::getInstance().resend(caName,pdu);//转发给我
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE://拒绝了添加好友的请求
        {
            char caPerName[32]={'\0'};
            char caName[32]={'\0'};
            strncpy(caPerName,pdu->caData,32);
            strncpy(caName,pdu->caData+32,32);
            //直接向发送方反馈消息
            MyTcpServer::getInstance().resend(caName,pdu);//转发给我
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char caName[32]={'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList ret = OpeDB::getInstance().handleFlushFriend(caName);
            uint uiMsgLen=ret.size()*32;
            PDU *respdu=mkPDU(uiMsgLen);
            respdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for(int i=0;i<ret.size();i++){
                memcpy((char*)(respdu->caMsg)+i*32
                       ,ret.at(i).toStdString().c_str()
                       ,ret.at(i).size());
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caSelfName[32]={'\0'};
            char caFriendName[32]={'\0'};
            strncpy(caSelfName,pdu->caData,32);
            strncpy(caFriendName,pdu->caData+32,32);
            OpeDB::getInstance().handleDelFriend(caSelfName,caFriendName);
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            strcpy(respdu->caData,DEL_FRIEND_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;

            MyTcpServer::getInstance().resend(caFriendName,pdu);
            break;

        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            //进行私聊请求的转发
            char caPerName[32]={"\0"};
            memcpy(caPerName,pdu->caData+32,32);
            MyTcpServer::getInstance().resend(caPerName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char caName[32]={'\0'};
            strncpy(caName,pdu->caData,32);
            QStringList onlineFriend = OpeDB::getInstance().handleFlushFriend(caName);
            for(int i=0;i<onlineFriend.size();i++){
                MyTcpServer::getInstance().resend(onlineFriend[i].toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath=QString("%1").arg((char*)pdu->caMsg);
            bool ret=dir.exists(strCurPath);

            PDU *respdu;
            respdu = mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
            if(ret){//当前目录存在
                char caNewDir[32]={'\0'};
                memcpy(caNewDir,pdu->caData+32,32);
                QString strNewPath=strCurPath+"/"+caNewDir;
                qDebug()<<strNewPath<<"eeeee";
                ret=dir.exists(strNewPath);
                if(ret){//创建的文件名已经存在
                    strcpy(respdu->caData,FILE_NAME_EXIST);
                }else{//创建的文件名不存在
                    dir.mkdir(strNewPath);//创建新目录

                    strcpy(respdu->caData,CREAT_DIR_OK);
                }
            }else{//当前目录不存在
                strcpy(respdu->caData,DIR_NO_EXIST);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath=new char[pdu->uiMsgLen];
            memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen);
            QDir dir(pCurPath);
            QFileInfoList fileInfoList=dir.entryInfoList();
            int iFileCount = fileInfoList.size();
            PDU *respdu=mkPDU(sizeof(FileInfo)*(iFileCount));
            respdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            for(int i=0;i<iFileCount;i++){
                pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                strFileName = fileInfoList[i].fileName();
                memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.size());
                if(fileInfoList[i].isDir()){
                    pFileInfo->iFileType=0;
                }else if(fileInfoList[i].isFile()){
                    pFileInfo->iFileType=1;
                }
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[32]={'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath<<"222111";

            QFileInfo fileInfo(strPath);
            bool ret =false;
            if(fileInfo.isDir()){
                QDir dir;
                dir.setPath(strPath);
                ret=dir.removeRecursively();//这个会删除这个文件夹下的所有子文件夹
            }
            else if(fileInfo.isFile()){
                ret=false;
            }
            PDU *respdu=NULL;
            if(ret){
                respdu=mkPDU(1);
                memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            }else{
                respdu=mkPDU(1);
                memcpy(respdu->caData,DEL_DIR_FAILUED,strlen(DEL_DIR_FAILUED));
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            char caOldName[32]={'\0'};
            char caNewName[32]={'\0'};
            strncpy(caOldName,pdu->caData,32);
            strncpy(caNewName,pdu->caData+32,32);
            char* pPath=new char[pdu->uiMsgLen];
            memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            QString strOldPath=QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath=QString("%1/%2").arg(pPath).arg(caNewName);

            qDebug()<<strOldPath<<"old";
            qDebug()<<strNewPath<<"new";
            QDir dir;
            bool ret=dir.rename(strOldPath,strNewPath);
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if(ret){
                strcpy(respdu->caData,RENAME_FILE_OK);
            }else{
                strcpy(respdu->caData,RENAME_FILE_FAILUED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caEnterName[32]={'\0'};
            strncpy(caEnterName,pdu->caData,32);
            char* pPath=new char[pdu->uiMsgLen];
            memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caEnterName);
            QFileInfo fileInfo(strPath);
            PDU *respdu = NULL;
            if(fileInfo.isDir()){
                QDir dir(strPath);
                QFileInfoList fileInfoList=dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                PDU *respdu=mkPDU(sizeof(FileInfo)*(iFileCount));
                respdu->uiMsgType=ENUM_MSG_TYPE_ENTER_DIR_RESPOND;//这里直接使用flush的
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                for(int i=0;i<iFileCount;i++){
                    pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                    strFileName = fileInfoList[i].fileName();
                    memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.size());
                    if(fileInfoList[i].isDir()){
                        pFileInfo->iFileType=0;
                    }else if(fileInfoList[i].isFile()){
                        pFileInfo->iFileType=1;
                    }
                }
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;

            }
            else if(fileInfo.isFile()){
                respdu = mkPDU(0);
                respdu->uiMsgType=ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData,ENTER_DIR_FAILURED);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char caFileName[32]={'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caData,"%s %11d",caFileName,&fileSize);
            char* pPath=new char[pdu->uiMsgLen];
            memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            qDebug()<<strPath;
            delete []pPath;
            pPath=NULL;

            m_file.setFileName(strPath);
            //以只写的方式打开文件，若文件打不开，则会自动创建文件
            if(m_file.open(QIODevice::WriteOnly)){
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
        {
            char caName[32]={'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caName);
            qDebug()<<strPath;

            QFileInfo fileInfo(strPath);
            bool ret =false;
            if(fileInfo.isDir()){
                ret=false;

            }
            else if(fileInfo.isFile()){
                QDir dir;
                ret=dir.remove(strPath);//这个会删除这个文件
            }
            PDU *respdu=NULL;
            if(ret){
                respdu=mkPDU(0);
                memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK));
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_RESPOND;
            }else{
                respdu=mkPDU(0);
                memcpy(respdu->caData,DEL_FILE_FAILUED,strlen(DEL_FILE_FAILUED));
                respdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_RESPOND;
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char caName[32]={'\0'};
            strcpy(caName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath=QString("%1/%2").arg(pPath).arg(caName);
            delete [] pPath;
            pPath=NULL;

            QFileInfo fileInfo(strPath);
            qint64 fileSize=fileInfo.size();
            PDU *respdu=mkPDU(0);
            sprintf(respdu->caData,"%s %11d",caName,fileSize);
            respdu->uiMsgType=ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char caSendName[32]={'\0'};
            int num=0;
            sscanf(pdu->caData,"%s %d",caSendName,&num);
            int size=num*32;
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caData,caSendName);
            memcpy(respdu->caMsg,(char*)(pdu->caMsg)+size,pdu->uiMsgLen-size);
            char caRecvName[32]={'\0'};
            for(int i=0;i<num;i++){
                memcpy(caRecvName,(char*)(pdu->caMsg)+i*32,32);
                MyTcpServer::getInstance().resend(caRecvName,respdu);
            }
            free(respdu);
            respdu = mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData,"share file ok");
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {
            QString strRecvPath = QString("./%1").arg(pdu->caData);
            QString strShareFilePath = QString("%1").arg((char*)pdu->caMsg);
            int index = strShareFilePath.lastIndexOf('/');
            QString strFileName=strShareFilePath.right(strShareFilePath.size()-index-1);
            strRecvPath=strRecvPath+"/"+strFileName;

            QFileInfo fileInfo(strShareFilePath);
            if(fileInfo.isFile()){
                QFile::copy(strShareFilePath,strRecvPath);
            }else if(fileInfo.isDir()){
                copyDir(strShareFilePath,strRecvPath);
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            char caFileName[32] = {"\0"};
            int srcLen=0;
            int destLen=0;
            sscanf(pdu->caData,"%d %d %s",&srcLen,&destLen,caFileName);
            char *pSrcPath = new char[srcLen+1];
            char *pDestPath = new char[destLen+1+32];
            memcpy(pSrcPath,"\0",srcLen+1);
            memcpy(pDestPath,"\0",destLen+1);
            memcpy(pSrcPath,pdu->caMsg,srcLen);
            memcpy(pDestPath,(char*)(pdu->caMsg)+(srcLen+1),destLen);

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType =ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            QFileInfo fileInfo(pDestPath);
            if(fileInfo.isDir()){
                strcat(pDestPath,"/");
                strcat(pDestPath,caFileName);
                bool ret = QFile::rename(pSrcPath,pDestPath);
                if(ret){
                    strcpy(pdu->caData,MOVE_FILE_OK);
                }else{
                    strcpy(pdu->caData,COMON_ERROR);
                }
            }else if(fileInfo.isFile()){
                strcpy(pdu->caData,MOVE_FILE_FAILUED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu=NULL;
    }
    else{//这个是接受数据。

        QByteArray buff=readAll();
        m_file.write(buff);
        m_iRecved+=buff.size();
        if(m_iTotal==m_iRecved){
            m_file.close();
            m_bUpload=false;
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(respdu->caData,UPLOAD_FILE_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }else if(m_iTotal<m_iRecved){
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
            strcpy(respdu->caData,UPLOAD_FILE_FAILURED);
            m_file.close();
            m_bUpload=false;
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }

    }
    // qDebug()<<caName<<caPwd<<pdu->uiMsgType;
}

void MyTcpSocket::clientOffline()
{

    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);//发送信号，使得signals处的函数可以调用。

}

void MyTcpSocket::sendFileToClient()
{
    char *pData = new char[4096];
    qint64 ret = 0;
    while(true){
        ret = m_file.read(pData,4096);
        if(ret>0 && ret <= 4096){
            write(pData,ret);
        }else if(0==ret){
            m_file.close();
            break;
        }else if(ret<0){
            qDebug()<<"send to client is failured";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData=NULL;
}
