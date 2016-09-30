
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>
#include "tstate.h"
#include "mcserial.h"
#include "message.h"
#include "utility.h"
#include "mcgestionedate.h"
#include <QDateTime>
#include <QDate>

TGDate IcoreSystemDate;
TGDate TempTerminalDate;
TGDate St32Date;
TGDate TempSt32Date;
TGDate RemLinkDate;


extern char buff_print[MAX_LEN_BUFFER_PRINT+1];
extern MCSerial SerialPrint;

mcgestionedate::mcgestionedate(QObject *parent) :
    QObject(parent)
{
    connect(&DateT, SIGNAL(timeout()), this, SLOT(GestioneDate()));
    TempTerminalDate.update=false;
    TempTerminalDate.force_update=false;
}


void mcgestionedate::Init(TimerMan *t)
{
    pREGTIMER=t;

    GpsTimer.Init((char*)"GpsTimer");
    pREGTIMER->RegisterTimer(&GpsTimer);

    GpsTimer.SetTimer(UN_SECONDO*5);
}

void mcgestionedate::Start(int time)
{
    DateT.setInterval(time);
    DateT.start();
    GdateState=GDATE_SET;
    St32Date.update=false;
}

void mcgestionedate::GestioneDate() //ogni 250ms
{
    QDateTime DateIcore;
    char tempBuffst32[12];
    qint32 DiffSecSt;
    QDateTime DateSt32;

    //qui leggo ora sistema
    //DateTimeIcore=QDateTime::currentDateTime(); //leggo data ora del sistema
    time_t now = time(0);
    struct tm  tstruct;

    tstruct = *localtime(&now);
    strftime(BuffDate, sizeof(BuffDate), "%d%m%y%H%M%S", &tstruct);

    strncpy(IcoreSystemDate.Date,BuffDate,T_HOUR_DATESIZE); //prima parte del buffer
    IcoreSystemDate.Date[T_HOUR_DATESIZE]=0;

    strncpy(IcoreSystemDate.Hour,&BuffDate[T_HOUR_DATESIZE],T_HOUR_DATESIZE); //seconda parte del buffer
    IcoreSystemDate.Hour[T_HOUR_DATESIZE]=0;



    switch(GdateState)
    {
        case GDATE_INIT:
            if(St32Date.update)
            {
                GdateState=GDATE_SET;
            }
        break;
        case GDATE_SET:
            if(St32Date.update)
            {
                Update(&St32Date);
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"ORA: ORA SISTEMA AGGIORNATA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                GdateState=GDATE_CHECK;
            }

        break;
        case GDATE_CHECK:
            //todo confronto se maggiore di 1 sec

            if((TempTerminalDate.force_update)&&(!TempTerminalDate.update))
            {

                GdateState=GDATE_RESTART;
                break;
            }

            //ora linux
            DateIcore=QDateTime::currentDateTime();                               //leggo data ora del sistema

            snprintf(&tempBuffst32[0],13,"%6s%6s",St32Date.Date,St32Date.Hour); //ora carico in buffer e stampo

            DateSt32=QDateTime::fromString(QString(tempBuffst32),"ddMMyyhhmmss");

            DiffSecSt=(qint32)(DateIcore.time().secsTo(DateSt32.time()));                   //se l'ora st32 > 1 sec

            if((qint32)DiffSecSt>(qint32)1)
            {
                Update(&St32Date);

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"ORA: SISTEMA AGGIORNATA DA STM32\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
            }

        break;
        case GDATE_RESTART:
                Update(&St32Date);
                TempTerminalDate.force_update=false;
                GdateState=GDATE_CHECK;
        break;

        default:
        break;

    }
}

bool mcgestionedate::Update(TGDate *pDate)
{
    char buf[36];
    sprintf(buf,"/bin/date -s %04d.%02d.%02d-%02d:%02d:%02d",pDate->GYear,pDate->GMounth,pDate->GDay,pDate->GHour,pDate->GMinutes,pDate->GSeconds);

    system(buf);
    system("/sbin/hwclock -w");

    pDate->update=false; //ok aggiornato

    return(true);
}


bool mcgestionedate::SetDateIsValid(char *str1, char *str2,TGDate *pDate)
{
    pDate->update=false;

    strncpy(pDate->Date,str1,T_HOUR_DATESIZE);
    pDate->Date[T_HOUR_DATESIZE]=0;

    strncpy(pDate->Hour,str2,T_HOUR_HOURSIZE);
    pDate->Hour[T_HOUR_HOURSIZE]=0;

    strncpy(GstrBuff,pDate->Date,ID_GDAY);  //giorno dd
    GstrBuff[ID_GDAY]=0;
    pDate->GDay=atoi(GstrBuff);


    strncpy(GstrBuff,&pDate->Date[ID_GMOUNTH],ID_GMOUNTH);  //mese mm
    GstrBuff[ID_GMOUNTH]=0;
    pDate->GMounth=atoi(GstrBuff);

    strncpy(GstrBuff,&pDate->Date[ID_GMOUNTH+ID_GYEARS],ID_GYEARS);       //anno yy
    GstrBuff[ID_GYEARS]=0;
    pDate->GYear=atoi(GstrBuff); //aggiunte cifre 20 per confronto data
    pDate->GYear+=2000;

    strncpy(GstrBuff,pDate->Hour,ID_GHOUR);       //ora hh
    GstrBuff[ID_GHOUR]=0;
    pDate->GHour=atoi(GstrBuff);

    strncpy(GstrBuff,&pDate->Hour[ID_GMIN],ID_GMIN);  //minuti mm
    GstrBuff[ID_GMIN]=0;
    pDate->GMinutes=atoi(GstrBuff);

    strncpy(GstrBuff,&pDate->Hour[ID_GMIN+ID_GSEC],ID_GSEC);  //secondi ss
    GstrBuff[ID_GSEC]=0;
    pDate->GSeconds=atoi(GstrBuff);


    return(   (strlen(pDate->Date)==T_HOUR_DATESIZE)
           && (strlen(pDate->Hour)==T_HOUR_HOURSIZE)
           && (IsNumber(pDate->Date))
           && (IsNumber(pDate->Hour))
           && (pDate->GDay>= 1) && (pDate->GDay<= 31)
           && (pDate->GMounth >= 1) && (pDate->GMounth <= 12)
           && (pDate->GHour < 24)
           && (pDate->GMinutes < 60)
           && (pDate->GSeconds < 60)
         );
}


