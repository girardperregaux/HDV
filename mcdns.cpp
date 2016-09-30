#include "mcdns.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QDebug>
#include <qstring.h>
#include <QFile>
#include "mcserial.h"
#include <QCryptographicHash>
#include "config.h"


extern TConfig Configuration;
extern MCSerial SerialPrint;
extern char buff_print[];

mcdns::mcdns(QObject *parent) :
    QObject(parent)
{
//costruttore

}

void mcdns::Init(TimerMan *t)
{

    pREGTIMER=t;

    TimerT.Init("mcdns");


    pREGTIMER->RegisterTimer(&TimerT);

    McFramingTimer.setInterval(50);
    McFramingTimer.start();
    connect(&McFramingTimer, SIGNAL(timeout()), this, SLOT(Run()));


}

void mcdns::Run()
{

}



