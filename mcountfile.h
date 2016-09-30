#ifndef MCOUNT_H
#define MCOUNT_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "mcdatetime.h"
#include <apptimer.h>
#include "timerman.h"
#include <qfile.h>
#include <QtNetwork>
#include "utility.h"
#include "DefCom.h"


#define STATE_MCFIND_NULL   0
#define STATE_MCFIND_INIT   1
#define STATE_MCFIND_WAIT   2
#define STATE_MCFIND_READY  3
#define STATE_MCFIND_NULL_INIT  4
#define STATE_MCFIND_ERROR  5
#define STRING_MCF_MAX  100



class MCountfile : public QObject
{
    Q_OBJECT
public:
    explicit MCountfile(QObject *parent = 0);
    AppTimer TimerCount;
    bool startfind;
    quint8 StateFind;
    quint32 countfile_mp4;
    bool printfilemp4;
signals:

public slots:
    void Init(TimerMan *t);
    void Run();
    void PrintObj();
    quint32 Countfile();

private:

    QTimer TimerRunCount;

    TimerMan *pREGTIMER;
};

#endif // MCOUNT_H
