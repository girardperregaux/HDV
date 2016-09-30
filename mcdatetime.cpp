#include "mcdatetime.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
#include <DefCom.h>


extern MCSerial SerialPrint;
extern char buff_print[];



McDateTime::McDateTime(QObject* parent)
    :QObject(parent)
{

}



void McDateTime::Update()
{
    time_t now = time(0);
    struct tm  tstruct;

    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%y%m%d%H%M%S", &tstruct);


    strncpy(strDate,buf,ID_STRING_DATE); //prima parte del buffer
    strDate[ID_STRING_DATE]=0;

    strncpy(strHour,&buf[ID_STRING_HOUR],ID_STRING_HOUR); //seconda parte del buffer
    strHour[ID_STRING_HOUR]=0;


    strcpy(datename,strDate); //salvo nome file
    strcat(datename,strHour); //salvo nome file


    strncpy(strBuff,strDate,ID_YEARS);       //anno yy
    strBuff[ID_YEARS]=0;
    Year=atoi(strBuff);

    strncpy(strBuff,&strDate[ID_MONTH],ID_MONTH);  //mese mm
    strBuff[ID_MONTH]=0;
    Month=atoi(strBuff);

    strncpy(strBuff,&strDate[ID_MONTH+ID_DAY],ID_DAY);  //giorno dd
    strBuff[ID_DAY]=0;
    Day=atoi(strBuff);


    strncpy(strBuff,strHour,ID_HOUR);       //ora hh
    strBuff[ID_HOUR]=0;
    Hour=atoi(strBuff);

    strncpy(strBuff,&strHour[ID_MIN],ID_MIN);  //minuti mm
    strBuff[ID_MIN]=0;
    Minutes=atoi(strBuff);

    strncpy(strBuff,&strHour[ID_MIN+ID_SEC],ID_SEC);  //secondi ss
    strBuff[ID_SEC]=0;
    Seconds=atoi(strBuff);

}


void  McDateTime::Init()
{
    Updatetimer.setInterval(500); //scatta ogni 500 ms e aggiorna i valori letti dall'orologio di sistema
    Updatetimer.start();
    connect(&Updatetimer, SIGNAL(timeout()), this, SLOT(Update()));
    Update();
}
char McDateTime::SendString()
{
    return(false);

}

bool McDateTime::IsValid()
{

    return(   (strlen(strDate)==HOUR_DATESIZE)
           && (strlen(strHour)==HOUR_HOURSIZE)
           && (IsNumber(strDate))
           && (IsNumber(strHour))
           && (Day >= 1) && (Day <= 31)
           && (Month >= 1) && (Month <= 12)
           && (Hour < 24)
           && (Minutes < 60)
           && (Seconds < 60)
         );

}

void McDateTime::PrintObj()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"data/ora : %s",buf);
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    if(IsValid())
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VALIDA\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"NO VALIDA\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }


  //  printf("valori int :\r\n");
  //  printf("%d\r\n",Year);
  //  printf("%d\r\n",Month);
  //  printf("%d\r\n",Day);
  //  printf("%d\r\n",Hour);
  //  printf("%d\r\n",Minutes);
  //  printf("%d\r\n",Seconds);

}



//fusi begin
void McDateTime::SetDefault(void)
{

    SetDate((char*)"01012000");
    SetHour((char*)"000000");
}

//   SetDate         -> Imposta una nuova Data
//   Parametri       ->
//   Result          ->
//----------------------
void McDateTime::SetDate(char *NewDate)
{
   if (strlen(NewDate) == HOUR_DATESIZE)
   {
      strcpy(strDate, NewDate);
      FillDate();
   }
}


//   SetHour         -> Imposta una nuova Ora
//   Parametri       ->
//   Result          ->
//----------------------
void McDateTime::SetHour(char *NewHour)
{
   if (strlen(NewHour) == HOUR_HOURSIZE)
   {
      strcpy(strHour, NewHour);
      FillHour();
   }
}

void McDateTime::FillHour(void)
{
   char strTemp[3];

   strncpy(strTemp,strHour+0,2);
   Hour = atoi(strTemp);
   strncpy(strTemp,strHour+2,2);
   Minutes = atoi(strTemp);
   strncpy(strTemp,strHour+4,2);
   Seconds = atoi(strTemp);
}

void McDateTime::FillDate(void)
{
   char strTemp[3];

   strTemp[2] = 0;
   strncpy(strTemp,strDate+0,2);
   Day = atoi(strTemp);
   strncpy(strTemp,strDate+2,2);
   Month = atoi(strTemp);
   strncpy(strTemp,strDate+6,2);
   Year = atoi(strTemp);
}


//   GetHour         -> ritorna Ora
//   Parametri       ->
//   Result          -> ora
//----------------------
quint8 McDateTime::GetHour(void)
{
    return(Hour);
}

//   GetMinutes      -> ritorna Minuti
//   Parametri       ->
//   Result          -> minuti
//----------------------
quint8 McDateTime::GetMinutes(void)
{
    return(Minutes);
}

//   GetSeconds      -> ritorna Secondi
//   Parametri       ->
//   Result          -> secondi
//----------------------
quint8 McDateTime::GetSeconds(void)
{
    return(Seconds);
}

//   GetDay          -> ritorna Giorno
//   Parametri       ->
//   Result          -> giorno
//----------------------
quint8 McDateTime::GetDay(void)
{
    return(Day);
}

//   GetMonth        -> ritorna Mese
//   Parametri       ->
//   Result          -> mese
//----------------------
quint8 McDateTime::GetMonth(void)
{
    return(Month);
}

//   GetYear         -> ritorna Anno
//   Parametri       ->
//   Result          -> anno
//----------------------
quint8 McDateTime::GetYear(void)
{
    return(Year);
}

//fusi end


