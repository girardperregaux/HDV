#ifndef MCGESTIONEDATE_H
#define MCGESTIONEDATE_H

#include <QObject>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"


#define T_BUFF_MAX            80
#define T_HOUR_DATESIZE        6    //  YYMMDD
#define T_HOUR_HOURSIZE        6    //  HHMMSS
#define ID_GYEARS             2
#define ID_GMOUNTH            2
#define ID_GDAY               2
#define ID_GHOUR              2
#define ID_GMIN               2
#define ID_GSEC               2
#define GTEMP_BUFF            4


#define GDATE_INIT    0
#define GDATE_SET     1
#define GDATE_CHECK   2
#define GDATE_RESTART 3


typedef struct
{
  bool update;
  bool force_update;
  char Date[T_HOUR_DATESIZE+1];
  char Hour[T_HOUR_DATESIZE+1];
  quint16 GYear;
  quint8 GMounth;
  quint8 GDay;
  quint8 GHour;
  quint8 GMinutes;
  quint8 GSeconds;

} TGDate;


class mcgestionedate : public QObject
{
    Q_OBJECT
public:
    explicit mcgestionedate(QObject *parent = 0);
    AppTimer GpsTimer;
   // TGDate TDate;
signals:

public slots:
    void Init(TimerMan *t); //per passare timer
    void GestioneDate();
    void Start(int time);
    bool SetDateIsValid(char *str1, char *str2,TGDate *pDate);
    bool Update(TGDate *pDate);

private:
    TimerMan *pREGTIMER;
    QTimer DateT;
    char GstrBuff[GTEMP_BUFF+1];
    int GdateState;
    char BuffDate[T_BUFF_MAX];                                      // Carico il valore letto da time.h

};

#endif // MCGESTIONEDATE_H
