#ifndef MCFSPACE_H
#define MCFSPACE_H

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


class mcfspace : public QObject
{
    Q_OBJECT
public:
    explicit mcfspace(QObject *parent = 0);
    AppTimer TimerSpaceFs;
    short startSpaceFs;
    short UpgradeSpaceFs;
    short ReadSpace;


    quint8 FirstSpaceFs;
    quint8 SpaceOptimize;

    bool SpaceFSDirRm(char *p);
    quint16 typeformat;
signals:

public slots:
    void Init(TimerMan *t);
    void Run();
    bool SpaceFSRemove(char * Tfile);

    private:
    TimerMan *pREGTIMER;
    QTimer TSpaceFSRun;
    #define STRING_MAX_BUFFER_FS     1000
    #define STRING_SPACE             100

private:
    quint8 StateSpaceFs;
    MCfind MCFindSpaceFs;
    QStringList SpaceFsStringList;

    //char StrBuffSpaceFs[STRING_SPACE+1];
    char StrBuffSTempFs[STRING_SPACE+1];
    quint32 indexSpaceFs;

};

#endif // MCFSPACE
