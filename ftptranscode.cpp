
#include "ftptranscode.h"
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
#include <qstringlist.h>
#include "utility.h"
#include "DefCom.h"
#include "mcgestionefs.h"
#include "mcfspace.h"
#include "vidaudiorec.h"
#include "ftpfast.h"
#include "mctranscode.h"

#define TRANSC_DATE_INIT    "000101"
#define TRANSC_HOUR_INIT    "000000"
#define TRANSC_DATE_FINISH  "990101"
#define TRANSC_HOUR_FINISH  "235959"
#define SIZE_TRANSC_100MB    100000

extern TConfig Configuration;
extern MCSerial SerialPrint;
extern char buff_print[];
extern Mcgestionefs GestioneFs;
extern mcfspace FSpace;
extern vidaudiorec Videoaudio;
extern ftpfast FtpFastRec;
extern mctranscode transcode;


ftptranscode::ftptranscode(QObject *parent) :
    QObject(parent)
{
    Gsttrasc= new QProcess(this);

}
void ftptranscode::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerTranscode.Init((char*)"transcode");
    TimerTranscode.SetTimer(UN_SECONDO*1);


    TimerDelayTranscode.Init((char*)"Delaytranscode");
    TimerDelayTranscode.SetTimer(UN_SECONDO*1);

    pREGTIMER->RegisterTimer(&TimerTranscode); //timer registrato

    TTranscodeRun.setInterval(100);
    TTranscodeRun.start();
    connect(&TTranscodeRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati
    StateTranscode=STATE_TRANS_NULL;
    startTranscode=false;
    UpgradeTranscode=false;
    MCFindTranscode.Init(MCFINDERFTPTRANSCODE);
    MCFindTranscode.Reset_Init();
    indexTranscode=0;
    countlog=0;
    Transc_sleep=false;
    FileFastTra=false;
    EndTranscode=false;


    Start_Time_transcoder=false;
    Time_transcode=0;
    QuantTrsF=0;
    FrameTrsF=0;
    BitrateTrsF=0;
}

bool ftptranscode::transc_rename(char * Tfile,quint8 formato)
{
    quint16 temp=0;
    QFile fileTranscode(Tfile);



    if(fileTranscode.exists())
    {
        strcpy(StrRenameTra,Tfile);
        temp=strlen(StrRenameTra);
        if(temp>0)
        {
            if(formato==TYPE_MP4)
            {
                StrRenameTra[temp-1]='4';
            }
            else if(formato==TYPE_TZ)
            {
                StrRenameTra[temp-1]='z';
            }
            else if(formato==TYPE_TX)
            {
                StrRenameTra[temp-1]='x';
            }
            else if(formato==TYPE_TY)
            {
                StrRenameTra[temp-1]='y';
            }

        }

        QFile::rename(Tfile,StrRenameTra); //cambio
        return (true);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS:Error file save : %s\r\n",StrBuffTemp);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (false);
    }

}
bool ftptranscode::transc_remove(char * Tfile)
{
    QFile fileTranscode(Tfile);

    if(fileTranscode.exists())
    {
        if(fileTranscode.remove())
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS:file cancellato : %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (true);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS:Error file remove : %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (false);
        }
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS:Error file non esiste : %s\r\n",Tfile);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }
    return (false);

}

bool ftptranscode::FileReady(char *FileFtp)
{
    if(strcasecmp(FileFtp,STR_NO)==0)return false;

    strcpy(StrBuffTemp,FileFtp);
    strcpy(StrBuffTra,FileFtp);
    return true;
}


//macchina a stati
void ftptranscode::Run()
{
    int temp;
    qint64 timesize;

    if(Start_Time_transcoder)
    {
        Time_transcode++; //ogni 10 ms incremento
    }

    //if(!startTrancode) StateTrancode=STATE_TRANS_NULL;

    switch(StateTranscode)
    {
        case STATE_TRANS_NULL :

            if(UpgradeTranscode)
            {
                //if(!GestioneFs.MsataIsPresent)break;  //se hard disk non presente
                //if(FSpace.FirstSpaceFs!=SPACE_FS_OK)break; //se non ho ancora finito di cancellare i file all'avvio .tw e .mpw esco
                UpgradeTranscode=false;

                startTranscode=true;

                TimerTranscode.SetTimer(0);
                StateTranscode=STATE_TRANS_WAIT;
            }
            else if(EndTranscode){
                EndTranscode=false;
                FtpFastRec.Ftp_sleep=true; //spengo non ho piu' file da trasferire

                //qui ritento con transcoder nel caso ci siano file non transcodificati
                transcode.UpgradeTranscode=true;

                SetState(TRANSCODE_RUN,false);
            }

        break;
        case STATE_TRANS_WAIT :

            if(Transc_sleep) //fermo e mi porto in sleep
            {
                Transc_sleep=false;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS : SLEEP\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTranscode=STATE_TRANS_NULL;
                SetState(TRANSCODE_RUN,false);
                startTranscode=false;
                break;
            }

          //  if(!TimerDelayTranscode.IsElapsed())break;

            if(FileReady(Videoaudio.file_ftpfast))
            {

                Start_Time_transcoder=true;
                Time_transcode=0;
                FreeCache(false);
                temp=strlen(StrBuffTemp);

                if(StrBuffTemp[temp-13]=='_')
                {
                    StrQuantF[0]=StrBuffTemp[temp-12]; //per quantizzazione
                    StrQuantF[1]=StrBuffTemp[temp-11]; //7
                    StrQuantF[2]=0;
                    QuantTrsF=atoi(StrQuantF);

                    StrFrameF[0]=StrBuffTemp[temp-9]; //per frame
                    StrFrameF[1]=StrBuffTemp[temp-8];
                    StrFrameF[2]=0;

                    FrameTrsF=atoi(StrFrameF);

                    StrBitrateF[0]=StrBuffTemp[temp-6];
                    StrBitrateF[1]=StrBuffTemp[temp-5];
                    StrBitrateF[2]=StrBuffTemp[temp-4];
                    BitrateTrsF=atoi(StrBitrateF);


                    StrBuffTemp[temp-13]='.';          //tolgo quantizzazione e frame
                    StrBuffTemp[temp-12]='m';
                    StrBuffTemp[temp-11]='p';
                    StrBuffTemp[temp-10]='w';
                    StrBuffTemp[temp-9]=0;
                }
                else
                {
                    FrameTrsF=0;
                    QuantTrsF=0;
                    BitrateTrsF=32; //default
                    StrBuffTemp[temp-2]='m';
                    StrBuffTemp[temp-1]='p';  //ora cambio da ts a mpw
                    StrBuffTemp[temp]='w';
                    StrBuffTemp[temp+1]=0;
                }

                memset(Buffstrasc,CHARNULL,STRING_MAX_BUFFER_TRANSC);
                pBufftrasc=&Buffstrasc[0];

                pBufftrasc+=sprintf(pBufftrasc,"-v -e -T filesrc location=%s ! decodebin async-handling=true name=demux { mp4mux name=mux ",&StrBuffTra[0]);
                pBufftrasc+=sprintf(pBufftrasc,"! filesink location=%s } { demux. ! queue ! audioconvert ! ffenc_aac bitrate=%d000 ",&StrBuffTemp[0],BitrateTrsF);

                if((QuantTrsF>=20)&&(QuantTrsF<=40))  //se ce' la quantizzazione sul nome del file
                {
                    pBufftrasc+=sprintf(pBufftrasc,"! queue ! mux. } { demux. ! queue ! vpuenc codec=6 cbr=false force-framerate=true framerate-nu=%d quant=%d ! mux. } ",FrameTrsF,QuantTrsF);

                }
                else
                {
                    pBufftrasc+=sprintf(pBufftrasc,"! queue ! mux. } { demux. ! queue ! vpuenc codec=6 ! mux. }");
                }

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS : %s\r\n",&Buffstrasc[0]);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                argtrasc=QString(Buffstrasc).split(" ");

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: Start transcoder fast\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                Gsttrasc->start("gst-launch", argtrasc);
                PidGstTransc = Gsttrasc->pid();

                //sprintf(StrPidTra," kill -9 %d",PidGstTransc);


                StateTranscode=STATE_TRANS_TRANSCODE;


                TimerTranscode.SetTimer(UN_SECONDO*20);
                indexTranscode++;
                SetState(TRANSCODE_RUN,true);

            }


        break;
        case STATE_TRANS_TRANSCODE :


            if(TimerTranscode.IsElapsed())
            {
                StateTranscode=STATE_TRANS_WAIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: TIMEOUT TRANSCOD FILE= %s\r\n",StrBuffTemp);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                Gsttrasc->close();
                TimerTranscode.SetTimer(UN_SECONDO*2);
               // TimerDelayTranscode.SetTimer(UN_SECONDO*2);
                FreeCache(false);
            }

            else if(Gsttrasc->state()==QProcess::NotRunning)
            {

                QFile fileTemp(StrBuffTemp);

                if (!fileTemp.open(QIODevice::ReadOnly))
                {
                   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: Errore apertura file%s\r\n",StrBuffTemp);
                   SerialPrint.Flush(PRINT_DEBUG_FUNC);
                   Start_Time_transcoder=false;
                   Time_transcode*=10;
                   StateTranscode=STATE_TRANS_NULL;
                   break;
                }

                qint64 size = (qint64 )fileTemp.size();

                fileTemp.close();

                if(size==0)
                {
                     snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: ++++++++++ File %s transcodificato a zero ++++++++++\r\n",StrBuffTemp);
                     SerialPrint.Flush(PRINT_DEBUG_ALL);
                     if(GetState(DEL_TS))fileTemp.remove();
                     Start_Time_transcoder=false;
                     Time_transcode*=10;
                     StateTranscode=STATE_TRANS_NULL;
                 //    TimerDelayTranscode.SetTimer(UN_SECONDO*2);
                     FreeCache(false);
                     break;
                }


                transc_rename(StrBuffTemp,TYPE_MP4); //da mpw a mp4 file transcodificato
               // TimerDelayTranscode.SetTimer(UN_SECONDO*2);
                FreeCache(false);
                StateTranscode=STATE_TRANS_RENAME;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: transcode ok\r\n");  //
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
        break;
        case STATE_TRANS_RENAME:


            strcpy(file_ftpfast,StrRenameTra); //salvo nome file

            //transc_rename(StrBuffTra,TYPE_TY); //provvisorio tengo il .ts in tX dopo sarà da rimuovere
            transc_remove(StrBuffTra);  //rimuovo il ts dato che per adesso transcode va

            if(Configuration.Config.modo_di_funzionamento !=VA_RF) //se ho cambiato modalità
            {

            }
            else
            {
                FtpFastRec.FileFastReady=true;// mando file_ftpfaste
            }
            Start_Time_transcoder=false;
            Time_transcode*=10;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: file transcodificato in %d ms\r\n",Time_transcode);//temp
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            startTranscode=false;
            StateTranscode=STATE_TRANS_NULL;
        break;

        case STATE_TRANS_ERROR :
            StateTranscode=STATE_TRANS_NULL;
            startTranscode=false;
            SetState(TRANSCODE_RUN,false);
        //todo
        break;
        default:
        break;
    }
}


