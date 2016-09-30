#include "apptimer.h"
#include "mcserial.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>

#define PERIODO_MS  125   //msec

extern MCSerial SerialPrint;
extern char buff_print[];

AppTimer::AppTimer(QObject* parent)
    :QObject(parent)
{
    count=0;
    Elapsed=true;
    Running=false;
    TempInSec=0;
}

void AppTimer::Init(char *str)
{

    strncpy(Name,str,MAX_LEN_STR_TIMER);
    count=0;
    Elapsed=true;
    Running=false;

    timer.setInterval(PERIODO_MS);
    timer.start();
    connect(&timer, SIGNAL(timeout()), this, SLOT(Run()));

}


void AppTimer::SetTimer(int period)
{
    if(period==0)
    {
        count=0;
        Running=false;
        Elapsed=true;
    }

    if(period>count)
    {
        count=period;
        Running=true;
        Elapsed=false;
    }

}

bool AppTimer::IsElapsed()
{
    if((count==0)||(Elapsed)) return (true);

    return (false);


}


//Scatta ogni 125ms
void AppTimer::Run()
{

    if(Running==true)
    {

        if(count>0)count--;

        if(count>UN_SECONDO)
        {
            TempInSec=(float)count/(float) UN_SECONDO;
        }
        else TempInSec=0;

    }

    if(count==0)
    {
        Elapsed=true;
        Running=false;
    }

}

void AppTimer::Info()
{
    if(TempInSec==0)snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"%s: SCADUTO\r\n",Name);
    else snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"%s: %d\r\n",Name,TempInSec);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
}


void AppTimer::PrintObj()
{
   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*Timer                : %s \r\n",Name);
   SerialPrint.Flush(PRINT_DEBUG_ALL);

}












