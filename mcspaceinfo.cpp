
#include "mcspaceinfo.h"
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

#define FSINFO_DATE_INIT    "000101"
#define FSINFO_HOUR_INIT    "000000"
#define FSINFO_DATE_FINISH  "990101"
#define FSINFO_HOUR_FINISH  "235959"

#define OneKbyte 1024

extern TConfig Configuration;
extern MCSerial SerialPrint;
extern char buff_print[];
extern Mcgestionefs GestioneFs;


TInfoSpace Infospace;

mcspaceinfo::mcspaceinfo(QObject *parent) :
    QObject(parent)
{

}
void mcspaceinfo::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerSpaceInfo.Init((char*)"spaceinfo");
    TimerSpaceInfo.SetTimer(UN_SECONDO*1);

    pREGTIMER->RegisterTimer(&TimerSpaceInfo); //timer registrato

    TInfoFSRun.setInterval(100);
    TInfoFSRun.start();
    connect(&TInfoFSRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati
    StateSpaceFs=STATE_SPACE_NULL;
    startInfoFs=false;
    UpgradeInfoFs=false;
    MCFindSpaceInfo.Init(MCFINDSPACEINFO);
    MCFindSpaceInfo.Reset_Init();
    indexSpaceInfo=0;
    typeInfo=TYPE_MPX;
    StateInfoFs=SPACE_FS_1;

    ReadInfo=false;
}





//macchina a stati
void mcspaceinfo::Run()
{
    int temp;

    switch(StateSpaceFs)
    {
        case STATE_SPACE_NULL:

            if(GestioneFs.MsataIsReady) //se ho letto lo spazio
            {

                if(UpgradeInfoFs)
                {

                    if(StateInfoFs==TYPE_TX)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP: run TYPE_TX \r\n");//temp
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        typeInfo=TYPE_TX;
                    }
                    else if(StateInfoFs==TYPE_MPX)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP: run TYPE_MPX\r\n");//temp
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        typeInfo=TYPE_MPX;
                    }
                    else if(StateInfoFs==TYPE_TS)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP: run TYPE_TS\r\n");//temp
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        typeInfo=TYPE_TS;
                    }
                    else if(StateInfoFs==TYPE_MP4)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP: run TYPE_MP4\r\n");//temp
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        typeInfo=TYPE_MP4;
                    }


                    UpgradeInfoFs=false;
                    MCFindSpaceInfo.Reset_Init();
                    indexSpaceInfo=0;
                    startInfoFs=true;
                }
            }

            if(startInfoFs)
            {
                if(MCFindSpaceInfo.SetValidDate((char*)FSINFO_DATE_INIT,(char*)FSINFO_HOUR_INIT,(char*)FSINFO_DATE_FINISH,(char*)FSINFO_HOUR_FINISH,typeInfo))
                {
                    StateSpaceFs=STATE_SPACE_INIT;
                    MCFindSpaceInfo.startfind=true; //avvio macchina a stati instanza MCFindSpaceInfo
                    //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP: start:=%d\r\n",typeInfo);//temp
                    //SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    indexSpaceInfo=0;
                }
                else StateSpaceFs=STATE_SPACE_ERROR;
            }
        break;
        case STATE_SPACE_INIT :

            if(MCFindSpaceInfo.StateFind==STATE_FIND_READY){

                SpaceFsStringList=MCFindSpaceInfo.SendList();

                //MCFindSpaceInfo.PrintObj();

                StateSpaceFs=STATE_SPACE_WAIT;
                //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP: STATE_SPACE_INITttttttttttttt\r\n");//temp
                //SerialPrint.Flush(PRINT_DEBUG_FUNC);
                indexSpaceInfo=0;
                SpaceFileMax=0;
                CountFileMax=SpaceFsStringList.count();


                break;
            }
            if((MCFindSpaceInfo.StateFind==STATE_FIND_NULL_INIT)||(MCFindSpaceInfo.StateFind==STATE_FIND_ERROR)){
                StateSpaceFs=STATE_SPACE_ERROR;
            }

        break;
        case STATE_SPACE_WAIT :
            //ok //prendo la mia lista

            if(indexSpaceInfo<CountFileMax)
            {
                strncpy(&StrBuffSTempFs[0],SpaceFsStringList.at(indexSpaceInfo).toLocal8Bit().constData(),STRING_SPACE);

                temp=strlen(StrBuffSTempFs);

                if(temp>0)
                {
                    quint64 size = 0;
                    QFile myFile(StrBuffSTempFs);
                    if (myFile.open(QIODevice::ReadOnly)){
                        size = myFile.size();  //when file does open.
                        if(size>OneKbyte)size=size/OneKbyte; //KB
                        else size=0;
                        SpaceFileMax+=size;
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP : name %s size KB %lld total %lld\r\n",StrBuffSTempFs,size,SpaceFileMax);
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        myFile.close();
                    }
                }
                indexSpaceInfo++;
                SetWorkPeriod(ID_CPU,30,true);
            }
            else
            {
                if(StateInfoFs==TYPE_TX)
                {
                    Infospace.TtipoTX=CountFileMax;
                    Infospace.TSizeTX=SpaceFileMax;
                    StateInfoFs=TYPE_MPX; //next
                    StateSpaceFs=STATE_SPACE_READY;
                    SetWorkPeriod(ID_CPU,30,true);
                }
                else if(StateInfoFs==TYPE_MPX)
                {
                    Infospace.TtipoMPX=CountFileMax;
                    Infospace.TSizeMPX=SpaceFileMax;
                    StateInfoFs=TYPE_TS; //next
                    StateSpaceFs=STATE_SPACE_READY;
                    SetWorkPeriod(ID_CPU,30,true);
                }
                else if(StateInfoFs==TYPE_TS)
                {
                    Infospace.TtipoTS=CountFileMax;
                    Infospace.TSizeTS=SpaceFileMax;
                    StateInfoFs=TYPE_MP4; //next
                    StateSpaceFs=STATE_SPACE_READY;
                    SetWorkPeriod(ID_CPU,30,true);
                }
                else if(StateInfoFs==TYPE_MP4)
                {
                    Infospace.TtipoMP4=CountFileMax;
                    Infospace.TSizeMP4=SpaceFileMax;
                    StateInfoFs=TYPE_NULL; //next
                    StateSpaceFs=STATE_SPACE_READY;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP : Finish \r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    SetState(COUNT_READY,true);
                    SetWorkPeriod(ID_CPU,30,true);
                }
            }

        break;

        case STATE_SPACE_READY :
            if(StateInfoFs!=TYPE_NULL)UpgradeInfoFs=true;
            SetWorkPeriod(ID_CPU,30,true);

            StateSpaceFs=STATE_SPACE_NULL;
            startInfoFs=false;
        break;

        case STATE_SPACE_ERROR :
            if(StateInfoFs!=TYPE_NULL)UpgradeInfoFs=true;

            if(StateInfoFs==TYPE_TX)
            {
                Infospace.TtipoTX=0;
                Infospace.TSizeTX=0;
                StateInfoFs=TYPE_MPX; //next
            }
            else if(StateInfoFs==TYPE_MPX)
            {
                Infospace.TtipoMPX=0;
                Infospace.TSizeMPX=0;
                StateInfoFs=TYPE_TS; //next
            }
            else if(StateInfoFs==TYPE_TS)
            {
                Infospace.TtipoTS=0;
                Infospace.TSizeTS=0;
                StateInfoFs=TYPE_MP4; //next
            }
            else if(StateInfoFs==TYPE_MP4)
            {
                Infospace.TtipoMP4=0;
                Infospace.TSizeMP4=0;
                StateInfoFs=TYPE_NULL; //finish
                UpgradeInfoFs=false;
                SetState(COUNT_READY,true);
            }
            SetWorkPeriod(ID_CPU,30,true);
            StateSpaceFs=STATE_SPACE_NULL;
            startInfoFs=false;
        break;
        default:
        break;
    }
}



void mcspaceinfo::PrintObj()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"***\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP::StateSpaceFs=%d \r\n",StateSpaceFs);//StateSpaceFs
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP::typeInfo=%d \r\n",typeInfo);//typeInfo
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP::File .tx=%d  size  %lld KByte \r\n",Infospace.TtipoTX,Infospace.TSizeTX);//
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP::File .mpx=%d size %lld KByte\r\n",Infospace.TtipoMPX,Infospace.TSizeMPX);//
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP::File .ts=%d  size  %lld KByte\r\n",Infospace.TtipoTS,Infospace.TSizeTS);//
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCSP::File .mp4=%d size %lld KByte\r\n",Infospace.TtipoMP4,Infospace.TSizeMP4);//
    SerialPrint.Flush(PRINT_DEBUG_ALL);



}










