
#include "ftprun.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QDebug>
#include <string.h>
#include <qstring.h>
#include <QFile>
#include <qdir.h>
#include "mcdatetime.h"
#include "config.h"
#include "tstate.h"
#include <QFile>
#include <string.h>
#include <tstate.h>

extern MCSerial SerialPrint;
extern char buff_print[];


#define PATH_SERVER "/mcsistemi.it/Test/ste/"
#define PATH_SERVER_PRO "/mcsistemi.it/fwREMHDV/"


ftprun::ftprun(QObject *parent) :
    QObject(parent)
{

}

void ftprun::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerUpgrade.Init((char*)"TimerUpgrade");
    TimerUpgrade.SetTimer(UN_SECONDO*1);

    pREGTIMER->RegisterTimer(&TimerUpgrade); //timer registrato


}

void ftprun::Set_ftp(QString server, QString username, QString password,char *path, int porta)
{

    strcpy(NameFile,path);

    strcpy(FilepathServer,PATH_SERVER_PRO);
    strcat(FilepathServer,NameFile);



    url.setScheme("ftp");
    url.setHost(server);
    url.setUserName(username);
    url.setPassword(password);
    url.setPath(FilepathServer);
    url.setPort(porta);

    network=new QNetworkAccessManager(this);
}

void ftprun::FtpUpload()
{

    strcpy(FilepathRemHdv,PATH_REM_HDV_RUN);
    strcat(FilepathRemHdv,NameFile);

    Fileftp =new QFile(FilepathRemHdv,this);

    if (Fileftp->open(QIODevice::ReadOnly))
    {
        ft_prequest.setUrl(url);
        Testreply = network->put(ft_prequest,Fileftp); //test
        connect(Testreply, SIGNAL(finished()), this, SLOT(slotFinishedUpload()));//test
        connect(Testreply, SIGNAL(error(QNetworkReply::NetworkError)),this, SLOT(slotError(QNetworkReply::NetworkError)));//test
        connect(Testreply, SIGNAL(uploadProgress(qint64, qint64)), SLOT(uploadProgress(qint64, qint64))); //test
    }
}

void ftprun::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    double temp;

    temp=(double)bytesSent/(double)bytesTotal;

    if(temp>0)
    {
        temp*=100;
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTP=Uploading %% %0.1f\r\n",temp); //ok
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }
}

void ftprun::slotFinishedUpload() {

    Fileftp->close();
    qDebug() << "slotFinished";
    qDebug() << Testreply->readAll();
    Testreply->deleteLater();
}

void ftprun::slotError(QNetworkReply::NetworkError e)
{
    qDebug() << "slotError" << e ;
    Fileftp->close();

}




void ftprun::FtpDownload()
{

    strcpy(FilepathRemHdv,PATH_REM_HDV_RUN);
    strcat(FilepathRemHdv,NameFile);
    fileDownload=new QFile(FilepathRemHdv,this);

    Testreply = network->get(QNetworkRequest(url));//test
    connect(network, SIGNAL(finished(QNetworkReply*)),this, SLOT(CloseDownloadFtp(QNetworkReply*)));
    connect(Testreply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(DownloadProgress(qint64, qint64))); //test

   // connect(Testreply, SIGNAL(readyRead()), SLOT(Downloadftp(QNetworkReply*))); //test


}

void ftprun::Downloadftp(QNetworkReply* reply)
{
    if(fileDownload->isOpen())
    {
        fileDownload->write(reply->readAll());
    }

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTP=scrivo su file\r\n"); //ok
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
}



void ftprun::DownloadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    double temp;

    temp=(double)bytesSent/(double)bytesTotal;

    if(temp>0)
    {
        temp*=100;
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTP=Download %% %0.1f\r\n",temp); //ok
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }

}

void ftprun::slotFinishedDownload() {

    Fileftp->close();
    qDebug() << "slotFinished";
    qDebug() << Testreply->readAll();
    Testreply->deleteLater();
}




void ftprun::CloseDownloadFtp (QNetworkReply *reply)
{

    if(reply->error())
    {
        const char *str;
        QByteArray ba;
        ba = reply->errorString().toLatin1();
        str = ba.data();
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTP= errore Download= %s\r\n",str);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);

    }
    else
    {
        strcpy(FilepathRemHdv,PATH_REM_HDV_RUN);
        strcat(FilepathRemHdv,NameFile);




        QFile *file = new QFile(FilepathRemHdv);
        if(file->open(QFile::Append))
        {
            file->write(reply->readAll());
            file->flush();
            file->close();
        }
        delete file;

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*FTP= Download file riuscito\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }
    reply->deleteLater();

}




