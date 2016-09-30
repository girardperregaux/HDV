#ifndef FTPRUN_H
#define FTPRUN_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "mcdatetime.h"
#include <apptimer.h>
#include "timerman.h"
#include <qfile.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtNetwork>

#include <qurl.h>
#define PATH_FILE_MAX 50
#define FILE_NAME_MAX 20


class ftprun : public QObject
{
    Q_OBJECT
public:
    explicit ftprun(QObject *parent = 0);
    AppTimer TimerUpgrade;

     QUrl url;
     QFile *Fileftp;
     QNetworkRequest ft_prequest;
     QNetworkAccessManager *network;
     QNetworkReply *Testreply;
     char FilepathServer[PATH_FILE_MAX+1];
     char FilepathRemHdv[PATH_FILE_MAX+1];
     char NameFile[FILE_NAME_MAX+1];

     QFile *fileDownload;

 signals:

 public slots:
     void Init(TimerMan *t);
     void Set_ftp(QString server, QString username, QString password,char *path, int porta);
     void FtpUpload();
     void FtpDownload();
     void CloseDownloadFtp (QNetworkReply *reply);
     void slotFinishedUpload();
     void slotFinishedDownload();
     void slotError(QNetworkReply::NetworkError e);
     void uploadProgress(qint64, qint64);
     void DownloadProgress(qint64, qint64);
     void Downloadftp(QNetworkReply *reply);

 private:
    TimerMan *pREGTIMER;

 };

 #endif // TUPGRADE_H
