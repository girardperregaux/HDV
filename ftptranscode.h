#ifndef FTPTRANSCODE_H
#define FTPTRANSCODE_H

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




class ftptranscode : public QObject
{
    Q_OBJECT
public:
    explicit ftptranscode(QObject *parent = 0);
    AppTimer TimerTranscode;
    AppTimer TimerDelayTranscode;
    short startTranscode;
    short UpgradeTranscode;
    quint16 countlog;
    short Transc_sleep;
    bool FileFastTra;
    char file_ftpfast[STRING_FILENAME_MAX+1];
    bool EndTranscode;
    bool Start_Time_transcoder;
    quint16 Time_transcode;

#define STATE_TRANS_NULL      0
#define STATE_TRANS_INIT      1
#define STATE_TRANS_WAIT      2
#define STATE_TRANS_TRANSCODE 3
#define STATE_TRANS_READY     4
#define STATE_TRANS_RENAME    5
#define STATE_TRANS_ERROR     6

signals:

public slots:
    void Init(TimerMan *t);
    void Run();
    bool transc_rename(char * Tfile,quint8 formato);
    bool transc_remove(char * Tfile);
    bool FileReady(char *FileFtp);

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
    char StrQuantF[STRING_TRANSC+1];
    char StrFrameF[STRING_TRANSC+1];
    char StrBitrateF[STRING_TRANSC+1];

    //  char StrLogTra[STRING_TRANSC+1];
    quint32 indexTranscode;
    quint32 size;
    quint16 QuantTrsF;
    quint8  FrameTrsF;
    quint8  BitrateTrsF;

};

#endif // FTPTRANSCODE_H
