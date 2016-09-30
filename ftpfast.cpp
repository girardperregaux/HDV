
#include "ftpfast.h"
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
#include "vidaudiorec.h"
#include "ftptranscode.h"

#define SIZE_TRANSFERT_100MB    100000

#define INIT_TO_LAST      1
#define ALWAYS_LAST       2

#define PATH_SERVER_FTP   "/mcsistemi.it/fwREMHDV/"



extern TConfig Configuration;
extern MCSerial SerialPrint;
extern char buff_print[];
extern MCModem Modem;
extern vidaudiorec Videoaudio;
extern ftptranscode FtpTranscodeRun;
extern quint16 CountFileTransfert;

ftpfast::ftpfast(QObject *parent) :
    QObject(parent)
{
}
void ftpfast::Init(TimerMan *t)
{
    pREGTIMER=t;

    Timertransfert.Init((char*)"ftpfast");
    ptimeout.Init((char*)"pythonfast");
    Timertransfert.SetTimer(UN_SECONDO*1);
    ptimeout.SetTimer(UN_SECONDO*1);


    pREGTIMER->RegisterTimer(&Timertransfert); //timer registrato
    pREGTIMER->RegisterTimer(&ptimeout); //timer registrato


    TTransfertRun.setInterval(11);//era 100ms
    TTransfertRun.start();

    StateTransfert=STATE_FTP_TRANS_NULL;
    UpgradeTransfer=false;
    startTransfer=false;
    MCFindTransfert.Init(MCFINDERTRANSFERFAST);
    MCFindTransfert.Reset_Init();
    PythonConnected=false;
    FtpConnected=false;

    connect(&socketpyfast, SIGNAL(connected()),this, SLOT(connectedpy()));
    connect(&socketpyfast, SIGNAL(disconnected()),this, SLOT(disconnectedpy()));
    connect(&socketpyfast, SIGNAL(readyRead()),this, SLOT(readyReadpy()));
    connect(&TTransfertRun, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati
    pytcpfast= new QProcess(this);
    pidpy=0;
    CommandRetry=N_RETRY;
    Ftp_sleep=false;
    FileFastReady=false;
    first_connection=true;

    Start_Time_transfer=false;
    Time_transfer=0;

}

bool ftpfast::removefile(char * Tfile)
{
    QFile file(Tfile);

    if(file.exists())
    {
        if(file.remove())
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST:file cancellato : %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (true);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST:Error file remove : %s\r\n",Tfile);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return (false);
        }
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST:Error file non esiste : %s\r\n",Tfile);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
    }
    return (false);

}

void ftpfast::connectedpy()
{
    PythonConnected=true;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: CONNECT PYTHON\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
}

void ftpfast::disconnectedpy()
{
    PythonConnected=false;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: DISCONNECT PYTHON\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
}

void ftpfast::readyReadpy()
{
    QByteArray dato;

    dato=socketpyfast.readAll();
    snprintf(&bufferTcpPy[0],STRING_TRANSC,"%s\r\n",dato.data());

    //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",bufferTcpPy);
    //SerialPrint.Flush(PRINT_DEBUG_ALL);

}

quint8 ftpfast::TcpParampy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: name server ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }

    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: name server ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}

quint8 ftpfast::Tcploginpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: login ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: login ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}

quint8 ftpfast::Tcpcdpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: path ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: path ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}

quint8 ftpfast::Tcpsourcepy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: source ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: source ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}


bool ftpfast::TcpuploadOKpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    return(false);
}

bool ftpfast::TcpuploadKOpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    return(false);
}

bool ftpfast::TcpuploadTimeoutpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload timeout")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    return(false);
}

bool ftpfast::TcpuploadRunning(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: upload in corso")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(true);
    }
    //memset(bufferTcpPy,0,STRING_TRANSC);
    return(false);
}


quint8 ftpfast::TcpuploadRenamepy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: rename ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: rename ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}


quint8 ftpfast::TcpuploadDisconnectpy(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: ftp quit ok")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_OK);
    }
    else if ((p = (char *)strstr(&bufferTcpPy[0], "PYT: ftp quit ko")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);
      memset(bufferTcpPy,0,STRING_TRANSC);
      return(VALUE_KO);
    }
    return(VALUE_NULL);
}


bool ftpfast::TCcpNameFileServerpy(char *name_file)
{
    quint8 len=0;
    int idx=0;
    char NameFileOpen[STRING_TRANSC+1];
    char buff_t_tamp[STRING_TRANSC+1];
    char buff_t_size[STRING_TRANSC+1];


    strcpy(bufferFileServerPy,Configuration.Config.codice_gruppo);     //preparo stringa per salvare server ftp
    strcat(bufferFileServerPy,Configuration.Config.codice_periferica);
    strcat(bufferFileServerPy,"_");

    strcpy(NameFileOpen,name_file); //salvo nome file
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

//aggiungere dimensione file

    QFile filefast(NameFileOpen);

    if (!filefast.open(QIODevice::ReadOnly))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: ERRORE APERTURA FILE %s\r\n",NameFileOpen);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return(false);                         //ed esco da qui
    }

    quint32 size = filefast.size(); //passo a quint32 (mando su 500kB quindi mi bastano 7 cifre

    sprintf(&buff_t_size[0],"%07d",size);

    filefast.close();

    buff_t_tamp[25]='_';

    buff_t_tamp[26]=buff_t_size[0];
    buff_t_tamp[27]=buff_t_size[1];
    buff_t_tamp[28]=buff_t_size[2];
    buff_t_tamp[29]=buff_t_size[3];
    buff_t_tamp[30]=buff_t_size[4];
    buff_t_tamp[31]=buff_t_size[5];
    buff_t_tamp[32]=buff_t_size[6];


    buff_t_tamp[33]=NameFileTempPy[idx+25];//.

    buff_t_tamp[34]='m';//lascio mp4 perche' controllo dimensione
    buff_t_tamp[35]='p';
    buff_t_tamp[36]='4';  //4
    buff_t_tamp[37]=0;

    strcat(bufferFileServerPy,buff_t_tamp);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: size %s\r\n",buff_t_size);
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

    return(true);

}



void ftpfast::SendCharTcpPy(char *c)
{
    if(PythonConnected)
        socketpyfast.write(c);
}

bool ftpfast::SetDate(char *dateinit,char *hourinit, char *datefinish,char *hourfinish)
{
    recheck_list=true;
    strcpy(FtpDate.TDInit,dateinit);
    strcpy(FtpDate.THInit,hourinit);
    strcpy(FtpDate.TDFinish,datefinish);
    strcpy(FtpDate.THFinish,hourfinish);

    return true;
}


bool ftpfast::FileReady(char *FileFtp)
{
    if(strcasecmp(FileFtp,STR_NO)==0)return false;

    strcpy(StrBuffTemp,FileFtp);

    return true;
}

//macchina a stati
void ftpfast::Run()
{
    char buff_send_command[STRING_TRANSC+1];
    //char fileTempRename[STRING_TRANSC+1];

    //vedere se connesso

    if(Start_Time_transfer)
    {
        Time_transfer++; //ogni 100 ms incremento
    }

    switch(StateTransfert)
    {
        case STATE_FTP_TRANS_PYHTON_LAUNCH:
            pidpy=0;
            PythonConnected=false;
            pytcpfast->start(STR_PY_FAST_PROG);
            pidpy = pytcpfast->pid();
            sprintf(StrPidpy," kill -9 %d",(int)pytcpfast);
            StateTransfert=STATE_FTP_TRANS_PYHTON_INIT;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: AVVIO SCRIPT\r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            ptimeout.SetTimer(0);
            ptimeout.SetTimer(UN_SECONDO*5);//
        break;
        case STATE_FTP_TRANS_PYHTON_KILL:

            FtpConnected=false;
            StateTransfert=STATE_FTP_TRANS_WAIT_KILL;
            startTransfer=false;                   //togliere
            Modem.ECMReqFtpFast=false;
            pytcpfast->close();
            PythonConnected=false;
            ptimeout.SetTimer(UN_MINUTO*2);
        break;
        case STATE_FTP_TRANS_WAIT_KILL:

            if(!ptimeout.IsElapsed())break;

            StateTransfert=STATE_FTP_TRANS_NULL;
        break;
        case STATE_FTP_TRANS_PYHTON_INIT:

            if(!ptimeout.IsElapsed())break;

            socketpyfast.connectToHost("localhost",PORT_PY_FAST);
            StateTransfert=STATE_FTP_TRANS_PYHTON_IS_CONNECT;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: CONNECT TO HOST\r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            ptimeout.SetTimer(UN_SECONDO*5);
        break;
        case STATE_FTP_TRANS_PYHTON_IS_CONNECT:
            if(PythonConnected)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: TCP CONNECT\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_NULL;
            }

            if(!pytcpfast->isOpen())
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

        //QUI  è false
            SetState(FTP_FAST_RUN,false);

            if(UpgradeTransfer)
            {
                SetWorkPeriod(ID_CPU,30,true);
                if(!PythonConnected)
                {
                    StateTransfert=STATE_FTP_TRANS_PYHTON_LAUNCH; //UpgradeTransfer va a false dopo che python si connette
                }
                else{
                    //controllare presenza hard disk pronto

                    UpgradeTransfer=false;
                    MCFindTransfert.Reset_Init();
                    indexTransfert=0;
                    startTransfer=true;
                    CommandRetry=N_RETRY;
                }

                strcpy(Modem.APNFtpFast,Configuration.Config.APN);
                Modem.ECMReqFtpFast=true;

            }
            if(startTransfer)
            {
                //qui e' true
               SetState(FTP_FAST_RUN,true);
               StateTransfert=STATE_FTP_TRANS_INIT;
               first_connection=true;
               ptimeout.SetTimer(UN_MINUTO*2);

               strcpy(Modem.APNFtpFast,Configuration.Config.APN);
               Modem.ECMReqFtpFast=true;
            }

        break;
        case STATE_FTP_TRANS_INIT :
            SetWorkPeriod(ID_CPU,30,true);

            if(ptimeout.IsElapsed())
            {
              Ftp_sleep=true;
              ptimeout.SetTimer(UN_SECONDO*5);
              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: TIME SLEEP\r\n");
              SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }

            //qui
            if(first_connection) //mi porto avanti con ftp già connesso
            {
                StateTransfert=STATE_FTP_TRANS_MODEM_ON;
                ptimeout.SetTimer(0);
            }
            else if(FileFastReady)  //ogni file pronto .mp4 parte
            {
                FileFastReady=false;

Start_Time_transfer=true;
Time_transfer=0;

                if(FileReady(FtpTranscodeRun.file_ftpfast)) //passo il file .mp4
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST :File ready to tranfert\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_MODEM_ON;
                    ptimeout.SetTimer(0);
                }
            }
            else if(Ftp_sleep)
            {
                Ftp_sleep=false;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: SLEEP\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_FINISH; //test
                ptimeout.SetTimer(0);
            }

        break;
        case STATE_FTP_TRANS_MODEM_ON:

            SetWorkPeriod(ID_CPU,30,true);

            if(!Modem.ECMReqFtpFast)
            {
                strcpy(Modem.APNFtpFast,Configuration.Config.APN);
                Modem.ECMReqFtpFast=true;


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
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST : MODEM CONNESSO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_IS_CONNECT;
            }
        break;

        case STATE_FTP_TRANS_IS_CONNECT:
            SetWorkPeriod(ID_CPU,30,true);
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

                socketpyfast.write(buff_send_command);
                StateTransfert=STATE_FTP_TRANS_CONNECTED;
                ptimeout.SetTimer(0);
                ptimeout.SetTimer(UN_SECONDO*5);
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST : STATE_FTP_TRANS_IS_CONNECT\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else
            {
                StateTransfert=STATE_FTP_TRANS_SOURCE; //sono già connesso e mando altro file

                strcpy(buff_send_command,"SOURCE ");
                strcat(buff_send_command,StrBuffTemp);

                socketpyfast.write(buff_send_command);//invio SOURCE + nome lista file modificato

                //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST :buff_send_command=%s\r\n",buff_send_command);
                //SerialPrint.Flush(PRINT_DEBUG_FUNC);
                ptimeout.SetTimer(UN_SECONDO*2);
                CommandRetry=N_RETRY;
            }
        break;
        case STATE_FTP_TRANS_CONNECTED:
            SetWorkPeriod(ID_CPU,30,true);
            if(!ptimeout.IsElapsed())break;

            if(TcpParampy()==VALUE_OK)
            {
                socketpyfast.write("LOGIN");
                ptimeout.SetTimer(UN_SECONDO*5);
                StateTransfert=STATE_FTP_TRANS_LOGIN;
                CommandRetry=N_RETRY;
            }
            else if(TcpParampy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: PARAM VALUE_KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;
            }
            else
            {
                CommandRetry--;
                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: PARAM error retry\r\n");//temp
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
            SetWorkPeriod(ID_CPU,30,true);
            if(!ptimeout.IsElapsed())break;

            if(Tcploginpy()==VALUE_OK)
            {
                char temp_folder[MAX_LEN_CAMPO+1];
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

                socketpyfast.write(buff_send_command);
                CommandRetry=N_RETRY;
            }

            else if(Tcploginpy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: LOGIN KO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: LOGIN error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    socketpyfast.write("LOGIN");
                    ptimeout.SetTimer(UN_SECONDO*5);
                }
            }

        break;
        case STATE_FTP_TRANS_CD:
            SetWorkPeriod(ID_CPU,30,true);
            if(!ptimeout.IsElapsed())break;

            if(Tcpcdpy()==VALUE_OK)
            {
                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_SOURCE;
                strcpy(buff_send_command,"SOURCE ");
                strcat(buff_send_command,StrBuffTemp);
                socketpyfast.write(buff_send_command);//invio SOURCE + nome lista file modificato
                CommandRetry=N_RETRY;
            }
            else if(Tcpcdpy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: CD KO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: CD error retry\r\n");//temp
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


                    socketpyfast.write(buff_send_command);
                }
            }


        break;
        case STATE_FTP_TRANS_SOURCE:
            SetWorkPeriod(ID_CPU,30,true);
            //if(!ptimeout.IsElapsed())break;

            if(first_connection)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: FIRST CONNECTION READY\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                first_connection=false;
                StateTransfert=STATE_FTP_TRANS_INIT;
                ptimeout.SetTimer(UN_MINUTO*2);
                break;
            }

            if(Tcpsourcepy()==VALUE_OK)
            {
                ptimeout.SetTimer(UN_MINUTO);

                StateTransfert=STATE_FTP_TRANS_UPLOAD;
                TCcpNameFileServerpy(StrBuffTemp);            //1 Calcolo bufferFileServerPy
                strcpy(buff_send_command,"EXTENSION ");       //2 Aggiungo comando
                strcat(buff_send_command,bufferFileServerPy); //3 stringa unica
                socketpyfast.write(buff_send_command);
                CommandRetry=N_RETRY;

                //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST:= %s\r\n",buff_send_command);//temp
                //SerialPrint.Flush(PRINT_DEBUG_FUNC);
                //ptimeout.SetTimer(0);
            }
            else if(Tcpsourcepy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: source KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere

                //qui dovrei passare al file successivo

            }
            else if(!ptimeout.IsElapsed())
            {
                break;
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: SOURCE error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    ptimeout.SetTimer(UN_SECONDO*2);
                    strcpy(buff_send_command,"SOURCE ");
                    strcat(buff_send_command,StrBuffTemp);
                    socketpyfast.write(buff_send_command);//invio SOURCE + nome lista file modificato

                    /*
                    TCcpNameFileServerpy(StrBuffTemp);            //1 Calcolo bufferFileServerPy
                    strcpy(buff_send_command,"EXTENSION ");       //2 Aggiungo comando
                    strcat(buff_send_command,bufferFileServerPy); //3 stringa unica

                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST:2EXTENSION= %s\r\n",buff_send_command);//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    socketpyfast.write(buff_send_command);

                    ptimeout.SetTimer(UN_SECONDO*5);
                    */
                }
            }

        break;

        case STATE_FTP_TRANS_UPLOAD:
            SetWorkPeriod(ID_CPU,30,true);
            if(ptimeout.IsElapsed())
            {
                //StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: UPLOAD KO timeout interno attendere 2 minuti\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                //ptimeout.SetTimer(UN_MINUTO*2);
                StateTransfert=STATE_FTP_TRANS_PYHTON_KILL;
                CommandRetry=N_RETRY;

            }

            if(TcpuploadOKpy())
            {
                ptimeout.SetTimer(0);
                //ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_DEL_RENAME_CONF;
                CommandRetry=N_RETRY;
                //non mi serve il rename su server (quindi mando diretto .mp4)
            }
            else if(TcpuploadKOpy())
            {
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //todo decidere se ko
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: UPLOAD KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else if(TcpuploadTimeoutpy())
            {
                ptimeout.SetTimer(UN_SECONDO*2);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //todo decidere se va in timeout cosa fare
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: UPLOAD Timeout python\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else if(TcpuploadRunning())
            {
                 ptimeout.SetTimer(UN_MINUTO*1); //ogni volta che mando su aggiorno timer sicurezza
            }
        break;

        case STATE_FTP_TRANS_DEL_RENAME_CONF:
            SetWorkPeriod(ID_CPU,30,true);

            Start_Time_transfer=false;
            Time_transfer*=10; //ho i ms
snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: file trasferito in %d ms\r\n",Time_transfer);//temp
SerialPrint.Flush(PRINT_DEBUG_FUNC);

if(CountFileTransfert>SIZE_TRANSF_MAX)CountFileTransfert=0;
CountFileTransfert+=1;

            if(ptimeout.IsElapsed())
            {

                removefile(StrBuffTemp);  //cancello i file dopo averli trasferiti

                StateTransfert=STATE_FTP_TRANS_INIT;
                ptimeout.SetTimer(UN_MINUTO*2);
                CommandRetry=N_RETRY;
             }
        break;
        case STATE_FTP_TRANS_FINISH :
            SetWorkPeriod(ID_CPU,30,true);
            socketpyfast.write("DISCONNECT");
            ptimeout.SetTimer(UN_SECONDO*2);
            StateTransfert=STATE_FTP_TRANS_DISCONNECT;
            startTransfer=false;
        break;
        case STATE_FTP_TRANS_DISCONNECT:
            SetWorkPeriod(ID_CPU,30,true);
            if(!ptimeout.IsElapsed())break;

            if(TcpuploadDisconnectpy()==VALUE_OK)
            {
                //ptimeout.SetTimer(UN_MINUTO*2);
                StateTransfert=STATE_FTP_TRANS_PYHTON_KILL;
                CommandRetry=N_RETRY;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: attendere 2 minuti \r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                SetState(FTP_FAST_RUN,false);
            }
            else if(TcpuploadDisconnectpy()==VALUE_KO)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: DISCONNECT KO\r\n");//temp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateTransfert=STATE_FTP_TRANS_ERROR;
                startTransfer=false;                   //togliere
            }
            else
            {
                CommandRetry--;

                if(CommandRetry==0)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: DISCONNECT error retry\r\n");//temp
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    StateTransfert=STATE_FTP_TRANS_ERROR;
                    startTransfer=false;
                    ptimeout.SetTimer(UN_MINUTO);
                }
                else
                {
                    ptimeout.SetTimer(UN_SECONDO*2);
                    socketpyfast.write("DISCONNECT");
                }
            }

        break;



        case STATE_FTP_TRANS_ERROR :
            SetState(FTP_FAST_RUN,false);
            SetWorkPeriod(ID_CPU,30,true);
            if(!ptimeout.IsElapsed())break;
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST: error!!!!!!!!!!!!!!!!!!!!\r\n");//temp
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            StateTransfert=STATE_FTP_TRANS_NULL;
            FtpConnected=false;
            Modem.ECMReqFtpFast=false;
            startTransfer=false;
        break;


        default:
        break;
    }
}


void ftpfast::PrintObj()
{
    quint8 video;

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST::StateTransfert=%d \r\n",StateTransfert);//StateTransfert
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST::FtpConnected=%d \r\n",FtpConnected);//FtpConnected
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST::startTransfer=%d \r\n",startTransfer);//startTransfer
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST::Modem.ECMReqFtpFast=%d \r\n",Modem.ECMReqFtpFast);//Modem.ECMReqFtpFast
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST::PythonConnected=%d \r\n",PythonConnected);//PythonConnected
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST::pidpy=%d \r\n",pidpy);//pidpy
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTPFAST::MCFindTransfert.startfind=%d \r\n",MCFindTransfert.StateFind);//pidpy
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
