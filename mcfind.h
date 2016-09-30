#ifndef MCFIND_H
#define MCFIND_H

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


#define STATE_FIND_NULL   0
#define STATE_FIND_INIT   1
#define STATE_FIND_WAIT   2
#define STATE_FIND_READY  3
#define STATE_FIND_NULL_INIT  4
#define STATE_FIND_ERROR  5


typedef struct
{
    QStringList DirAllList;
    QStringList fList;
    QStringList fInitList;
    QStringList fFinishList;
    char TDateInit[MAX_LEN_STR_FOLDER+1];
    char THourInit[MAX_LEN_STR_FOLDER+1];
    char TDateFinish[MAX_LEN_STR_FOLDER+1];
    char THourFinish[MAX_LEN_STR_FOLDER+1];
    quint8 Ttipo;

} TFile;


class MCfind : public QObject
{
    Q_OBJECT
public:
    explicit MCfind(QObject *parent = 0);
    AppTimer TimerDownload;
    bool startfind;
    quint8 StateFind;
    bool printlist;
    bool folder_finish_not_found;
    quint8 Id;
signals:

public slots:
    void Init(quint8 id);
    void Reset_Init();
    void Run();
    void PrintObj();
    bool SetValidDate(char *dateinit,char *hourinit, char *datefinish,char *hourfinish,quint8 tipo);

    quint8 SetDateInit_SetDateFinish(char *dateinit,char *hourinit, char *datefinish,char *hourfinish,quint8 tipo);
    bool SetHourInit(char *p,char *hourinit,char *hourfinish,quint8 tipo);
    bool SetHourFinish(char *p,char *hourfinish,quint8 tipo);
    QStringList SetList(char *p,quint8 tipo);

    bool SetInit(char *dateinit,char *hourinit,char *datefinish,char *hourfinish,quint8 tipo);
    bool SetFinish(char *datefinish,char *hourfinish,quint8 tipo);

    QStringList SendList();

    bool SetScanList(int idx_init,int idx_hour_init,int idx_finish,int idx_hour_max,quint8 tipo,bool print);
private:
   // TimerMan *pREGTIMER;
    QTimer TimerRun;

    char directory[STRING_F_MAX+1];
    char FileDirectory[STRING_F_MAX+1];
    QDir FinDir;
    QDir FindFile;
    QStringList DirList;
    QStringList fileList;
    QStringList fileInitList;

    quint8 idx_min;
    quint8 idx_max;

    quint8 idx_hour_min;
    quint8 idx_hour_max;

    QStringList ListScanResult;

};

#endif // MCFIND_H
