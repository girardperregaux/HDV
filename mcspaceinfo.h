#ifndef MCSPACEINFO_H
#define MCSPACEINFO_H

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

#define STATE_SPACE_NULL      0
#define STATE_SPACE_INIT      1
#define STATE_SPACE_WAIT      2
#define STATE_SPACE_READY     3
#define STATE_SPACE_ERROR     4

#define STATE_SPACE_VALUE     5


typedef struct
{
    quint32 TtipoTX;
    quint32 TtipoMPX;
    quint32 TtipoTS;
    quint32 TtipoMP4;

    quint64 TSizeTX;
    quint64 TSizeMPX;
    quint64 TSizeTS;
    quint64 TSizeMP4;

} TInfoSpace;



class mcspaceinfo : public QObject
{
    Q_OBJECT
public:
    explicit mcspaceinfo(QObject *parent = 0);
    AppTimer TimerSpaceInfo;
    short startInfoFs;
    short UpgradeInfoFs;
    short ReadInfo;
    quint8 StateInfoFs;
    quint16 typeInfo;
    quint64 SpaceFileMax;
    quint64 CountFileMax;



signals:

public slots:
    void Init(TimerMan *t);
    void Run();
    void PrintObj();
    private:
    TimerMan *pREGTIMER;
    QTimer TInfoFSRun;
    #define STRING_MAX_BUFFER_FS     1000
    #define STRING_SPACE             100

private:
    quint8 StateSpaceFs;
    MCfind MCFindSpaceInfo;
    QStringList SpaceFsStringList;

    char StrBuffSTempFs[STRING_SPACE+1];
    quint32 indexSpaceInfo;

};

#endif // MCSPACEINFO
