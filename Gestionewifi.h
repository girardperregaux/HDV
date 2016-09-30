#ifndef GESTIONEWIFI_H
#define GESTIONEWIFI_H
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"

#define MAX_NET_BUFFER_WIFI 50

enum WIFI_CLIENT {

    WIFI_CL_INIT =0,
    WIFI_CL_ON,
    WIFI_CL_WAIT_ON,
    WIFI_CL_READY,
    WIFI_CL_NULL,
    WIFI_CL_SCAN,
    WIFI_CL_ESSID,
    WIFI_CL_CONNECT,
    WIFI_CL_CONNECT2,
    WIFI_CL_UDHCPC,
    WIFI_CL_PINGWAN,
    WIFI_CL_PINGOOGLE
};



class WifiClient : public QObject
{
    Q_OBJECT

public:
    explicit WifiClient(QObject* parent=0);
    AppTimer TimerT;
    bool ReqOn;
    char bufferwifi[MAX_NET_BUFFER_WIFI+1];
    QStringList Scan();
    QStringList ListWifi;
    void Stop();

public slots:

    void Init(TimerMan *t);

    void GestioneWifi();
    void PrintObj();
    void wifiSignalfinish(int,QProcess::ExitStatus );
    char * ReadSsid();
    bool ReadyScan();

private:

    TimerMan *pREGTIMER;
    QTimer Mcwifitimer;
    int State;

    bool WifiOn;

    QProcess  *WifiProcess;

    int ScanEssid(char *c,QStringList reti);
    bool Connect();
    bool Udhcpc();
    int PingWWW();
    bool wifi_down();
    bool wifi_up();
    bool connected;
    int CountNoConnected;
    int CountErrPing;
    char stringssid[STRING_F_MAX];
    bool finish_process;
    bool command;
    //QByteArray OutputString;


};

#endif // GESTIONEWIFI_H
