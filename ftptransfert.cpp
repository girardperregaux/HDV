#include "ftptransfert.h"
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
#include <QFile>
#include <string.h>
#include <qstringlist.h>
#include "utility.h"
#include "DefCom.h"
#include "mcmodem.h"


#define SIZE_TRANSFERT_100MB    100000
#define INIT_TO_LAST      1
#define ALWAYS_LAST       2

#define PATH_SERVER_FTP   "/mcsistemi.it/fwREMHDV/"


extern TConfig Configuration;
extern MCSerial SerialPrint;
extern char buff_print[];
extern MCModem Modem;
extern quint16 CountFileTransfert;
extern Message MESSAGE;

char GFStransf[MAX_LEN_SMS+1];


TMM MMGestionetransfer;

ftptransfert::ftptransfert(QObject *parent) :
    QObject(parent)
{
}
void ftptransfert::Init(TimerMan *t)
{
    pREGTIMER=t;

    Timertransfert.Init((char*)"transfert");
    ptimeout.Init((char*)"pythonto");
    Timertransfert.SetTimer(UN_SECONDO*1);
    ptimeout.SetTimer(UN_SECONDO*1);


    pREGTIMER->RegisterTimer(&Timertransfert); //timer registrato
    pREGTIMER->RegisterTimer(&ptimeout); //timer registrato


    TTransfertRun.setInterval(100);
    TTransfertRun.start();

    StateTransfert=STATE_FTP_TRANS_NULL;
    UpgradeTransfer=false;
    startTransfer=false;
    MCFindTransfert.Init(MCFINDTRANSFERT);
    MCFindTransfert.Reset_Init();
    PythonConnected=false;
    FtpConnected=false;

    connect(&socketpy, SIGNAL(connected()),this, SLOT(connectedpy()));
    connect(&socketpy, SIGNAL(disconnected()),this, SLOT(disconnectedpy()));
    connect(&socketpy, SIGNAL(readyRead()),this, SLOT(readyReadpy()));
    connect(&TTransfertRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati
    pytcp= new QProcess(this);
    pidpy=0;
    CommandRetry=N_RETRY;
    Ftp_sleep=false;
    FtpTransfertRun=false;
}

bool ftptransfert::removefile(char * Tfile)
{
    QFile file(Tfile);

    if(file.exists())
    {
        if(file.remove())
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: ELIMINO IL FILE: %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (true);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: IMPOSSIBILE ELIMINARE: %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (false);
        }
    }
    else
    {
  //      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: IMPOSSIBILE ELIMINARE IL FILE : %s\r\n",Tfile);
  //      SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }
    return (false);

}

void ftptransfert::connectedpy()
{
    PythonConnected=true;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: PYTHON CONNESSO\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
}

void ftptransfert::disconnectedpy()
{
    PythonConnected=false;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: PYTHON DISCONNESSO\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
}

void ftptransfert::readyReadpy()
{
    QByteArray dato;

    dato=socketpy.readAll();
    snprintf(&bufferTcpPy[0],STRING_TRANSC,"%s\r\n",dato.data());

    //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",bufferTcpPy);
    //SerialPrint.Flush(PRINT_DEBUG_FUNC);

}

quint8 ftptransfert::TcpParampy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: name server ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }

    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: name server ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}

quint8 ftptransfert::Tcploginpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: login ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: login ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}

quint8 ftptransfert::Tcpcdpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: path ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: path ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}

quint8 ftptransfert::Tcpsourcepy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: source ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: source ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}


bool ftptransfert::TcpuploadOKpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    return(false);
}

bool ftptransfert::TcpuploadKOpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    return(false);
}

bool ftptransfert::TcpuploadTimeoutpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload timeout")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    return(false);
}

bool ftptransfert::TcpuploadRunning(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload in corso")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    //memset(bufferTcpPy,0,STRING_TRANSC);
    return(false);
}


quint8 ftptransfert::TcpuploadRenamepy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: rename ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: rename ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}


quint8 ftptransfert::TcpuploadDisconnectpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: ftp quit ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: ftp quit ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}


bool ftptransfert::TCcpNameFileServerpy(char *name_file)
{
    quint8 len=0;
    int idx=0;

    char buff_t_tamp[STRING_TRANSC+1];

    strcpy(bufferFileServerPy,Configuration.Config.codice_gruppo);     //preparo stringa per salvare server ftp
    strcat(bufferFileServerPy,Configuration.Config.codice_periferica);
    strcat(bufferFileServerPy,"_");

    strcpy(NameFileTempPy,name_file); //salvo nome file

    len=strlen(NameFileTempPy);      //lunghezza stringa

    for(idx=len;idx>0;idx--)    //cerco dalla fine della stringa all'inizio lo slash
    {
        if(NameFileTempPy[idx]=='/')
        {
            idx=idx+1;  //tolgo lo slash
            break;
        }
    }


    buff_t_tamp[0]=NameFileTempPy[idx+4];//anno
    buff_t_tamp[1]=NameFileTempPy[idx+5];//anno
    buff_t_tamp[2]=NameFileTempPy[idx+2];//mese
    buff_t_tamp[3]=NameFileTempPy[idx+3];//mese
    buff_t_tamp[4]=NameFileTempPy[idx];  //giorno
    buff_t_tamp[5]=NameFileTempPy[idx+1];//giorno

    buff_t_tamp[6]=NameFileTempPy[idx+6];//ora
    buff_t_tamp[7]=NameFileTempPy[idx+7];//ora
    buff_t_tamp[8]=NameFileTempPy[idx+8];//min
    buff_t_tamp[9]=NameFileTempPy[idx+9];//min
    buff_t_tamp[10]=NameFileTempPy[idx+10];//sec
    buff_t_tamp[11]=NameFileTempPy[idx+11];//sec
    buff_t_tamp[12]=NameFileTempPy[idx+12];//_


    buff_t_tamp[13]=NameFileTempPy[idx+17];//anno
    buff_t_tamp[14]=NameFileTempPy[idx+18];//anno
    buff_t_tamp[15]=NameFileTempPy[idx+15];//mese
    buff_t_tamp[16]=NameFileTempPy[idx+16];//mese
    buff_t_tamp[17]=NameFileTempPy[idx+13];//giorno
    buff_t_tamp[18]=NameFileTempPy[idx+14];//giorno

    buff_t_tamp[19]=NameFileTempPy[idx+19];//ora
    buff_t_tamp[20]=NameFileTempPy[idx+20];//ora
    buff_t_tamp[21]=NameFileTempPy[idx+21];//min
    buff_t_tamp[22]=NameFileTempPy[idx+22];//min
    buff_t_tamp[23]=NameFileTempPy[idx+23];//sec
    buff_t_tamp[24]=NameFileTempPy[idx+24];//sec

    buff_t_tamp[25]=NameFileTempPy[idx+25];//.

    if(Configuration.Config.transcoder) //usato per abilitare/disabilitare transcoder
    {
        buff_t_tamp[26]='m';//trasformo in mpw .mp4
        buff_t_tamp[27]='p';
        buff_t_tamp[28]='w';
        buff_t_tamp[29]=0;
    }
    else
    {
        buff_t_tamp[26]='t';//trasformo in tw .ts
        buff_t_tamp[27]='w';
        buff_t_tamp[28]=0;
    }

    strcat(bufferFileServerPy,buff_t_tamp);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: buff_t_send %s\r\n",bufferFileServerPy);
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

    return(true);

}



void ftptransfert::SendCharTcpPy(char *c)
{
    if(PythonConnected)
        socketpy.write(c);
}

bool ftptransfert::SetDate(char *dateinit,char *hourinit, char *datefinish,char *hourfinish)
{


    if((strlen(dateinit)!=MAX_LEN_STR_FOLDER)  ||
      (strlen(hourinit)!=MAX_LEN_STR_FOLDER)   ||
      (strlen(datefinish)!=MAX_LEN_STR_FOLDER) ||
      (strlen(hourfinish)!=MAX_LEN_STR_FOLDER))
    {
        return (false);
    }

    strcpy(FtpDate.TDInit,dateinit);
    strcpy(FtpDate.THInit,hourinit);
    strcpy(FtpDate.TDFinish,datefinish);
    strcpy(FtpDate.THFinish,hourfinish);
    recheck_list=true;
    UpgradeTransfer=true;

    return(true);
}


//macchina a stati
void ftptransfert::Run()
{
    int temp;
    qint64 timesize = 1;
    int temp_list;
    char buff_send_command[STRING_TRANSC+1];
    char fileTempRename[STRING_TRANSC+1];
    quint8 formato_video;
    //vedere se connesso
  //  isrun=false;

    switch(StateTransfert)
    {
        case STATE_FTP_TRANS_PYHTON_LAUNCH:
            pidpy=0;
            PythonConnected=false;
            pytcp->start(STR_PYTHON_PROG);
            pidpy = pytcp->pid();
            sprintf(StrPidpy," kill -9 %d",(int)pytcp);
            StateTransfert=STATE_FTP_TRANS_PYHTON_INIT;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: AVVIO SCRIPT\r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_ALL);
            ptimeout.SetTimer(0);
            ptimeout.SetTimer(UN_SECONDO*5);
        break;
        case STATE_FTP_TRANS_PYHTON_KILL:

            FtpConnected=false;
            StateTransfert=STATE_FTP_TRANS_WAIT_KILL;
            startTransfer=false;                   //togliere
            Modem.ECMReqFtp=false;
            pytcp->close();
            PythonConnected=false;
            ptimeout.SetTimer(UN_MINUTO*2);

        break;
        case STATE_FTP_TRANS_WAIT_KILL:

            if(!ptimeout.IsElapsed())break;

            StateTransfert=STATE_FTP_TRANS_NULL;
        break;
        case STATE_FTP_TRANS_PYHTON_INIT:

            if(ptimeout.IsElapsed())break;


            if(!pytcp->isOpen())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: PROCESSO PYTHON KO!!!!!!!!!!\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_ALL);

            }
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: CONNESSIONE A PYTHON\r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_ALL);

            socketpy.connectToHost("localhost",PORT_PY_PROG);
            StateTransfert=STATE_FTP_TRANS_PYHTON_IS_CONNECT;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: CONNECT TO HOST\r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_ALL);

            ptimeout.SetTimer(UN_SECONDO*5);
        break;
        case STATE_FTP_TRANS_PYHTON_IS_CONNECT:
            if(PythonConnected)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: TCP CONNECT\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_NULL;
            }

            if(!pytcp->isOpen())
            {
                StateTransfert=STATE_FTP_TRANS_PYHTON_KILL; //se il processo python si è ammazzato
                ptimeout.SetTimer(0);
                ptimeout.SetTimer(UN_SECONDO*5);
                UpgradeTransfer=true;                       //ritento
            }

            if(ptimeout.IsElapsed())
            {
                StateTransfert=STATE_FTP_TRANS_PYHTON_INIT;
                ptimeout.SetTimer(UN_MINUTO*2);
            }

            //se si chiude python cosa fare? retry e poi ammazzare python e riattivare
        break;

        case STATE_FTP_TRANS_NULL :

            FtpTransfertRun=false;

            if(UpgradeTransfer)
            {

                SetWorkPeriod(ID_CPU,60,true);
                if(!PythonConnected)
                {

                    StateTransfert=STATE_FTP_TRANS_PYHTON_LAUNCH; //UpgradeTransfer va a false dopo che python si connette
                }
                else
                {

                    //controllare presenza hard disk pronto
                    UpgradeTransfer=false;
                    MCFindTransfert.Reset_Init();
                    indexTransfert=0;
                    startTransfer=true;
                    CommandRetry=N_RETRY;
                }

            }

            if(startTransfer)
            {
                if(Configuration.Config.transcoder) //usato per abilitare/disabilitare transcoder
                {
                    formato_video=TYPE_MP4; //file transcodificati
                }
                else
                {
                    formato_video=TYPE_TS; //file originali
                }
                recheck_list=false;
//snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"recheck_list=false 1!!!!!!!!!!!!!!!!!!!\r\n");
//SerialPrint.Flush(PRINT_DEBUG_ALL);

                if(MCFindTransfert.SetValidDate(FtpDate.TDInit,FtpDate.THInit,FtpDate.TDFinish,FtpDate.THFinish,formato_video))
                {

                    StateTransfert=STATE_FTP_TRANS_INIT;
                    MCFindTransfert.startfind=true; //avvio macchina a stati instanza mcfind
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: STATE_FTP_TRANS_NULL\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    indexTransfert=0;
                    FtpTransfertRun=true;
                }
                else StateTransfert=STATE_FTP_TRANS_ERROR;
            }

        break;
        case STATE_FTP_TRANS_INIT :

                if((Configuration.Config.modo_scarico==FTP_AUTO)||(Configuration.Config.modo_di_funzionamento ==VA_RF))
                //if(Configuration.Config.modo_scarico==FTP_AUTO)
                {
                QFile file(REQFTP_AUTO);

                if(!file.exists())                  //se non esiste
                {
                     if((Modem.GSMRegistrato)&& ((Modem.atc==MODO_0ATC)||(Modem.atc==MODO_1ATC)||(Modem.atc==MODO_2ATC))) //non trasferisco
                     {
                         Ftp_sleep=true;

                         if(IsTelephoneNumber(MMGestionetransfer.ConnectedNum))
                         {
                             sprintf(GFStransf,"FTPR: DISATTIVATO PER RETE ATC=%d",Modem.atc);
                             SerialPrint.Flush(PRINT_DEBUG_FUNC);
                             MESSAGE.MessageUserMMPost(&MMGestionetransfer,(char *)&GFStransf[0]);
                             MMGestionetransfer.ConnectedNum[0]=0;
                         }
                     }
                }
            }

            //qui
            if(Ftp_sleep)
            {
                Ftp_sleep=false;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: SLEEP\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                indexTransfert=0;//test
                StateTransfert=STATE_FTP_TRANS_FINISH; //test
                break;
            }

            if(recheck_list)
            {
                StateTransfert=STATE_FTP_TRANS_NULL;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: AGGIORNO LISTA\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                break;
            }


            if(MCFindTransfert.StateFind==STATE_FIND_READY)
            {
                if(IsTelephoneNumber(MMGestionetransfer.ConnectedNum))
                {
                    //GIRARE LA DATA PER LA RISPOSTA
                    //sprintf(GFStransf,"FTPR ATTIVATO NELL'INTERVALLO DA %s UTC %s A %s UTC %s",FtpDate.TDInit,FtpDate.THInit,FtpDate.TDFinish,FtpDate.THFinish);

                    //controllo se campo non è vuoto
                    //MMGestionetransfer.campo2
                    //MMGestionetransfer.campo3
                    //MMGestionetransfer.campo4
                    //MMGestionetransfer.campo5

                    sprintf(GFStransf,"FTPR ATTIVATO NELL'INTERVALLO (UTC) %s %s %s %s",MMGestionetransfer.campo2,MMGestionetransfer.campo3,MMGestionetransfer.campo4,MMGestionetransfer.campo5);
                    MESSAGE.MessageUserMMPost(&MMGestionetransfer,(char *)&GFStransf[0]);
                    MMGestionetransfer.ConnectedNum[0]=0;
                }

                SetWorkPeriod(ID_CPU,40,true);
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: STATE_FTP_TRANS_INIT\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                TranscStringList=MCFindTransfert.SendList();
                if((Configuration.Config.modo_di_funzionamento ==VA_RF))
                {
                    StateTransfert=STATE_FTP_TRANS_INIT_TO_LAST;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: DAL PIU' VECCHIO\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }
                else if((Configuration.Config.ordine_ottimizzato==ALWAYS_LAST))
                {
                    StateTransfert=STATE_FTP_TRANS_LAST_TO_INIT;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: STATE_FTP_TRANS_LAST_TO_INIT\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }
                else if(Configuration.Config.ordine_ottimizzato==INIT_TO_LAST)
                {
                    StateTransfert=STATE_FTP_TRANS_INIT_TO_LAST;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: STATE_FTP_TRANS_INIT_TO_LAST\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }
                break;
            }

            else if(MCFindTransfert.StateFind==STATE_FIND_NULL_INIT)
            {
                if(IsTelephoneNumber(MMGestionetransfer.ConnectedNum))
                {
                    //GIRARE LA DATA PER LA RISPOSTA
                    //sprintf(GFStransf,"FTPR NESSUN FILE DA TRASFERIRE NELL'INTERVALLO DA %s UTC %s A %s UTC %s",FtpDate.TDInit,FtpDate.THInit,FtpDate.TDFinish,FtpDate.THFinish);
                    sprintf(GFStransf,"FTPR NESSUN FILE DA TRASFERIRE NELL'INTERVALLO (UTC) %s %s %s %s",MMGestionetransfer.campo2,MMGestionetransfer.campo3,MMGestionetransfer.campo4,MMGestionetransfer.campo5);
                    //sprintf(GFStransf,"FTPR NESSUN FILE DA TRASFERIRE NELL'INTERVALLO ");
                    MESSAGE.MessageUserMMPost(&MMGestionetransfer,(char *)&GFStransf[0]);
                    MMGestionetransfer.ConnectedNum[0]=0;
                }


                StateTransfert=STATE_FTP_TRANS_ERROR_NULL;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: NESSUN FILE DA TRASFERIRE\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                ptimeout.SetTimer(0);
                ptimeout.SetTimer(UN_SECONDO*5);

                //cancello file
                if((Configuration.Config.modo_scarico==FTP_AUTO)||(Configuration.Config.modo_di_funzionamento ==VA_RF))
                //if(Configuration.Config.modo_scarico==FTP_AUTO)
                {
                     QFile file(REQFTP_AUTO);

                     if(file.exists())                  //se esiste file cancello
                     {
                         if(file.remove())
                         {
                             snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: DELETE FILE FTP AUTO\r\n");
                             SerialPrint.Flush(PRINT_DEBUG_FUNC);
                         }
                         else
                         {
                             snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: ERROR DELETE FILE FTP AUTO \r\n");
                             SerialPrint.Flush(PRINT_DEBUG_FUNC);
                         }
                     }
                }

            }

            else if(MCFindTransfert.StateFind==STATE_FIND_ERROR)
            {

                if(IsTelephoneNumber(MMGestionetransfer.ConnectedNum))
                {
                    sprintf(GFStransf,"FTPR ERROR");
                    MESSAGE.MessageUserMMPost(&MMGestionetransfer,(char *)&GFStransf[0]);
                    MMGestionetransfer.ConnectedNum[0]=0;
                }

                StateTransfert=STATE_FTP_TRANS_ERROR;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: STATE_FTP_TRANS_ERROR \r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                ptimeout.SetTimer(0);
                ptimeout.SetTimer(UN_SECONDO*5);
            }

        break;
        case STATE_FTP_TRANS_INIT_TO_LAST :
            //ok //prendo la mia lista
            SetWorkPeriod(ID_CPU,40,true);


            if(indexTransfert<TranscStringList.count())
            {



                strncpy(&StrBuffTra[0],TranscStringList.at(indexTransfert).toLocal8Bit().constData(),STRING_TRANSC);
                strncpy(&StrBuffTemp[0],TranscStringList.at(indexTransfert).toLocal8Bit().constData(),STRING_TRANSC);
                temp=strlen(StrBuffTemp);


                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",StrBuffTemp);//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                StateTransfert=STATE_FTP_TRANS_MODEM_ON;

                Timertransfert.SetTimer(UN_MINUTO*timesize);
                indexTransfert++;
            }
            else
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA : STATE_FTP_TRANS_INIT_TO_LAST Finish\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                indexTransfert=0;//test
                StateTransfert=STATE_FTP_TRANS_FINISH; //test

            }
        break;
        case STATE_FTP_TRANS_LAST_TO_INIT :
            //ok //prendo la mia lista


            if(indexTransfert<TranscStringList.count())
            {

                temp_list=(TranscStringList.count()-1)-indexTransfert;

                strncpy(&StrBuffTra[0],TranscStringList.at(temp_list).toLocal8Bit().constData(),STRING_TRANSC);
                strncpy(&StrBuffTemp[0],TranscStringList.at(temp_list).toLocal8Bit().constData(),STRING_TRANSC);
                temp=strlen(StrBuffTemp);


                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: %s\r\n",StrBuffTemp);//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                StateTransfert=STATE_FTP_TRANS_MODEM_ON;
                Timertransfert.SetTimer(UN_MINUTO*timesize);
                indexTransfert++;

            }
            else
            {


             //    QFile file(REQFTP_REQ);

               //  if(file.exists())                  //se esiste file cancello
               //  {
                //     if(file.remove())
                //     {
                //         snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: DELETE FILE FTP REQ\r\n");
                //         SerialPrint.Flush(PRINT_DEBUG_FUNC);
                //     }
                // }


                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA:STATE_FTP_TRANS_LAST_TO_INIT Finish\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_FINISH;
                CommandRetry=N_RETRY;
            }
        break;

        case STATE_FTP_TRANS_MODEM_ON:

            SetWorkPeriod(ID_CPU,40,true);

            if(!Modem.ECMReqFtp)
            {
                Modem.ECMReqFtp=true;
                strcpy(Modem.APNFtp,Configuration.Config.APN);
            }
            StateTransfert=STATE_FTP_TRANS_MODEM_CONNECT;
        break;
        case STATE_FTP_TRANS_MODEM_CONNECT:

            if(!GetState(MODEM_CONNECTED) && !GetState(MODEM_ASSENTE))
            {
                break;  //TODO !!!!!
            }
            else
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA : MODEM CONNESSO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_IS_CONNECT;
            }


        break;

        case STATE_FTP_TRANS_IS_CONNECT:
            SetWorkPeriod(ID_CPU,40,true);
            if(!FtpConnected)
            {
                strcpy(buff_send_command,"PARAM ");
                strcat(buff_send_command,Configuration.Config.ip_server_ftp);
                strcat(buff_send_command," ");
                strcat(buff_send_command,Configuration.Config.username_server_ftp);
                strcat(buff_send_command," ");
                strcat(buff_send_command,Configuration.Config.password_server_ftp);
                strcat(buff_send_command," ");
                strcat(buff_send_command,Configuration.Config.porta_server_ftp);

                //socketpy.write("PARAM ftp.mcsistemi.it 1928272@aruba.it o5chad6z");
                socketpy.write(buff_send_command);
                StateTransfert=STATE_FTP_TRANS_CONNECTED;
                ptimeout.SetTimer(0);
                ptimeout.SetTimer(UN_SECONDO*5);
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA : STATE_FTP_TRANS_IS_CONNECT\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else
            {
                StateTransfert=STATE_FTP_TRANS_SOURCE; //sono già connesso e mando altro file

                strcpy(buff_send_command,"SOURCE ");
                strcat(buff_send_command,StrBuffTemp);
                socketpy.write(buff_send_command);//invio SOURCE + nome lista file modificato
                CommandRetry=N_RETRY;
            }
        break;
        case STATE_FTP_TRANS_CONNECTED:
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;

            if(TcpParampy()==VALUE_OK)
            {
                socketpy.write("LOGIN");
                ptimeout.SetTimer(UN_SECONDO*5);
                StateTransfert=STATE_FTP_TRANS_LOGIN;
                CommandRetry=N_RETRY;
            }
            else if(TcpParampy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: PARAM VALUE_KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;
            }
            else
            {
                CommandRetry--;
                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: PARAM error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    StateTransfert=STATE_FTP_TRANS_IS_CONNECT;
                }
            }

        break;
        case STATE_FTP_TRANS_LOGIN:
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;

            if(Tcploginpy()==VALUE_OK)
            {
                char temp_folder[MAX_LEN_CAMPO+1];
                //char *p;
                FtpConnected=true;
                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_CD;

                strcpy(buff_send_command,"CD ");

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: prima<%s>\r\n",Configuration.Config.folder_server_ftp );//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                strncpy(&temp_folder[0],Convert_link_ftp(Configuration.Config.folder_server_ftp),MAX_LEN_CAMPO);

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: dopo<%s>\r\n",temp_folder );//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                strcat(buff_send_command,temp_folder);
                socketpy.write(buff_send_command);
                CommandRetry=N_RETRY;
            }

            else if(Tcploginpy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: LOGIN KO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: LOGIN error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    socketpy.write("LOGIN");
                    ptimeout.SetTimer(UN_SECONDO*5);
                }
            }

        break;
        case STATE_FTP_TRANS_CD:
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;

            if(Tcpcdpy()==VALUE_OK)
            {
                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_SOURCE;
                strcpy(buff_send_command,"SOURCE ");
                strcat(buff_send_command,StrBuffTemp);
                socketpy.write(buff_send_command);//invio SOURCE + nome lista file modificato
                CommandRetry=N_RETRY;
            }
            else if(Tcpcdpy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: CD KO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: CD error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    char temp_folder[MAX_LEN_CAMPO+1];
                    FtpConnected=true;
                    ptimeout.SetTimer(UN_SECONDO*5);
                    StateTransfert=STATE_FTP_TRANS_CD;
                    strcpy(buff_send_command,"CD ");

                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: prima<%s>\r\n",Configuration.Config.folder_server_ftp );//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    strncpy(&temp_folder[0],Convert_link_ftp(Configuration.Config.folder_server_ftp),MAX_LEN_CAMPO);

                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: dopo<%s>\r\n",temp_folder );//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    strcat(buff_send_command,temp_folder);
                    socketpy.write(buff_send_command);
                }
            }


        break;
        case STATE_FTP_TRANS_SOURCE:
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;

            if(Tcpsourcepy()==VALUE_OK)
            {
                ptimeout.SetTimer(UN_MINUTO);

                StateTransfert=STATE_FTP_TRANS_UPLOAD;
                TCcpNameFileServerpy(StrBuffTemp);            //1 Calcolo bufferFileServerPy
                strcpy(buff_send_command,"EXTENSION ");       //2 Aggiungo comando
                strcat(buff_send_command,bufferFileServerPy); //3 stringa unica
                socketpy.write(buff_send_command);
                CommandRetry=N_RETRY;
            }
            else if(Tcpsourcepy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: source KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: SOURCE error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    ptimeout.SetTimer(UN_SECONDO*5);
                    strcpy(buff_send_command,"SOURCE ");
                    strcat(buff_send_command,StrBuffTemp);
                    socketpy.write(buff_send_command);//invio SOURCE + nome lista file modificato

                /*
                    TCcpNameFileServerpy(StrBuffTemp);            //1 Calcolo bufferFileServerPy
                    strcpy(buff_send_command,"EXTENSION ");       //2 Aggiungo comando
                    strcat(buff_send_command,bufferFileServerPy); //3 stringa unica
                    socketpy.write(buff_send_command);

                    ptimeout.SetTimer(UN_SECONDO*5);
                */
                }
            }

        break;

        case STATE_FTP_TRANS_UPLOAD:
            SetWorkPeriod(ID_CPU,40,true);
            if(ptimeout.IsElapsed())
            {
                //StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: UPLOAD KO timeout interno attendere 2 minuti\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                //ptimeout.SetTimer(UN_MINUTO*2);
                StateTransfert=STATE_FTP_TRANS_PYHTON_KILL;
                CommandRetry=N_RETRY;
//qui
            }

            if(TcpuploadOKpy())
            {
                ptimeout.SetTimer(0);
                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_RENAME;
                //TCcpNameFileServerpy(StrBuffTemp);         //1 Calcolo bufferFileServerPy (gia calcolato)
                strcpy(buff_send_command,"RENAME ");         //2 Aggiungo comando

                quint16 temp;
                temp=strlen(bufferFileServerPy);
                if(temp>0)
                {
                    if(Configuration.Config.transcoder) //usato per abilitare/disabilitare transcoder
                    {
                        bufferFileServerPy[temp-1]='4';
                    }
                    else
                    {
                        bufferFileServerPy[temp-1]='s';
                    }
                }

                strcat(buff_send_command,bufferFileServerPy); //3 stringa unica
                socketpy.write(buff_send_command);
                CommandRetry=N_RETRY;

            }
            else if(TcpuploadKOpy())
            {
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //todo decidere se ko
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: UPLOAD KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else if(TcpuploadTimeoutpy())
            {
                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //todo decidere se va in timeout cosa fare
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: UPLOAD Timeout python\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else if(TcpuploadRunning())
            {
                 ptimeout.SetTimer(UN_MINUTO*1); //ogni volta che mando su aggiorno timer sicurezza
            }
        break;

        case STATE_FTP_TRANS_RENAME:
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;

            if(TcpuploadRenamepy()==VALUE_OK)
            {
                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_DEL_RENAME_CONF;
                CommandRetry=N_RETRY;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: File completato e rinominato\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

//qui
                if(CountFileTransfert>SIZE_TRANSF_MAX)CountFileTransfert=0;
                CountFileTransfert+=1;

                //StateTransfert=STATE_FTP_TRANS_DISCONNECT;
                //socketpy.write("DISCONNECT");

            }
            else if(TcpuploadRenamepy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: RENAME KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: RENAME error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    ptimeout.SetTimer(UN_SECONDO*2);
                    strcpy(buff_send_command,"RENAME ");         //2 Aggiungo comando

                    quint16 temp;
                    temp=strlen(bufferFileServerPy);
                    if(temp>0)
                    {
                        if(Configuration.Config.transcoder) //usato per abilitare/disabilitare transcoder
                        {
                            bufferFileServerPy[temp-1]='4';
                        }
                        else
                        {
                            bufferFileServerPy[temp-1]='s';
                        }
                    }

                    strcat(buff_send_command,bufferFileServerPy); //3 stringa unica
                    socketpy.write(buff_send_command);
                }
            }
            break;
        case STATE_FTP_TRANS_DEL_RENAME_CONF:
            SetWorkPeriod(ID_CPU,40,true);

            removefile(StrBuffTemp);

            if(ptimeout.IsElapsed())
            {

                if(!Configuration.Config.canc_dopo_trasferimento)
                {
                    //canc_dopo_trasferimento
                    QFile file(StrBuffTemp);

                    if(file.exists())
                    {
                        quint16 temp;
                        temp=strlen(StrBuffTemp);

                        if(temp>0)
                        {
                            strcpy(fileTempRename,StrBuffTemp);

                            fileTempRename[temp-1]='x';
                            QFile::rename(StrBuffTemp, fileTempRename); //cambio

                            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA:rinomino: %s \r\n",fileTempRename);
                            SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        }
                    }
                }
                else
                {
                   removefile(StrBuffTemp);
                }

                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_INIT;
                CommandRetry=N_RETRY;
             }
        break;
        case STATE_FTP_TRANS_FINISH :

            if(IsTelephoneNumber(MMGestionetransfer.ConnectedNum))
            {
                sprintf(GFStransf,"FTPR: FILE TRASFERITI CON SUCCESSO");
                MESSAGE.MessageUserMMPost(&MMGestionetransfer,(char *)&GFStransf[0]);
                MMGestionetransfer.ConnectedNum[0]=0;
            }
snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: STATE_FTP_TRANS_FINISH1\r\n");//temp
SerialPrint.Flush(PRINT_DEBUG_ALL);

            SetWorkPeriod(ID_CPU,40,true);
            socketpy.write("DISCONNECT");
            ptimeout.SetTimer(UN_SECONDO*2);
            StateTransfert=STATE_FTP_TRANS_DISCONNECT;
            startTransfer=false;
        break;
        case STATE_FTP_TRANS_DISCONNECT:
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;

snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: STATE_FTP_TRANS_DISCONNECT\r\n");//temp
SerialPrint.Flush(PRINT_DEBUG_ALL);

            //if(TcpuploadDisconnectpy()==VALUE_OK)
            if(!PythonConnected)
            {
                StateTransfert=STATE_FTP_TRANS_PYHTON_KILL;
                CommandRetry=N_RETRY;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: attendere 2 minuti \r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                FtpTransfertRun=false;
            }
         //   else if(TcpuploadDisconnectpy()==VALUE_KO)
         //   {
         //       snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: DISCONNECT KO\r\n");//temp
         //       SerialPrint.Flush(PRINT_DEBUG_FUNC);
         //       StateTransfert=STATE_FTP_TRANS_ERROR;
         //       startTransfer=false;                   //togliere
         //   }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: DISCONNECT error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    ptimeout.SetTimer(UN_SECONDO*2);
                    socketpy.write("DISCONNECT");
                }
            }

        break;


      
        case STATE_FTP_TRANS_ERROR :
            FtpTransfertRun=false;
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: error \r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            StateTransfert=STATE_FTP_TRANS_NULL;
            FtpConnected=false;
            Modem.ECMReqFtp=false;
            startTransfer=false;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: PROGRAMMO LA SVEGLIA TRA 5 min\r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            SetSveglia(SV_TRANSFER,5);
        break;
        case STATE_FTP_TRANS_ERROR_NULL:
            FtpTransfertRun=false;
            SetWorkPeriod(ID_CPU,40,true);
            if(!ptimeout.IsElapsed())break;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA: nessun file \r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            StateTransfert=STATE_FTP_TRANS_NULL;
            FtpConnected=false;
            Modem.ECMReqFtp=false;
            startTransfer=false;
        break;
        default:
        break;
    }
}


void ftptransfert::PrintObj()
{
    quint8 video;

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::StateTransfert=%d \r\n",StateTransfert);//StateTransfert
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::FtpConnected=%d \r\n",FtpConnected);//FtpConnected
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::startTransfer=%d \r\n",startTransfer);//startTransfer
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::Modem.ECMReqFtp=%d \r\n",Modem.ECMReqFtp);//Modem.ECMReqFtp
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::PythonConnected=%d \r\n",PythonConnected);//PythonConnected
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::pidpy=%d \r\n",pidpy);//pidpy
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::MCFindTransfert.startfind=%d \r\n",MCFindTransfert.StateFind);//pidpy
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    if(Configuration.Config.transcoder) //usato per abilitare/disabilitare transcoder
    {
        video=TYPE_MP4; //file transcodificati
    }
    else{
        video=TYPE_TS; //file originali
    }

 //   if(MCFindTransfert.SetValidDate(FtpDate.TDInit,FtpDate.THInit,FtpDate.TDFinish,FtpDate.THFinish,video))
 //   {
 //       snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPTRA::startfind\r\n");//pidpy
 //       SerialPrint.Flush(PRINT_DEBUG_ALL);
 //       MCFindTransfert.printlist=true;
 //   }


}

