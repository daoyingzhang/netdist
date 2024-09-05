#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QButtonGroup>
#include <QListWidget>
class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);
    static ShareFile &getInstance();
    void updateFriend(QListWidget *pFriendList);

signals:
public slots:
    void cancelSelect();
    void selectAll();
    void okShare();
    void cancel();

private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCancelSelectPB;

    QPushButton *m_pOKPB;
    QPushButton*m_pCancelPB;
    QScrollArea *m_pSA;
    QWidget *m_pFriendw;
    QVBoxLayout *m_pFriendWVBL;
    QButtonGroup *m_pButtonGroup;
};

#endif // SHAREFILE_H
