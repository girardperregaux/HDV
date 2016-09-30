#ifndef GESTIONEMULTIMEDIA_H
#define GESTIONEMULTIMEDIA_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>

#include "mcdatetime.h"
#include <apptimer.h>
#include "timerman.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <qfile.h>
#include "HDV.h"



class GestMultimedia : public QObject
{
    Q_OBJECT
public:
    explicit GestMultimedia(QObject *parent = 0);

    AppTimer TimerMultimedia;
    int GMState;
    bool Startgest;
    bool Stategest;

    bool Update_mod_function;

    quint8 mod_video;

    char mod_video_param[2];

    #define STATE_MEDIA_NULL                         0
    #define STATE_MEDIA_INIT                         1
    #define STATE_MEDIA_WAIT                         2
    #define STATE_MEDIA_READY_STREAM                 3
    #define STATE_MEDIA_READY_REC                    4
    #define STATE_MEDIA_READY_REC_STREAM             5
    #define STATE_MEDIA_READY_REC_FTP                6
    #define STATE_MEDIA_STOP                         7

    #define STRING_MAX_BUFFER_VID_AUD             1000


signals:

public slots:
   void Init(TimerMan *t);
   bool Type(quint8 mod);
   void Run();
   void PrintObj();
   void Stop();
   void Start();

private:

   QTimer Multimedia;
   TimerMan *pREGTIMER;


};

#endif // GESTIONEMULTIMEDIA_H
