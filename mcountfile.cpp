
#include "mcountfile.h"
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
#include <string.h>
#include <qstringlist.h>
#include "utility.h"
#include "DefCom.h"
#include "mcgestionefs.h"

extern MCSerial SerialPrint;
extern char buff_print[];

extern Mcgestionefs GestioneFs;



MCountfile::MCountfile(QObject *parent) :
    QObject(parent)
{

}

void MCountfile::Init(TimerMan *t)
{
    pREGTIMER=t;
    TimerCount.Init((char*)"Countfile");
    TimerCount.SetTimer(UN_SECONDO*1);

    pREGTIMER->RegisterTimer(&TimerCount); //timer registrato

    TimerRunCount.setInterval(100);
    TimerRunCount.start();
    connect(&TimerRunCount, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati

}


quint32 MCountfile::Countfile()
{

    quint32 count_file_max;

    QDir dir(SCAN_DIR);

    dir.setCurrent(SCAN_DIR);
    dir.setSorting(QDir::Name);
    dir.setFilter(QDir::Dirs |QDir::NoDotAndDotDot);
    QStringList search=dir.entryList();
    dir.setCurrent("/mnt/"); //imposto la directory principale

    count_file_max=0;

    if(search.count()<1)
    {
        return (count_file_max);
    }

    for (int i=0; i<search.count(); i++)
    {
        char FileDirIF[STRING_F_MAX+1];
        strcpy(FileDirIF,SCAN_DIR);
        strcat(FileDirIF,search.at(i).toLocal8Bit().constData());//aggiungo percorso
        //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"<%s>\r\n",FileDirIF);
        //SerialPrint.Flush(PRINT_DEBUG_ALL);

        QDir dirFile(FileDirIF);
        dirFile.setCurrent(FileDirIF);
        dirFile.setSorting(QDir::Name);
        dirFile.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        dirFile.setNameFilters(QStringList()<< "*.mp4");      //filtro per file .mp4

        QStringList searchFile=dirFile.entryList();

        dirFile.setCurrent("/mnt/"); //imposto la directory principale

        if(searchFile.count()<1)
        {

        }
        else
        {
            for (int i=0; i<searchFile.count(); i++)
            {
              //  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"file=<%s>\r\n",searchFile.at(i).toLocal8Bit().constData());
              //  SerialPrint.Flush(PRINT_DEBUG_ALL);
                count_file_max+=1;
            }
        }
    }

    //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"file trovati n=<%d>\r\n",count_file_max);
    //SerialPrint.Flush(PRINT_DEBUG_ALL);

    return (count_file_max);

}



//macchina a stati
void MCountfile::Run()
{
    switch(StateFind)
    {
        case STATE_MCFIND_NULL :

            if(startfind)
            {
                if(!GestioneFs.MsataIsPresent)break;  //se hard disk non presente
                countfile_mp4=0;
                StateFind=STATE_MCFIND_INIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCOUNT: startfind \r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                //printfilemp4=false;
            }
        break;
        case STATE_MCFIND_INIT :

            countfile_mp4=Countfile();

            StateFind=STATE_MCFIND_WAIT;
            if(printfilemp4)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCOUNT: MP4=<%d> \r\n",countfile_mp4);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            TimerCount.SetTimer(UN_SECONDO*5);

        break;
        case STATE_MCFIND_WAIT :
            StateFind=STATE_MCFIND_READY;

        break;
        case STATE_MCFIND_READY :

            if(!startfind)
            {
                StateFind=STATE_MCFIND_NULL;
            }
            else if(TimerCount.IsElapsed())
            {
                StateFind=STATE_MCFIND_INIT;
            }

        break;

        default:
        break;
    }
}



void MCountfile::PrintObj()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCOUNT: Print**************\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

}



