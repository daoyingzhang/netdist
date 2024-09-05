#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include <QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu);
    void clearEnterDir();
    QString enterDir();
    void setDownloadStatus(bool status);
    bool getDownloadStatus();
    QString getSaveFilePath();
    QString getShareFileName();
    qint64 m_iTotal;
    qint64 m_iReceive;

signals:
    void offline();
public slots:
    void creatDir();
    void flushFile();
    void delDir();
    void delFile();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPre();
    void uploadFile();
    void uploadFileData();
    void downloadFile();
    void shareFile();
    void moveFile();
    void selectDestDir();




private:
    QListWidget *m_pBookListW;

    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QString m_strEnterDir;
    QString m_strUploadFilePath;
    QTimer *m_pTimer;
    QString m_strSaveFilePath;
    bool m_bDownload;
    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;
};

#endif // BOOK_H
