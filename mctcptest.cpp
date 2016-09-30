#include "mctcptest.h"
#include "mcserial.h"
#include "mcgestionedate.h"
#include "mcmodem.h"
#include "tstate.h"
#include "utility.h"

extern MCSerial SerialPrint;
extern char buff_print[];

extern TGDate RemLinkDate;
extern TGDate TempTerminalDate;
extern mcgestionedate Gdate;

extern MCModem Modem;




mctcptest::mctcptest(QObject *parent) :
    QObject(parent)
{

}
void mctcptest::Init(TimerMan *t)
{
    pREGTIMER=t;
    TimerConnect.Init((char*)"tcpconnect");
    TimerLogin.Init((char*)"tcplogin");
    TimeOut.Init((char*)"RLKTimeOut");

    pREGTIMER->RegisterTimer(&TimerConnect);
    pREGTIMER->RegisterTimer(&TimerLogin);
    pREGTIMER->RegisterTimer(&TimeOut);

    TimerTcp.setInterval(500);
    TimerTcp.start();

    connect(&socket, SIGNAL(connected()),this, SLOT(connected()));
    connect(&socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(&socket, SIGNAL(readyRead()),this, SLOT(readyRead()));
    connect(&TimerTcp, SIGNAL(timeout()), this, SLOT(Run()));
    TcpConnected=false;
    //TcpLogin=false;
    tcpState=REMLINK_READY;

    Tcp_index_buffer_in=0;
    Tcp_index_buffer_out=0;
    Tcp_input_index=0;
    Abort();

}

quint8 mctcptest::GetResult()
{
    return(true);
}

quint8 mctcptest::Start(char *ip, char *port,char *login,char *password,char *APN,char *NumTelSMS)
{


    //if(!TimerLogin.IsElapsed())
    if(!available)
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: RIPROVA TRA 2 MINUTI\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return(REMLINK_ERROR_DELAY);
    }

    if(tcpState!=REMLINK_READY)return(REMLINK_NO_READY);

    strncpy(ConnectionParam.ip,ip,MAX_LEN_CAMPO);        //ip
    strncpy(ConnectionParam.port,port,MAX_LEN_CAMPO);   //porta
    strncpy(ConnectionParam.login,login,MAX_LEN_CAMPO);  //login
    strncpy(ConnectionParam.password,password,MAX_LEN_CAMPO);      //psw
    strncpy(ConnectionParam.APN,APN,MAX_LEN_CAMPO);
    strncpy(ConnectionParam.NumTelSMS,NumTelSMS,MAX_LEN_CAMPO);

    if(strcmp(ConnectionParam.APN,STR_NO)==0)return(REMLINK_NO_APN);


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: CONNESSIONE IN CORSO...\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    go=true;

    // ip e porta connessione tcp
    //socket.connectToHost(RemLink.IpRemlink, port);

    // se in 5 secondi non si connette
 //   if(!socket.waitForConnected(10000))
  //  {
  //      TcpConnected=false;
  //      socket.close();
  //      tcpState=TCP_STATE_INIT;
  //      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TCP: error timeout\r\n");
  //      SerialPrint.Flush(PRINT_DEBUG_ALL);

  //  }
//    tcpState=TCP_STATE_INIT;
    TimerConnect.SetTimer(UN_SECONDO*40);
//    TcpLogin=true;
}


void mctcptest::Abort()
{
    go=false;
    strncpy(ConnectionParam.ip,STR_NO,MAX_LEN_CAMPO);
    strncpy(ConnectionParam.port,STR_NO,MAX_LEN_CAMPO);
    strncpy(ConnectionParam.login,STR_NO,MAX_LEN_CAMPO);
    strncpy(ConnectionParam.password,STR_NO,MAX_LEN_CAMPO);
    strncpy(ConnectionParam.APN,STR_NO,MAX_LEN_CAMPO);
    strncpy(ConnectionParam.NumTelSMS,STR_NO,MAX_LEN_CAMPO);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: ABORT!!!!!\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    TcpConnected=false;


}

void mctcptest::SClose()
{
    TcpConnected=false;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: CLOSE CONNECTION\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
}


void mctcptest::connected()
{
    connectionTcpOpened=true;
    TcpConnected=false;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: SIGNAL CONNESSO AL REMLINK\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    //TcpLogin=true;
}

void mctcptest::disconnected()
{
    connectionTcpOpened=false;
    TcpConnected=false;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: DISCONNECTED\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    tcpState=TCP_STATE_CLOSE;
}

void mctcptest::readyRead()
{
    QByteArray dato;

    if(tcpState==TCP_STATE_RUN)
    {
        while(socket.bytesAvailable())
        {
            socket.read(&bufferTcp[Tcp_index_buffer_in++],1);
            if(Tcp_index_buffer_in>=MAX_LEN_MCSERIAL_BUFFER)Tcp_index_buffer_in=0;
        }
    }
    else
    {
        dato=socket.readAll();
        snprintf(&bufferTcpLogin[0],MAX_LEN_MCTCP_BUFFER,"%s\r\n",dato.data());
    }
    TimerConnect.SetTimer(UN_SECONDO*40);
}

bool mctcptest::GetCharTcp(char *c)
{                             // Ottiene un carattere
    if(Tcp_index_buffer_out!=Tcp_index_buffer_in)
    {
        *c=bufferTcp[Tcp_index_buffer_out++];

        if(Tcp_index_buffer_out>=MAX_LEN_MCTCP_BUFFER)Tcp_index_buffer_out=0;
        return(true);
    }
    return(false);
}



bool mctcptest::TcpGetDateHour(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpLogin[0], "REMLINK DATETIME : ")))
    {

      p += strlen("REMLINK DATETIME : ");

      strncpy(RemLinkDate.Date,p,T_HOUR_DATESIZE); //prima parte del buffer
      RemLinkDate.Date[T_HOUR_DATESIZE]=0;

      p+=T_HOUR_DATESIZE; //mi sposto per leggere ora

      strncpy(RemLinkDate.Hour,p,T_HOUR_HOURSIZE); //seconda parte del buffer
      RemLinkDate.Hour[T_HOUR_HOURSIZE]=0;

      if(Gdate.SetDateIsValid(RemLinkDate.Date,RemLinkDate.Hour,&TempTerminalDate))
      {
          TempTerminalDate.update=true;

      }
      else{
          snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: errore date hour\r\n");
          SerialPrint.Flush(PRINT_DEBUG_FUNC);
          return(false);
      }
      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: data %s ora %s\r\n",RemLinkDate.Date,RemLinkDate.Hour);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);

      SetSveglia(SV_HOUR,720);
      return(true);
    }

    return(false);
}


bool mctcptest::TcpGetLogin(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpLogin[0], "LOGIN")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);

      return(true);
    }
    return(false);
}

bool mctcptest::TcpGetPsw(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpLogin[0], "PASSWORD")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);

      return(true);
    }
    return(false);

}

bool mctcptest::TcpGetWait(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpLogin[0], "WAITING TO CONNECT...")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: %s\r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);

      return(true);
    }
    return(false);

}


bool mctcptest::TcpGetLFailed(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpLogin[0], "LOGIN FAILED")))
    {

      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: %s ATTENDERE 2 MINUTI \r\n",p);
      SerialPrint.Flush(PRINT_DEBUG_FUNC);

      return(true);
    }
    return(false);

}




bool mctcptest::TcpGetConnected(void)
{
    char *p;

    if ((p = (char *)strstr(&bufferTcpLogin[0], "CONNECTED")))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK[Rx]: %s\r\n",p);
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        TimerConnect.SetTimer(UN_SECONDO*40);
        return(true);
    }
    return(false);

}





char mctcptest::SendCharTcp(char c)
{
 // Invia un carattere e se non può attende finche può
    char Tcptmparray[2];
    Tcptmparray[0]=c;
    Tcptmparray[1]=0;

    socket.write(&Tcptmparray[0]);

    return(c);
}


void mctcptest::Run()
{

    if(!GetState(MODEM_CONNECTED))
    {
        connectionTcpOpened=false;
    }
    if(!TimerLogin.IsElapsed())available=false;
    else available=true;




    switch(tcpState)
    {


        case REMLINK_READY:
            connectionTcpOpened=false;



            if(go)
            {
                tcpState=REMLINK_REQ_ECM;
                SetSveglia(SV_HOUR,30);
            }
        break;

        case REMLINK_REQ_ECM:
            if(!go)
            {
              Abort();
              tcpState=REMLINK_READY;
              break;
            }

            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: REQ ECM (tout=60s)\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            Modem.ECMReqRemLink=true;
            strcpy(Modem.APNRemLink,ConnectionParam.APN);
            connectionTcpOpened=false;
            tcpState=REMLINK_WAIT_ECM;
            TimeOut.SetTimer(60*UN_SECONDO);
        break;

        case REMLINK_WAIT_ECM:


            if(GetState(MODEM_CONNECTED))
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: ECM CONNECTED\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                tcpState=REMLINK_OPEN_CONNECTION;
                break;
            }

            if(TimeOut.IsElapsed() || (!go))
            {
                if(TimeOut.IsElapsed())
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: ECM TIMEOUT !!!\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }
                Abort();
                Modem.ECMReqRemLink=false;
                tcpState=REMLINK_READY;
                break;
            }


        break;

        case REMLINK_OPEN_CONNECTION:


            if((!GetState(MODEM_CONNECTED)) || (!go))
            {
                if(!GetState(MODEM_CONNECTED))
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: DISCONNESSO x ^NDISSTATQRY: 0\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                }
                if((!go))
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: DISCONNESSO x ABORT\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                }

                Abort();
                Modem.ECMReqRemLink=false;
                tcpState=REMLINK_READY;
                break;
            }

            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: CONNECT TO HOST %s %s (tout=60s)\r\n",ConnectionParam.ip,ConnectionParam.port);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            connectionTcpOpened=false;
            socket.connectToHost(ConnectionParam.ip, atoi(ConnectionParam.port));
            TimeOut.SetTimer(60*UN_SECONDO);
            tcpState=REMLINK_WAIT_OPEN_CONNECTION;

        break;

        case REMLINK_WAIT_OPEN_CONNECTION:
            if((!GetState(MODEM_CONNECTED)) || (!go))
            {
                Abort();
                Modem.ECMReqRemLink=false;
                tcpState=REMLINK_READY;
                break;
            }

            if (socket.state() == QAbstractSocket::ConnectedState)
            {
                connectionTcpOpened=true;
            }
            else if(TimeOut.IsElapsed())
            {
                Abort();
                Modem.ECMReqRemLink=false;
                tcpState=REMLINK_READY;
                connectionTcpOpened=false;
                break;
            }

            if(connectionTcpOpened)
            {
                tcpState=TCP_STATE_DATE_HOUR;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: HOST CONNECTED !!!!\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                TimerLogin.SetTimer(UN_SECONDO*2);

                // TODO da qu, in tt gli stati successivo se go va a false, fare socket close!!!
            }
        break;

        case RLK_STATE_INIT:
            Modem.ECMReqRemLink=false;
            tcpState=REMLINK_READY;

        break;
        case TCP_STATE_DATE_HOUR:

            if(TcpGetDateHour())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: HOUR OK\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                tcpState=TCP_STATE_LOGIN;
                TimerLogin.SetTimer(UN_SECONDO);
            }
            else if(!go)
            {
                tcpState=TCP_STATE_CLOSE;
            }
            else
            {
                tcpState=TCP_STATE_CLOSE;
            }

        break;

        case TCP_STATE_LOGIN:

            if(!TimerLogin.IsElapsed())break;

            if(TcpGetLogin())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: LOGIN\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                socket.write(ConnectionParam.login);
                socket.write("\r\n");
                tcpState=TCP_STATE_PSW;
                TimerLogin.SetTimer(UN_SECONDO);
            }
            else if(!go)
            {
                tcpState=TCP_STATE_CLOSE;
            }
            else
            {
                tcpState=TCP_STATE_CLOSE;
            }

        break;
        case TCP_STATE_PSW:

            if(!TimerLogin.IsElapsed())break;

            if(TcpGetPsw())
            {
                tcpState=TCP_STATE_WAIT;
                socket.write(ConnectionParam.password);
                socket.write("\r\n");
                TimerLogin.SetTimer(UN_MINUTO*2);
            }
            else if(!go)
            {
                tcpState=TCP_STATE_CLOSE;
            }
            else
            {
                tcpState=TCP_STATE_CLOSE;
            }

        break;
        case TCP_STATE_WAIT:

            if(TcpGetWait())
            {
                tcpState=TCP_STATE_CONNECT;
                TimerLogin.SetTimer(UN_MINUTO*2);
            }
            else if(TcpGetLFailed())
            {
                TimerLogin.SetTimer(UN_MINUTO*2);
                tcpState=TCP_STATE_CLOSE;
            }
            else if(TcpGetConnected())
            {
                tcpState=TCP_STATE_RUN;
                TcpConnected=true;
                TimerLogin.SetTimer(UN_SECONDO);
            }

            else if(TimerLogin.IsElapsed())
            {
                tcpState=TCP_STATE_CLOSE;
            }
            else if(!go)
            {
                tcpState=TCP_STATE_CLOSE;
            }

        break;
        case TCP_STATE_CONNECT:

            if(TcpGetConnected())
            {
                tcpState=TCP_STATE_RUN;
                TcpConnected=true;
                TimerLogin.SetTimer(UN_SECONDO);
            }
            else if(TimerLogin.IsElapsed())
            {
                tcpState=TCP_STATE_CLOSE;
            }
            else if(!go)
            {
                tcpState=TCP_STATE_CLOSE;
            }
        break;

        case TCP_STATE_RUN:
//TimerConnect.SetTimer(UN_SECONDO*40);  //rrrrrrrrrr

            if((TimerConnect.IsElapsed())||
               (!TcpConnected) ||
               (!go))
            {
                tcpState=TCP_STATE_CLOSE;
            }
        break;

        case TCP_STATE_CLOSE:
            TimerLogin.SetTimer(UN_MINUTO*2);
            //TcpLogin=false;
            TcpConnected=false;
            socket.close();
            tcpState=RLK_STATE_INIT;
            Abort();
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"RLK: CLOSE (delay=2m)\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        break;
        default:
        break;
    }

}


void mctcptest::Write(char *str)
{
    if(TcpConnected)
    {
        socket.write(str);
        socket.waitForBytesWritten();
    }

}


void mctcptest::PrintObj()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"***\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* CONFIGMOBILE\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* tcpState=%d\r\n",tcpState);
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"***\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

}

