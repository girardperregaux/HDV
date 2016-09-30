
#include "mctranscode.h"
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

mctranscode::mctranscode(QObject *parent) :
    QObject(parent)
{
    Gsttrasc= new QProcess(this);

}
void mctranscode::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerTranscode.Init((char*)"transcode");
    TimerTranscode.SetTimer(UN_SECONDO*1);

    pREGTIMER->RegisterTimer(&TimerTranscode); //timer registrato

    TTranscodeRun.setInterval(10);
    TTranscodeRun.start();
    connect(&TTranscodeRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati
    StateTranscode=STATE_TRANS_NULL;
    startTranscode=false;
    UpgradeTranscode=false;
    MCFindTranscode.Init(MCFINDTRANSODE);
    MCFindTranscode.Reset_Init();
    indexTranscode=0;
    countlog=0;
    Transc_sleep=false;
    QuantTrs=0;
    FrameTrs=0;
    BitrateTrs=0;
}

bool mctranscode::transc_rename(char * Tfile,quint8 formato)
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
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS:Error file save : %s\r\n",StrBuffTemp);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (false);
    }

}
bool mctranscode::transc_remove(char * Tfile)
{
    QFile fileTranscode(Tfile);

    if(fileTranscode.exists())
    {
        if(fileTranscode.remove())
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS:file cancellato : %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (true);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS:Error file remove : %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (false);
        }
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS:Error file non esiste : %s\r\n",Tfile);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }
    return (false);

}

//macchina a stati
void mctranscode::Run()
{
    int temp;
    qint64 timesize;



    //if(!startTrancode) StateTrancode=STATE_TRANS_NULL;

    return;  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    switch(StateTranscode)
    {
        case STATE_TRANS_NULL :

         //   return;
            if(UpgradeTranscode)
            {
                if(!GestioneFs.MsataIsPresent)break;  //se hard disk non presente
                if(FSpace.FirstSpaceFs!=SPACE_FS_OK)break; //se non ho ancora finito di cancellare i file all'avvio .tw e .mpw esco

                Transcode_finish=false;
                UpgradeTranscode=false;
                MCFindTranscode.Reset_Init();
                indexTranscode=0;
                startTranscode=true;
                SetState(TRANSCODE_RUN,false);
                TimerTranscode.SetTimer(0);
                TimerTranscode.SetTimer(UN_MEZZO);

            }

            if(startTranscode)
            {
                if(!TimerTranscode.IsElapsed()) break;

                if(MCFindTranscode.SetValidDate((char*)TRANSC_DATE_INIT,(char*)TRANSC_HOUR_INIT,(char*)TRANSC_DATE_FINISH,(char*)TRANSC_HOUR_FINISH,TYPE_TS))
                {
                    StateTranscode=STATE_TRANS_INIT;
                    MCFindTranscode.startfind=true; //avvio macchina a stati istanza mcfind
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: STATE_TRANS_NULL\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    indexTranscode=0;
                }
                else StateTranscode=STATE_TRANS_ERROR;
            }
        break;
        case STATE_TRANS_INIT :

            if(MCFindTranscode.StateFind==STATE_FIND_READY){

                TranscStringList=MCFindTranscode.SendList();

                StateTranscode=STATE_TRANS_WAIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: STATE_TRANS_INIT\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                break;
            }
            if((MCFindTranscode.StateFind==STATE_FIND_NULL_INIT)||(MCFindTranscode.StateFind==STATE_FIND_ERROR)){
                StateTranscode=STATE_TRANS_ERROR;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: STATE_TRANS_ERROR\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }

        break;
        case STATE_TRANS_WAIT :
            //ok //prendo la mia lista


            if(Transc_sleep)
            {
                Transc_sleep=false;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS : SLEEP\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTranscode=STATE_TRANS_READY;
                SetState(TRANSCODE_RUN,false);
                break;
            }

            if(indexTranscode<(quint32)TranscStringList.count())
            {
                //sprintf(&StrBuffTra[0],"%s\r\n",TranscStringList.at(indexTranscode).toLocal8Bit().constData());//temp
                strncpy(&StrBuffTra[0],TranscStringList.at(indexTranscode).toLocal8Bit().constData(),STRING_TRANSC);
                strncpy(&StrBuffTemp[0],TranscStringList.at(indexTranscode).toLocal8Bit().constData(),STRING_TRANSC);
                temp=strlen(StrBuffTemp);

                QFile filetranscode(StrBuffTemp);
                FreeCache(false);

                if (!filetranscode.open(QIODevice::ReadOnly))
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: ERRORE APERTURA FILE %s\r\n",StrBuffTemp);
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTranscode=STATE_TRANS_WAIT; //continuo
                    indexTranscode++;               //provo file successivo
                    return;                         //ed esco da qui
                }

                qint64 size = qint64(filetranscode.size()/1000);

                if(size>SIZE_TRANSC_100MB)
                {
                    //timesize=3+(size/SIZE_TRANSC_100MB);
                    timesize=(6*((size/SIZE_TRANSC_100MB)+1));

                }
                else timesize=5;

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: DIM: %lld KByte TIMEOUT: %lld min\r\n",size,timesize);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                filetranscode.close();


                if(temp>0)
                {
                    if(StrBuffTemp[temp-13]=='_')
                    {
                        StrQuant[0]=StrBuffTemp[temp-12]; //per quantizzazione
                        StrQuant[1]=StrBuffTemp[temp-11]; //7
                        StrQuant[2]=0;
                        QuantTrs=atoi(StrQuant);

                        StrFrame[0]=StrBuffTemp[temp-9]; //per frame
                        StrFrame[1]=StrBuffTemp[temp-8];
                        StrFrame[2]=0;

                        FrameTrs=atoi(StrFrame);

                        StrBitrate[0]=StrBuffTemp[temp-6];
                        StrBitrate[1]=StrBuffTemp[temp-5];
                        StrBitrate[2]=StrBuffTemp[temp-4];
                        BitrateTrs=atoi(StrBitrate);


                        StrBuffTemp[temp-13]='.';          //tolgo quantizzazione e frame
                        StrBuffTemp[temp-12]='m';
                        StrBuffTemp[temp-11]='p';
                        StrBuffTemp[temp-10]='w';
                        StrBuffTemp[temp-9]=0;
                    }
                    else
                    {
                        FrameTrs=0;
                        QuantTrs=0;
                        BitrateTrs=32; //default
                        StrBuffTemp[temp-2]='m';
                        StrBuffTemp[temp-1]='p';  //ora cambio da ts a mpw
                        StrBuffTemp[temp]='w';
                        StrBuffTemp[temp+1]=0;
                    }
                }
                else
                {
                    indexTranscode++;               //provo file successivo
                    return;                         //ed esco da qui
                }
                memset(Buffstrasc,CHARNULL,STRING_MAX_BUFFER_TRANSC);
                pBufftrasc=&Buffstrasc[0];

                pBufftrasc+=sprintf(pBufftrasc,"-v -e filesrc location=%s ! decodebin  async-handling=true name=demux { mp4mux name=mux ",&StrBuffTra[0]);
                pBufftrasc+=sprintf(pBufftrasc,"! filesink location=%s } { demux. ! queue ! audioconvert ! ffenc_aac bitrate=%d000 ",&StrBuffTemp[0],BitrateTrs);


                if((QuantTrs>=20)&&(QuantTrs<=40))  //se ce' la quantizzazione sul nome del file
                {
                    pBufftrasc+=sprintf(pBufftrasc,"! mux. } { demux. ! queue ! vpuenc codec=6 force-framerate=true framerate-nu=%d cbr=false quant=%d ! mux. } ",FrameTrs,QuantTrs);

                }
                else
                {
                    pBufftrasc+=sprintf(pBufftrasc,"! queue ! mux. } { demux. ! queue ! vpuenc codec=6 ! mux. }");
                }
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS : %s\r\n",&Buffstrasc[0]);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                argtrasc=QString(Buffstrasc).split(" ");

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: STATE_TRANS_WAIT\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

//sprintf(StrLogTra,"/mnt/msata/loggst%d.txt",countlog);
//Gsttrasc->setStandardOutputFile(StrLogTra);
//countlog++;
//if(countlog==100)countlog=0;

                Gsttrasc->start("gst-launch", argtrasc);
                PidGstTransc = Gsttrasc->pid();

                //sprintf(StrPidTra," kill -9 %d",PidGstTransc);

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS : %d\r\n",Gsttrasc->state());
                SerialPrint.Flush(PRINT_DEBUG_FUNC);


                StateTranscode=STATE_TRANS_TRANSCODE;


                TimerTranscode.SetTimer(UN_MINUTO*timesize);
                indexTranscode++;
                SetState(TRANSCODE_RUN,true);
            }
            else
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS :FINISH\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTranscode=STATE_TRANS_READY;
                SetState(TRANSCODE_RUN,false);
            }

        break;
        case STATE_TRANS_TRANSCODE :


            if(TimerTranscode.IsElapsed())
            {
                StateTranscode=STATE_TRANS_WAIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: ERRORE TRANSCOD \r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                Gsttrasc->close();
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: RIMUOVO IL FILE %s\r\n",StrBuffTemp);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                transc_remove(StrBuffTemp); //elimino file non transcodificato
                TimerTranscode.SetTimer(UN_SECONDO*1);
                //transc_rename(StrBuffTra,TYPE_TZ); //tengo come tampone e non transcodifico(che controllo poi)
                if(GetState(DEL_TS))transc_remove(StrBuffTra); //elimino file non transcodificato
                else transc_rename(StrBuffTra,TYPE_TZ); //tengo come tampone e non transcodifico(che controllo poi)


            }
            else if(Gsttrasc->state()==QProcess::NotRunning)
            {
                QFile fileTemp(StrBuffTemp);
                if (!fileTemp.open(QIODevice::ReadOnly))
                {
                   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: Errore apertura file%s\r\n",StrBuffTemp);
                   SerialPrint.Flush(PRINT_DEBUG_FUNC);
                   StateTranscode=STATE_TRANS_WAIT;
                   break;
                }
                qint64 size = (qint64 )fileTemp.size();

                fileTemp.close();

                if(size==0)
                {
                     snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: File transcodificato a zero %s\r\n",StrBuffTemp);
                     SerialPrint.Flush(PRINT_DEBUG_FUNC);
                     snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRS: ++++++++++ File %s transcodificato a zero ++++++++++\r\n",StrBuffTemp);
                     SerialPrint.Flush(PRINT_DEBUG_ALL);

                     fileTemp.remove();
                     if(GetState(DEL_TS))transc_remove(StrBuffTra);  //rimuovo il ts dato che per adesso transcode va
                     else transc_rename(StrBuffTra,TYPE_TZ); //tengo come tampone e non transcodifico(che controllo poi)
                     StateTranscode=STATE_TRANS_WAIT;
                     break;
                }

                transc_rename(StrBuffTemp,TYPE_MP4); //da mpw a mp4 file transcodificato

                StateTranscode=STATE_TRANS_RENAME;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRS: transcode ok\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
        break;
        case STATE_TRANS_RENAME:

           //transc_rename(StrBuffTra,TYPE_TY); //tengo come tampone e non transcodifico(che controllo poi)
           transc_remove(StrBuffTra);  //rimuovo il ts dato che per adesso transcode va
           //else transc_rename(StrBuffTra,TYPE_TY);
            StateTranscode=STATE_TRANS_WAIT;
        break;
        case STATE_TRANS_READY :
            StateTranscode=STATE_TRANS_NULL;
            startTranscode=false;
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


