#ifndef MCMODEM_H
#define MCMODEM_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"
#include <QtGlobal>
#include "message.h"


#define MAX_LEN_BUFFER_MODEM 600

#define STATE_ATC_IDLE       0  // è possibile inviare comandi
#define STATE_ATC_WAIT       1
#define STATE_ATC_OK         2
#define STATE_ATC_TOUT       3
#define STATE_ATC_ERROR      4
#define STATE_ATC_WAIT_CTS   5
#define STR_ATC_RESET       "ZZRESETK"



/* Exported types ------------------------------------------------------------*/
enum MODEMGSM_STATE {


    MODEMGSM_INIT =0,
    MODEMGSM_OFF,
    MODEMGSM__WAITING_OFF,
    MODEMGSM_ON,
    MODEMGSM_WAITING_ON,
    MODEMGSM_CHECK_SERIAL,


    MODEMGSM_SEND_REQ_ON,
    MODEMGSM_WAIT_ON,

    MODEMGSM_WAIT_SERIAL,
    MODEMGSM_AT,
    MODEMGSM_ATE,
    MODEMGSM_ATI,
    MODEMGSM_INIT_CGSN,
    MODEMGSM_SYSCFGEX,
    MODEMGSM_WAKEUPCFG,
    MODEMGSM_INIT_CREG,
    MODEMGSM_INIT_CMGF,
    MODEMGSM_INIT_CPMS,
    MODEMGSM_INIT_CNMI,
    MODEMGSM_CFUN4,

    MODEMGSM_INIT_CLIP,
    MODEMGSM_INIT_CRC,
    MODEMGSM_DELAY_CONNECTED,

    MODEMGSM_INIT_CBST,
    MODEMGSM_INIT_CSMP,

    MODEMGSM_SETUP_APN,
    MODEMGSM_ACTIVATE_GPRS,
    MODEMGSM_CHECK_CONN_GPRS,
    MODEMGSM_SIGNAL_CGATT,
    MODEMGSM_SIGNAL_UPSD,
    MODEMGSM_DEACTIVATE_GPRS,

    MODEMGSM_READY,
    MODEMGSM_READY_RF_OFF,


    MODEMGSM_SIGNAL_CSQ,
    MODEMGSM_SIGNAL_CGED,
    MODEMGSM_SIGNAL_UREG,
    MODEMGSM_SIGNAL_CREG,
    MODEMGSM_SIGNAL_MONSC,
    MODEMGSM_SIGNAL_CPAS,
    MODEMGSM_SIGNAL_CMGL,
    MODEMGSM_CMGR,
    MODEMGSM_CMGD,
    MODEMGSM_CMGD_ALL,
    MODEMGSM_CMGS,
    MODEMGSM_SEND_SMSDATA,
    MODEMGSM_ATA,
    MODEMGSM_ATH,
    MODEMGSM_ATDV,
    MODEMGSM_INIT_URAT,
    MODEMGSM_COPS0,

    MODEMGSM_COPS2,
    MODEMGSM_UCGDFLT,

    MODEMGSM_GPS_ON,
    MODEMGSM_GPS_OFF,
    MODEMGSM_GPS_CHECK,
    MODEMGSM_CFUN_CHECK,
    MODEMGSM_DEREG_UREG2,
    MODEMGSM_INIT_UREG,
    MODEMGSM_CFUN_0,
    MODEMGSM_CFUN_1,
    MODEMGSM_GPS_ON_RF_OFF,
    MODEMGSM_GPS_OFF_RF_OFF,
    MODEMGSM_GPS_CHECK_RF_OFF,
    MODEMGSM_GPS_CHECK_RF_OFF_A,
    MODEMGSM_IN_SLEEP,
    MODEMGSM_ATEK3,
    MODEMGSM_INIT_UPSV,
    MODEMGSM_SLEEP_DTROFF,
    MODEMGSM_SLEEP_DTRON,
    MODEMGSM_NDISDUP,
    MODEMGSM_NDISSTATQRY,
    MODEMGSM_URAT,
    MODEMGSM_UCGATT,
    MODEMGSM_CGEREP,
    MODEMGSM_CGDCONT,
    MODEMGSM_CGACT,
    MODEMGSM_CGACT_0,
    MODEMGSM_SIGNAL_CGDCONT,
    MODEMGSM_SIGNAL_CGACT,
    MODEMGSM_CFUN1,

    MODEMGSM_SIGNAL_CGEQREQ,
    MODEMGSM_ASSENTE,
    MODEMGSM_UIPTABLES,
    MODEMGSM_USOCR,
    MODEMGSM_UDNSR,
    MODEMGSM_USOST,
    MODEMGSM_SEND_CTRLZ,
    MODEMGSM_IPINIT,
    MODEMGSM_IPOPEN,
    MODEMGSM_IPCLOSE,
    MODEMGSM_IPSEND,
    MODEMGSM_IPREQOPEN,
    MODEMGSM_IPLINSTEN,
    MODEMGSM_NDISDUP_OFF,



    MODEMGSM_WAIT_OFF


};


class MCModem : public QObject
{
    Q_OBJECT

public:
    explicit MCModem(QObject* parent=0);
    AppTimer ModemTimer;
    AppTimer ModemSignal;
    AppTimer ModemDelay;
    AppTimer RegControl;
    AppTimer Dereg;
    bool SMSDaInviare;
    bool ModemGSMSendSMS(TSMS *SMS);
    void ReStart();
    void PrintObj();

    bool cgactStatus;
    bool IPOPEN();
    bool ECMReq;
    bool ECMReqUpgrade;
    bool ECMReqStream;
    bool ECMReqRemLink;
    bool ECMReqUpgradeSt;
 	bool ECMReqFtp;
    bool ECMReqFtpFast;
    bool ECMReqDownload;

    bool ReqOn;

    char APNUpgrade[MAX_LEN_CAMPO+1];
    char APNStream[MAX_LEN_CAMPO+1];
    char APNRemLink[MAX_LEN_CAMPO+1];
    char APNUpgradeSt[MAX_LEN_CAMPO+1];
 	char APNFtp[MAX_LEN_CAMPO+1];
    char APNFtpFast[MAX_LEN_CAMPO+1];
    char APNReqDownload[MAX_LEN_CAMPO+1];
    bool apnIsSet;
    bool reStart;
    quint8 GSMRegistrato;        // indica quando il modem è registrato alla rete GSM
    quint8 GSMFieldIntensity;
    quint8 GSMCSQ;
    quint8 ureg;
    quint8 uBIS_link_id;
    quint8 bBIS_link_inited;
    quint8 uBIS_link_state;
    quint8 uBIS_link_opened;
    bool UDPReq;
    bool UDPSend;
    bool IPV4connected;
    quint8 stat;
    quint8 atc;
    quint16 lac;
    quint32 ci;
    quint8 network_select;
    char ImeiString[IMEI+1];
    quint16 mcc;
    quint16 mnc;


public slots:
    void GestioneModem();
    void Start(int time);
    void Init(TimerMan *t); //per passare timer
    void RequestOn();

private:
    int m_a;
    int count;
    char buffer[MAX_LEN_BUFFER_MODEM+1];
    quint16 input_index;

    bool SMSRicevuto;
    quint8 SMSIndex;
    bool SMSDaEliminare;
    quint8 SMSIndexDaEliminare;


    QTimer ModemT;
    QTimer timer;

    MCSerial myserial;
    quint8 AtCommand(char *AtCmdString,int Timeout);
    void AtNotification();
    int GetCommandId(char *command);
    bool NewLine();

    int CSQ();
    int CREG();
    bool UREG(); 
    bool CMGL();
    bool CMGR();
    bool NDISSTATQRY();
    bool CGSN();
    bool MONSC();

    int Command;
    int CommandState;
    int ModemState;





    bool PDPContextDefined;
    bool PDPContextActivated;



    quint8 CntCommandTimeOut;
    quint8 CntCommandError;
    quint8 CntCheckUSB;


    TimerMan *pREGTIMER;


};

#endif // MCMODEM_H
