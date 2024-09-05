#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QStringList>>
class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();//定义静态的成员函数。
    void init();
    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    void handleOffline(const char *name);
    QStringList handleAllOnline();
    int handleSearchUsr(const char *name);
    //这个地方跟视频里的不一样，需要变成静态的，否则没法使用。或者按照现在的做法
    int handleAddFriend(const char *pername,const char *name);
    void handleAddRelationship(const char *pername,const char *name);
    QStringList handleFlushFriend(const char *name);
    bool handleDelFriend(const char *name,const char *friendname);
    ~OpeDB();
signals:

public slots:
private:
    QSqlDatabase m_db;//用来连接数据库。
};

#endif // OPEDB_H
