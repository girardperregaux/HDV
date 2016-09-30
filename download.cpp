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
#include "mcmodem.h"
#include "download.h"
#include "GestioneEventi.h"

extern MCSerial SerialPrint;
extern char buff_print[];
extern TConfig Configuration;
extern MCModem Modem;
extern GestioneEventi GEventi;


TDownload::TDownload(QObject *parent) :
    QObject(parent)
{

}




void TDownload::Init(TimerMan *t)
{
    pREGTIMER=t;
    TimerDownload.Init((char*)"Download");
    pREGTIMER->RegisterTimer(&TimerDownload); //timer registrato

    TimerDownload.SetTimer(UN_SECONDO*1);

    connect(&TimerRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati

    TimerRun.setInterval(100);
    TimerRun.start();

    StartUpgrade=false;
    StateRun=STATE_DOWNLOAD_INIT;
    Modem.ECMReqDownload=false;
}


//macchina a stati
void TDownload::Run()
{
    char PathFileServer[MAX_LEN_CAMPO+1];


    switch(StateRun)
    {
        case STATE_DOWNLOAD_INIT :
            Modem.ECMReqDownload=false;

            if(StartUpgrade)
            {
                SetWorkPeriod(ID_CPU,30,true);

                strcpy(Modem.APNReqDownload,Configuration.Config.APN);
                Modem.ECMReqDownload=true;

                strcpy(PathFileServer,FtpPath);
                strcat(PathFileServer,FtpFile);

                url.setScheme("ftp");
                url.setHost(FtpAddress);
                url.setUserName(FtpUser);
                url.setPassword(FtpPsw);
                url.setPath(PathFileServer);
                url.setPort(FtpPort);

                network=new QNetworkAccessManager(this);

                StateRun=STATE_WAIT_MODEM_CONNECTED;
                FTPResult=RESULT_NULL;
                TimerDownload.SetTimer(UN_SECONDO*45);
                //StateDownload=RESULT_NULL;

            }
        break;
        case STATE_WAIT_MODEM_CONNECTED :
            if(TimerDownload.IsElapsed())
            {
                StateRun=STATE_DOWNLOAD_ERROR;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWN: ABORT PER TIMEOUT CONNESSIONE MODEM\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                break;
            }

            if(GetState(MODEM_CONNECTED))
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWN: RUNNING...\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                StateRun=STATE_DOWNLOAD_WAIT1;
                Testreply = network->get(QNetworkRequest(url));
                connect(network, SIGNAL(finished(QNetworkReply*)),this, SLOT(CloseDownloadFtp(QNetworkReply*)));
                connect(Testreply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(DownloadProgress(qint64, qint64))); //test
                TimerDownload.SetTimer(UN_SECONDO*30);
            }
        break;

        case STATE_DOWNLOAD_WAIT1 :

            SetWorkPeriod(ID_CPU,30,true);
            if(TimerDownload.IsElapsed() || (FTPResult==RESULT_ERROR))
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWN: NON RIUSCITO\r\n"); //ok
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateRun=STATE_DOWNLOAD_ERROR;
            }
            else if(FTPResult==RESULT_OK)
            {

                StateRun=STATE_DOWNLOAD_OK;
            }

        break;

        break;
        case STATE_DOWNLOAD_ERROR:
            Modem.ECMReqDownload=false;
            StateRun=STATE_DOWNLOAD_INIT;
            StartUpgrade=false;
            strncpy(FtpAddress,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpUser,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpPsw,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpPath,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpFile,STR_NO,MAX_LEN_CAMPO);
            FtpPort=0;

        break;
        case STATE_DOWNLOAD_OK:
            Modem.ECMReqDownload=false;
            StateRun=STATE_DOWNLOAD_INIT;
            StartUpgrade=false;
            strncpy(FtpAddress,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpUser,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpPsw,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpPath,STR_NO,MAX_LEN_CAMPO);
            strncpy(FtpFile,STR_NO,MAX_LEN_CAMPO);
            FtpPort=0;
            GEventi.AvvisoAggiornamento();

        break;


        default:
        break;
    }

}

quint8 TDownload::Go(char * server, char * username, char * password,char *path_server,char *name, quint16 porta)
{
snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWN: GO\r\n");
SerialPrint.Flush(PRINT_DEBUG_FUNC);


    if((strcasecmp(server,STRNULL)==0)||
      (strcasecmp(username,STRNULL)==0)||
      (strcasecmp(password,STRNULL)==0)||
      (strcasecmp(name,STRNULL)==0)||
      (porta==0)) return(false);

    strncpy(FtpAddress,server,MAX_LEN_CAMPO);
    strncpy(FtpUser,username,MAX_LEN_CAMPO);
    strncpy(FtpPsw,password,MAX_LEN_CAMPO);
    strncpy(FtpPath,path_server,MAX_LEN_CAMPO);
    strncpy(FtpFile,name,MAX_LEN_CAMPO);
    FtpPort=porta;

    StartUpgrade=true;

    return(true);
}



void TDownload::DownloadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    qint64 a, b;

    a = bytesSent;
    b = bytesTotal;

    b = a;
    a = b;

    TimerDownload.SetTimer(UN_SECONDO*30); //ricarico 30 secondi
}



void TDownload::CloseDownloadFtp (QNetworkReply *reply)
{
    QFile file;
    char PathFileRemHdv[MAX_LEN_CAMPO+1];

    if(reply->error())
    {
        const char *str;
        QByteArray ba;
        ba = reply->errorString().toLatin1();
        closeResult =reply->error();

        str = ba.data();
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWN: ERRORE =(%d) %s\r\n",closeResult,str);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        FTPResult=RESULT_ERROR;
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWN: RIUSCITO\r\n"); //ok
        SerialPrint.Flush(PRINT_DEBUG_FUNC);


        strcpy(PathFileRemHdv,PATH_REM_HDV_RUN);
        strcat(PathFileRemHdv,FtpFile);

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWN: SALVO IL FILE IN %s\r\n",PathFileRemHdv); //ok
        SerialPrint.Flush(PRINT_DEBUG_FUNC);

        file.setFileName(PathFileRemHdv);
        file.remove();

        file.setFileName(PathFileRemHdv);

        //QFile *file = new QFile(PathFileRemHdv);
        if(file.open(QFile::WriteOnly))
        {
            file.write(reply->readAll());
            file.flush();
            file.close();
        }
        file.destroyed();

        FTPResult=RESULT_OK;
    }
    reply->deleteLater();
}


void TDownload::PrintObj()
{

}



