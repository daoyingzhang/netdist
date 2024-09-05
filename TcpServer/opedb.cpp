#include "opedb.h"


OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db=QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;//在静态的成员函数里边定义静态的成员变量。
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("E:\\qt_wangpan\\TcpServer/cloud.db");
    if(m_db.open()){
        QSqlQuery query;
        query.exec("select * from userInfor");
        while(query.next()){
            QString data=QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug()<<data;
        }
    }else{
        qDebug()<<"fiale";
    }
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(NULL==name || NULL==pwd){
        return false;
    }
    QString data=QString("insert into userInfor(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);//如果执行失败，可能是name表里有了，所以会返回false。
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(NULL==name || NULL==pwd){
        return false;
    }
    QString data=QString("select * from userInfor where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        data=QString("update userInfor set online=1 where name = \'%1\' and pwd = \'%2\'").arg(name).arg(pwd);
        QSqlQuery query;
        query.exec(data);
        return true;
    }else{
        return false;
    }

}

void OpeDB::handleOffline(const char *name)
{
    if(NULL!=name){
        QString data=QString("update userInfor set online=0 where name = \'%1\'").arg(name);
        QSqlQuery query;
        query.exec(data);
    }else{
        qDebug()<<"name is null";
    }
}

QStringList OpeDB::handleAllOnline()
{
    QString data=QString("select name from userInfor where online=1");
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    while(query.next()){
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if(NULL==name){
        return -1;
    }
    QString data=QString("select online from userInfor where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        int ret=query.value(0).toInt();
        if(1==ret){
            return 1;
        }else if(0==ret){
            return 0;
        }
    }else{
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(NULL==pername || NULL==name){
        return -1;
    }
    qDebug()<<pername<<name;
    QString data=QString("select * from friend where (id=(select id from userInfor where name=\'%1\') "
                           "and friendId = (select id from userInfor where name=\'%2\' ))"
                           "or (id=(select id from userInfor where name=\'%3\') "
                           "and friendId = (select id from userInfor where name=\'%4\' ))").arg(pername).arg(name).arg(name).arg(pername);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        return 0;//双方已经是好友
    }else{
        QString data=QString("select online from userInfor where name=\'%1\'").arg(pername);
        QSqlQuery query;
        query.exec(data);
        if(query.next()){
            int ret=query.value(0).toInt();
            if(1==ret){
                return 1;//不是好友且在线。
            }else if(0==ret){
                return 2;//不在线
            }
        }else{
            return 3;//表示不存在这个人。
        }
    }
}
void OpeDB::handleAddRelationship(const char *friendName, const char *myName)
{
    if(NULL==friendName||NULL==myName)  return;
    QString selectFridendId=QString("select id from userInfor where name=\'%1\'").arg(friendName);
    QString selectMyId=QString("select id from userInfor where name=\'%1\'").arg(myName);
    QString sqlStatement=QString("insert into friend (id,friendId) values ((%1),(%2))").arg(selectMyId).arg(selectFridendId);
    QSqlQuery query;
    query.exec(sqlStatement);//执行插入
}

QStringList OpeDB::handleFlushFriend(const char *name)
{

    QStringList strFriendList;
    strFriendList.clear();
    if(NULL==name){
        return strFriendList;
    }
    QString data=QString("select name from userInfor where online=1 and id in (select id from friend where friendId in "
                           "(select id from userInfor where name=\'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(data);
    while (query.next()){
        strFriendList.append(query.value(0).toString());
        qDebug()<<query.value(0).toString();
    }
    data=QString("select name from userInfor where online=1 and id in (select friendId from friend where id in "
                   "(select id from userInfor where name=\'%1\'))").arg(name);
    query.exec(data);
    while (query.next()){
        strFriendList.append(query.value(0).toString());
        qDebug()<<query.value(0).toString();
    }
    return strFriendList;
}

bool OpeDB::handleDelFriend(const char *name, const char *friendname)
{
    if(NULL == name || NULL==friendname){
        return false;
    }
    QString data=QString("delete from friend where id in (select id from userInfor where name =\'%1\') "
                           "and  friendId = (select id from userInfor where name = \'%2\')")
                       .arg(name).arg(friendname);
    QSqlQuery query;
    query.exec(data);

    data=QString("delete from friend where id in (select id from userInfor where name =\'%1\') "
                   "and  friendId = (select id from userInfor where name = \'%2\')")
               .arg(friendname).arg(name);

    query.exec(data);
    return true;
}


OpeDB::~OpeDB(){
    m_db.close();
}


