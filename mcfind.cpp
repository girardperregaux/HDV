
#include "mcfind.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QDebug>
#include <string.h>
#include <qstring.h>
#include <QFile>
#include <qdir.h>
#include "mcdatetime.h"
#include "config.h"
#include "tstate.h"
#include <string.h>
#include <qstringlist.h>
#include "utility.h"
#include "DefCom.h"
#include "mcgestionefs.h"

extern MCSerial SerialPrint;
extern char buff_print[];

extern Mcgestionefs GestioneFs;

#define SCAN_DIR2     "/mnt/msata/videoaudio2/"

#define DATE_INIT    "151013"
#define HOUR_INIT    "103100"
#define DATE_FINISH  "151121"
#define HOUR_FINISH  "090100"
#define HOUR_DEF     "000000"
#define HOUR_DEF_FIN "235959"

#define mc_err     2
#define mc_ok      1
#define mc_null    0

TFile FFile;

MCfind::MCfind(QObject *parent) :
    QObject(parent)
{

}

void MCfind::Init(quint8 id)
{

    TimerRun.setInterval(100);
    TimerRun.start();
    connect(&TimerRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati
    printlist=false;
    folder_finish_not_found=false;
    Id=id;

}
void MCfind::Reset_Init()
{
    StateFind=STATE_FIND_NULL;
    startfind=false;
    idx_min=0;
    idx_max=0;
    idx_hour_min=0;
    idx_hour_max=0;
}



QStringList MCfind::SetList(char *p,quint8 tipo)
{
    char FileDirIF[STRING_F_MAX+1];
    QDir FinFileIF;
    QStringList StringNull;

    QStringList StringReturn;


    StringNull<< STR_NO;

    strcpy(FileDirIF,SCAN_DIR);


    if(tipo==TYPE_DIR)
    {
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Dirs |QDir::NoDotAndDotDot);
    }
    else if(tipo==TYPE_MP4)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        FinFileIF.setNameFilters(QStringList()<<"*.mp4");      //filtro per file .mp4
    }
    else if(tipo==TYPE_MPX)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        FinFileIF.setNameFilters(QStringList()<<"*.mpx");      //filtro per file .mpx
    }
    else if(tipo==TYPE_MPW)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        FinFileIF.setNameFilters(QStringList()<<"*.mpw");      //filtro per file .mpw
    }
    else if(tipo==TYPE_TW)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        FinFileIF.setNameFilters(QStringList()<<"*.tw");      //filtro per file .tw
    }
    else if(tipo==TYPE_TX)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        FinFileIF.setNameFilters(QStringList()<<"*.tx");      //filtro per file .tx
    }
    else if(tipo==TYPE_MPZ)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        FinFileIF.setNameFilters(QStringList()<<"*.mpz");      //filtro per file .mpz
    }
    else if(tipo==TYPE_TS)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);
        FinFileIF.setNameFilters(QStringList()<<"*.ts");      //filtro per file .ts
    }
    else if(tipo==TYPE_ALL)
    {
        strcat(FileDirIF,p);//aggiungo percorso
        FinFileIF.setCurrent(FileDirIF);
        FinFileIF.setSorting(QDir::Name);
        FinFileIF.setFilter(QDir::Files |QDir::NoDotAndDotDot);

        if(FinFileIF.count()<1)
        {
             snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Nessun file nella cartella %s \r\n",FileDirIF);
             SerialPrint.Flush(PRINT_DEBUG_FUNC);

             if(FinFileIF.rmdir(FileDirIF)){
                 snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"rimossa cartella %s\r\n",FileDirIF);
                 SerialPrint.Flush(PRINT_DEBUG_FUNC);
             }
             else{
                 snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"errore cartella %s \r\n",FileDirIF);
                 SerialPrint.Flush(PRINT_DEBUG_FUNC);
             }
        }
        FinFileIF.setCurrent(SCAN_DIR); //imposto la directory principale
        //FinFileIF.setCurrent("/mnt/"); //imposto la directory principale
    }


    if(FinFileIF.count()<1) //mi servirà per cancellare cartelle vuote (dipende dal filtro sopra non vero)
    {
        //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Nessun file nella cartella2 %s \r\n",FileDirIF);
        //SerialPrint.Flush(PRINT_DEBUG_FUNC);

        //FinFileIF.setCurrent("/mnt/"); //imposto la directory principale
        //return(StringNull);
    }

    StringReturn=FinFileIF.entryList();
    FinFileIF.setCurrent("/mnt/"); //imposto la directory principale

    return (StringReturn);

}

bool MCfind::SetValidDate(char *dateinit,char *hourinit, char *datefinish,char *hourfinish,quint8 tipo)
{

    // se il nome della cartella di inizio o quella finale non è nel formato GGMMAA esco...
    if((!IsNumber(dateinit) || strlen(dateinit)!=MAX_LEN_STR_FOLDER)||
      (!IsNumber(hourinit) || strlen(hourinit)!=MAX_LEN_STR_FOLDER) ||
      (!IsNumber(hourfinish) || strlen(hourfinish)!=MAX_LEN_STR_FOLDER) ||(tipo==0)||
      (!IsNumber(datefinish) || strlen(datefinish)!=MAX_LEN_STR_FOLDER))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): ERRORE FORMATO DATA/ORA\r\n",Id);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return(false);
    }

    strcpy(FFile.TDateInit,dateinit);
    strcpy(FFile.THourInit,hourinit);
    strcpy(FFile.TDateFinish,datefinish);
    strcpy(FFile.THourFinish,hourfinish);
    FFile.Ttipo=tipo;

    startfind=true;
    return (true);
}


quint8 MCfind::SetDateInit_SetDateFinish(char *dateinit,char *hourinit, char *datefinish,char *hourfinish,quint8 tipo)
{

    // se il nome della cartella di inizio o quella finale non è nel formato GGMMAA esco...
    if((!IsNumber(dateinit) || strlen(dateinit)!=MAX_LEN_STR_FOLDER)||
      (!IsNumber(hourinit) || strlen(hourinit)!=MAX_LEN_STR_FOLDER) ||
      (!IsNumber(hourfinish) || strlen(hourfinish)!=MAX_LEN_STR_FOLDER) ||
      (!IsNumber(datefinish) || strlen(datefinish)!=MAX_LEN_STR_FOLDER))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): ERRORE FORMATO DATA/ORA\r\n",Id);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return(mc_err);
    }

    if(!SetInit(dateinit,hourinit,datefinish,hourfinish,tipo))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): NESSUN INIZIO\r\n",Id);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (mc_null);
    }
    if(!SetFinish(datefinish,hourfinish,tipo))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): NESSUN FINE\r\n",Id);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (mc_null);
    }

    return (mc_ok);
}


bool MCfind::SetHourFinish(char *p,char *hourfinish,quint8 tipo)
{
    //char TempHourTemp[MAX_LEN_STR_FOLDER+1];
    char TempHourTemp[MAX_LEN_STR_HOUR_FOLDER+1];
    int lenstr=0;
    char Hour[MAX_LEN_STR_HOUR_FOLDER+1];


    FFile.fFinishList=SetList(p,tipo);


    for (int i=FFile.fFinishList.count()-1;i>=0; i--)
    {

        //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD:ora f:%s \r\n",FFile.fFinishList.at(i).toLocal8Bit().constData());
        //SerialPrint.Flush(PRINT_DEBUG_FUNC);


        //DEVO TOGLIERE DATA
        snprintf(&TempHourTemp[0],13,"%s",FFile.fFinishList.at(i).toLocal8Bit().constData());//temp


        lenstr=strlen(TempHourTemp);

        if(lenstr>11)
        {
            Hour[0]=TempHourTemp[6];
            Hour[1]=TempHourTemp[7];
            Hour[2]=TempHourTemp[8];
            Hour[3]=TempHourTemp[9];
            Hour[4]=TempHourTemp[10];
            Hour[5]=TempHourTemp[11];
            Hour[6]=TempHourTemp[12];

          //  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD:ora finale:  %s \r\n",Hour);
          //  SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }

        if(HourCompare(hourfinish,Hour)>=0)
        {
            idx_hour_max=i; //indice ora finale
           // snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): GIORNO %s ORE %s\r\n",Id,p,TempHourTemp);
           // SerialPrint.Flush(PRINT_DEBUG_FUNC);

//            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Folder finish %s File finish n=%d %s\r\n",p,i,TempHourTemp);
//            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (true);
        }
    }
    return (false);
}

bool MCfind::SetHourInit(char *p,char *hourinit,char *hourfinish,quint8 tipo)
{
    char TempHourTemp[MAX_LEN_STR_HOUR_FOLDER+1];

    char Hour[MAX_LEN_STR_HOUR_FOLDER+1];
    int lenstr=0;

    if(!QDir(SCAN_DIR).exists()) return (false);

    if(SetList(p,tipo).isEmpty())return (false);

    FFile.fInitList=SetList(p,tipo);


    for (int i=0; i<FFile.fInitList.count(); i++)
    {
        snprintf(&TempHourTemp[0],13,"%s",FFile.fInitList.at(i).toLocal8Bit().constData());//temp

        //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"xxxxxxxxxxx=%s\r\n",TempHourTemp);
        //SerialPrint.Flush(PRINT_DEBUG_FUNC);

        lenstr=strlen(TempHourTemp);

        if(lenstr>11)
        {
            Hour[0]=TempHourTemp[6];
            Hour[1]=TempHourTemp[7];
            Hour[2]=TempHourTemp[8];
            Hour[3]=TempHourTemp[9];
            Hour[4]=TempHourTemp[10];
            Hour[5]=TempHourTemp[11];
            Hour[6]=TempHourTemp[12];

//            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): ORA INIZIALE %s\r\n",Id,Hour);
//            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }
        else
        {
            return (false);
        }

        if(HourCompare(Hour,hourinit)>=0 &&  HourCompare(Hour,hourfinish)<=0)
        {

            idx_hour_min=i; //indice ora iniziale
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): ORA INIZIALE %s\r\n",Id,Hour);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Folder init %s File init n=%d %s \r\n",p,i,TempHourTemp);
            //SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (true);
        }
        else if (i==FFile.fInitList.count()-1)
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): ORA INIZIALE NON TROVATA\r\n",Id);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            //settare ora a 000000 perche' non ho trovato nessun file utile e passo a giorno successivo
            return (false);
        }
    }
    return (false);
}

bool MCfind::SetInit(char *dateinit,char *hourinit,char *datefinish,char *hourfinish,quint8 tipo)
{

    char TempDateTemp[MAX_LEN_STR_FOLDER+1];

    FFile.DirAllList=SetList((char*)SCAN_DIR,TYPE_DIR);

    if(!QDir(SCAN_DIR).exists()) return (false);

    if(SetList((char*)SCAN_DIR,TYPE_DIR).isEmpty())return (false);



    for (int i=0; i<FFile.DirAllList.count(); i++)
    {
        sprintf(&TempDateTemp[0],"%s",FFile.DirAllList.at(i).toLocal8Bit().constData());//temp

        //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): GIORNO %s\r\n",Id,TempDateTemp);
        //SerialPrint.Flush(PRINT_DEBUG_FUNC);

        if(DateCompare(TempDateTemp,dateinit)==0) //se data uguale allora cerco ora nella cartella
        {
            idx_min=i; //indice data iniziale

            if( (DateCompare(TempDateTemp,dateinit)==0) && (DateCompare(TempDateTemp,datefinish)==0) )
            {
                if(SetHourInit(TempDateTemp,hourinit,hourfinish,tipo)) //se trovo ora iniziale ok
                {

                    return (true); //indice ora iniziale ok trovato
                }
            }
            else if(SetHourInit(TempDateTemp,hourinit,(char*)"235959",tipo)) //se trovo ora iniziale ok
            {
                return (true); //indice ora iniziale ok trovato
            }

        }
        else if(DateCompare(TempDateTemp,dateinit)>0 && DateCompare(TempDateTemp,datefinish)<=0 ) //data maggiore
        {
            idx_min=i; //indice data iniziale
           // snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Ora iniziale default 000000\r\n");
           // SerialPrint.Flush(PRINT_DEBUG_FUNC);

            if(SetHourInit(TempDateTemp,(char*)HOUR_DEF,(char*)"235959",tipo))
            {
                return (true); //ho trovato almeno 1 file dopo le 000000
            }
            else
            {
             //   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Nessun Folder init da trasferire1 \r\n");
            //    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                //non ho trovato nessun file dopo le 000000 quindi o nessun file nella cartella oppure file di altra estensione
                //return (false);
            }
        }
        else if (i==FFile.DirAllList.count()-1)
        {
            return (false);
        }

    }

    return (false);
}

bool MCfind::SetFinish(char *datefinish,char *hourfinish,quint8 tipo)
{

    char TempDateTemp[MAX_LEN_STR_FOLDER+1];

    FFile.DirAllList=SetList((char*)SCAN_DIR,TYPE_DIR);

    for (int i=FFile.DirAllList.count()-1; i>=0; i--) //parto da ultima data
    {
        sprintf(&TempDateTemp[0],"%s",FFile.DirAllList.at(i).toLocal8Bit().constData());//temp


        if(DateCompare(TempDateTemp,datefinish)==0)
        {

            idx_max=i; //indice data finale trovato
            if(SetHourFinish(TempDateTemp,hourfinish,tipo))
            {
                return(true); //indice ora finale
            }
            else
            {

//snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: hourfinish %s\r\n",hourfinish);
//SerialPrint.Flush(PRINT_DEBUG_FUNC);



                //non trovo ora finale (cartella vuota o data file maggiore di quella che cerco) dovrei passare a giorno precedente
                //con Ora iniziale default 235959 e cosi via
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Data finale non trovata\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                //return (false);
            }
        }
        else if(DateCompare(datefinish,TempDateTemp)>0) //data maggiore
        {

//snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWDdebug1: %s %i\r\n",TempDateTemp,tipo);
//SerialPrint.Flush(PRINT_DEBUG_FUNC);

            idx_max=i; //indice data finale trovato

            if(SetHourFinish(TempDateTemp,(char*)HOUR_DEF_FIN,tipo))
            {
              //  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD:indice ora finale default 235959  \r\n");
              //  SerialPrint.Flush(PRINT_DEBUG_FUNC);
                folder_finish_not_found=false;
                return (true); //indice ora finale 235959
            }
            else
            {
               // snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Cartella finale non trovata\r\n");
               // SerialPrint.Flush(PRINT_DEBUG_FUNC);
                folder_finish_not_found=true;
                //non ho trovato nessun file prima delle 235959 quindi o nessun file nella cartella oppure file di altra estensione

                //pero' ho trovato ora iniziale
                  return (true);
                //return (false);
            }
        }
        else if (i==0) //non dovrebbe mai succedere perche' ce prima controlla data iniziale
        {
            //idx_max=i; //indice data finale settato a ultima data disponibile
            //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Nessun Folder finish da trasferire  \r\n");
            //SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (false);
        }
    }
    return (false);
}






//macchina a stati
void MCfind::Run()
{

    quint8 result;

    if(!startfind) StateFind=STATE_FIND_NULL;

    switch(StateFind)
    {
        case STATE_FIND_NULL :

            if(startfind)
            {
                if(!GestioneFs.MsataIsPresent)break;  //se hard disk non presente

                StateFind=STATE_FIND_INIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): AVVIO RICERCA\r\n",Id);

     SerialPrint.Flush(PRINT_DEBUG_FUNC);
     snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"          TIPO:%d DAL:%s ORE:%s ALL:%s ORE:%s\r\n",FFile.Ttipo,FFile.TDateInit,FFile.THourInit, FFile.TDateFinish,FFile.THourFinish);
     SerialPrint.Flush(PRINT_DEBUG_FUNC);

            }
        break;
        case STATE_FIND_INIT :

            result=SetDateInit_SetDateFinish(FFile.TDateInit,FFile.THourInit, FFile.TDateFinish,FFile.THourFinish,FFile.Ttipo);


            if(result==mc_null)
            {
                StateFind=STATE_FIND_NULL_INIT;
            }
            else if(result==mc_ok)
            {
                StateFind=STATE_FIND_WAIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): FILES TROVATI\r\n",Id);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else
            {
                StateFind=STATE_FIND_ERROR;
            }
        break;
        case STATE_FIND_WAIT :
            if(SetScanList(idx_min,idx_hour_min,idx_max,idx_hour_max,FFile.Ttipo,printlist)) //controllo se ci sono stringhe
            {

                if(printlist)
                {
                    Reset_Init();
                }
                else
                {
                    StateFind=STATE_FIND_READY;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): READY\r\n",Id);
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                }
                //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: STATE_FIND_WAIT\r\n");
                //SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else
            {
                StateFind=STATE_FIND_ERROR;
            }

        break;
        case STATE_FIND_READY :

        break;
        case STATE_FIND_NULL_INIT :
snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): NESSUN FILE TROVATO\r\n",Id);
SerialPrint.Flush(PRINT_DEBUG_FUNC);
            startfind=false;
            StateFind=STATE_FIND_NULL;
        break;
        case STATE_FIND_ERROR :
snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FIND(%02d): ERROR\r\n",Id);
SerialPrint.Flush(PRINT_DEBUG_FUNC);
            startfind=false;
            StateFind=STATE_FIND_NULL;
        break;
        default:
        break;
    }
}

QStringList MCfind::SendList()
{
    return(ListScanResult);

}

bool MCfind::SetScanList(int idx_init,int idx_hour_init,int idx_finish,int idx_hour_max,quint8 tipo,bool print)
{
    char tempDirectory[STRING_F_MAX+1];
    char TempHourTemp[STRING_F_MAX+1];
    char TempFolderLinux[STRING_F_MAX+1];
    QStringList TempScanResult;

    if(idx_init==idx_finish)
    {
        if(idx_hour_max>=idx_hour_init)
        {
            sprintf(&tempDirectory[0],"%s",FFile.DirAllList.at(idx_init).toLocal8Bit().constData());//temp

            for(int e=idx_hour_init; e<=idx_hour_max;e++)
            {
                sprintf(&TempHourTemp[0],"%s",FFile.fInitList.at(e).toLocal8Bit().constData());//temp

                snprintf(&TempFolderLinux[0],MAX_LEN_BUFFER_PRINT,"%s%s/%s",SCAN_DIR,tempDirectory,TempHourTemp);

                TempScanResult<< &TempFolderLinux[0];//riempio lista

            }
        }
    }
    else
    {
        for (int i=idx_init; i<=idx_finish; i++)
        {

            sprintf(&tempDirectory[0],"%s",FFile.DirAllList.at(i).toLocal8Bit().constData());//temp

            if(i==idx_init) //prima cartella devo partire da ora iniziale
            {
                for(int e=idx_hour_init; e<FFile.fInitList.count();e++)
                {
                    sprintf(&TempHourTemp[0],"%s",FFile.fInitList.at(e).toLocal8Bit().constData());//temp

                    snprintf(&TempFolderLinux[0],MAX_LEN_BUFFER_PRINT,"%s%s/%s",SCAN_DIR,tempDirectory,TempHourTemp);

                     TempScanResult<< &TempFolderLinux[0];//riempio lista

                }
            }
            else if((i==idx_finish)&&(!folder_finish_not_found)) //ultima cartella trovo ora finale
            {
                    for(int a=0; a<=idx_hour_max;a++)
                    {
                        sprintf(&TempHourTemp[0],"%s",FFile.fFinishList.at(a).toLocal8Bit().constData());//temp

                        snprintf(&TempFolderLinux[0],MAX_LEN_BUFFER_PRINT,"%s%s/%s",SCAN_DIR,tempDirectory,TempHourTemp);

                        TempScanResult << &TempFolderLinux[0];//riempio lista
                    }
            }
            else
            {
                FFile.fList=SetList(tempDirectory,tipo);

                for (int i=0; i<FFile.fList.count(); i++)
                {
                    sprintf(&TempHourTemp[0],"%s",FFile.fList.at(i).toLocal8Bit().constData());//temp
                    snprintf(&TempFolderLinux[0],MAX_LEN_BUFFER_PRINT,"%s%s/%s",SCAN_DIR,tempDirectory,TempHourTemp);

                    TempScanResult << &TempFolderLinux[0]; //riempio lista

                }
            }
        }
    }


    if(print)
        for (int i=0; i<TempScanResult.count(); i++)
        {
            sprintf(&TempFolderLinux[0],"%s",TempScanResult.at(i).toLocal8Bit().constData());//temp
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"****%s\r\n",TempFolderLinux);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }

    ListScanResult=TempScanResult;

    return(true);
}


void MCfind::PrintObj()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"DWD: Print**************\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

}



