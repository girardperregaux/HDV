#ifndef MCTRANSCODE_H
#define MCTRANSCODE_H

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
#include <mcfind.h>
#include <QStringList>

#define STATE_TRANS_NULL      0
#define STATE_TRANS_INIT      1
#define STATE_TRANS_WAIT      2
#define STATE_TRANS_TRANSCODE 3
#define STATE_TRANS_READY     4
#define STATE_TRANS_RENAME    5
#define STATE_TRANS_ERROR     6


class mctranscode : public QObject
{
    Q_OBJECT
public:
    explicit mctranscode(QObject *parent = 0);
    AppTimer TimerTranscode;
    short startTranscode;
    short UpgradeTranscode;
    quint16 countlog;
    short Transc_sleep;
    short Transcode_finish;
signals:

public slots:
    void Init(TimerMan *t);
    void Run();
    bool transc_rename(char * Tfile,quint8 formato);
    bool transc_remove(char * Tfile);

    private:
    TimerMan *pREGTIMER;
    QTimer TTranscodeRun;
    #define STRING_MAX_BUFFER_TRANSC 1000
    #define STRING_PID_TRANSC          20
    #define STRING_TRANSC             100

private:
    quint8 StateTranscode;
    MCfind MCFindTranscode;
    QStringList TranscStringList;


    QStringList argtrasc;
    char Buffstrasc[STRING_MAX_BUFFER_TRANSC+1];
    char *pBufftrasc;
    QProcess  *Gsttrasc;
    int PidGstTransc;
    //char StrPidTra[STRING_PID_TRANSC+1];
    char StrBuffTra[STRING_TRANSC+1];
    char StrRenameTra[STRING_TRANSC+1];
    char StrBuffTemp[STRING_TRANSC+1];

    char StrQuant[STRING_TRANSC+1];
    char StrFrame[STRING_TRANSC+1];

    char StrBitrate[STRING_TRANSC+1];

    quint8  QuantTrs;
    quint8  FrameTrs;

    quint8  BitrateTrs;



  //  char StrLogTra[STRING_TRANSC+1];
    quint32 indexTranscode;
    int timestamp;
    quint32 size;
};

#endif // TRANSCODE_H
