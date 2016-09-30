#ifndef CONFIG_MOBILE_H
#define CONFIG_MOBILE_H
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"
#include "config.h"


enum CONFIG_MOBILE_STATE {

    INIT =0,
    READY,
    REQ_REMLINK,
    WAIT_REQ_CONNECTED,
    WAIT_COMMAND,
    POST_ACK_CONFIGURAZIONE,
    GET_INFO_PARAMETRO,
    SEND_INFO_PARAMETRO,
    WAIT_START_SEND_PARAM,
    WAIT_ACK_INFO_PARAMETRO,
    WAIT_PARAM



};

#define MAX_STR_LEN_CC 6
#define MAX_STR_LEN_CG 2
#define MAX_STR_LEN_CP 4
#define MAX_LEN_PCK_TCP (MAX_LEN_SMS*2)


// define dell'header pacchetto
#define  PRO_PCKID_IDX                 0
#define  PRO_IDN_IDX                   1
#define  PRO_TIPOCENTRALE_IDX          1
#define  PRO_CODICECENTRALE_IDX        2
#define  PRO_CODICEGRUPPO_IDX          8
#define  PRO_CODICEPRIFERICA_IDX       10
#define  PRO_PROGRESSIVO_IDX           14
#define  PRO_DIRECTION_IDX             17
#define  PRO_RICHIESTACONFERMA_IDX     17
#define  PRO_RISPOSTA_IDX              18
#define  PRO_DATA_IDX                  19

// define (offset da PRO_DATA_IDX) parte data del pacchetto
#define  PRO_CNF_IDSESSIONE_IDX 0
#define  PRO_CNF_SID_IDX       PRO_CNF_IDSESSIONE_IDX+2
#define  PRO_CNF_CNF_IDX       PRO_CNF_SID_IDX+2


#define  MOBILE_INDEX_TIPO_PACCHETTO  0
#define  MOBILE_INDEX_SENDER          1
#define  MOBILE_INDEX_VER_PROTOCOL    2
#define  MOBILE_INDEX_CODICECENTRALE  3
#define  MOBILE_INDEX_CODICEGRUPPO    9
#define  MOBILE_INDEX_CODICEPRIFERICA 11
#define  MOBILE_INDEX_PROGRESSIVO     15
#define  MOBILE_INDEX_ACK_RICHIESTO   18
#define  MOBILE_INDEX_RISPOSTA        19
#define  MOBILE_INDEX_CONTENUTO       20


#define MOBILE_PING                   '1'
#define MOBILE_RICHIESTA_GENERALITA   '2'
#define MOBILE_RICHIESTA_CONFIGURAZIONE '3'
#define MOBILE_PARAMETRI_CONFIGURAZIONE   '4'
#define MOBILE_START_INVIO_CONFIGURAZIONE  '5'
#define MOBILE_INVIO_PARAMETRO    '6'

#define ACK_PARAMETRI_CONFIGURAZIONE '4'



/* Exported types ------------------------------------------------------------*/

typedef struct
{
  char Pacchetto[MAX_LEN_SMS+1];
  char TipoPacchetto;
  char Sender;
  char ProtocolVer;
  char CC[MAX_STR_LEN_CC+1];
  char CG[MAX_STR_LEN_CG+1];
  char CP[MAX_STR_LEN_CP+1];
  char RichiestaRisposta;


  quint16 Progressivo;
}TMobileMessageIn;

typedef struct
{
  char Pacchetto[MAX_LEN_PCK_TCP+1];
  quint8 retry;

}TMobileMessageOut;



class ConfigMobile : public QObject
{
    Q_OBJECT

public:
    explicit ConfigMobile(QObject* parent=0);

    quint8 ConfigMobileGetInputStr(void);
    void ConfigMobilePutString(char *s);

    // void ConfgMobilePrint(TConfiguration *p);
    quint8 ConfgMobileSave(void);

    quint8 ConfigMobileMessageInCreate(TMobileMessageIn *MobileMessageIn);
    quint8 ConfigMobileGetMessageIn(TMobileMessageIn *MobileMessageIn);
    void FillRichiestaConfigurazione(TMobileMessageOut *MobileMessageOut);
    void FillGeneralita(TMobileMessageOut *MobileMessageOut);
    void FillParametro(TMobileMessageOut *MobileMessageOut,quint16 ID);
    void FillRichiestaScritturaConfigurazione(TMobileMessageOut *MobileMessageOut,quint8 ok);
    void FillAckParametro(TMobileMessageOut *MobileMessageOut);
    void FillStatoPosizioneSpont(TMobileMessageOut *MobileMessageOut);
    void FillStatoPosizione(TMobileMessageOut *MobileMessageOut);




    void Init(TimerMan *t);
    bool Start(char *ip, char *porta,char *login,char *password,char *APN,char *NumTelSMS);
    void Abort();

    AppTimer Delay;
    AppTimer TimeOut;

    AppTimer LiveTimer;
    quint8 state;




public slots:
    void Run();

private:

    quint8 stato;
    bool go;


    char ip[MAX_LEN_CAMPO+1];
    char port[MAX_LEN_CAMPO+1];
    char RemLinkLogin[MAX_LEN_CAMPO+1];
    char RemLinkPassword[MAX_LEN_CAMPO+1];

//    TBuffer BufferRx;

    char ConnectedProtocolVer;
    char ConnectedCC[MAX_STR_LEN_CC+1];
    char ConnectedCG[MAX_STR_LEN_CG+1];
    char ConnectedCP[MAX_STR_LEN_CP+1];


    quint16 ParamID;
    quint16 NumParam;
    quint8 ConfigEnable;
    quint8 State;
    quint8 ConfigChanged;
    quint16 ProgOut;
    quint16 ProgIn;

    quint8 RemLinkConnected;

    quint16 ProgOutPing;

    quint8 AckRicevuto;
    quint16 ProgAckAtteso;
    quint16 ProgAckRicevuto;


    QTimer TimerRun;
    TimerMan *pREGTIMER;
    quint8 fsmState;


    TMobileMessageIn  MobileMessageIn;
    TMobileMessageOut MobileMessageOut;
    TMobileMessageOut MobileMessageFsmOut;

    TMobileMessageIn MobileMessagePingIn;


    char ConfigMobileLineIn[MAX_LEN_PCK_TCP+1];
    quint16 ConfigMobileCount;
    char    ConfigMobileLineOut[MAX_LEN_PCK_TCP+1];
    quint16 ConfigMobileLineOutCount;

    char CONFIGMOBILE_CAMPO1 [MAX_LEN_CAMPO+1];
    char CONFIGMOBILE_CAMPO2 [MAX_LEN_CAMPO+1];
    char CONFIGMOBILE_CAMPO3 [MAX_LEN_CAMPO+1];

    TConnectionParam InfoCon;




};

#endif // CONFIG_MOBILE_H
