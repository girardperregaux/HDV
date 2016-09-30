#ifndef MCDATETIME_H
#define MCDATETIME_H
#include <time.h>
#include <QTimer>
#include <QObject>
#include <QProcess>
#include "utility.h"

#define STRING_DATE_MAX     22
#define BUFF_MAX            80
#define HOUR_DATESIZE        8    //  YYYYMMDD //fusi da 6 a 8
#define HOUR_HOURSIZE        6    //  HHMMSS
#define TEMP_BUFF            4    //

#define ID_STRING_DATE       6
#define ID_STRING_HOUR       6

#define ID_YEARS             2
#define ID_MONTH             2
#define ID_DAY               2
#define ID_HOUR              2
#define ID_MIN               2
#define ID_SEC               2


class McDateTime : public QObject
{
    Q_OBJECT

public:
    explicit McDateTime(QObject* parent=0);


public slots:
   void Update();
   void PrintObj();

   bool IsValid();
   char SendString();

public:
   void Init();
   char strDate[HOUR_DATESIZE+1];                           // Data in stringa DDMMYY
   char strHour[HOUR_HOURSIZE+1];                           // Ora in stringa HHMMSS
   char datename[STRING_DATE_MAX+1];
   void SetDefault(void);
   void SetDate(char *NewDate); //fusi+
   void SetHour(char *NewHour); //fusi+
   quint8 GetHour(void);
   quint8 GetMinutes(void);
   quint8 GetSeconds(void);
   quint8 GetDay(void);
   quint8 GetMonth(void);
   quint8 GetYear(void);

private:
    // Campi
    //-------
    quint8  Year;                                               // Anno a 2 cifre a partire dal 2000
    quint8  Month;                                              // Mese Dell'Anno  1..12
    quint8  Day;                                                // Giorno del Mese 1..31

    quint8  Hour;                                               // Ora del Giorno 0..23
    quint8  Minutes;                                            // Minuto all'interno dell'ora
    quint8  Seconds;                                            // Secondi all'interno del minuto
    char    buf[BUFF_MAX];                                      // Carico il valore letto da time.h

    char    strBuff[TEMP_BUFF+1];

    QTimer Updatetimer;

    //void SetDate(char *NewDate);  //fusi -
    //void SetHour(char *NewHour);  /fusi -
    void FillDate(void);
    void FillHour(void);
};



#endif // MCDATETIME_H
