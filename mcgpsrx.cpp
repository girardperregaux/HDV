#include "mcgpsrx.h"


#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>
#include <QSerialPort>
#include "HDV.h"
#include "tstate.h"
#include "position.h"

extern MCSerial SerialPrint;
extern TConfig Configuration;
extern char buff_print[];

#define STR_SERIAL_GPS "/dev/ttymxc2"
#define SYNC_CHR1 0xB5
#define SYNC_CHR2 0x62
#define PDOP_POS	18
#define PACC_POS	22
#define STATIC_HOLD_THR_POS 26
#define GPS_MODE_POS	4
// define pacchetto binario
////////////////////////////
#define GPS_SIRF_START  0xA0
#define GPS_SIRF_B0     0xB0
#define GPS_SIRF_B3     0xB3
// define pacchetti attesi come risposta
#define VERSION         0x06
#define PARAM           0x13
// OFFSET
#define PAYLOAD_OFFSET           5
#define CLASS_OFFSET             2
#define ID_OFFSET                3
#define OFFSET_PAYLOAD           6

#define CLASS_AID             0x0b
#define ID_ALPSRV             0x32


#define HWMON_ASTATUS_OFFSET     20
#define HWMON_APOWER_OFFSET      21
#define HWMON_NOISEPERMS_OFFSET  16
#define VERMON_SW_OFFSET         0
#define VERMON_HW_OFFSET         30
#define PCK_CLASS_MON	0x0A
#define PCK_ID_HW		   0x09
#define PCK_ID_VER		0x04

#define POS_TALKER	       2
#define NUM_TALKER_NEED	   3
#define TALKER_ID_NONE	   0
#define TALKER_ID_GPS	  'G'
#define TALKER_ID_GLONASS 'L'
#define TALKER_ID_ANY	  'N'

/*
#define a1200bps 1200
#define a2400bps 2400
#define a4800bps 4800
#define a9600bps 9600
#define a19200bps 19200
#define a38400bps 38400
#define a38400bps 38400*/
/*
enum GPSRXHWSTATES {
   GPSRXHW_INIT = 0,
   GPSRXHW_IS_OFF,
   GPSRXHW_REQ_ON,
   GPSRXHW_WAIT_ON,
   GPSRXHW_INIT_PARAM,
   GPSRXHW_REQ_OFF,
   GPSRXHW_WAIT_OFF,
   GPSRXHW_ON,
   GPSRXHW_WAIT_FOR_INIT
};*/

enum GPSRX_STATE {

    GPSRX_STATE_INIT_SERIAL =0,
    GPSRX_STATE_WAIT_SERIAL_OPEN,
    GPSRX_STATE_INIT_PARAM,
    GPSRX_STATE_DEACTIVATE_EXTENDED,
    GPSRX_STATE_SET_TALKER,
    GPSRX_STATE_RUN,

};

enum GPSRX_STATE_RUN {

    GPSRX_STATE_RUN_IDLE =0,
    GPSRX_STATE_RUN_GETNMEA,
    GPSRX_STATE_RUN_GETUBX,

};



/* Private macro -------------------------------------------------------------*/
#define GPS_NUM_RETRY_MUX_OPEN 3

//GGA
quint8 UBX_CFG_MSG_E_GGA[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x00, 0x00, 0x01, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_GGA[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00};
// GLL
quint8 UBX_CFG_MSG_E_GLL[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x01, 0x00, 0x02, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_GLL[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00};
// GSA
quint8 UBX_CFG_MSG_E_GSA[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x02, 0x00, 0x04, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_GSA[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00};
// GSV
quint8 UBX_CFG_MSG_E_GSV[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x03, 0x00, 0x04, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_GSV[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00};
// RMC
quint8 UBX_CFG_MSG_E_RMC[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x04, 0x00, 0x01, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_RMC[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00};
// VTG
quint8 UBX_CFG_MSG_E_VTG[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x05, 0x00, 0x02, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_VTG[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00};
// GRS
quint8 UBX_CFG_MSG_E_GRS[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x06, 0x00, 0x02, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_GRS[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x06, 0x00, 0x00, 0x00, 0x00};
// GST
quint8 UBX_CFG_MSG_E_GST[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x07, 0x00, 0x02, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_GST[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x07, 0x00, 0x00, 0x00, 0x00};
// ZDA
quint8 UBX_CFG_MSG_E_ZDA[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x08, 0x00, 0x02, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_ZDA[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00};
// GBS
quint8 UBX_CFG_MSG_E_GBS[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x09, 0x00, 0x02, 0x00, 0x00};
quint8 UBX_CFG_MSG_D_GBS[] = {0x06, 0x01, 0x06, 0x00, 0xF0, 0x09, 0x00, 0x00, 0x00, 0x00};

// Pacchetto MON_VER
// Legge la versione
quint8 MON_VER[] = {0x0A,0x04,0x00,0x00};

// Pacchetto MON_HW
quint8 MON_HW[] = {0x0A,0x09,0x00,0x00};

quint8 UBX_CFG_NMEA[] = {0x06,0x17,0x04,0x00,0x00,0x23,0x00,0x02};
quint8 UBX_CFG_ITFM[] = {0x06,0x39,0x08,0x00,0xF3,0xAC,0x62,0xAD,0x1E,0x6B,0x00,0x00};


// RESET
quint8 UBX_CFG_MSG_RST_WAR[] ={0x06, 0x04, 0x04, 0x00, 0x01, 0x00, 0x02, 0x00};
quint8 UBX_CFG_MSG_RST_MC[] ={0x06, 0x04, 0x04, 0x00, 0xED, 0xFE, 0x02, 0x00};
quint8 CFG_RST[] = {0x06,0x04,0x04,0x00,0xff,0xff,0x00,0x00};





//Pacchetto UBX_CFG_NAV5
quint8 UBX_CFG_NAV5[] = { 0x06, 0x24,
                           0x24, 0x00,    // lunghezza
                           0xFF, 0xFF,    // mask
                           0x04,          // dynModel
                           0x03,          // fixMode

                           0x00, 0x00, 0x00, 0x00,
                           0x10, 0x27, 0x00, 0x00,
                           0x05,
                           0x00,
                           0xFA, 0x00,
                                         0xFA, 0x00,
                           0x64, 0x00,
                           0x2C, 0x01,
                           0x00,             //Static hold threshold
                           0x00,
                           0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00,

                         };


// Pacchetto UBX_CFG_RXM
quint8 UBX_CFG_RXM[]	   = {0x06, 0x11, 0x02, 0x00, 0x02, 0x00};

// Pacchetto UBX_CFG_INF
// Messaggio INF solo su UART1 su protocollo NMEA
// Messaggi abilitati : ERROR WARNING NOTICE USER
quint8 UBX_CFG_INF[]	  = {0x06, 0x02, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xE7, 0x00, 0x00};
//uint8_t UBX_CFG_MSG_E_ALP[] = {0x06, 0x01, 0x08, 0x00, 0x0B, 0x32, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
//uint8_t UBX_CFG_MSG_D_ALP[] = {0x06, 0x01, 0x08, 0x00, 0x0B, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

quint8 UBX_zzzz[] = {0x06, 0x01, 0x08, 0x00, 0x01, 0x30, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00};

//const char UBX_CFG_MSG_E_AID_ALM[] = {0x06, 0x01, 0x08, 0x00, 0x0B, 0x30, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00};
quint8 UBX_CFG_SAVE[]	  = {0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,0x01};

quint8 UBX_CFG_ANT[]      = {0x06, 0x13, 0x04, 0x00, 0x1b, 0x00, 0x8b, 0x29};

//Satellite Based Augmentation Systems
quint8 UBX_CFG_SBAS_DISABLE[]=     {0x06, 0x16, 0x08, 0x00, 0x00, 0x03, 0x03, 0x00, 0xD5, 0xCA, 0x06, 0x00};
quint8 UBX_CFG_SBAS_ENABLE_EGNOS[]={0x06, 0x16, 0x08, 0x00, 0x01, 0x03, 0x03, 0x00, 0x51, 0x08, 0x00, 0x00};

//quint8 UBX_CFG_SBAS_DISABLE[]=     {0x06, 0x16, 0x08, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
//quint8 UBX_CFG_SBAS_ENABLE_EGNOS[]={0x06, 0x16, 0x08, 0x00, 0x01, 0x03, 0x03, 0x00, 0x51, 0x00, 0x00, 0x00};

//fusi begin
/*
0 GPS
1 SBAS
2 Galileo
3 BeiDou
4 IMES
5 QZSS
6 GLONASS
*/
//modificata sigCfgMask
const char UBX_CFG_GNSS_GPS_SBAS_GLONASS[] ={
0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00, 0x20, 0x05,
0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00,
0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00,
0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00,
0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
0x06, 0x08, 0x0E, 0x00, 0x01, 0x00, 0x01, 0x00 };

const char UBX_CFG_GNSS_GPS_GLONASS[] ={
0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00, 0x20, 0x05,
0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00,
0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00,
0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
0x06, 0x08, 0x0E, 0x00, 0x01, 0x00, 0x01, 0x00 };

//modificata sigCfgMask
const char UBX_CFG_GNSS_GPS_SBAS[] ={
0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00, 0x20, 0x05,
0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00,
0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00,
0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00,
0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
0x06, 0x08, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x00};

const char UBX_CFG_GNSS_GPS[] ={
0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00, 0x20, 0x05,
0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x00,
0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00,
0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00,
0x06, 0x08, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x00};

//fusi end

quint8 UBX_CFG_NAVX5_ASSISTNOW_AUT_ENABLE[]={0x06, 0x23, 0x28, 0x00, 0x00, 0x00, 0x4C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x10, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
quint8 UBX_CFG_NAVX5_ASSISTNOW_AUT_DISABLE[] ={0x06, 0x23, 0x28, 0x00, 0x00, 0x00, 0x4C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x10, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };







MCGpsRx::MCGpsRx(QObject *parent) :
    QObject(parent)
{

    connect(&GpsRxT, SIGNAL(timeout()), this, SLOT(RunGpsRx()));
    //input_index=0;
    //fusi begin
    index_buffer_in=0;
    index_buffer_out=0;
    memset(SWVersion, 0, sizeof(SWVersion));
    memset(HWVersion, 0, sizeof(HWVersion));
    APower = 0;
    NoisePerMS = 0;
    jamInd = 0;
    GpsRxState=GPSRX_STATE_INIT_SERIAL;
    //fusi end
}

void MCGpsRx::Init(TimerMan *t)
{
    pREGTIMER=t;

    GpsRxTimer.Init((char*)("GpsRxTimer"));
    pREGTIMER->RegisterTimer(&GpsRxTimer);
    GpsRxDelay.Init((char*)("GpsRxDelay")); //fusi+
    pREGTIMER->RegisterTimer(&GpsRxDelay); //fusi+

    gpsnmea = new MCGpsNmea(this); //fusi +
    gpsserial = new QSerialPort(this); //fusi +
    connect(gpsserial, &QSerialPort::readyRead, this, &MCGpsRx::GpsSerialIrqRx); //fusi +
    //connect(gpsserial, SIGNAL(&QSerialPort::readyRead),this, SLOT(GPSSerialIrqRx())); //fusi - //non funziona scritto così - non va in irq
    GpsRxState=GPSRX_STATE_INIT_SERIAL;
}

void MCGpsRx::Start(int time)
{
    GpsRxT.setInterval(time);
    GpsRxT.start();
}


void MCGpsRx::ReStart(void)
{
    GpsRxT.stop();
    while(1)
    {
        if(GpsRxT.isActive() == false)
            break;
    }
    GpsRxState = GPSRX_STATE_INIT_SERIAL;
    gpsserial->clear(QSerialPort::AllDirections);
    gpsserial->close();
    GpsRxT.start();

}

void MCGpsRx::RunGpsRx()
{


    char RxData=0;
    quint16 k;
    bool endwhile = false;



    if(gpsserial->isOpen() == false)
    {
        //seriale chiusa
        GpsRxState = GPSRX_STATE_INIT_SERIAL;
        if(GpsRxDelay.IsElapsed())
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"%s","Seriale GPS chiusa -(V.20)\r\n");
            SerialPrint.Flush(PRINT_DEBUG_GPS);
            GpsRxDelay.SetTimer(2*UN_SECONDO);
        }
    }


    switch(GpsRxState)
    {
        case GPSRX_STATE_INIT_SERIAL:
            iStatus = GPSRECRUN_STATUS_MAINTSEQ;
            if(GetState(REQ_GPS_ON)) //non corretto ma stm32 non restituisce ancora lostato
            {
                if(!GpsSerialInit((char *)STR_SERIAL_GPS, QSerialPort::Baud9600, QSerialPort::Data8, QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl))
                {
                    GpsRxState = GPSRX_STATE_INIT_SERIAL;
                    break;
                }
                if(!GpsSerialOpen())
                {
                    GpsRxState = GPSRX_STATE_INIT_SERIAL;
                    break;
                }
                GpsRxTimer.SetTimer(2*UN_SECONDO);
                GpsRxState = GPSRX_STATE_WAIT_SERIAL_OPEN;
            }

        break;
        case GPSRX_STATE_WAIT_SERIAL_OPEN:
            iStatus = GPSRECRUN_STATUS_MAINTSEQ;
            if(!GpsRxTimer.IsElapsed())
            {
                break;
            }
            if(gpsserial->isOpen() == false)
            {
                GpsRxState = GPSRX_STATE_INIT_SERIAL;
                break;
            }

            GpsRxState = GPSRX_STATE_INIT_PARAM;
        break;
        case GPSRX_STATE_INIT_PARAM:
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_GGA, sizeof(UBX_CFG_MSG_D_GGA));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_GLL, sizeof(UBX_CFG_MSG_D_GLL));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_GSA, sizeof(UBX_CFG_MSG_D_GSA));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_GSV, sizeof(UBX_CFG_MSG_D_GSV));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_RMC, sizeof(UBX_CFG_MSG_D_RMC));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_VTG, sizeof(UBX_CFG_MSG_D_VTG));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_GRS, sizeof(UBX_CFG_MSG_D_GRS));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_GST, sizeof(UBX_CFG_MSG_D_GST));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_ZDA, sizeof(UBX_CFG_MSG_D_ZDA));
            SendUBXSequence((quint8 *)UBX_CFG_MSG_D_GBS, sizeof(UBX_CFG_MSG_D_GBS));
            SendUBXSequence((quint8 *)UBX_CFG_ITFM, sizeof(UBX_CFG_ITFM));
            GpsRxTimer.SetTimer(1*UN_OTTAVO);
            GpsRxState = GPSRX_STATE_DEACTIVATE_EXTENDED;
            iStatus = GPSRECRUN_STATUS_MAINTSEQ;

        break;
        case GPSRX_STATE_DEACTIVATE_EXTENDED:
            if(!GpsRxTimer.IsElapsed()) break;
            GpsRecDeactivateExtended();
            GpsRxTimer.SetTimer(1*UN_OTTAVO);
            GpsRxState = GPSRX_STATE_SET_TALKER;
            iStatus = GPSRECRUN_STATUS_MAINTSEQ;
            break;
       case GPSRX_STATE_SET_TALKER:
            if(!GpsRxTimer.IsElapsed()) break;
            GpsCnfParamSet();
            ReqVer = true;
            InPacketNmea = false;
            InPacketUbx = false;
            LastByteRx = '\0';
            PckSizeUbx = 0;
            GpsRxState = GPSRX_STATE_RUN;
            GpsRxStateRun = GPSRX_STATE_RUN_IDLE;
            iStatus = GPSRECRUN_STATUS_MAINTSEQ;
        break;
        case GPSRX_STATE_RUN:


            if(ReqVer && GpsRxTimer.IsElapsed())
            {
                if(GetState(DEBUG_FUNC)||GetState(DEBUG_GPS))
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GPS: REQ VER\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_GPS);
                }

                strcpy(HWVersion,STR_NO);
                strcpy(SWVersion,STR_NO);
                SendUBXSequence((quint8*)MON_VER, sizeof(MON_VER));
                SendUBXSequence((quint8*)MON_HW, sizeof(MON_HW));
                ReqVer=false;
                GpsRxTimer.SetTimer(UN_SECONDO*5);
            }

            if(GpsRxTimer.IsElapsed())
            {

                if(strcasecmp((char *)HWVersion,STR_NO)==0)ReqVer = true;
                if(GetState(DEBUG_FUNC)||GetState(DEBUG_GPS))
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GPS: UPDATE\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_GPS);
                }
                SendUBXSequence((quint8*)MON_HW, sizeof(MON_HW));
                GpsRxTimer.SetTimer(UN_SECONDO*10);
            }

            while(1)
            {
                if(!GpsGetChar(&RxData))
                {
                    iStatus = GPSRECRUN_STATUS_IDLE;
                    break;
                    //return(GPSRECRUN_STATUS_IDLE);
                }

                switch (GpsRxStateRun)
                {
                    case GPSRX_STATE_RUN_IDLE:
                        iStatus = GPSRECRUN_STATUS_WORKING;
                        if((LastByteRx == '$') && (RxData == 'G'))
                        {
                           CountRxNmea = 0;
                           BufferNmea[CountRxNmea++] = '$';
                           BufferNmea[CountRxNmea++] = RxData;
                           InPacketNmea = true;
                           GpsRxStateRun = GPSRX_STATE_RUN_GETNMEA;
                           break;
                        }
                        if((LastByteRx == SYNC_CHR1) && (RxData == SYNC_CHR2))
                        {
                           CountRxUbx = 0;
                           BufferUbx[CountRxUbx++] = SYNC_CHR1;
                           BufferUbx[CountRxUbx++] = RxData;
                           PckSizeUbx = 0;
                           InPacketUbx = true;
                           GpsRxStateRun = GPSRX_STATE_RUN_GETUBX;
                           break;
                        }
                        break;
                    case GPSRX_STATE_RUN_GETNMEA:
                        //memorizzo nel buffer il carattere ricevuto
                        //Memorizzo il byte come ultimo byte Rx
                        if(CountRxNmea < MAX_LEN_BUFFER_GPSRX_NMEA)
                        {
                            if(RxData != LF && RxData != CR)
                            {
                                BufferNmea[CountRxNmea++] = RxData;
                            }
                            if(RxData == LF)
                            {
                              BufferNmea[CountRxNmea] = 0;
                              // Aggiunge carattere NUL finale
                              // Analizza adeguatamente il pacchetto ricevuto e indica il risultato
                              InPacketNmea = false;
                              LastByteRx = RxData;
                              CountRxNmea = 0;  // Azzero Buffer

                              if(BufferNmea[POS_TALKER]==TALKER_ID_GPS)
                              {
                                  ActualTalkerID=TALKER_ID_GPS;
                              }
                              else if(BufferNmea[POS_TALKER]==TALKER_ID_GLONASS)
                              {
                                  ActualTalkerID=TALKER_ID_GLONASS;
                              }
                              else if(BufferNmea[POS_TALKER]==TALKER_ID_ANY)
                              {
                                  ActualTalkerID=TALKER_ID_ANY;
                              }
                              else
                              {
                                  ActualTalkerID=TALKER_ID_NONE;
                              }



                              GpsRxStateRun = GPSRX_STATE_RUN_IDLE;
                              iStatus = gpsnmea->ReadGpsPacket(BufferNmea);
                              endwhile = true;
                              break;
                              //return(iStatus);
                            }
                        }
                        else
                        {
                            if (GetState(DEBUG_GPS))
                            {
                                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GPS: BUFFER NMEA OVERFLOW!\r\n");
                                SerialPrint.Flush(PRINT_DEBUG_GPS);
                            }
                            InPacketNmea = false;
                            CountRxNmea = 0;  // Azzero Buffer
                            memset(BufferNmea,0,sizeof(BufferNmea));
                            GpsRxStateRun = GPSRX_STATE_RUN_IDLE;
                            iStatus = GPSRECRUN_STATUS_WORKING;
                            endwhile = true;
                            break;
                            //return(GPSRECRUN_STATUS_WORKING);
                        }


                        iStatus = GPSRECRUN_STATUS_WORKING;
                        break;
                    case GPSRX_STATE_RUN_GETUBX:
                        iStatus = GPSRECRUN_STATUS_WORKING;
                        if( CountRxUbx < MAX_LEN_BUFFER_GPSRX_UBX)
                        {
                            BufferUbx[CountRxUbx++] = RxData;
                        }
                        else
                        {
                            InPacketUbx = false;
                            //if ((GetState(DEBUG_GPS)) || (GetState(DEBUG_ERROR))  ) printf("GPS: BUFFER UBX OVERFLOW!\r\n");
                            if(GetState(DEBUG_GPS))
                            {
                                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GPS: BUFFER UBX OVERFLOW!\r\n");
                                SerialPrint.Flush(PRINT_DEBUG_GPS);
                            }
                            CountRxUbx = 0;  // Azzero Buffer
                            memset(BufferUbx,0,sizeof(BufferUbx));
                            GpsRxStateRun = GPSRX_STATE_RUN_IDLE;
                            break;
                        }
                        if((CountRxUbx >= 6) && (PckSizeUbx == 0))
                        {
                            PckSizeUbx = ((quint16)(BufferUbx[5]<<8)) + BufferUbx[4];
                        }

                        if((PckSizeUbx != 0) && (CountRxUbx >= (6 + PckSizeUbx + 2)))
                        {

                            if(GetState(DEBUG_GPS))
                            {
                                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nGPS[RX(UBX)] : ");
                                SerialPrint.Flush(PRINT_DEBUG_GPS);
                                for (k = 0; k < CountRxUbx; k++)
                                {
                                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"0x%02X ", BufferUbx[k]);
                                    SerialPrint.Flush(PRINT_DEBUG_GPS);
                                }
                                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\n");
                                SerialPrint.Flush(PRINT_DEBUG_GPS);

                            }

                            if(!UBXVerCheckSumPacket(BufferUbx))
                            {

                                PckSizeUbx = 0;
                                CountRxUbx = 0;
                                InPacketUbx = false;
                                GpsRxStateRun = GPSRX_STATE_RUN_IDLE;
                                break;
                            }
                            else
                            {
                                PckSizeUbx = 0;
                                CountRxUbx = 0;
                                InPacketUbx = false;
                                UBXReadPacket(BufferUbx);
                                GpsRxStateRun = GPSRX_STATE_RUN_IDLE;
                                iStatus = GPSRECRUN_STATUS_WORKING; ///verificare
                                endwhile = true;
                                break;
                                //return(GPSRECRUN_STATUS_WORKING); ///verificare
                            }
/*
                            if(UBXReadPacket(BufferUbx)
                            {
                                thisGPSReceiver->InPacketUBX=false;
                                thisGPSReceiver->PckSize=0;
                                return(true); // Analizza il pacchetto Sirf fornito e aggiorna
                            }

                            if (thisGPSReceiver->PckClassWaiting == thisGPSReceiver->GpsUBLOXBuffer.buff[2]
                            && thisGPSReceiver->PckIDWaiting == thisGPSReceiver->GpsUBLOXBuffer.buff[3]
                            )
                            {
                             //aaa4 ReadUBXPacket1();
                             thisGPSReceiver->InPacketUBX=false;
                             thisGPSReceiver->GpsUBLOXBuffer.count=0;
                             return(true);
                            }
                            else
                            {
                             thisGPSReceiver->InPacketUBX=false;
                             thisGPSReceiver->GpsUBLOXBuffer.count=0;
                            }*/
                        }

                        break;
                    default:
                        break;
                }//end switch


                LastByteRx = RxData;
                if(endwhile == true)
                {
                    endwhile = false;
                    break;
                }
            }//end while

        break;

        default:
        break;
    }

    //return (iStatus);
}

bool MCGpsRx::NewLine(quint8* pPack)
{
  if (strstr((char*)pPack,"\r\n"))return(true);
  else return(false);
}

//setta stato per accensione GPS
void MCGpsRx::GPSOn(void)
{
    SetState(REQ_GPS_ON,true);
}

//setta stato per spegnimento GPS
void MCGpsRx::GPSOff(void)
{
    SetState(REQ_GPS_ON,false);
}

//ritorna lo stato corrente della macchina a  stati
quint16 MCGpsRx::GetGpsRxStatus(void)
{
    return(iStatus);
}

void MCGpsRx::SendUBXSequence(quint8 *pChars, quint8 Length)
{

    quint16 Count;
    quint8 ck1=0;
    quint8 ck2=0;
    quint8 * pnt=pChars;


    for(Count=0;Count<Length;Count++)
    {
        ck1 = ck1 + *pnt++;
        ck2 = ck2 + ck1;
    }
    SendCharToGps(SYNC_CHR1);
    SendCharToGps(SYNC_CHR2);

    if(GetState(DEBUG_GPS))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nGPS[TX] : ");
        SerialPrint.Flush(PRINT_DEBUG_GPS);
    }
    for(Count = 0; Count < Length; Count++)
    {
      SendCharToGps(pChars[Count]);
      if(GetState(DEBUG_GPS))
      {
          snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"0x%02x ",pChars[Count]);
          SerialPrint.Flush(PRINT_DEBUG_GPS);
      }

    }
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\n");
    SerialPrint.Flush(PRINT_DEBUG_GPS);

    SendCharToGps(ck1);
    SendCharToGps(ck2);

}

bool MCGpsRx::UBXVerCheckSumPacket(quint8 *pPack)
{

    quint8 ck1=0;
    quint8 ck2=0;
    quint16 len;
    quint16 i;

    len = UBXGetLenPayload(pPack);

    for(i=2;i<len+6;i++)
    {
        ck1 = ck1 + pPack[i];
        ck2 = ck2 + ck1;
    }

    if(ck1==pPack[len+6] && ck2==pPack[len+7]) return(true);
    else return(false);

}

quint16 MCGpsRx::UBXGetLenPayload(quint8 *pPack)
{
    return(((quint16)(pPack[5]<<8)) + pPack[4]);
}

bool MCGpsRx::UBXReadPacket(quint8 *pPack)
{


    switch (pPack[CLASS_OFFSET])
    {

        case PCK_CLASS_MON: //0x0A
            if(pPack[ID_OFFSET] == PCK_ID_HW) //0x09
            {

                APower = pPack[27];
                NoisePerMS = ((quint16)(pPack[22+1]<<8)) + pPack[22];
                jamInd = (pPack[28] & 0x0C);


                if(jamInd == 0x0C) SetState(JAMMING_GPS,true);
                else SetState(JAMMING_GPS,false);

                return(true);
            }

            if(pPack[ID_OFFSET] == PCK_ID_VER) //0x04
            {
                quint8* pt;
                pt = &pPack[6];
                strncpy(SWVersion, (char*)pt, LEN_SW_VERSION);
                SWVersion[LEN_SW_VERSION] = 0;

                pt = &pPack[36];
                strncpy(HWVersion,(char*)pt,LEN_HW_VERSION);
                HWVersion[LEN_HW_VERSION] = 0;
                return(true);
            }
            break;
        default:
        break;
    }
    return(false);
}


//fusi begin
bool MCGpsRx::GpsSerialInit(char *pName, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity Parity, QSerialPort::StopBits nStop, QSerialPort::FlowControl ctrlFlow )
{

    gpsserial->setPortName(pName);
    if(!gpsserial->setBaudRate(baudRate))
    {
        return(false);
    }
    if(!gpsserial->setDataBits(dataBits))
    {
        return(false);
    }
    if(!gpsserial->setParity(Parity))
    {
        return(false);
    }
    if(!gpsserial->setStopBits(nStop))
    {
        return(false);
    }
    if(!gpsserial->setFlowControl(ctrlFlow))
    {
        return(false);
    }


    return(true);
}

//apre la seriale
bool MCGpsRx::GpsSerialOpen(void)
{
    if(gpsserial->open(QIODevice::ReadWrite))
    {
        if(!gpsserial->clear(QSerialPort::AllDirections))
        {
            return(false);
        }
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"serial open ok\r\n");
        SerialPrint.Flush(PRINT_DEBUG_GPS);
        return(true);
    }
    else
    {
        return(false);
    }
}
//riempie buffer di ricezione seriale
bool MCGpsRx::GpsSerialIrqRx(void)
{
    while(gpsserial->bytesAvailable())
     {
         if(gpsserial->read(&gpsRxBuffer[index_buffer_in++],1) != 1)
             return(false);
         if(index_buffer_in>=MAX_LEN_GPSSERIAL_BUFFER)index_buffer_in=0;
     }
     return(true);
}
//svota buffer ricezione seriale
bool MCGpsRx::GpsGetChar(char *c)
{                             // Ottiene un carattere
    if(index_buffer_out!=index_buffer_in)
    {
        *c=gpsRxBuffer[index_buffer_out++];
        if(index_buffer_out>=MAX_LEN_GPSSERIAL_BUFFER)index_buffer_out=0;
        return(true);
    }
    return(false);
}
//
bool MCGpsRx::SendCharToGps(char c)
{
    qint64 wchar;
    QByteArray tmparray;

    tmparray.append(c);

    wchar = gpsserial->write(tmparray);
    if(wchar == 1)
        return(true);
    return(false);
}

void MCGpsRx::GpsRecActivateExtended(void)
{
    WorkData.bSatNumber = 0;
    WorkData.bPRNNumber = 0;
    WorkData.PDOP = 0;
    WorkData.VDOP = 0;

    // GGA ENABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_E_GGA,sizeof(UBX_CFG_MSG_E_GGA));
    // GSV ENABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_E_GSV,sizeof(UBX_CFG_MSG_E_GSV));
    // GSA ENABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_E_GSA,sizeof(UBX_CFG_MSG_E_GSA));
    // RMC ENABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_E_RMC,sizeof(UBX_CFG_MSG_E_RMC));
    Extend=true;


}
void MCGpsRx::GpsRecDeactivateExtended(void)
{
    // Azzera buffer informazioni estese
    WorkData.bSatNumber = 0;
    WorkData.bPRNNumber = 0;
    WorkData.PDOP = 0;
    WorkData.VDOP = 0;

    // GGA ENABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_E_GGA,sizeof(UBX_CFG_MSG_E_GGA));
    // GSV DISABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_D_GSV,sizeof(UBX_CFG_MSG_D_GSV));
    // GSA DISABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_D_GSA,sizeof(UBX_CFG_MSG_D_GSA));
    // RMC ENABLE
    SendUBXSequence((quint8*)UBX_CFG_MSG_E_RMC,sizeof(UBX_CFG_MSG_E_RMC));
    Extend=false;

}


//   funzione
//   GPSCnfParamSet  -> Imposta i parametri del ricevitore uBLOX GPS come da CNF
//   Parametri       -> void
//   Result          -> void
//----------------------
void MCGpsRx::GpsCnfParamSet(void)
{
    quint8 lenPacket;
    quint8 Message[128];
    quint8 j=0;
    quint8 sht = 0;
    bool egnos = false;

    for (j=0;j<4;j++) // Numero di pacchetti da inviare
    {
        lenPacket = 0;

        switch(j)
        {
            case 0:// UBX_CFG_NAV5
                lenPacket=sizeof(UBX_CFG_NAV5);
                memcpy(Message,UBX_CFG_NAV5,sizeof(UBX_CFG_NAV5));
                //Imposto il valore di PDOP da CNF
                sht = atoi(Configuration.ConfigGetActualParamValue(ID_GPS_FILTRO_POS_IN_SOSTA));
                if(sht != 0)
                {
                    Message[STATIC_HOLD_THR_POS] = sht;  //(cm/s) Static hold threshold
                }
                else Message[STATIC_HOLD_THR_POS] = 0;
                break;

            case 1:
                lenPacket = 0;
                if(atoi(Configuration.ConfigGetActualParamValue(ID_GPS_ABILITA_EGNOS)))
                {
                     SendUBXSequence((quint8*)UBX_CFG_SBAS_ENABLE_EGNOS, sizeof(UBX_CFG_SBAS_ENABLE_EGNOS));
                     egnos = true;
                }
                else
                {
                    SendUBXSequence((quint8*)UBX_CFG_SBAS_DISABLE, sizeof(UBX_CFG_SBAS_DISABLE));
                    egnos = false;
                }
                break;
            case 2:
                lenPacket = 0;
                if(atoi(Configuration.ConfigGetActualParamValue(ID_GPS_ABILITA_GLONASS)))
                {
                    if(egnos) SendUBXSequence((quint8*)UBX_CFG_GNSS_GPS_SBAS_GLONASS, sizeof(UBX_CFG_GNSS_GPS_SBAS_GLONASS));
                    else SendUBXSequence((quint8*)UBX_CFG_GNSS_GPS_GLONASS, sizeof(UBX_CFG_GNSS_GPS_GLONASS));
                }
                else
                {
                    if(egnos) SendUBXSequence((quint8*)UBX_CFG_GNSS_GPS_SBAS, sizeof(UBX_CFG_GNSS_GPS_SBAS));
                    else SendUBXSequence((quint8*)UBX_CFG_GNSS_GPS, sizeof(UBX_CFG_GNSS_GPS));
                }
                break;
            case 3:
                lenPacket = 0;
                if(atoi(Configuration.ConfigGetActualParamValue(ID_GPS_MODALITA_AGPS)))
                {
                    SendUBXSequence((quint8*)UBX_CFG_NAVX5_ASSISTNOW_AUT_ENABLE, sizeof(UBX_CFG_NAVX5_ASSISTNOW_AUT_ENABLE));
                }
                else SendUBXSequence((quint8*)UBX_CFG_NAVX5_ASSISTNOW_AUT_DISABLE, sizeof(UBX_CFG_NAVX5_ASSISTNOW_AUT_DISABLE));
                break;

            default:
                lenPacket=0;
                break;
        }

        if(lenPacket)
            SendUBXSequence(Message,lenPacket);


    }
}






void MCGpsRx::PrintGpsRxObj(void)
{

   //char strtimer[TIMER_USERTIMESIZE+1];


   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*** INFO UBLOX ***\r\n");
   SerialPrint.Flush(PRINT_DEBUG_ALL);

   // Inizio scrittura oggetto

   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GPSReceiver: ");
   SerialPrint.Flush(PRINT_DEBUG_ALL);

   if(GetState(GPS_ON)) snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT," Attivo (+++verificare lo stato+++)\r\n");
   else snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT," Inattivo (+++verificare lo stato+++)\r\n");
   SerialPrint.Flush(PRINT_DEBUG_ALL);


   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"      POWER ANTENNA   %d\r\n", APower);
   SerialPrint.Flush(PRINT_DEBUG_ALL);
   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"      NoisePerMS      %d\r\n", NoisePerMS);
   SerialPrint.Flush(PRINT_DEBUG_ALL);
   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"      Versione SW     %s\r\n", SWVersion);
   SerialPrint.Flush(PRINT_DEBUG_ALL);
   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"      Versione HW     %s\r\n", HWVersion);
   SerialPrint.Flush(PRINT_DEBUG_ALL);
   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"      Jamming         ");
   SerialPrint.Flush(PRINT_DEBUG_ALL);
   if(jamInd == 0x04) snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"OK");
   else if(jamInd == 0x0C) snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CRITICAL");
   else if(jamInd == 0x08) snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WARNING");
   else snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DISABLED");
   SerialPrint.Flush(PRINT_DEBUG_ALL);
   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\n");
   SerialPrint.Flush(PRINT_DEBUG_ALL);


   /*
   // Valori interni e timer
   printf("      RunningState : %d\r\n", thisGPSReceiver->bRunningState);
   printf("      %-" TIMER_NAMESIZE_S "s: %s\r\n", &(thisGPSReceiver->GpsTimer.strName),
   TTimerObj(&thisGPSReceiver->GpsTimer)->UserTime(strtimer)); //KW: ATT!

   // Dati Codec
   TNMEACodecObj(&thisGPSReceiver->CodecNMEA)->PrintObj();

   // Fine scrittura oggetto
   printf("**\r\n");*/
}

//fusi end

