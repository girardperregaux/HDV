#include "AccessPointWifi.h"
#include <stdlib.h>
#include <stdio.h>
#include <DefCom.h>
#include <qdebug.h>
#include <qfile.h>
#include <tstate.h>


#define ACCESS_POINT_IP "192.168.0.1"
#define NAME_FILE_UDHCPD_PID "/var/run/udhcpd.pid"

extern MCSerial SerialPrint;
extern char buff_print[];



AccessPointWifi::AccessPointWifi(QObject* parent)
    :QObject(parent)
{
    ProcessAP = new QProcess(this);
    ReqOn=false;
    IsOn=false;


}



void AccessPointWifi::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerT.Init((char*)"APTimer");
    pREGTIMER->RegisterTimer(&TimerT);
    TimerT.SetTimer(UN_SECONDO);

    TimerRun.setInterval(100);
    TimerRun.start();

    connect(&TimerRun, SIGNAL(timeout()), this, SLOT(Run()));

    fsmState=ACCESSPOINTWIFI_INIT;
}
void AccessPointWifi::On()
{

}






void AccessPointWifi::Run()
{
    QByteArray output;
    char str_tmp[MAX_LEN_SMS+1];
    char StrPid[MAX_LEN_CAMPO+1];
    QFile file(NAME_FILE_UDHCPD_PID);
    char *p;
    p=(char *)StrPid;




    switch(fsmState)
    {
        case ACCESSPOINTWIFI_INIT:
            fsmState=ACCESSPOINTWIFI_READY;
            IsOn=false;
            TimerT.SetTimer(5*UN_SECONDO);
        break;
        case ACCESSPOINTWIFI_READY:
            if(!TimerT.IsElapsed())break;


            if(ReqOn && !IsOn)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: CONFIGURAZIONE...\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                fsmState=ACCESSPOINTWIFI_SETUP_IP;
            }

        break;
        case ACCESSPOINTWIFI_SETUP_IP:

            if(!ReqOn)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: CONFIGURAZIONE...ANNULLATA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                fsmState=ACCESSPOINTWIFI_READY;
            }

            ProcessAP->start("sh");
            sprintf(str_tmp,"ifconfig wlan0 %s",ACCESS_POINT_IP);
            ProcessAP->write(str_tmp);
            ProcessAP->closeWriteChannel();
            Pid=ProcessAP->pid();
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: PID  %d\r\n",Pid);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            ProcessAP->waitForFinished();
            output = ProcessAP->readAll();
            ProcessAP->close();
            fsmState=ACCESSPOINTWIFI_SETUP_UDHCPD;
            TimerT.SetTimer(2*UN_SECONDO);
        break;


        case ACCESSPOINTWIFI_SETUP_UDHCPD:
            if(!ReqOn)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: CONFIGURAZIONE...ANNULLATA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                fsmState=ACCESSPOINTWIFI_READY;
            }

            if(!TimerT.IsElapsed())break;
            ProcessAP->start("udhcpd");
            ProcessAP->closeWriteChannel();
            ProcessAP->waitForFinished();
            output = ProcessAP->readAll();
            ProcessAP->close();

            fsmState=ACCESSPOINTWIFI_SETUP_AP;
            TimerT.SetTimer(2*UN_SECONDO);

        case ACCESSPOINTWIFI_SETUP_AP:

            if(!TimerT.IsElapsed())break;
            ProcessAP->start("sh");
            ProcessAP->write("iwpriv wlan0 AP_SET_CFG ASCII_CMD=AP_CFG,SSID=\"WIFI_REMHDV\",SEC=\"open\",KEY=\"\",CHANNEL=1,PREAMBLE=1,MAX_SCB=2,END");
            ProcessAP->closeWriteChannel();
            ProcessAP->waitForFinished();
            output = ProcessAP->readAll();
            ProcessAP->close();

            fsmState=ACCESSPOINTWIFI_SWITCHING_ON;
            TimerT.SetTimer(2*UN_SECONDO);


        case ACCESSPOINTWIFI_SWITCHING_ON:
            if(!TimerT.IsElapsed())break;
            ProcessAP->start("sh");
            ProcessAP->write("iwpriv wlan0 AP_BSS_START");
            ProcessAP->closeWriteChannel();
            ProcessAP->waitForFinished();
            output = ProcessAP->readAll();
            ProcessAP->close();

            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: %s\r\n",output.data());
            SerialPrint.Flush(PRINT_DEBUG_FUNC);



            fsmState=ACCESSPOINTWIFI_ON;
            TimerT.SetTimer(2*UN_SECONDO);

        case ACCESSPOINTWIFI_ON:

            if(!TimerT.IsElapsed())break;

            if(!IsOn)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: ATTIVATO!!!\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                IsOn=true;

            }

     //       output = ProcessAP->readAll();


            if(!ReqOn)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: DISATTIVAZIONE...\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                fsmState=ACCESSPOINTWIFI_SWITCHING_OFF;
                IsOn=false;
            }
            break;


        case ACCESSPOINTWIFI_SWITCHING_OFF:
            if(!TimerT.IsElapsed())break;
            ProcessAP->start("sh");
            ProcessAP->write("iwpriv wlan0 AP_BSS_STOP");
            ProcessAP->closeWriteChannel();
            ProcessAP->waitForFinished();
            output = ProcessAP->readAll();
            ProcessAP->close();

            if(file.exists())
            {
                if(file.open(QFile::ReadOnly))
                {
                    file.read(p,MAX_LEN_CAMPO);
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: PID LETTO %s\r\n",StrPid);
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }
                file.close();
                sprintf(str_tmp,"kill -9 %s",StrPid);

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: KILL %s\r\n",str_tmp);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                system(str_tmp);

            }

            fsmState=ACCESSPOINTWIFI_INIT;
            //TimerT.SetTimer(2*UN_SECONDO);
        break;


        default:
        break;
    }

//    if(strlen(output.data())>0)
//    {
//        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"AP: %s\r\n",output.data());
 //       SerialPrint.Flush();
 //   }
}



