#include "Gestionewifi.h"
#include <stdlib.h>
#include <stdio.h>
#include <DefCom.h>
#include <qdebug.h>
#include <qfile.h>


extern MCSerial SerialPrint;
extern char buff_print[];

WifiClient::WifiClient(QObject* parent)
    :QObject(parent)
{

    WifiProcess= new QProcess(this);
    connect(WifiProcess, SIGNAL(finished(int , QProcess::ExitStatus )), this, SLOT(wifiSignalfinish(int , QProcess::ExitStatus )));
    connected=false;
    CountNoConnected=0;
    CountErrPing=0;
}



void WifiClient::Init(TimerMan *t)
{
    pREGTIMER=t;
    TimerT.Init((char*)"WifiClient");

    pREGTIMER->RegisterTimer(&TimerT);

    Mcwifitimer.setInterval(100);
    Mcwifitimer.start();
    connect(&Mcwifitimer, SIGNAL(timeout()), this, SLOT(GestioneWifi()));

    WifiOn=false;
    connected=false;
    CountNoConnected=0;
    CountErrPing=0;
    State=WIFI_CL_INIT;
    finish_process=false;
    command=false;
}


void WifiClient::Stop()
{
    State=WIFI_CL_INIT;
    connected=false;
    ReqOn=false;
}


void WifiClient::GestioneWifi()
{

    switch(State)
    {
        case WIFI_CL_INIT:
            connected=false;
            ReqOn=false;

            if(!command)
            {
                command=true;
                WifiProcess->start("sh");
                WifiProcess->write("ifconfig wlan0 down");
                WifiProcess->closeWriteChannel();
            }

            if(wifi_down())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: WIFI STOP\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                TimerT.SetTimer(5*UN_SECONDO);
                State=WIFI_CL_ON;
                command=false;
            }
        break;
        case WIFI_CL_ON:
            if(!TimerT.IsElapsed())break;

            if(ReqOn)
            {
                if(!command)
                {
                    command=true;
                    WifiProcess->start("sh");
                    WifiProcess->write("ifconfig wlan0 up");
                    WifiProcess->closeWriteChannel();
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: WIFI AVVIO... \r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }

                if(wifi_up())
                {
                    State=WIFI_CL_WAIT_ON;
                    TimerT.SetTimer(2*UN_SECONDO);
                    command=false;
                }
            }

        break;
        case WIFI_CL_WAIT_ON:
            if(!TimerT.IsElapsed())break;

            if(connected) //sono già connesso
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: CONNESSO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                State=WIFI_CL_READY;
                TimerT.SetTimer(2*UN_SECONDO);
            }
            else
            {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: SCANSIONE RETI WI-FI\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    ListWifi=Scan(); //scarico lista reti trovate
                    State=WIFI_CL_SCAN;
                    TimerT.SetTimer(2*UN_SECONDO);

            }

        break;

        case WIFI_CL_READY:
            if(!TimerT.IsElapsed())break;
        //qui to doi
            if(connected)
            {
                State=WIFI_CL_PINGOOGLE;
                break;
            }
            else
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC:Scan init\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                TimerT.SetTimer(2*UN_SECONDO);
                State=WIFI_CL_SCAN;
            }
        break;

        case WIFI_CL_SCAN:
            if(!TimerT.IsElapsed())break;



            strcpy(stringssid,ReadSsid());             //qui apro file e leggo sid (è in /etc/wpa_supplicant.conf
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: leggo<%s>\r\n",stringssid);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);


            if(ScanEssid(stringssid,ListWifi))    //Imposto la rete wi-fi
            {
                 TimerT.SetTimer(2*UN_SECONDO);
                 State=WIFI_CL_CONNECT;
                 break;
            }
            else
            {
                State=WIFI_CL_INIT;
            }

        break;

        case WIFI_CL_CONNECT:
            if(!TimerT.IsElapsed())break;

            if(Connect())                        //mi connetto al modulo wi-fi
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wi-fi connect init\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                TimerT.SetTimer(5*UN_SECONDO);
                State=WIFI_CL_UDHCPC;
            }

        break;
        case WIFI_CL_UDHCPC:
            if(!TimerT.IsElapsed())break;

            if(Udhcpc())//ottengo indirizzo ip
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wi-fi udhcpc\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                TimerT.SetTimer(5*UN_SECONDO);
                State=WIFI_CL_PINGOOGLE;
            }


        break;
        case WIFI_CL_PINGOOGLE:
            if(!TimerT.IsElapsed())break;

            if(PingWWW())                     //Ping verso il modulo
            {
                State=WIFI_CL_READY;
                TimerT.SetTimer(UN_SECONDO*5);
            }
            else
            {
                TimerT.SetTimer(UN_SECONDO*5); //retry (max3 volte)
            }


        break;

        default:
        break;
    }
}




int WifiClient::ScanEssid(char *c,QStringList reti)
{

    char ESSID[MAX_NET_BUFFER_WIFI+1];

    strcpy(ESSID,c);

    for (int i=0; i<reti.count(); i++)
    {
        snprintf(&bufferwifi[0],MAX_LEN_BUFFER_PRINT,"%s",reti.at(i).toLocal8Bit().constData());

        if(strcasecmp(bufferwifi,ESSID)==0)
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wi-fi net :%s found\r\n",bufferwifi);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            return(true);
        }
    }

    bufferwifi[0]=0;
    return(false);

}


void WifiClient::wifiSignalfinish(int,QProcess::ExitStatus )
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: Signal process finished\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    finish_process=true;
}


bool WifiClient::wifi_up()
{


    //WifiProcess->start("sh");
    //WifiProcess->write("ifconfig wlan0 up");
    //WifiProcess->closeWriteChannel();

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: test2\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

    if(finish_process)
    {
        WifiProcess->close();
        finish_process=false;
        //WifiProcess->waitForFinished();
        //WifiProcess->close();
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wif-fi up\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (true);
    }

    return (false);
}

bool WifiClient::wifi_down()
{

    //WifiProcess->start("sh");
    //WifiProcess->write("ifconfig wlan0 down");
    //WifiProcess->closeWriteChannel();

    //WifiProcess->waitForFinished();
    //WifiProcess->close();

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: test1\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

    if(finish_process)
    {
        finish_process=false;
        WifiProcess->close();
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wif-fi down\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (true);
    }

    return (false);
}



bool WifiClient::ReadyScan()
{
    /*
    WifiProcess->start("sh");
    WifiProcess->write("iwlist scan  | grep ESSID");
    WifiProcess->closeWriteChannel();
    WifiProcess->waitForFinished();
    OutputString = WifiProcess->readAll();

    if(finish_process)
    {

       WifiProcess->close();
       finish_process=false;
       return (true);
    }
    */
    return (false);
}

QStringList WifiClient::Scan()
{
    QStringList StringReturn;
    QByteArray output;
    char *p;
    int temp;
    int temp_a;
    int temp_b;
    char *x;
    char buffer[5000];

    WifiProcess->start("sh");
    WifiProcess->write("iwlist scan  | grep ESSID");
    WifiProcess->closeWriteChannel();
    WifiProcess->waitForFinished();
    output = WifiProcess->readAll();

   if(finish_process)
   {
       finish_process=false;



        if((p=strstr(output.data(),"ESSID:\"")))
        {
            p+=strlen("ESSID:\"");


            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: RETI RILEVATE\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            for(int i=0;i<output.data()[i];i++)
            {
                if((x=strstr(p,"ESSID:\"")))
                {
                    temp_a=strlen(p);
                    temp_b=strlen(x);
                    temp=temp_a-temp_b;
                    strncpy(buffer,p,temp);
                    p=strstr(buffer,"\"");
                    p[0]=0;                 //tronco la stringa

                    StringReturn<< buffer;
                    p=x;
                    p+=strlen("ESSID:\"");
                }
                else   //ultima rete rilevata
                {
                    temp=strlen(p);
                    strncpy(buffer,p,temp);
                    p=strstr(buffer,"\"");
                    p[0]=0;                 //tronco la stringa
                    StringReturn<< buffer;
                    break;
                }
             }
        }



        if(StringReturn.isEmpty())
        {
            StringReturn<<"Nessuna rete wi-fi rilevata";
        }

        for (int i=0; i<StringReturn.count(); i++)
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: rete wif-fi %d=  %s\r\n",i,StringReturn.at(i).toLocal8Bit().constData());
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

        }

        return (StringReturn);

    }
}

bool WifiClient::Connect()
{

    QStringList args;
    char buffer[200];
    char *p;

    p=&buffer[0];
    p+=sprintf(buffer,".conf /usr/local/sbin/wpa_supplicant -B -Dwext -iwlan0 -c/etc/wpa_supplicant.conf");
    p[0]=0;
    args=QString(buffer).split(" ");

    WifiProcess->start("wpa_supplicant",args);
    //WifiProcess->waitForFinished();

    if(finish_process)
    {
        finish_process=false;
        WifiProcess->close();
        connected=true;
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wif-fi connesso\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (true);
    }
    return (false);
}

bool WifiClient::Udhcpc()
{
    //QProcess pudhcpc;
    QStringList args;
    char buffer[20];
    char *p;

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wif-fi udhcpc start\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

    p=&buffer[0];
    p+=sprintf(buffer,"-v -4 wlan0");
    p[0]=0;
    args=QString(buffer).split(" ");


    WifiProcess->start("dhclient",args);

    //pudhcpc.waitForFinished();
    if(finish_process)
    {
        finish_process=false;
        WifiProcess->close();

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wif-fi udhcpc end\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        return (true);
    }

    return (false);

    /*
    p=&buffer[0];
    p+=sprintf(buffer,"eth0 down");
    p[0]=0;
    args=QString(buffer).split(" ");


    pudhcpc.start("ifconfig",args);

    pudhcpc.waitForFinished();
    pudhcpc.close();
    */

}


int WifiClient::PingWWW()
{

    QProcess pping;
    QByteArray output;

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: wif-fi ping google\r\n");
    SerialPrint.Flush(PRINT_DEBUG_FUNC);

    pping.start("sh");

    pping.write("ping -c 1 -q 8.8.8.8");
    pping.closeWriteChannel();
    pping.waitForFinished();
    output = pping.readAll();


    //leggi ping ok
    if(strstr(output.data()," 0% packet loss"))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: ping google: ok\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        pping.close();
        CountErrPing=0;
        return (true);
    }

    pping.close();

    CountErrPing++;
    if(CountErrPing>2)
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC: ping google: error\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        State=WIFI_CL_INIT;
    }
    return (false);

}

char * WifiClient::ReadSsid()
{

    QFile caFile(WPA_SUPPLICANT);
    char buff_ssid[60];
    char buff_tempssid[60];

    quint8 len;

    char *pssid=&buff_ssid[0];

    caFile.open(QIODevice::ReadOnly | QIODevice::Text);

    if(!caFile.isOpen()){

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"Errore apertura file1\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
        return(buff_tempssid); //qui gestire errore
    }
    QTextStream in(&caFile);

    QString search_name("ssid=");
    QString line;
    while (!in.atEnd())
    {
        line=in.readLine();

        if (line.contains(search_name, Qt::CaseSensitive)) {
            sprintf(&buff_ssid[0],"%s",line.toUtf8().constData());
            //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"<%s>",buff_ssid);//temp
            //SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
    }
    caFile.flush();

    caFile.close();


    if ((pssid = (char *)strstr(pssid, "ssid=\"")))
    {
        pssid+=strlen("ssid=\"");
        len=strlen(pssid);
        pssid[len-2]=0;

        strcpy(buff_tempssid,pssid);

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"ssid <%s>\r\n",&buff_tempssid[0]);//temp
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    return(buff_tempssid);
}

void WifiClient::PrintObj()
{

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC= PrintObj\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC= *State :%d\r\n",State);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WFC= *ReqOn :%d\r\n",ReqOn);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
}




