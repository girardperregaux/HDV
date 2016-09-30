#ifndef ACCESSPOINTWIFI_H
#define ACCESSPOINTWIFI_H
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"

enum ACCESSPOINTWIFI_STATE {

    ACCESSPOINTWIFI_INIT =0,
    ACCESSPOINTWIFI_READY,
    ACCESSPOINTWIFI_SETUP_IP,
    ACCESSPOINTWIFI_SETUP_UDHCPD,
    ACCESSPOINTWIFI_SETUP_AP,
    ACCESSPOINTWIFI_SETUPED,
    ACCESSPOINTWIFI_SWITCHING_ON,
    ACCESSPOINTWIFI_ON,
    ACCESSPOINTWIFI_SWITCHING_OFF
};


class AccessPointWifi : public QObject
{
    Q_OBJECT

public:
    explicit AccessPointWifi(QObject* parent=0);
    AppTimer TimerT;

    void Init(TimerMan *t);
    void On();
    void Off();

    bool ReqOn;
    bool IsOn;
    quint16 Pid;



public slots:
    void Run();

private:

    QTimer TimerRun;
    TimerMan *pREGTIMER;
  //  QTimer timer;
    quint8 fsmState;


    QProcess  *ProcessAP;


};

#endif // ACCESSPOINTWIFI_H
