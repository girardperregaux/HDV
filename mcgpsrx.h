#ifndef MCGPSRX_H
#define MCGPSRX_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include <GpsDef.h>
#include "timerman.h"
#include <mcgpsnmea.h>
#include <position.h>


#define MAX_LEN_BUFFER_GPSRX_NMEA   512
#define MAX_LEN_BUFFER_GPSRX_UBX    512
#define MAX_LEN_GPSSERIAL_BUFFER    300 //fusi+
#define LEN_SW_VERSION  30
#define LEN_HW_VERSION  10
#define WARM_RST 1
#define MC_RST   2
#define COLD	 3
#define OFF_ON	 4




/* Exported types ------------------------------------------------------------*/




class MCGpsRx : public QObject
{
    Q_OBJECT
public:
    explicit MCGpsRx(QObject *parent = 0);
    void GPSOn(void);
    void GPSOff(void);
    void PrintGpsRxObj(void);
    quint16 GetGpsRxStatus(void);
signals:

public slots:
    void RunGpsRx(void);
    void Start(int time);
    void Init(TimerMan *t); //per passare timer
    bool GpsSerialIrqRx(void); //fusi+


private:
    TGPSData WorkData;//fusi +
    AppTimer GpsRxTimer;
    AppTimer GpsRxDelay; //fusi+
    void GpsRecActivateExtended(void);
    void GpsRecDeactivateExtended(void);
    void SendUBXSequence(quint8 *pChars, quint8 Length);
    quint8 GpsRxHwGetChar(char *c);
    bool UBXVerCheckSumPacket(quint8 *pPack);
    bool UBXReadPacket(quint8 *pPack);
    void UBXCnfParamSet(void);
    //fusi begin
    bool GpsSerialInit(char *pName, QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity Parity, QSerialPort::StopBits nStop, QSerialPort::FlowControl ctrlFlow );
    bool GpsSerialOpen(void);
    bool GpsGetChar(char *c);
    bool SendCharToGps(char c);
    quint16 UBXGetLenPayload(quint8 *pPack);
    void GpsCnfParamSet(void);
    void ReStart(void);
    //fusi end

    char gpsRxBuffer[MAX_LEN_GPSSERIAL_BUFFER]; //fusi+
    QTimer GpsRxT;
    quint16 index_buffer_in; //fusi+
    quint16 index_buffer_out; //fusi+

    //MCSerial myserial;   //fusi -
    QSerialPort* gpsserial; //fusi +
    MCGpsNmea* gpsnmea; //fusi +
    quint16 iStatus;  //fusi+

    int GpsRxState;
    int GpsRxStateRun; //fusi +
    void GpsNotification();
    TimerMan *pREGTIMER;
    bool NewLine(quint8* pPack);

    quint8 On;
    quint8 ReqOn;
    quint8 State;



    quint8 ReqExtend;
    quint8 Extend;


    quint8 APower;
    quint16 NoisePerMS;
    quint8 ReqVer;
    quint8 jamInd;
    quint8 AStatus;
    quint8 ReqWARRST;
    quint8 ReqMCRST;
    quint8 Reset;
    quint8 ReqCOLD;
    quint8 Restart;


    char SWVersion[LEN_SW_VERSION+1];
    char HWVersion[LEN_HW_VERSION+1];


    bool    InPacketNmea;
    bool    InPacketUbx;
    quint8  LastByteRx;
    quint16 PckSizeUbx;
    quint8  BufferNmea[MAX_LEN_BUFFER_GPSRX_NMEA+1];
    quint8  BufferUbx[MAX_LEN_BUFFER_GPSRX_UBX+1];
    quint16 CountRxNmea;
    quint16 CountRxUbx;
    quint8  ActualTalkerID;

};

#endif // MCGPSRX_H
