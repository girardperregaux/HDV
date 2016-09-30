#include "mcmodem.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>
#include "HDV.h"
#include "tstate.h"
#include "config.h"
#include "mcserial.h"
#include "message.h"
#include "utility.h"

#define UREG_LTE 7
#define UDP_LINK 1


#define MAX_LEN_STRCOMMAND 300

#define NETWORK_SELECT_SINGLE_UMTS  2
#define NETWORK_SELECT_SINGLE_LTE   3
#define NETWORK_SELECT_DUAL_UMTS_LTE 6
#define NETWORK_SELECT_TRI_UMTS_LTE_GSM 4



#define COMMAND_NONE 0
#define COMMAND_NOP  1
#define COMMAND_AT   2
#define COMMAND_OK   3
#define COMMAND_CSQ  4
#define COMMAND_ATI  5
#define COMMAND_CREG2  6
#define COMMAND_CREG  7
#define COMMAND_COPS0 8
#define COMMAND_ATE0  9
#define COMMAND_CNMI 10
#define COMMAND_COPS2 11
#define COMMAND_UCGDFLT 12
#define COMMAND_NDISDUP 13
#define COMMAND_NDISSTATQRY 14
#define COMMAND_URAT 15
#define COMMAND_UCGATT 16
#define COMMAND_CGEREP 17
#define COMMAND_CGDCONT 18
#define COMMAND_CGACT 19
#define COMMAND_CGACT_0 20
#define COMMAND_SIGNAL_CGACT 21
#define COMMAND_SIGNAL_UREG 22
#define COMMAND_CFUN 23
#define COMMAND_CMGF 24
#define COMMAND_CPMS 25
#define COMMAND_CGEQREQ 26
#define COMMAND_CMGL 27
#define COMMAND_CMGR 28
#define COMMAND_CMGD 29
#define COMMAND_UIPTABLES 30
#define COMMAND_USOCR 31
#define COMMAND_UDNSR 32
#define COMMAND_USOST 33
#define COMMAND_CMGS 34
#define COMMAND_CTRLZ 35
#define COMMAND_IPINIT 36
#define COMMAND_IPOPEN 37
#define COMMAND_IPSEND 38
#define COMMAND_IPCLOSE 39
#define COMMAND_IPREQOPEN 40
#define COMMAND_IPLISTEN 41
#define COMMAND_SYSCFGEX 42
#define COMMAND_WAKEUPCFG 43
#define COMMAND_CGSN 44
#define COMMAND_MONSC 45


extern MCSerial SerialPrint;
extern char buff_print[];

extern TConfig Configuration;



TSMS ModemRxedSMS;
TSMS SMSInUscita;
TSMS SMSS;


MCModem::MCModem(QObject* parent)
    :QObject(parent)
{

    connect(&ModemT, SIGNAL(timeout()), this, SLOT(GestioneModem()));
    count = 0;
    input_index=0;
    GSMCSQ=0;
    ECMReq=false;
    ECMReqStream=false;
    ReqOn=false;
    ECMReqFtp=false;
    ECMReqFtpFast=false;
    GSMRegistrato=0;
    GSMFieldIntensity=0;
    GSMCSQ=0;
    CntCommandTimeOut=0;
    CntCommandError=0;
    cgactStatus=false;

    PDPContextDefined=false;
    PDPContextActivated=false;
    ureg=0;

    uBIS_link_id=0;
    bBIS_link_inited=false;
    uBIS_link_state=0;
    uBIS_link_opened=0;


    SMSRicevuto=false;
    SMSIndex=0;




}

void MCModem::Init(TimerMan *t)
{
    pREGTIMER=t;

    ModemTimer.Init((char*)"ModemTimer");
    ModemSignal.Init((char*)"ModemSignal");
    ModemDelay.Init((char*)"ModemDelay");
    RegControl.Init((char*)"RegControl");
    Dereg.Init((char*)"Dereg");




    pREGTIMER->RegisterTimer(&ModemTimer);
    pREGTIMER->RegisterTimer(&ModemSignal);
    pREGTIMER->RegisterTimer(&ModemDelay);
    pREGTIMER->RegisterTimer(&RegControl);
    pREGTIMER->RegisterTimer(&Dereg);





    ModemSignal.SetTimer(UN_SECONDO);
    ModemTimer.SetTimer(UN_SECONDO);
    ModemDelay.SetTimer(UN_SECONDO);


    ModemState=MODEMGSM_INIT;

    Command = COMMAND_NONE;
    CommandState = STATE_ATC_IDLE;

    strcpy(APNUpgrade,STR_NO);
    strcpy(APNStream,STR_NO);
    strcpy(APNRemLink,STR_NO);
    strcpy(APNFtp,STR_NO);
    strcpy(APNFtpFast,STR_NO);


}

void MCModem::Start(int time)
{
    ModemT.setInterval(time);
    ModemT.start();
    count = 0;
}




void MCModem::GestioneModem()
{

    quint8 result = 20;
    quint8 i;
    bool USBSerialConnected=false;
    char ActualAPN[MAX_LEN_CAMPO+1];

    char strCommand[MAX_LEN_STRCOMMAND+1];

    char  strPortName[MAX_LEN_CAMPO+1];
    char  strDescription[MAX_LEN_CAMPO+1];
    char  strManufacturer[MAX_LEN_CAMPO+1];

//    if(!ReqOn)
//    {
//        ModemState=MODEMGSM_OFF;
//        SetState(MODEM_ASSENTE,false);
//        SetState(REQ_MODEM_ON,false);
//    }
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        snprintf(&strPortName[0],MAX_LEN_CAMPO,"%s",qPrintable(info.portName()));
        snprintf(&strDescription[0],MAX_LEN_CAMPO,"%s",qPrintable(info.description()));
        snprintf(&strManufacturer[0],MAX_LEN_CAMPO,"%s",qPrintable(info.manufacturer()));

     //   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: strManufacturer %s\r\n",strManufacturer);
     //   SerialPrint.Flush(PRINT_DEBUG_ALL);

     //   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: strDescription %s\r\n",strDescription);
     //   SerialPrint.Flush(PRINT_DEBUG_ALL);


        if(strstr(strManufacturer,"Huawei Technologies") && strstr(strPortName,"ttyUSB2"))
        {
            SetState(MODEM_ASSENTE,false);
            USBSerialConnected=true;
            break;
        }
        else SetState(MODEM_ASSENTE,true);

    }

    strcpy(ActualAPN,STR_NO);

    if(!ECMReqUpgrade)strcpy(APNUpgrade,STR_NO);
    if(!ECMReqUpgradeSt)strcpy(APNUpgradeSt,STR_NO);
    if(!ECMReqStream)strcpy(APNStream,STR_NO);
    if(!ECMReqRemLink)strcpy(APNRemLink,STR_NO);
    if(!ECMReqFtp)strcpy(APNFtp,STR_NO);
    if(!ECMReqFtpFast)strcpy(APNFtpFast,STR_NO);
    if(!ECMReqDownload)strcpy(APNReqDownload,STR_NO);


    if(ECMReqRemLink)strcpy(ActualAPN,APNRemLink);
    else if(ECMReqStream)strcpy(ActualAPN,APNStream);
    else if(ECMReqUpgrade)strcpy(ActualAPN,APNUpgrade);
    else if(ECMReqUpgradeSt)strcpy(ActualAPN,APNUpgradeSt);
    else if(ECMReqFtp)strcpy(ActualAPN,APNFtp);
    else if(ECMReqFtpFast) strcpy(ActualAPN,APNFtpFast);
    else if(ECMReqDownload)strcpy(ActualAPN,APNReqDownload);


    ECMReq=(ECMReqUpgrade | ECMReqStream | ECMReqRemLink | ECMReqUpgradeSt | ECMReqFtp | ECMReqFtpFast| ECMReqDownload) ;

    if(ECMReq && (strcmp(ActualAPN,STR_NO)==0))ECMReq=false;

    if(ECMReq)SetWorkPeriod(ID_CPU,30,true);



    switch(ModemState)
    {
        case MODEMGSM_INIT:
            if(!GetState(ST_STATE_UPDATED))break;
            if(GetState(MODEM_ON))
            {

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: GIA' ACCESO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                system("echo 'on' > /sys/bus/usb/devices/1-1.1/power/control" );
                apnIsSet=false;
                ModemState=MODEMGSM_CHECK_SERIAL;
                ModemDelay.SetTimer((60*UN_SECONDO));
                break;
            }
            else  ModemState=MODEMGSM_OFF;

           ModemState=MODEMGSM_OFF;
           CntCheckUSB=0;

        break;


        case MODEMGSM_OFF:
            apnIsSet=false;
            GSMRegistrato=0;
            GSMFieldIntensity=0;
            GSMCSQ=0;
            cgactStatus=false;
            PDPContextDefined=false;
            PDPContextActivated=false;
            ureg=0;
            Command=COMMAND_NONE;
            CommandState=STATE_ATC_IDLE;
            CntCommandTimeOut=0;
            CntCommandError=0;
            uBIS_link_id=0;
            bBIS_link_inited=false;
            uBIS_link_state=0;
            uBIS_link_opened=0;
            if(!ReqOn)break;
            if(myserial.IsOpen())myserial.Close(MCSERIAL_ID_SERIAL_MODEM);
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: OFF\r\n");
            SerialPrint.Flush(PRINT_DEBUG_TEL);

            ModemDelay.SetTimer(10*UN_SECONDO);
            SetState(REQ_MODEM_ON,false);
            ModemState=MODEMGSM__WAITING_OFF;
        break;

        case MODEMGSM__WAITING_OFF:
            if(!ModemDelay.IsElapsed())break;
             ModemState=MODEMGSM_ON;
             apnIsSet=false;
        break;


        case MODEMGSM_ON:
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: ACCENSIONE\r\n");
            SerialPrint.Flush(PRINT_DEBUG_TEL);

            SetState(REQ_MODEM_ON,true);
            ModemState=MODEMGSM_WAITING_ON;
            ModemDelay.SetTimer(10*UN_SECONDO);
            apnIsSet=false;
        break;

        case MODEMGSM_WAITING_ON:
            if(GetState(MODEM_ON))
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: ACCESO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_TEL);
                ModemState=MODEMGSM_CHECK_SERIAL;
                ModemDelay.SetTimer((60*UN_SECONDO));
            }
            if(ModemDelay.IsElapsed())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: ACCENSIONE KO!!!\r\n");
                SerialPrint.Flush(PRINT_DEBUG_TEL);
                ModemState=MODEMGSM_OFF;
            }
            apnIsSet=false;
        break;
        case MODEMGSM_CHECK_SERIAL:


            if(USBSerialConnected)
            {
                Command=COMMAND_NONE;
                CommandState=STATE_ATC_IDLE;

                ModemState=MODEMGSM_AT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: SERIALE USB RILEVATA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: INIT\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                ModemDelay.SetTimer(0);
                ModemDelay.SetTimer(UN_SECONDO*10);
                myserial.Init(MCSERIAL_ID_SERIAL_MODEM);
                myserial.Open(MCSERIAL_ID_SERIAL_MODEM);
                CntCheckUSB=0;
                break;
            }

            else if(ModemDelay.IsElapsed())
            {

                if(myserial.IsOpen())myserial.Close(MCSERIAL_ID_SERIAL_MODEM);

                ModemDelay.SetTimer(UN_SECONDO*5);
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: MODEM NON PRESENTE !!!\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                ModemState=MODEMGSM_OFF;
                CntCheckUSB++;
//snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"STE: ELIMINARE CntCheckUSB=0!!!!!!!!!!\r\n");
//SerialPrint.Flush(PRINT_DEBUG_ALL);

//CntCheckUSB=0;
                if(CntCheckUSB>3)
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: MODEM RESET OPEN USB!!!\r\n");

                    SerialPrint.Flush(PRINT_DEBUG_ALL);
                    SetState(RESET_RITARDATO,true);
                }
            }
            apnIsSet=false;
        break;

        case MODEMGSM_AT:
            apnIsSet=false;
            if(!ModemDelay.IsElapsed())break;


            result=AtCommand((char*)"AT\r",4);

            if(result==STATE_ATC_OK)
            {
                //SetState(MODEM_ASSENTE,false);
                ModemState=MODEMGSM_ATE;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }

        break;

        case MODEMGSM_ATE:
            if(!ModemDelay.IsElapsed())break;

            result=AtCommand((char*)"ATE0\r",10);
            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_ATI;
                ModemDelay.SetTimer(UN_SECONDO);
            }

        break;


        case MODEMGSM_ATI:
            if(!ModemDelay.IsElapsed())break;
            result=AtCommand((char*)"ATI\r",10);
 //           if(Configuration.Config.network_select==2)
//            {
//                result=AtCommand("AT^SYSCFGEX=\"02\",3FFFFFFF,0,2,7FFFFFFFFFFFFFFF,,\r",10);
//            }
//            else if(Configuration.Config.network_select==3)
//            {
//                result=AtCommand("AT^SYSCFGEX=\"03\",3FFFFFFF,0,2,7FFFFFFFFFFFFFFF,,\r",10);
//
//            }
//            else
//            {
//                result=AtCommand("AT^SYSCFGEX=\"030201\",3FFFFFFF,0,2,7FFFFFFFFFFFFFFF,,\r",10);
//            }


            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_INIT_CGSN;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }
        break;

        case MODEMGSM_INIT_CGSN:

            if(!ModemDelay.IsElapsed())break;
            result=AtCommand((char*)"AT+CGSN\r",10);

            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_SYSCFGEX;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }
        break;
        case MODEMGSM_SYSCFGEX:
            if(!ModemDelay.IsElapsed())break;

             network_select=Configuration.Config.network_select;

            if(Configuration.Config.network_select==2)
            {
                result=AtCommand((char*)"AT^SYSCFGEX=\"02\",3FFFFFFF,0,2,7FFFFFFFFFFFFFFF,,\r",10);
            }
            else if(Configuration.Config.network_select==3)
            {
                result=AtCommand((char*)"AT^SYSCFGEX=\"03\",3FFFFFFF,0,2,7FFFFFFFFFFFFFFF,,\r",10);

            }
            else
            {
                result=AtCommand((char*)"AT^SYSCFGEX=\"030201\",3FFFFFFF,0,2,7FFFFFFFFFFFFFFF,,\r",10);
            }


            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_WAKEUPCFG;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }

        break;

        case MODEMGSM_WAKEUPCFG:
            if(!ModemDelay.IsElapsed())break;

                result=AtCommand((char*)"AT^WAKEUPCFG=1,3,7\r",10);


            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_INIT_CREG;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }

        break;



        case MODEMGSM_INIT_CREG:
             //TODO S
            if(!ModemDelay.IsElapsed())break;
            result= AtCommand((char*)"AT+CREG=2\r",30);

            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_INIT_CMGF;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }
        break;

        case MODEMGSM_INIT_CMGF:
            //TODO S
           if(!ModemDelay.IsElapsed())break;
           result= AtCommand((char*)"AT+CMGF=1\r",10);
           if(result==STATE_ATC_OK)
           {
             ModemState=MODEMGSM_INIT_CPMS;
             ModemDelay.SetTimer(UN_SECONDO*1);
           }
        break;


        case MODEMGSM_INIT_CPMS:
            //TODO S
            if(!ModemDelay.IsElapsed())break;

            result= AtCommand((char*)"AT+CPMS=\"SM\",\"SM\",\"SM\"\r",10); //Only "SM" message storage is supported.
            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_INIT_CNMI;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }
        break;

        case MODEMGSM_INIT_CNMI:
            if(!ModemDelay.IsElapsed())break;
            result= AtCommand((char*)"AT+CNMI=2,1,0,0,0\r",90);
            if(result==STATE_ATC_OK)
            {
              ModemState=MODEMGSM_COPS0;
              ModemDelay.SetTimer(UN_SECONDO*1);
            }
        break;


        case MODEMGSM_COPS0:
            if(!ModemDelay.IsElapsed())break;
            apnIsSet=false;
            result= AtCommand((char*)"AT+COPS=0\r",60);
            if(result==STATE_ATC_OK)
            {
              ModemState=MODEMGSM_READY;
              ModemDelay.SetTimer(UN_SECONDO*1);
              RegControl.SetTimer(5*UN_MINUTO);
              Dereg.SetTimer(10*UN_MINUTO);
            }
            if(result==STATE_ATC_ERROR)
            {
                ModemState=MODEMGSM_READY;
                ModemDelay.SetTimer(UN_SECONDO*1);
                RegControl.SetTimer(5*UN_MINUTO);
                Dereg.SetTimer(10*UN_MINUTO);
            }

        break;
        case MODEMGSM_COPS2:
            if(!ModemDelay.IsElapsed())break;
            ModemState=MODEMGSM_OFF;
            apnIsSet=false;

//            result= AtCommand("AT+COPS=2\r",30);
//            if(result==STATE_ATC_OK)
//            {
//              ModemState=MODEMGSM_COPS0;
//              ModemDelay.SetTimer(UN_SECONDO*5);
//            }
//            if(result==STATE_ATC_ERROR)
//            {
//                ModemState=MODEMGSM_COPS0;
//                ModemDelay.SetTimer(UN_SECONDO*5);
//            }


        break;




        case MODEMGSM_NDISDUP:
            if(!ModemDelay.IsElapsed())break;

            //snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^NDISDUP=1,1,\"%s\"\r",Configuration.Config.tipo_centrale1);
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^NDISDUP=1,1,\"%s\"\r",ActualAPN);
            result=AtCommand(strCommand,10);

            if(result==STATE_ATC_OK)
            {
              apnIsSet=true;
              ModemState=MODEMGSM_READY;
              ModemDelay.SetTimer(UN_SECONDO*5);
            }
        break;


        case MODEMGSM_NDISDUP_OFF:
            if(!ModemDelay.IsElapsed())break;
            system("ifdown usb0");
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^NDISDUP=1,0\r");
            result=AtCommand(strCommand,10);

            if(result==STATE_ATC_OK)
            {
              apnIsSet=false;
              SetState(MODEM_CONNECTED,false);
              ModemState=MODEMGSM_READY;
              ModemDelay.SetTimer(UN_SECONDO*1);
            }
            if(result==STATE_ATC_ERROR)
            {
                apnIsSet=false;
                SetState(MODEM_CONNECTED,false);
                ModemState=MODEMGSM_READY;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }
        break;

        case MODEMGSM_DELAY_CONNECTED:
            if(!ModemDelay.IsElapsed())break;
            ModemState=MODEMGSM_READY;
            SetState(MODEM_CONNECTED,true);

        break;
        case  MODEMGSM_WAIT_OFF:
            if(!ModemDelay.IsElapsed())break;
            ModemState=MODEMGSM_OFF;
            SetState(MODEM_ASSENTE,false);
            //SetState(REQ_MODEM_ON,false);
        break;

        case MODEMGSM_READY:

            if(!ReqOn)
            {
                ModemState=MODEMGSM_WAIT_OFF;
                ModemDelay.SetTimer(UN_SECONDO*10);
                //SetState(MODEM_ASSENTE,false);
                //SetState(REQ_MODEM_ON,false);
                break;
            }
            if(network_select!=Configuration.Config.network_select && !ECMReq && !SMSDaInviare)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: CAMBIO NETWORK\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                ModemState=MODEMGSM_COPS2;
                break;

            }

            if(ECMReq || UDPReq)
            {
              Dereg.SetTimer(10*UN_MINUTO);
            }
            if(Dereg.IsElapsed())
            {

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: DEREG PERIODICA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                ModemState=MODEMGSM_COPS2;
                break;

            }
            if(GSMRegistrato)RegControl.SetTimer(5*UN_MINUTO);

            if(RegControl.IsElapsed())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: CONTROLLO REG\r\n");
                SerialPrint.Flush(PRINT_DEBUG_ALL);
                ModemState=MODEMGSM_COPS2;
                break;

            }

            if(!GSMRegistrato || !uBIS_link_opened)UDPSend=false;

            if(uBIS_link_opened && UDPSend)
            {
                ModemState=MODEMGSM_IPSEND;
                UDPSend=false;
                break;
            }

            if(SMSDaInviare && GSMRegistrato)
            {
                ModemState=MODEMGSM_CMGS;
                break;
            }

            if(!UDPReq &&
               bBIS_link_inited &&
               uBIS_link_opened
                    )
            {
                ModemState=MODEMGSM_IPCLOSE;
                break;
            }

            if(UDPReq &&
               GSMRegistrato &&
               bBIS_link_inited &&
               !uBIS_link_opened
                    )
            {
                ModemState=MODEMGSM_IPOPEN;
                break;
            }


            if(UDPReq &&
               GSMRegistrato &&
               !bBIS_link_inited )
            {
                ModemState=MODEMGSM_IPINIT;
                break;
            }



            if((!ECMReq) && apnIsSet)
            {
                ModemState=MODEMGSM_NDISDUP_OFF;

            }
            if((ECMReq) && !apnIsSet)
            {
                ModemState=MODEMGSM_NDISDUP;
            }

            if(ModemSignal.IsElapsed())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: CONTROLLO SEGNALI\r\n");
                SerialPrint.Flush(PRINT_DEBUG_TEL);
                ModemState=MODEMGSM_SIGNAL_CSQ;
                break;
            }


        break;

        case MODEMGSM_SIGNAL_CSQ:


            if(!ModemDelay.IsElapsed())break;


            result=AtCommand((char*)"AT+CSQ\r",30);

            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_SIGNAL_CREG;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }

        break;
        case MODEMGSM_SIGNAL_CREG:
            if(!ModemDelay.IsElapsed())break;
            result=AtCommand((char*)"AT+CREG?\r",30);

            if(result==STATE_ATC_OK)
            {


                //if(PDPContextActivated)ModemState=MODEMGSM_SIGNAL_CGDCONT;
                //else
                ModemDelay.SetTimer(UN_SECONDO*1);

               //if(GSMRegistrato)ModemState=MODEMGSM_SIGNAL_MONSC;
               //else ModemState=MODEMGSM_NDISSTATQRY;
                ModemState=MODEMGSM_NDISSTATQRY;
                ModemSignal.SetTimer(UN_SECONDO*5);
            }
        break;
    case MODEMGSM_SIGNAL_MONSC:
        if(!ModemDelay.IsElapsed())break;
        result=AtCommand((char*)"AT^MONSC\r",30);

        if(result==STATE_ATC_OK)
        {

            ModemDelay.SetTimer(UN_SECONDO*1);

            ModemState=MODEMGSM_NDISSTATQRY;
            ModemSignal.SetTimer(UN_SECONDO*5);
        }
    break;
            //QUI

        case MODEMGSM_NDISSTATQRY:
            if(!ModemDelay.IsElapsed())break;
            result=AtCommand((char*)"AT^NDISSTATQRY?\r",30);

            if(result==STATE_ATC_OK)
            {

                //if(PDPContextActivated)ModemState=MODEMGSM_SIGNAL_CGDCONT;
                //else
                ModemDelay.SetTimer(UN_SECONDO*1);

                ModemState=MODEMGSM_SIGNAL_CMGL;
                ModemSignal.SetTimer(UN_SECONDO*5);
            }
        break;






        case MODEMGSM_SIGNAL_CMGL:
            if(!ModemDelay.IsElapsed())break;
            result=AtCommand((char*)"AT+CMGL\r",60);
            if(result==STATE_ATC_OK)
            {
                ModemDelay.SetTimer(UN_SECONDO*1);

                if(SMSRicevuto)
                {
                   ModemState=MODEMGSM_CMGR;
                }
                else
                {
                    ModemState=MODEMGSM_IPREQOPEN;
                    ModemSignal.SetTimer(UN_SECONDO*5);
                   // ModemState=MODEMGSM_READY;

                }

            }
        break;
        case MODEMGSM_CMGR:
            if(!ModemDelay.IsElapsed())break;
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT+CMGR=%d\r",SMSIndex);
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_CMGD;
                SMSRicevuto=false;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }
            else if(result==STATE_ATC_ERROR)
            {
                ModemState=MODEMGSM_READY;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }

        break;

        case MODEMGSM_CMGD:
            if(!ModemDelay.IsElapsed())break;
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT+CMGD=%d,4\r",SMSIndex);
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                 ModemState=MODEMGSM_IPREQOPEN;
                // ModemSignal.SetTimer(UN_SECONDO*5);
                 ModemDelay.SetTimer(UN_SECONDO*1);

            }
            else if(result==STATE_ATC_ERROR)
            {
                ModemState=MODEMGSM_READY;
                ModemDelay.SetTimer(UN_SECONDO*1);
            }



        break;




        case MODEMGSM_CMGS:
            if(!ModemDelay.IsElapsed())break;
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT+CMGS=\"%s\"\r",SMSInUscita.ConnectedNum);
            result=AtCommand(strCommand,10);

            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_SEND_SMSDATA;
            }
            else if(result==STATE_ATC_TOUT)
            {
                ModemState=MODEMGSM_SEND_SMSDATA;
            }
            else if(result==STATE_ATC_ERROR)
            {
                ModemState=MODEMGSM_READY;
            }
        break;





        case MODEMGSM_SEND_SMSDATA:

            for(i=0;i<strlen(SMSInUscita.testo);i++)
            {
                myserial.SendChar(SMSInUscita.testo[i]);
            }
            ModemState=MODEMGSM_SEND_CTRLZ;
        break;
        case MODEMGSM_SEND_CTRLZ:


            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"%c",CTRLZ);
            result=AtCommand(strCommand,3);


            if(result==STATE_ATC_OK)
            {

              SMSDaInviare=false;
              ModemState=MODEMGSM_READY;
            }
            else if(result==STATE_ATC_TOUT)
            {
              SMSDaInviare=false;
              ModemState=MODEMGSM_READY;
            }
            else if(result==STATE_ATC_ERROR)
            {
              SMSDaInviare=false;
              ModemState=MODEMGSM_READY;
            }
        break;

        case MODEMGSM_IPINIT:
            if(!ModemDelay.IsElapsed())break;
            //snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^IPINIT=\"web.omnitel.it\"\r");
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^IPINIT=\"%s\"\r",Configuration.Config.tipo_centrale1);
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                bBIS_link_inited=true;
                ModemState=MODEMGSM_READY;
                ModemDelay.SetTimer(UN_SECONDO*5);
            }

        break;
        case MODEMGSM_IPOPEN:
            if(!ModemDelay.IsElapsed())break;
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^IPOPEN=%d,\"UDP\",\"webmc.it\",8002\r",UDP_LINK);
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                ModemDelay.SetTimer(UN_SECONDO*1);
                ModemState=MODEMGSM_IPLINSTEN;
                uBIS_link_opened=true;
            }
        break;
        case MODEMGSM_IPLINSTEN:
            if(!ModemDelay.IsElapsed())break;
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^IPLISTEN=\"UDP\",10000\r");
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_IPREQOPEN;
                ModemDelay.SetTimer(UN_SECONDO*1);

            }
        break;

        case MODEMGSM_IPREQOPEN:
            if(!ModemDelay.IsElapsed())break;
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^IPOPEN?\r");
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_READY;
                ModemDelay.SetTimer(UN_SECONDO*5);

            }
        break;

        case MODEMGSM_IPSEND:
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^IPSEND=%d,\"%s\"\r",UDP_LINK,"1R8210307023232051SN040320141507530000000N00000000E0015100000010040320141508400000000000002042000000000001A6900001F65");
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                ModemState=MODEMGSM_READY;
                //ModemDelay.SetTimer(UN_SECONDO*5);
            }

        break;

        case MODEMGSM_IPCLOSE:
            snprintf(&strCommand[0],MAX_LEN_STRCOMMAND,"AT^IPCLOSE=%d\r",UDP_LINK);
            result=AtCommand(strCommand,10);
            if(result==STATE_ATC_OK)
            {
                bBIS_link_inited=false;
                uBIS_link_opened=false;
                ModemState=MODEMGSM_READY;
                ModemDelay.SetTimer(UN_SECONDO*5);
            }




        break;


        default:
        ModemState=MODEMGSM_INIT;
        break;
    }

    if(result==STATE_ATC_OK)
    {
       CntCommandTimeOut=0;
       CntCommandError=0;
    }
    if(result==STATE_ATC_TOUT)
    {
      CntCommandTimeOut++;
      ModemDelay.SetTimer(UN_SECONDO*5);
      if(CntCommandTimeOut >= 3)
      {
          snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: MODEM OFF PER CNT TOUT\r\n");
          SerialPrint.Flush(PRINT_DEBUG_TEL);

          ModemState=MODEMGSM_OFF;
          CntCommandTimeOut=0;
      }

    }
    if(result==STATE_ATC_ERROR)
    {

      ModemDelay.SetTimer(UN_SECONDO*10);
      CntCommandError++;
      if(CntCommandError >= 3)
      {
          snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: MODEM OFF PER ERROR\r\n");
          SerialPrint.Flush(PRINT_DEBUG_TEL);

          ModemState=MODEMGSM_OFF;

      }

    }



}


quint8  MCModem::AtCommand(char *AtCmdString,int Timeout)
{
    unsigned int idx;

    AtNotification(); //prova

    if(strcmp(AtCmdString,STR_ATC_RESET)==0)
    {
      Command=COMMAND_NONE;
      CommandState=STATE_ATC_IDLE;
      return(STATE_ATC_OK);

    }
    //printf("command=%d , command_id %d\r\n",Command,GetCommandId(AtCmdString));



    // restituisco lo  stato del comando in esecuzione
    if(Command==GetCommandId(AtCmdString))
    {

       if(CommandState==STATE_ATC_WAIT)
       {
          return(STATE_ATC_WAIT);
       }
       if(CommandState==STATE_ATC_TOUT)
       {
          Command=COMMAND_NONE;
          CommandState=STATE_ATC_IDLE;
          return(STATE_ATC_TOUT);
       }
       if(CommandState==STATE_ATC_OK)
       {
          Command=COMMAND_NONE;
          CommandState=STATE_ATC_IDLE;
          return(STATE_ATC_OK);
       }
       if(CommandState==STATE_ATC_ERROR)
       {
          Command=COMMAND_NONE;
          CommandState=STATE_ATC_IDLE;
          return(STATE_ATC_ERROR);
       }
    }

  //  AtNotification(); //prova


    // Se non ho inviato nessun comando e  lo stato di rx è in idle. allora posso inviare un comando al modem
    if(Command==COMMAND_NONE && CommandState==STATE_ATC_IDLE)
    {
        Command=GetCommandId(AtCmdString);
       // Cambio stato
        CommandState=STATE_ATC_WAIT;
        ModemTimer.SetTimer(UN_SECONDO*Timeout);

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM[Tx]:%s\r\n",AtCmdString);
        SerialPrint.Flush(PRINT_DEBUG_TEL);

        for(idx=0; idx<strlen(AtCmdString);idx++)
            myserial.SendChar(AtCmdString[idx]);

        return(STATE_ATC_WAIT);
      }

      return(STATE_ATC_ERROR);



}

void MCModem::AtNotification()
{

    char c;
    char *p;

    while(myserial.GetChar(&c))
    {
        if(input_index<MAX_LEN_BUFFER_MODEM-1)
        {
            buffer[input_index++]=c;
            buffer[input_index]=0;
        }
        else
        {
            input_index=0;
            buffer[input_index]=0;
        }
    }

    if(input_index>0)
    {
        if(NewLine())
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM:%s\r\n",buffer);
            SerialPrint.Flush(PRINT_DEBUG_TEL);


            if((p=strstr(buffer, "+CMTI: ")))
            {
                //todo
            }

            if((p=strstr(buffer, "^IPSTATE: ")))
            {
                //<CR><LF>^IPSTATE: <link_id>,<state>,<errcode><CR><LF>
                p+=strlen("^IPSTATE: ");
                //link_id=atoi(p);
               // p += strcspn(p,",");
               // link_state=atoi(p);
//snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: %d %d\r\n",link_id,link_state);
//SerialPrint.Flush(PRINT_DEBUG_TEL);


            }

            if((p=strstr(buffer, "^NDISSTAT:0")))
            {
snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: IPV4 DISCONNECTED\r\n");
SerialPrint.Flush(PRINT_DEBUG_TEL);
            IPV4connected=false;


            }

            if((p=strstr(buffer, "^NDISSTAT:1")))
            {
snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: IPV4 CONNECTED\r\n");
SerialPrint.Flush(PRINT_DEBUG_TEL);
            IPV4connected=true;


            }







        }
    }

    // Se non sono in attesa di un risposta dal modem cancello il buffer
    if(CommandState!=STATE_ATC_WAIT)
    {
      if(input_index>0)
      {
          input_index=0;
          buffer[input_index]=0;
      }
    }

    if(CommandState==STATE_ATC_WAIT)
    {
        if(ModemTimer.IsElapsed())
        {
            input_index=0;
            buffer[input_index]=0;
            CommandState=STATE_ATC_TOUT;
            return;
        }

        if (strstr(buffer,"ERROR"))
        {
            input_index=0;
            buffer[input_index]=0;
            CommandState=STATE_ATC_ERROR;
            return;
        }

            switch (Command)
            {
                case COMMAND_AT:
                case COMMAND_ATI:
                case COMMAND_ATE0:
                case COMMAND_CSQ:
                case COMMAND_CREG2:
                case COMMAND_CREG:
                case COMMAND_COPS0:
                case COMMAND_CNMI:
                case COMMAND_COPS2:
                case COMMAND_UCGDFLT:
                case COMMAND_NDISDUP:
                case COMMAND_NDISSTATQRY:
                case COMMAND_URAT :
                case COMMAND_UCGATT :
                case COMMAND_CGEREP :
                case COMMAND_CGDCONT:
                case COMMAND_CGACT:
                case COMMAND_CGACT_0:
                case COMMAND_SIGNAL_CGACT:
                case COMMAND_SIGNAL_UREG:
                case COMMAND_CFUN:
                case COMMAND_CMGF:
                case COMMAND_CMGL:
                case COMMAND_CMGR:
                case COMMAND_CMGD:
                case COMMAND_CPMS:
                case COMMAND_CGEQREQ:
                case COMMAND_UIPTABLES:
                case COMMAND_USOCR:
                case COMMAND_UDNSR:
                case COMMAND_USOST:
                case COMMAND_CTRLZ:
                case COMMAND_IPINIT:
                case COMMAND_IPOPEN:
                case COMMAND_IPREQOPEN:
                case COMMAND_IPCLOSE:
                case COMMAND_IPSEND:
                case COMMAND_IPLISTEN:
                case COMMAND_SYSCFGEX:
                case COMMAND_WAKEUPCFG:
                case COMMAND_CGSN:
                case COMMAND_MONSC:





                    if (strstr(buffer, "\nOK\r\n"))
                    {
                        input_index=0;  // come vecchio modem

                        switch (Command)
                        {
                            case COMMAND_AT:
                            case COMMAND_CREG2:
                            case COMMAND_ATE0:
                            case COMMAND_ATI:
                            case COMMAND_CNMI:
                            case COMMAND_COPS2:
                            case COMMAND_UCGDFLT:
                            case COMMAND_NDISDUP:

                            case COMMAND_URAT :
                            case COMMAND_UCGATT :
                            case COMMAND_CGEREP :
                            case COMMAND_CGDCONT:
                            case COMMAND_CGACT:
                            case COMMAND_CGACT_0:
                            case COMMAND_CFUN:
                            case COMMAND_CMGF:
                            case COMMAND_CPMS:
                            case COMMAND_CGEQREQ:
                            case COMMAND_UIPTABLES:
                            case COMMAND_USOCR:
                            case COMMAND_UDNSR:
                            case COMMAND_USOST:
                            case COMMAND_CTRLZ:
                            case COMMAND_CMGD:
                            case COMMAND_IPINIT:
                            case COMMAND_IPOPEN:
                            case COMMAND_IPCLOSE:
                            case COMMAND_IPSEND:
                            case COMMAND_IPLISTEN:
                            case COMMAND_SYSCFGEX:
                            case COMMAND_WAKEUPCFG:


                                CommandState=STATE_ATC_OK;
                            break;

                            case COMMAND_NDISSTATQRY:
                                NDISSTATQRY();
                                CommandState=STATE_ATC_OK;
                            break;
                            case COMMAND_IPREQOPEN:
                                IPOPEN();
                                CommandState=STATE_ATC_OK;

                            break;
                            case COMMAND_CMGL:
                                CMGL();
                                CommandState=STATE_ATC_OK;
                            break;
                            case COMMAND_CMGR:
                                if(CMGR())CommandState=STATE_ATC_OK;
                            break;
                            case COMMAND_MONSC:
                                if(MONSC())CommandState=STATE_ATC_OK;
                            break;
                            case COMMAND_SIGNAL_UREG:
                                if(UREG())CommandState=STATE_ATC_OK;
                            break;

                            case COMMAND_CREG: if (CREG()) CommandState=STATE_ATC_OK;
                            break;
                            case COMMAND_CSQ:   if (CSQ()) CommandState=STATE_ATC_OK;
                            break;
                            case COMMAND_COPS0:
                                CommandState=STATE_ATC_OK;
                                GSMCSQ=0;
                                GSMFieldIntensity=0;
                            break;
                            case COMMAND_CGSN:
                                if(CGSN())CommandState=STATE_ATC_OK;
                            break;
                        }

                        input_index=0;
                        buffer[input_index]=0;
                }
                break;
                case COMMAND_CMGS:

                    if (strstr(&buffer[0], ">"))
                    {
                        CommandState=STATE_ATC_OK;
                        input_index=0;
                        buffer[input_index]=0;
                    }
                break;

        }
    }

}






int MCModem::GetCommandId(char * command)
{

    if (strcasecmp(command, "NOP\r")==0) return(COMMAND_NOP);
    if (strcasecmp(command, "AT\r")==0)  return(COMMAND_AT);
    if (strcasecmp(command, "ATE0\r")==0)  return(COMMAND_ATE0);
    if (strcasecmp(command, "AT+CSQ\r")==0)  return(COMMAND_CSQ);
    if (strcasecmp(command, "ATI\r")==0)  return(COMMAND_ATI);
    if (strncasecmp(command, "AT^WAKEUPCFG",strlen("AT^WAKEUPCFG"))==0)  return(COMMAND_WAKEUPCFG);

    if (strncasecmp(command, "AT+CGSN\r",strlen("AT+CGSN"))==0)  return(COMMAND_CGSN);

    if (strncasecmp(command, "AT^SYSCFGEX",strlen("AT^SYSCFGEX"))==0)  return(COMMAND_SYSCFGEX);
    if (strcasecmp(command, "AT+CREG=2\r")==0)  return(COMMAND_CREG2);
    if (strcasecmp(command, "AT+CREG?\r")==0)  return(COMMAND_CREG);
    if (strcasecmp(command, "AT+COPS=0\r")==0)  return(COMMAND_COPS0);
    if (strncasecmp(command, "AT+CNMI",strlen("AT+CNMI"))==0)  return(COMMAND_CNMI);
    if (strcasecmp(command, "AT+COPS=2\r")==0)  return(COMMAND_COPS2);
    if (strncasecmp(command, "AT+UCGDFLT=",strlen("AT+UCGDFLT="))==0)  return(COMMAND_UCGDFLT);
    if (strncasecmp(command, "AT+URAT=",strlen("AT+URAT="))==0)  return(COMMAND_URAT);
    if (strncasecmp(command, "AT+UREG?",strlen("AT+UREG?"))==0)  return(COMMAND_SIGNAL_UREG);
    if (strncasecmp(command, "AT+UCGATT=",strlen("AT+UCGATT="))==0)  return(COMMAND_UCGATT);
    if (strncasecmp(command, "AT+CGEREP=",strlen("AT+CGEREP="))==0)  return(COMMAND_CGEREP);
    if (strncasecmp(command, "AT+CGACT=",strlen("AT+CGACT="))==0)  return(COMMAND_CGACT);
    if (strncasecmp(command, "AT+CGDCONT=",strlen("AT+CGDCONT="))==0)  return(COMMAND_CGDCONT);
    if (strncasecmp(command, "AT+CGDCONT?",strlen("AT+CGDCONT?"))==0)  return(COMMAND_CGDCONT);

    if (strncasecmp(command, "AT+CFUN=",strlen("AT+CFUN="))==0)  return(COMMAND_CFUN);
    if (strncasecmp(command, "AT+CMGF=",strlen("AT+CMGF="))==0)  return(COMMAND_CMGF);
    if (strncasecmp(command, "AT+CPMS=",strlen("AT+CPMS="))==0)  return(COMMAND_CPMS);
    if (strcasecmp(command, "AT+CMGL\r")==0)  return(COMMAND_CMGL);
    if (strncasecmp(command, "AT+CMGR",strlen("AT+CMGR"))==0)  return(COMMAND_CMGR);
    if (strncasecmp(command, "AT+CMGD",strlen("AT+CMGD"))==0)  return(COMMAND_CMGD);
    if (strncasecmp(command, "AT+CGEQREQ?",strlen("AT+CGEQREQ?"))==0)  return(COMMAND_CGEQREQ);
    if (strncasecmp(command, "AT+UIPTABLES",strlen("AT+UIPTABLES"))==0)  return(COMMAND_UIPTABLES);
    if (strncasecmp(command, "AT+USOCR",strlen("AT+USOCR"))==0)  return(COMMAND_USOCR);
    if (strncasecmp(command, "AT+UDNSRN",strlen("AT+UDNSRN"))==0)  return(COMMAND_UDNSR);
    if (strncasecmp(command, "AT+USOST",strlen("AT+USOST"))==0)  return(COMMAND_USOST);
    if (strncasecmp(command, "AT+CMGS=",strlen("AT+CMGS="))==0)  return(COMMAND_CMGS);
    if (strncasecmp(command, "AT^NDISDUP=1",strlen("AT^NDISDUP=1"))==0)  return(COMMAND_NDISDUP);
    if (strncasecmp(command, "AT^NDISSTATQRY?",strlen("AT^NDISSTATQRY?"))==0)  return(COMMAND_NDISSTATQRY);

    if (strncasecmp(command, "AT^IPINIT=",strlen("AT^IPINIT="))==0)  return(COMMAND_IPINIT);
    if (strncasecmp(command, "AT^IPOPEN=",strlen("AT^IPOPEN="))==0)  return(COMMAND_IPOPEN);
    if (strncasecmp(command, "AT^IPOPEN?",strlen("AT^IPOPEN?"))==0)  return(COMMAND_IPREQOPEN);
    if (strncasecmp(command, "AT^IPSEND=",strlen("AT^IPSEND="))==0)  return(COMMAND_IPSEND);
    if (strncasecmp(command, "AT^IPCLOSE=",strlen("AT^IPCLOSE="))==0)  return(COMMAND_IPCLOSE);
    if (strncasecmp(command, "AT^IPLISTEN=",strlen("AT^IPLISTEN="))==0)  return(COMMAND_IPLISTEN);


    if (strncasecmp(command, "AT^MONSC\r",strlen("AT^MONSC\r"))==0)  return(COMMAND_MONSC);



    if (strchr(command, CTRLZ))return(COMMAND_CTRLZ);


}

bool MCModem::NewLine()
{
  if (strstr(buffer,"\r\n"))return(true);
  else return(false);
}



bool MCModem::CMGL()
{
    char *p;

    if ((p = (char *)strstr(&buffer[0], "+CMGL: ")))
    {        
       // if(strstr(&buffer[0], "REC UNREAD"))
       // {
            p+=strlen("+CMGL: ");
            SMSRicevuto=true;
            SMSIndex=atoi(p);
        //}
    return(true);
    }
    else return(false);


}

bool MCModem::CGSN()
{
    char *str;
    if ((str = (char *)strstr(&buffer[0], "\r\n")))
    {
        str+=strlen("\r\n");
        str[IMEI]=0;

        sprintf(&ImeiString[0],str);
        return (true);
    }
    return (false);

}


bool MCModem::CMGR()
{
////////////////
    char *str=(char *)strstr(&buffer[0],"+CMGR: ")+strlen("+CMGR: ");
    quint8 index;
    char testo[MAX_LEN_SMS+1];
    char numero[ATFIELD_LEN+1];

    memset(&numero[0], 0, sizeof(numero));
    memset(&testo[0], 0, sizeof(testo));

    SMSCreate((TSMS *)&ModemRxedSMS);

    //if(strstr(&buffer[0],"REC UNREAD")==0)
   // {
        SMSDaEliminare=true;
        SMSIndexDaEliminare=SMSIndex;
       // return(true);
   // }


    str += strspn(str," \"");              // elimino eventuali spazi preliminari e doppi apici

    // stato
    index = strcspn(str,",\"");            // lettura campo
    str += index;                          // passo al campo sussessivo
    str += strspn(str,",\"");              //

    // num
    index = strcspn(str,",\"");            // lettura campo

    strncpy(numero,str,mcmin(index,ATFIELD_LEN));

     if(IsTelephoneNumber(numero))
     {
       strncpy(ModemRxedSMS.ConnectedNum,numero,ATFIELD_LEN);

       str += index;                          // passo al campo sussessivo
       str += mcmin(strspn(str,",\""),3);       //

       // nome
       index = strcspn(str,",");              // lettura campo
       str += index;                          // passo al campo sussessivo
       str += strspn(str,",\"");              //

       // ora
       index = strcspn(str,"\"\r\n");         // lettura campo
       str += index;                          // passo al campo sussessivo
       str += strspn(str,"\"\r\n");           //

       // testo
       index = strcspn(str,"\r\n");           // lettura campo
       strncpy(&testo[0],str,mcmin(index,MAX_LEN_SMS));

       testo[mcmin(index,MAX_LEN_SMS)]=0;

//       if(IsSMSMcStr(&testo[0]))
//       {
         strncpy(&ModemRxedSMS.testo[0],testo,MAX_LEN_SMS);
         ModemRxedSMS.testo[MAX_LEN_SMS]=0;
         ModemRxedSMS.canale=ID_CANALE_SMS;
         strcpy(ModemRxedSMS.ConnectedPort,STR_NO);
//         SetWorkPeriod(CPU,30*UN_SECONDO);

//       }
//       else
//       {
//         if(GetState(DEBUG_MODEM)) printf("SMS SCARTATO, TESTO CON CARATTERI NON AMMESSI\r\n");
//         snprintf(&strLog[0],MAX_LEN_STR_LOG,"SMS SCARTATO CARATTERE");
//         LogPrint(strLog);
//       }

     }
//     else
//     {
//       if(GetState(DEBUG_MODEM)) printf("SMS SCARTATO, PER FORMATO NUMERO MITTENTE ERRATO\r\n");
//       snprintf(&strLog[0],MAX_LEN_STR_LOG,"SMS SCARTATO MITTENTE");
//       LogPrint(strLog);
//     }
     return(true);

////////////////
}



bool MCModem::NDISSTATQRY()
{
    //int field;
    char *p;

    if ((p = (char *)strstr(&buffer[0], "^NDISSTATQRY: 0")))
    {
        SetState(MODEM_CONNECTED,false);


    }
    else if ((p = (char *)strstr(&buffer[0], "^NDISSTATQRY: 1")))
    {
        SetState(NETWORK_RESTART,true);

        //if(!GetState(MODEM_CONNECTED))SetState(NETWORK_RESTART,true);
        //SetState(MODEM_CONNECTED,true);
    }
    return(true);

}

int MCModem::CSQ()
{
     int field;
     char *p;

     if ((p = (char *)strstr(&buffer[0], "+CSQ: ")))
     {
       p += strlen("+CSQ: ");
       field = atoi(p);

       GSMCSQ=field;

       if(!GSMRegistrato)
       {
           GSMFieldIntensity= 0;
           return(true);
       }
       if ((field < 4) || (field == 99))  GSMFieldIntensity= 0;
       else if(field<10)  GSMFieldIntensity = 1;
       else if(field<16)  GSMFieldIntensity = 2;
       else if(field<22)  GSMFieldIntensity = 3;
       else if(field<28)  GSMFieldIntensity = 4;
       else if(field>=28) GSMFieldIntensity = 5;
       else  GSMFieldIntensity =0xFF;

       return(true);
     }

     return(false);
}

bool MCModem::MONSC()
{
    char *p;

    if ((p = (char *)strstr(&buffer[0], "^MONSC: ")))
    {
         p += strlen("^MONSC: ");

         if ((p = (char *)strstr(p, ",")))
         {
            p += strlen(",");
            mcc = atoi(p);
         }

         if ((p = (char *)strstr(p, ",")))
         {
             p += strlen(",");
             mnc = atoi(p);

         }

         return(true);
    }

    return(false);
}

int MCModem::CREG()
{
  quint8 field;
  char *p,*k;


  if ((p = (char *)strstr(&buffer[0], "+CREG: 2,")))
  {

    p += strlen("+CREG: 2,");
    field = atoi(p);
    stat=field;
    p+=1;

    if(stat==1)
    {
      GSMRegistrato=stat;
    }
    else   GSMRegistrato=0;


    if((stat!=0)&&(*p==',')) // todo!!!!!
    {


        if ((p = (char *)strstr(p, ",\"")))
        {
          p += strlen(",\"");
          lac =(quint16) strtoul (p, &k, 16);

        }
        //ci
        if ((p = (char *)strstr(p, ",\"")))
        {
          p += strlen(",\"");
          ci =(quint32) strtoul (p, &k, 16);

        }

        if ((p = (char *)strstr(p, ",")))
        {
          p += strlen(",");
          atc=atoi(p);

        }


        if(stat==1)
        {
          GSMRegistrato=stat;
        }
        else   GSMRegistrato=0;
    }

    return(true);

  }

  return(false);

}

bool MCModem::UREG()
{
  quint8 field;
  char *p;


  if ((p = (char *)strstr(&buffer[0], "+UREG: 0,")))
  {

    p += strlen("+UREG: 0,");
    field = atoi(p);
    ureg=field;
    return(true);

  }

  return(true);

}


bool MCModem::IPOPEN()
{
    char *p;
    quint8 tmp;

    if ((p = (char *)strstr(&buffer[0], "^IPOPEN: ")) )
    {
        p += strlen("^IPOPEN: ");
        tmp = atoi(p);
        if(tmp==UDP_LINK)uBIS_link_opened=true;
        else uBIS_link_opened=false;
    }

    if( uBIS_link_opened )
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: LINK OPEN!!!! \r\n");
        SerialPrint.Flush(PRINT_DEBUG_TEL);
        return(true);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MDM: LINK NO OPEN!!!!\r\n");
        SerialPrint.Flush(PRINT_DEBUG_TEL);
        return(false);
    }

    return(false);

}




void MCModem::RequestOn()
{
    ECMReq=true;
}


void MCModem::PrintObj()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*** MODEM ***\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ECMReqUpgrade: %d\r\n",ECMReqUpgrade);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ECMReqStream: %d\r\n",ECMReqStream);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ECMReqRemLink: %d\r\n",ECMReqRemLink);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ECMReqUpgradeSt: %d\r\n",ECMReqUpgradeSt);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ECMReqFtp: %d\r\n",ECMReqFtp);
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ECMReqFtpFast: %d\r\n",ECMReqFtpFast);
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* apnIsSet: %d \r\n",apnIsSet);
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ECMReq: %d \r\n",ECMReq);
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* REG=%d GSMFieldIntensity=%d GSMCSQ=%d act=%d\r\n",GSMRegistrato,GSMFieldIntensity,GSMCSQ,atc);
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* ModemState=%d CommandState=%d Command=%d\r\n",ModemState,CommandState,Command);
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* CntCommandTimeOut=%d CntCommandError=%d\r\n",CntCommandTimeOut,CntCommandError);
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* stat=%d lac=%d ci=%d\r\n",stat,lac,ci);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* MODEM_CONNECTED=%d %d\r\n",GetState(MODEM_CONNECTED), atc);
    SerialPrint.Flush(PRINT_DEBUG_ALL);


}


bool MCModem::ModemGSMSendSMS(TSMS *SMS)
{
  if(SMSDaInviare)return(false); // se c'e' gia un SMS da inviare, esco
  memcpy(&SMSInUscita,SMS,sizeof(TSMS));
  SMSDaInviare=true;
  return(true);
}


void MCModem::ReStart()
{

    reStart=true;
}

