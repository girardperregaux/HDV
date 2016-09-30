#include "Log.h"
#include <stdlib.h>
#include <stdio.h>
#include <DefCom.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include "tstate.h"
#include "mcdatetime.h"
#include "mcgestionefs.h"

#define TEST_LOGS "/mnt/msata/logs/"
#define MAX_FOLDER_LOG 15

extern Mcgestionefs GestioneFs;
extern MCSerial SerialPrint;
extern char buff_print[];

extern McDateTime SystemDate;

#define LOG_TEMPO_FILE_SEC (10*UN_MINUTO)

TLog::TLog(QObject* parent)
    :QObject(parent)
{

}



void TLog::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerT.Init((char*)"LogTimer");
    pREGTIMER->RegisterTimer(&TimerT);
    TimerT.SetTimer(UN_SECONDO);

    TimerRun.setInterval(1000);
    TimerRun.start();
    fileAperto=false;

    connect(&TimerRun, SIGNAL(timeout()), this, SLOT(Run()));
}


void TLog::Run()
{

}

void TLog::DelFolder()
{

    QDir dirlog(TEST_LOGS);
    char bufferlog[STRING_LINUX];

    dirlog.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList search=dirlog.entryList();

    if(search.count()>MAX_FOLDER_LOG)
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"LOG Folder >%d\r\n",MAX_FOLDER_LOG);
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        strcpy(bufferlog,TEST_LOGS);
        strcat(bufferlog,search.at(0).toLocal8Bit().constData());

        QDir dirdel(bufferlog);
        dirdel.setFilter(QDir::Dirs |QDir::Files|QDir::NoDotAndDotDot);

        if(dirdel.removeRecursively()){
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"LOG =rimossa cartella %s\r\n",bufferlog);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }
    }

}



void TLog::Write(char *str)
{


    bool check_folder=false;
    char tmpStrDate[HOUR_DATESIZE+1];
    char tmpStrHour[HOUR_HOURSIZE+1];

    if(GetState(REQ_LOG_DEBUG))SetState(LOG_DEBUG,true);
    else SetState(LOG_DEBUG,false);

    if(!GetState(LOG_DEBUG) && (fileAperto==true))
    {
        file.close();
        fileAperto=false;
    }

    if(!GetState(FASE_ATTIVA) && (fileAperto==true))
    {
        file.close();
        fileAperto=false;
    }

    if(!GetState(FASE_ATTIVA))return;
    if(!GetState(INIT_STATE))return;
    if(!GestioneFs.MsataIsPresent)return;  //se hard disk non presente


    if(!GetState(LOG_DEBUG))return;

    if(!SystemDate.IsValid())return;

    strncpy(tmpStrDate,SystemDate.strDate,HOUR_DATESIZE);
    strncpy(tmpStrHour,SystemDate.strHour,HOUR_HOURSIZE);
    tmpStrDate[HOUR_DATESIZE]=0;
    tmpStrHour[HOUR_HOURSIZE]=0;
    sprintf(log_path,"%s%s/","/mnt/msata/logs/",tmpStrDate);


 printf("-->%s!!!!\r\n",log_path);

    QDir dir(log_path);


    if(!fileAperto)
    {
        if (!dir.exists())
        {
            if(dir.mkpath(log_path))
            {
//qui
                check_folder=true;
            }

        }
        else
        {
          //  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"LOG: LA CARTELLA ESISTE !!!\r\n");
          //  SerialPrint.Flush();
        }

        if (!dir.exists())return;

        sprintf(log_path,"%s%s/%s.log","/mnt/msata/logs/",tmpStrDate,tmpStrHour);

        file.setFileName(log_path);

        if (!file.open(QIODevice::WriteOnly))
        {
           // snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"LOG: IMPOSSIBILE APRIRE IL FILE %s !!!\r\n",log_path);
           // SerialPrint.Flush();
            return;
        }
        fileAperto=true;

        outStream.setDevice(&file);
        TimerT.SetTimer(LOG_TEMPO_FILE_SEC);

    }
    if(fileAperto)
    {

        outStream << tmpStrHour << " " << str;

        if(TimerT.IsElapsed())
        {
            file.close();
            fileAperto=false;

        }
    }

    if(check_folder) //se ho creato nuova cartella controllo che il totale delle cartelle non superino 15 giorni
    {
        DelFolder();
    }




/*

    sprintf(log_path,"%sfile%02d.log","/mnt/msata/logs/",count);

    file.setFileName(log_path);

    if (!file.open(QIODevice::WriteOnly))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"LOG: IMPOSSIBILE APRIRE IL FILE %s !!!\r\n",log_path);
        SerialPrint.Flush();
        return;
    }
    //file.write("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC!!!!!");
   // QTextStream outStream(&file);
    outStream.setDevice(&file);
    outStream << "Victorxxxxxy!\n";
    file.close();
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"LOG: SCRITTURA ESEGUITA!!!\r\n");
    SerialPrint.Flush();

*/

}


