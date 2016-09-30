#include "MCTimer.h"
#include "mcserial.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>

extern MCSerial SerialPrint;
extern char buff_print[];

MCTimer::MCTimer(QObject* parent)
    :QObject(parent)
{

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(GestioneTimer()));
    count = 0;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1);
    stato_timer=0;

}

//scatta ogni 1 ms
void MCTimer::update()
{
    if(count>0)
    {
        count--;
    }
}
//Stampa il timer
void MCTimer::PrintTimer()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"Gestione timer\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"Timer:  sec %d\r\n",count);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
}

void MCTimer::Start(int time)
{

    m_timer.setInterval(time); //
    m_timer.start();
    count = 0;

}

void MCTimer::GestioneTimer()
{

    if(stato_timer==0)
    {
        count=5000;
        stato_timer=1;
    }

    if(count==1000)
    {
        PrintTimer();
    }

    if(count==0)
    {
        stato_timer=0;
    }
}

