
#include "mcfspace.h"
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

#define FSPACE_DATE_INIT    "000101"
#define FSPACE_HOUR_INIT    "000000"
#define FSPACE_DATE_FINISH  "990101"
#define FSPACE_HOUR_FINISH  "235959"
#define SIZE_TRANSC_100MB    100000

#define SIZE_MAX_HD  95
#define SIZE_MIN_HD  85

#define ALREADY_TRANSFERT 1
#define ON_HARD_DISK      2



extern TConfig Configuration;
extern MCSerial SerialPrint;
extern char buff_print[];
extern Mcgestionefs GestioneFs;


mcfspace::mcfspace(QObject *parent) :
    QObject(parent)
{
    //Gsttrasc= new QProcess(this);

}
void mcfspace::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerSpaceFs.Init((char*)"spacefs");
    TimerSpaceFs.SetTimer(UN_SECONDO*1);

    pREGTIMER->RegisterTimer(&TimerSpaceFs); //timer registrato

    TSpaceFSRun.setInterval(100);
    TSpaceFSRun.start();
    connect(&TSpaceFSRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati
    StateSpaceFs=STATE_SPACE_NULL;
    startSpaceFs=false;
    UpgradeSpaceFs=false;
    MCFindSpaceFs.Init(MCFINDSPACEFS);
    MCFindSpaceFs.Reset_Init();
    indexSpaceFs=0;
    typeformat=TYPE_MPX;
    FirstSpaceFs=SPACE_FS_1;

    SpaceOptimize=ALREADY_TRANSFERT;
    ReadSpace=false;
}


bool mcfspace::SpaceFSRemove(char * Tfile)
{
    QFile fileSpaceFs(Tfile);

    if(fileSpaceFs.exists())
    {
        if(fileSpaceFs.remove())
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: FILE %s RIMOSSO\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (true);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS::IMPOSSIBILE ELIMINARE IL FILE %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (false);
        }
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: RM FILE %s NON ESISTE\r\n",Tfile);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }
    return (false);

}

bool mcfspace::SpaceFSDirRm(char *p)
{
    char TempDateTemp[MAX_LEN_STR_FOLDER+1];

    QStringList DirectoryList;
    QStringList Directoryfile;


    if(!QDir(SCAN_DIR).exists()) return (false);

    DirectoryList=MCFindSpaceFs.SetList(p,TYPE_DIR);

    for (int i=0; i<DirectoryList.count(); i++)
    {
        sprintf(&TempDateTemp[0],"%s",DirectoryList.at(i).toLocal8Bit().constData());
        //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFSxxxxxxxxxxxxxxxxxxxxxxxx: %s \r\n",TempDateTemp);//temp
        //SerialPrint.Flush(PRINT_DEBUG_FUNC);

        Directoryfile=MCFindSpaceFs.SetList(TempDateTemp,TYPE_ALL); //rimuovo cartella se vuota
    }
    return (false);

}




//macchina a stati
void mcfspace::Run()
{
    quint32 temp;


    switch(StateSpaceFs)
    {
        case STATE_SPACE_NULL:

            if(GestioneFs.MsataIsReady) //se ho letto lo spazio
            {
                if(FirstSpaceFs==SPACE_FS_1)
                {
                    //all'avvio cerco i file .tw (chiusura inaspettata gstreamer o spegnimento)
                    //come condizione gstreamer deve essere a false
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: RICERCA AVVIO *.tw\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    typeformat=TYPE_TW;
                    UpgradeSpaceFs=true;
                }
                else if(FirstSpaceFs==SPACE_FS_2)
                {
                    //all'avvio cerco i file .mpw (chiusura inaspettata transcoder o spegnimento)
                    //come condizione gstreamer deve essere a false
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: RICERCA AVVIO *.mpw\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    typeformat=TYPE_MPW;
                    UpgradeSpaceFs=true;
                }
                else if(GestioneFs.VMsata>SIZE_MAX_HD)//SIZE_MAX_HD
                {
                    SetWorkPeriod(ID_CPU,30,true);
                    ReadSpace=true; //leggo spazio ogni file cancellato

                    //step 1 cancello dal piu' vecchio i file già trasferiti
                    if(SpaceOptimize==ALREADY_TRANSFERT)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: run space fs\r\n");//temp
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);

                        if(Configuration.Config.transcoder) //usato per abilitare/disabilitare transcoder
                        {
                            typeformat=TYPE_MPX; //file transcodificati
                        }
                        else{
                            typeformat=TYPE_TX; //file originali
                        }
                        UpgradeSpaceFs=true;
                        SpaceOptimize=ON_HARD_DISK;
                    }
                    else if(SpaceOptimize==ON_HARD_DISK) //todo se spazio non basta cancellare .mp4
                    {

                        if(Configuration.Config.transcoder) //usato per abilitare/disabilitare transcoder
                        {
                            typeformat=TYPE_MP4; //file non trasferiti mp4
                        }
                        else{
                            typeformat=TYPE_TS; //file originali
                        }
                        UpgradeSpaceFs=true;
                        SpaceOptimize=ALREADY_TRANSFERT; //quando ottimizzato riprovo da quelli già trasferiti
                    }
                }
            }

            if(UpgradeSpaceFs)
            {
                if(!GestioneFs.MsataIsReady)break; //se non ho letto lo spazio esco e aspetto

                UpgradeSpaceFs=false;
                MCFindSpaceFs.Reset_Init();
                indexSpaceFs=0;
                startSpaceFs=true;
            }

            if(startSpaceFs)
            {
                if(MCFindSpaceFs.SetValidDate((char*)FSPACE_DATE_INIT,(char*)FSPACE_HOUR_INIT,(char*)FSPACE_DATE_FINISH,(char*)FSPACE_HOUR_FINISH,typeformat))
                {
                    StateSpaceFs=STATE_SPACE_INIT;
                    //MCFindSpaceFs.startfind=true;
                    indexSpaceFs=0;
                }
                else StateSpaceFs=STATE_SPACE_ERROR;
            }
        break;
        case STATE_SPACE_INIT :

            if(MCFindSpaceFs.StateFind==STATE_FIND_READY)
            {

                SpaceFsStringList=MCFindSpaceFs.SendList();
                StateSpaceFs=STATE_SPACE_WAIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: %d FILE DA ELIMINARE\r\n",SpaceFsStringList.count());
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                break;
            }
            if((MCFindSpaceFs.StateFind==STATE_FIND_NULL_INIT)||(MCFindSpaceFs.StateFind==STATE_FIND_ERROR)){

                SpaceFSDirRm((char*)SCAN_DIR);
                StateSpaceFs=STATE_SPACE_ERROR;
            }

        break;
        case STATE_SPACE_WAIT :

            if(indexSpaceFs < (quint32)SpaceFsStringList.count())
            {
                strncpy(&StrBuffSTempFs[0],SpaceFsStringList.at(indexSpaceFs).toLocal8Bit().constData(),STRING_SPACE);
                temp=strlen(StrBuffSTempFs);

                if(temp>0)
                {
                    if((GestioneFs.VMsata<=SIZE_MIN_HD) && (typeformat!=TYPE_TW)&&(typeformat!=TYPE_MPW))
                    //if((GestioneFs.VMsata<=1) && (typeformat!=TYPE_TW)&&(typeformat!=TYPE_MPW))
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: SPAZIO OTTIMIZZATO %d%%\r\n",GestioneFs.VMsata);
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        SpaceFSDirRm((char*)SCAN_DIR);
                        SpaceOptimize=ALREADY_TRANSFERT; //ok la prox volta riparto da quelli trasferiti
                        StateSpaceFs=STATE_SPACE_READY;
                    }
                    else
                    {
                        SetWorkPeriod(ID_CPU,30,true);
                        SpaceFSRemove(StrBuffSTempFs);//qui cancello file
                        ReadSpace=true; //leggo spazio ogni file calcellato
                    }

                }

                indexSpaceFs++;
            }
            else
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SPFS: FINE\r\n");
                SpaceFSDirRm((char*)SCAN_DIR);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateSpaceFs=STATE_SPACE_READY;
            }

        break;

        case STATE_SPACE_READY :
            if(FirstSpaceFs==SPACE_FS_1)
            {
                FirstSpaceFs=SPACE_FS_2;
            }
            else if(FirstSpaceFs==SPACE_FS_2)
            {
                FirstSpaceFs=SPACE_FS_OK;
            }

            StateSpaceFs=STATE_SPACE_NULL;
            startSpaceFs=false;
        break;

        case STATE_SPACE_ERROR :
            if(FirstSpaceFs==SPACE_FS_1)
            {
                FirstSpaceFs=SPACE_FS_2;
            }
            else if(FirstSpaceFs==SPACE_FS_2)
            {
                FirstSpaceFs=SPACE_FS_OK;
            }
            StateSpaceFs=STATE_SPACE_NULL;
            startSpaceFs=false;
        break;
        default:
        break;
    }
}


