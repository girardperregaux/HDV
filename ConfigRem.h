#ifndef CONFIGREM_H
#define CONFIGREM_H
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"
#include "config.h"





/* Exported types ------------------------------------------------------------*/



class ConfigRem : public QObject
{
    Q_OBJECT

public:
    explicit ConfigRem(QObject* parent=0);

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
    quint8 ConfigRemotaGetInputStr(void);// 1 pri
    void ConfigRemotaPutString(char *s); //2 pri
    quint8 StartConfigRemota(void); //3 pri
    quint8 AbortConfigRemota(void); //4 pri
    void ConfigRemotaPrintMenu(void); //5 pri
    void ConfigRemotaPrintSubMenu(void); //6 pri
    void ConfigCommand(void); //7 pri
    void ConfigRemotaPrint(TConfiguration *p); //8 pri
    quint8 ConfigRemotaShowParam(quint8 ID); //9 pri
    quint8 ConfigRemotaInsShowParam(quint8 newID,quint8 verso); //10 pri
    quint8 ConfigRemotaSave(void); //11 pri

    quint8 stato;
    bool go;

    char ip[MAX_LEN_CAMPO+1];
    char port[MAX_LEN_CAMPO+1];
    char RemLinkLogin[MAX_LEN_CAMPO+1];
    char RemLinkPassword[MAX_LEN_CAMPO+1];

    quint16 ParamID;

    quint8 ConfigEnable;
    quint8 State;
    quint8 ConfigChanged;
    quint8 StateGestUser;
    quint8 RemLinkConnected;

    TimerMan *pREGTIMER;
    QTimer TimerRun;

    char ConfigRemotaLineIn[MAX_LEN_SMS+1];
    quint16 ConfigRemotaLineInCount;
    char    ConfigRemotaLineOut[MAX_LEN_SMS+1];
    quint16 ConfigRemotaLineOutCount;

    TConnectionParam InfoCon;

};

#endif // CONFIGREM_H
