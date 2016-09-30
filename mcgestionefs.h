#ifndef MCGESTIONEFS_H
#define MCGESTIONEFS_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "mcdatetime.h"
#include <apptimer.h>
#include "timerman.h"



class Mcgestionefs : public QObject
{
    Q_OBJECT
public:
    explicit Mcgestionefs(QObject *parent = 0);

    AppTimer TimerFs;
    quint8 FsState;
    quint8 VMsata;
    quint8 retry;

    bool ReqFormat;

    bool MsataIsPresent;
    bool MsataIsReady;
    bool MsataIsFormat;

signals:

public slots:
   void Init(TimerMan *t);
   void Run();
   void ReadFs();
   bool ReadMnt();
   void Format();


private:
   QTimer FsTimerRun;
   TimerMan *pREGTIMER;

};

#endif // MCGESTIONEFS
