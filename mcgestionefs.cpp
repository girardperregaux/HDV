#include "mcgestionefs.h"
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
#include "message.h"
#include "mcfspace.h"
#include "utility.h"
#include "mcountfile.h"


#define STATE_FS_NULL        0
#define STATE_FS_MOUNT       1
#define STATE_FS_INIT        2
#define STATE_FS_INIT_MNT    3
#define STATE_FS_CHECK       4
#define STATE_FS_UMOUNT      5
#define STATE_FS_READY       6
#define STATE_FS_FORMAT      7
#define STATE_FS_WAIT        8
#define STATE_FS_F_NEW       9
#define STATE_FS_ASSENT     10
#define STATE_FS_LOG_OFF    11


#define STR_DIR    "/mnt"
#define STR_FILE_HD   "/mnt/msata/msata.org"
#define STR_DIR_HD    "/mnt/msata"
#define STR_FILE_SIZE "/mnt/ramdisk/sizehd.txt"
#define STR_FILE_MNT  "/mnt/ramdisk/checkmnt.txt"


extern TConfig Configuration;
extern char buff_print[];

extern MCSerial SerialPrint;
extern Message MESSAGE;

extern mcfspace FSpace;
extern MCountfile Countfile;


TMM MMGestioneFs;
char GFSstrtmp[MAX_LEN_SMS+1];


Mcgestionefs::Mcgestionefs(QObject *parent) :
    QObject(parent)
{

}

void Mcgestionefs::Init(TimerMan *t)
{

    pREGTIMER=t;

    TimerFs.Init((char*)"filesystem");
    TimerFs.SetTimer(UN_SECONDO*10);


    pREGTIMER->RegisterTimer(&TimerFs); //timer registrato


    FsTimerRun.setInterval(100);
    FsTimerRun.start();
    connect(&FsTimerRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati

    FsState=STATE_FS_NULL;
    VMsata=0xFF;
    ReqFormat=false;
    MsataIsPresent=false;
    MsataIsFormat=false;
    MsataIsReady=false;
    retry=0;
}


bool Mcgestionefs::ReadMnt()
{
   QFile file(STR_FILE_MNT);

   char TempArray[100];
   char *p;
   p=&TempArray[0];

   if(!file.exists()) return (false);

   if(file.open(QFile::ReadOnly))
   {
       file.read(p, sizeof(TempArray));

       file.close();
   }

   if((p=strstr(TempArray,"%")))
   {
       snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: Hd presente\r\n");
       SerialPrint.Flush(PRINT_DEBUG_FUNC);

       return (true);
   }
   else{
       snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: Hd non presente\r\n");
       SerialPrint.Flush(PRINT_DEBUG_FUNC);
       return (false);
   }

}

void Mcgestionefs::ReadFs()
{


    QFile file(STR_FILE_SIZE);

    char TempArray[100];
    char TempValue[10];
    char *p;
    p=&TempArray[0];

    if(!file.exists()) return;

    if(file.open(QFile::ReadOnly))
    {
        file.read(p, sizeof(TempArray));

        file.close();
    }


    if((p=strstr(TempArray,"%")))
    {
        p-=3; //indietro di 3 per leggere percentuale

        TempValue[0]=*p++;
        TempValue[1]=*p++;
        TempValue[2]=*p++;
        TempValue[3]=0;

        if(IsNumber(&TempValue[0]))      //3 cifre
        {
            VMsata=atoi(&TempValue[0]);
        }
        else if(IsNumber(&TempValue[1])) //2 cifre
        {
            VMsata=atoi(&TempValue[1]);
        }
        else if(IsNumber(&TempValue[2])) //1 cifra
        {
            VMsata=atoi(&TempValue[2]);
        }

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: MSata spazio occupato = %d\r\n",VMsata);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        retry=0;

        TimerFs.SetTimer(UN_SECONDO*20); //controllo spazio libero hd ogni 20 sec
        //qui
        MsataIsReady=true;
    }
    else
    {
        TimerFs.SetTimer(UN_SECONDO*5); //controllo spazio libero hd

        retry++;

        if(retry>3)
        {
             FsState=STATE_FS_ASSENT;
             snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*GFS: Hard disk assente o non montato\r\n");
             SerialPrint.Flush(PRINT_DEBUG_FUNC);
             MsataIsReady=false;
        }
    }

}

void Mcgestionefs::Format()
{
    ReqFormat=true;
}

void Mcgestionefs::Run()
{
    QFile file;
    QDir dir(STR_DIR);

    switch(FsState)
    {
        case STATE_FS_NULL:
            if(TimerFs.IsElapsed()) FsState=STATE_FS_MOUNT; //dopo 10 sec
        break;
        case STATE_FS_MOUNT:
            system("mkdir /mnt/msata" ); //creo sempre la cartella
            system("mount /dev/sda /mnt/msata" ); //monto hard disk
            system("df | grep /dev/sda > /mnt/ramdisk/checkmnt.txt");

            FsState=STATE_FS_INIT_MNT;
            TimerFs.SetTimer(UN_SECONDO);
        break;
        case STATE_FS_INIT_MNT:

            if(ReadMnt()) //se hd presente
            {
                FsState=STATE_FS_INIT;

                //qui

                MsataIsPresent=true;
            }
            else
            {
                FsState=STATE_FS_ASSENT;
            }

        break;
        case STATE_FS_INIT:
            if(!TimerFs.IsElapsed()) break;

            file.setFileName(STR_FILE_HD);

            if(file.exists())
            {
                MsataIsFormat=true;
                FsState=STATE_FS_READY;
                TimerFs.SetTimer(UN_SECONDO);
                Countfile.startfind=true; //avvio ricerche dei file
                LoadFileT((char*)STR_FILE_TR_COUNT);

            }
            else
            {
                QDir FinFileIF;
                FinFileIF.setCurrent("/mnt/"); //imposto la directory principale

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*GFS: file msata.org non presente\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                FsState=STATE_FS_CHECK;
                MsataIsFormat=false;
            }

        break;
        case STATE_FS_CHECK:

            dir.setCurrent(STR_DIR_HD);

            if (dir.exists()) //controllo se hd presente fisicamente o no
            {
                FsState=STATE_FS_UMOUNT;
            }
            else
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*GFS: HDD ASSENTE\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                FsState=STATE_FS_ASSENT;
            }

        break;

        case STATE_FS_UMOUNT:
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: SMONTO HDD /mnt/msata\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            MsataIsFormat=false;
            system("umount -l /mnt/msata");

            TimerFs.SetTimer(3*UN_SECONDO);

            FsState=STATE_FS_FORMAT;
        break;

        case STATE_FS_FORMAT:
            if(!TimerFs.IsElapsed()) break;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: AVVIO FORMAT\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            system("mkfs.ext4 /dev/sda -F >> /home/root/result.txt"); //format
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: FORMAT ESEGUITO\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            FsState=STATE_FS_WAIT;
        break;
        case STATE_FS_WAIT:
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*GFS: MOUNT(/dev/sda /mnt/msata)\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            system("mount /dev/sda /mnt/msata"); //monto hard disk
            TimerFs.SetTimer(UN_SECONDO);
            FsState=STATE_FS_F_NEW;
        break;
        case STATE_FS_F_NEW:
            //qui
             if(!TimerFs.IsElapsed()) break;
             file.setFileName(STR_FILE_HD);
             if(file.exists())
             {
                 SetState(REQ_LOG_DEBUG,true);
                 FsState=STATE_FS_MOUNT;

                 snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: FORMAT NON ESEGUITO !!!!!\r\n");
                 SerialPrint.Flush(PRINT_DEBUG_FUNC);

                 if(IsTelephoneNumber(MMGestioneFs.ConnectedNum))
                 {
                     sprintf(GFSstrtmp,"FORMAT NON ESEGUITO, RIPROVA");
                     MESSAGE.MessageUserMMPost(&MMGestioneFs,(char *)&GFSstrtmp[0]);
                     MMGestioneFs.ConnectedNum[0]=0;
                 }
                 Countfile.startfind=true;
            }
            else if (!file.open(QIODevice::WriteOnly))
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GFS: IMPOSSIBILE APRIRE IL FILE MSATA.ORG\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                MMGestioneFs.ConnectedNum[0]=0;
                FsState=STATE_FS_ASSENT; // to

            }
            else
            {
                file.write("test");
                file.close();

                system("df | grep /dev/sda > /mnt/ramdisk/sizehd.txt");
                ReadFs();
                SetState(REQ_LOG_DEBUG,true);
            }
            FsState=STATE_FS_MOUNT;
            if(IsTelephoneNumber(MMGestioneFs.ConnectedNum))
            {
                sprintf(GFSstrtmp,"FORMAT ESEGUITO");
                MESSAGE.MessageUserMMPost(&MMGestioneFs,(char *)&GFSstrtmp[0]);
                MMGestioneFs.ConnectedNum[0]=0;
            }
            Countfile.startfind=true; //avvio ricerche dei file

        break;

        case STATE_FS_READY:

            if(FSpace.ReadSpace) //se ho superato spazio hard disk leggo
            {
                system("df | grep /dev/sda > /mnt/ramdisk/sizehd.txt");
                ReadFs();
                FSpace.ReadSpace=false;
            }

            if(TimerFs.IsElapsed())
            {
                system("df | grep /dev/sda > /mnt/ramdisk/sizehd.txt");
                ReadFs();

            }

            if(ReqFormat)
            {
                TimerFs.SetTimer(0);
                TimerFs.SetTimer(UN_SECONDO);
                //log off
                SetState(REQ_LOG_DEBUG,false);
                QDir FinFileIF;
                FinFileIF.setCurrent("/mnt/"); //imposto la directory principale
                //cancello file
                ReqFormat=false;
                FsState=STATE_FS_LOG_OFF;
                Countfile.startfind=false; //fermo ricerche dei file
            }

            //check hd

        break;
        case STATE_FS_LOG_OFF:
            if(!TimerFs.IsElapsed()) break;
            FsState=STATE_FS_UMOUNT;
            ReqFormat=false;
            break;
//////////////////////
            ReqFormat=false;
            remove_directory((char*)"logs");
            remove_directory((char*)"videoaudio");
            FsState=STATE_FS_READY;
            Countfile.startfind=true; //avvio ricerche dei file
            if(IsTelephoneNumber(MMGestioneFs.ConnectedNum))
            {
                sprintf(GFSstrtmp,"FORMAT ESEGUITO");
                MESSAGE.MessageUserMMPost(&MMGestioneFs,(char *)&GFSstrtmp[0]);
                MMGestioneFs.ConnectedNum[0]=0;
            }


            //FsState=STATE_FS_UMOUNT;

        break;
        case STATE_FS_ASSENT:
            //retry????
            MsataIsPresent=false;

        break;
        default:
        break;
    }

}



