#include "mcgps.h"
#include "tstate.h"





enum GPS_STATE
{
    GPS_STATE_INIT=0,
    GPS_STATE_WAIT_READ_POSITION,
    GPS_STATE_START,
    GPS_STATE_STOP,
    GPS_STATE_WAIT_STOPPED,
    GSP_STATE_WAIT_STARTED,
    GPS_STATE_RUN,

};

MCGps::MCGps(QObject *parent) :
    QObject(parent)
{
    connect(&GpsT, SIGNAL(timeout()), this, SLOT(RunGps()));
    restart = false;
}



void MCGps::Init(TimerMan *t)
{
    pREGTIMER=t;

    GpsTimer.Init((char*)("GpsTimer"));
    pREGTIMER->RegisterTimer(&GpsTimer);

    GpsState = GPS_STATE_WAIT_READ_POSITION;
}

void MCGps::Start(int time)
{
    GpsT.setInterval(time);
    GpsT.start();
}


void MCGps::GpsRun(void)
{

    switch (GpsState)
    {
        case GPS_STATE_INIT:
            GpsTimer.SetTimer(1*UN_OTTAVO);
            GpsState = GPS_STATE_WAIT_READ_POSITION;
        break;
        //attesa per la lettura del file position.pos
        case GPS_STATE_WAIT_READ_POSITION:
            if(GpsTimer.IsElapsed())
            {
                if(GetState(POSITION_RUN_STATE))
                {
                    GpsState = GPS_STATE_START;
                    break;
                }
                GpsTimer.SetTimer(1*UN_OTTAVO);
            }
        break;
        //invia comando STM32 per avvio gps
        case GPS_STATE_START:
        break;
        //attesa flag di partito da STM32 e partenza GPSRX
        case GSP_STATE_WAIT_STARTED:

        break;

        case GPS_STATE_STOP:

        break;

        case GPS_STATE_WAIT_STOPPED:

        break;

        case GPS_STATE_RUN:

        break;

    }//end switch

}
