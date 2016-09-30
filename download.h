#ifndef DOWNLOAD_H
#define DOWNLOAD_H

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
#include "utility.h"
#include <qurl.h>



enum DOWNLOAD_STATE {
    STATE_DOWNLOAD_INIT=0,
    STATE_WAIT_MODEM_CONNECTED,
    STATE_DOWNLOAD_WAIT1,
    STATE_DOWNLOAD_OK,
    STATE_DOWNLOAD_ERROR

};



class TDownload : public QObject
{
    Q_OBJECT
public:
    explicit TDownload(QObject *parent = 0);


    QUrl url;
    QNetworkAccessManager *network;
    QNetworkReply *Testreply;
    bool StartUpgrade;
    void Init(TimerMan *t);

    quint8 Go(char * server, char * username, char * password,char *path_server,char *name, quint16 porta);

    void PrintObj();

signals:

public slots:

    void DownloadProgress(qint64, qint64);
    void CloseDownloadFtp (QNetworkReply *reply);
    void Run();

private:
    TimerMan *pREGTIMER;
    QTimer TimerRun;
    AppTimer TimerDownload;
    quint8 StateRun;
    quint16 closeResult;
    char FtpAddress[MAX_LEN_CAMPO+1];
    char FtpUser[MAX_LEN_CAMPO+1];
    char FtpPsw[MAX_LEN_CAMPO+1];
    char FtpPath[MAX_LEN_CAMPO+1];
    char FtpFile[MAX_LEN_CAMPO+1];
    quint16 FtpPort;
    quint8  FTPResult;

};

#endif // DOWNLOAD_H
