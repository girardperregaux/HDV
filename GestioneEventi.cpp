#include "AccessPointWifi.h"
#include "ConfigMobile.h"
#include <stdlib.h>
#include <stdio.h>
#include <DefCom.h>
#include <qdebug.h>
#include <qfile.h>
#include "GestioneEventi.h"
#include "tstate.h"
#include "multisinkvidaudio.h"
#include "config.h"
#include "utility.h"
#include "mcmodem.h"
#include <qfile.h>
#include "mctcptest.h"
#include "ftptransfert.h"
#include "vidaudiorec.h"
#include "GestioneMultimedia.h"
#include "vidaudiostream.h"
#include "terminal.h"
#include "framing.h"
#include "stm32.h"
#include "position.h"

extern TInfoSt32 InfoSt32;
extern quint16 stati[];
extern quint16 old_stati[];


extern MultisinkVidAudio Mtv;

extern vidaudiorec Videoaudio;
extern GestMultimedia Multimedia;
extern vidaudiostream Videoaudiostream;



extern MCSerial SerialPrint;
extern char buff_print[];

extern TConfig Configuration;
extern MCModem Modem;
extern mctcptest REMLINK;
extern Message MESSAGE;

extern quint16 VoxValueCalibration;
extern ftptransfert FTPtransf;

extern Terminal *pTERMINAL;
extern Framing Fram;
extern FtpDateReq FtpR;

extern Position CurrPosition; //fusi+
extern Position LastPosition; //fusi+
extern Position TmpPosition; //fusi+
extern QMutex   MtxPosData; //fusi+

void GestioneMovimento();
void GestioneVox();
void GestioneAvvisoAggiornamento();



#define MIN_FAN_VALUE  44
#define MAX_FAN_VALUE  55

TMM MMGestioneAvvisoAggiornamento;
char strGestioneAvvisoAggiornamento[MAX_LEN_SMS+1];


quint16 tempo_sensori_attivi=0;

GestioneEventi::GestioneEventi(QObject* parent)
    :QObject(parent)
{

}



void GestioneEventi::Init(TimerMan *t)
{
    pREGTIMER=t;

    TimerT.Init((char*)"GEventi");
    TimerRead.Init((char*)"TimerRead");
    TimerSosta.Init((char*)"Sosta");
    Temperatura_cpu.Init((char*)"Tempcpu");
    TimerVoxRit.Init((char*)"VoxRitenuto");
    TimerSpegnimento.Init((char*)"Spegnimento");
    TimerSensoriAttivi.Init((char*)"SensAttivi");
    TimerResetRitardato.Init((char*)"DelayReset");
    TimerDelayScarico.Init((char*)"DelayScarico");
    pREGTIMER->RegisterTimer(&TimerRead);
    pREGTIMER->RegisterTimer(&TimerT);
    pREGTIMER->RegisterTimer(&TimerSosta);
    pREGTIMER->RegisterTimer(&Temperatura_cpu);
    pREGTIMER->RegisterTimer(&TimerVoxRit);
    pREGTIMER->RegisterTimer(&TimerSpegnimento);
    pREGTIMER->RegisterTimer(&TimerSensoriAttivi);
    pREGTIMER->RegisterTimer(&TimerResetRitardato);
    pREGTIMER->RegisterTimer(&TimerDelayScarico);


    TimerT.SetTimer(UN_SECONDO*5);
    TimerRead.SetTimer(UN_SECONDO*3);
    Temperatura_cpu.SetTimer(UN_SECONDO*5);

    TimerRun.setInterval(50);
    TimerRun.start();
    udpSocket=new QUdpSocket(this);
    udpSocket->bind(1234);

    connect(&TimerRun, SIGNAL(timeout()), this, SLOT(Run()));
   // connect(&TimerRead, SIGNAL(timeout()), this, SLOT(ReadValueCpu()));
    ValueTemperature=0;
    ValueTemperDec=0;
}


void GestioneEventi::ReadValueCpu()
{
 //   QFile file(NAME_FILE_UDHCPD_PID);


    if(TimerRead.IsElapsed())
    {

        TimerRead.SetTimer(UN_SECONDO*10);
    }
}

void GestioneEventi::readPendingDatagrams()
{

}


void GestioneEventi::xDatagrams()
{
}

void GestioneEventi::ReadTemperature()
{

    QFile tempFile("/sys/class/thermal/thermal_zone0/temp");
    char TempArray[10];
    char *p;
    p=&TempArray[0];

    if(Temperatura_cpu.IsElapsed())
    {
        if (!tempFile.open(QIODevice::ReadOnly)) return;

        tempFile.read(p, sizeof(TempArray));

        tempFile.close();

        ValueTemperature=atoi(TempArray);

        ValueTemperDec=ValueTemperature/1000;

        if(ValueTemperDec>150)ValueTemperDec=150;

        if(ValueTemperDec<-40)ValueTemperDec=-40;

       if(ValueTemperDec>=Configuration.Config.temp_fan_on) SetState(REQ_FAN_ON,true);

       if(ValueTemperDec<=Configuration.Config.temp_fan_off) SetState(REQ_FAN_ON,false);



       if(GetState(PRINT_TEMP) && GetState(DEBUG_FUNC) )
       {
           snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: TEMP: %dC\r\n",ValueTemperDec);
           SerialPrint.Flush(PRINT_DEBUG_FUNC);
       }


       if(GetOldState(REQ_FAN_ON)^GetState(REQ_FAN_ON))
       {
           if(GetState(REQ_FAN_ON))
           {
               snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: ACCENSIONE VENTOLA\r\n");
               SerialPrint.Flush(PRINT_DEBUG_FUNC);
           }
           else
           {
               snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: SPEGNIMENTO VENTOLA\r\n");
               SerialPrint.Flush(PRINT_DEBUG_FUNC);
           }

       }

        Temperatura_cpu.SetTimer(UN_SECONDO*5);
    }

}


void GestioneEventi::GestioneMovimento()
{

    if (GetState(MOVIMENTO_IN_ATTO))
    {
        TimerSosta.SetTimer((Configuration.Config.tempo_di_sosta*UN_SECONDO*60));
        SetState(PARTITO,true);
    }

    if(TimerSosta.IsElapsed())
    {
        SetState(PARTITO,false);
    }

    if (GetState(PARTITO) && !GetOldState(PARTITO))
    {
        if(GetState(DEBUG_FUNC))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: PARTITO !!!\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }
    }

    if (!GetState(PARTITO) && GetOldState(PARTITO))
    {
        if(GetState(DEBUG_FUNC))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: IN SOSTA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }
    }

}


void GestioneEventi::GestioneVox()
{
    if(GetState(ST_CALIBRAZIONE_IN_CORSO))SetWorkPeriod(ID_CPU,30,true);
    if(GetState(REQ_CALIBRA_VOX) && GetState(ST_CALIBRAZIONE_IN_CORSO))
    {
        SetState(REQ_CALIBRA_VOX,false);
    }

    if(GetState(ST_CALIBRAZIONE_IN_CORSO) && !GetOldState(ST_CALIBRAZIONE_IN_CORSO))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO CALIBRAZIONE VOX\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    if(!GetState(ST_CALIBRAZIONE_IN_CORSO) && GetOldState(ST_CALIBRAZIONE_IN_CORSO))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX: CALIBRAZIONE TERMINATA\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if (GetState(VOX_ATTIVO))
    {
        TimerVoxRit.SetTimer((Configuration.Config.tempo_ritenuta_vox*UN_SECONDO*60));
        SetState(VOX_ATTIVO_RITENUTO,true);
    }

    if(TimerVoxRit.IsElapsed())
    {
        SetState(VOX_ATTIVO_RITENUTO,false);
    }

    if (GetState(VOX_ATTIVO_RITENUTO) && !GetOldState(VOX_ATTIVO_RITENUTO))
    {
        if(GetState(DEBUG_FUNC))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: VOX ATTIVO RITENUTO!!!\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }
    }

    if (!GetState(VOX_ATTIVO_RITENUTO) && GetOldState(VOX_ATTIVO_RITENUTO))
    {
        if(GetState(DEBUG_FUNC))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: VOX NON ATTIVO RITENUTO!!!\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }
    }




}
//fusi begin
void GestioneEventi::GestionePosition(void)
{
    if(GetOldState(PARTITO)^GetState(PARTITO))
    {
        if(!GetState(PARTITO))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: VERIFICA SALVATAGGIO POSITION\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            MtxPosData.lock();
            if(TmpPosition.IsValidate() == true)
            {
                TmpPosition.CopyPosition(&LastPosition);//copia posizione corrente nella last
                if(TmpPosition.SavePosition())
                {
                    TmpPosition.PrintObj();
                    LastPosition.PrintObj();
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: POSITION SALVATA\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }
                else
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: POSITION NON SALVATA\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                }
                TmpPosition.SetValid(false);

            }
            else
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: POSITION NON VALIDATA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

            }
            MtxPosData.unlock();


        }
    }
}



//fusi end


void GestioneEventi::Run()
{
    quint8 i;

    QByteArray Data;
    Data.append("Hello from UDP");

  // SetWorkPeriod(ID_CPU,30); // TODO !!!!!!!!!!!!!!!

    if(GetState(DEBUG_ON))SetWorkPeriod(ID_CPU,30,true);


    if(GetState(FASE_ATTIVA))
    {
        //SetState(REQ_GPS_ON,true); //fusi-
    }
    else
    {
        //SetState(REQ_GPS_ON,false); //fusi-
    }


    if(GetState(RESET_RITARDATO) && !GetOldState(RESET_RITARDATO))
    {
        SaveFileT((char*)STR_FILE_TR_COUNT);
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: RESET TRA 10 Sec\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        SetWorkPeriod(ID_CPU,30,true);
        TimerResetRitardato.SetTimer(10*UN_SECONDO);
    }

    if(GetState(RESET_RITARDATO) && TimerResetRitardato.IsElapsed())
    {
        SetState(REQ_RESET,true);
        SetState(RESET_RITARDATO,false);
        SetWorkPeriod(ID_CPU,30,true);
    }


    if(GetState(SV_TRANSFER) && !GetOldState(SV_TRANSFER))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: SVEGLIA FTP(%02x)\r\n",SV_TRANSFER);
        SerialPrint.Flush(PRINT_DEBUG_ALL);
        SetState(AVVIO_SCARICO,true);
        SetWorkPeriod(ID_CPU,30,true);

    }

    if(GetState(SV_HOUR) && !GetOldState(SV_HOUR))
    {
        if( (strcasecmp(Configuration.Config.remlink_address,STR_NO)!=0) &&
            (strcasecmp(Configuration.Config.remlink_port,STR_NO)!=0) &&
            (strcasecmp(Configuration.Config.APN,STR_NO)!=0))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: SVEGLIA HOUR(%02x)\r\n",SV_HOUR);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

            pTERMINAL->RemoteStart((char*)Configuration.Config.remlink_address,(char*)Configuration.Config.remlink_port,(char*)"hour",(char*)"hour",(char*)Configuration.Config.APN,(char*)STR_NO);

        }
        else  SetSveglia(SV_HOUR,30);

        SetWorkPeriod(ID_CPU,30,true);

    }




    if(GetState(TRANSCODE_RUN) ||
       GetState(REC_ON_SENS)
       )
    {
        SetWorkPeriod(ID_CPU,30,true);
    }


    if(IsEggUser() ||
       IsEggCode())
    {
        SetWorkPeriod(ID_CPU,30,true);
        SetState(FUORI_SERVIZIO,true);
    }

    if((GetState(REQ_OFF)^GetOldState(REQ_OFF)) && GetState(REQ_OFF))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: SHUTDOWN!!!!\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        system("halt");
    }



    if(GetState(REBOOT)^GetOldState(REBOOT))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: REBOOT ICORE!!!!\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        if(GetState(REBOOT)) system("/sbin/reboot -f");
    }


    GestioneMovimento();
    //fusi begin
    GestionePosition();
    //fusi end
    GestioneVox();
    ReadTemperature();

    //SetState(REQ_MODEM_ON,true); // !!!!!!!!!!!!!!!!!!!!!!!!

    if(!GetState(FASE_ATTIVA))
    {
       Modem.ReqOn = false;
    }
    else
    {
       Modem.ReqOn = true;
    }

    if(GetState(IN_SLEEP))
    {

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: SLEEP 2 !!!\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
        system("halt");
    }
    if(!GetState(FASE_ATTIVA)  && TimerSpegnimento.IsElapsed())
    //if(!GetState(FASE_ATTIVA) && !GetState(MODEM_ON) && TimerSpegnimento.IsElapsed() )
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: SLEEP 1 !!!\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
        SetState(REQ_SLEEP,true);

    }

    if(GetState(SENSORI_ATTIVI_A_TEMPO) && (!TimerSensoriAttivi.IsElapsed() ||Configuration.Config.funzione_vox==SENSORI_SEMPRE_ATTIVI))
    {
       SetState(SENS_PULSE,true);
       SetWorkPeriod(ID_CPU,30,true);
    }

   if(!GetOldState(SENSORI_ATTIVI_A_TEMPO) && GetState(SENSORI_ATTIVI_A_TEMPO))
   {
        TimerSensoriAttivi.SetTimer(UN_MINUTO*tempo_sensori_attivi);
        SetState(SENS_PULSE,true);
   }

   if(GetState(SENSORI_ATTIVI_A_TEMPO) && TimerSensoriAttivi.IsElapsed() && Configuration.Config.funzione_vox!=SENSORI_SEMPRE_ATTIVI)
   {
       SetState(SENSORI_ATTIVI_A_TEMPO,false);
   }




    if(GetState(PARTITO) || GetState(VOX_ATTIVO_RITENUTO) )
    {

        SetWorkPeriod(ID_CPU,30,true);
    }

    if(TimerSpegnimento.IsElapsed() && GetState(FASE_ATTIVA))
    {
        SaveFileT((char*)STR_FILE_TR_COUNT);
        system("echo 'auto' > /sys/bus/usb/devices/1-1.1/power/control" ); //MODEM IN SLEEP!!!!!
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: INIZIO SLEEP !!!\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        //SetSveglia(SV_TRANSFER,3);

        SetState(FASE_ATTIVA,false);
    }



    //
    if(((Configuration.Config.funzione_mov && GetState(PARTITO)) ||
       (Configuration.Config.funzione_vox && GetState(VOX_ATTIVO_RITENUTO))) &&
       !GetState(FUORI_SERVIZIO)

            )
    {
        if(GetState(DEBUG_FUNC) && !GetState(REC_ON_SENS))
        {
          if(GetState(PARTITO))
          {
         //     snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO REGISTRAZIONE PER MOVIMENTO!!!!\r\n");
         //     SerialPrint.Flush(PRINT_DEBUG_FUNC);
          }
          if(GetState(VOX_ATTIVO_RITENUTO))
          {
          //    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO REGISTRAZIONE PER VOX!!!!\r\n");
          //    SerialPrint.Flush(PRINT_DEBUG_FUNC);
          }

        }
        SetState(REC_ON_SENS,true);

    }

    if( ((Configuration.Config.funzione_mov && !GetState(PARTITO)) || Configuration.Config.funzione_mov==0) &&
        ((Configuration.Config.funzione_vox && !GetState(VOX_ATTIVO_RITENUTO)) || Configuration.Config.funzione_vox==0) ||
         GetState(FUORI_SERVIZIO) )
    {
        SetState(REC_ON_SENS,false);
    }


    if((!GetOldState(REC_ON_SENS) && GetState(REC_ON_SENS)) || GetState(FUORI_SERVIZIO) || IsEggFtp()||
       (!GetOldState(FTP_FAST_RUN) && GetState(FTP_FAST_RUN)))
    {
        FTPtransf.Ftp_sleep=true;
    }


    if( (GetOldState(REC_ON_SENS) && !GetState(REC_ON_SENS) && !GetState(TRANSCODE_RUN)) ||
        (GetOldState(TRANSCODE_RUN) && !GetState(TRANSCODE_RUN) && !GetState(REC_ON_SENS)) )
    {
        if(!GetState(FUORI_SERVIZIO) && !IsEggFtp())
        {
            SetState(AVVIO_SCARICO,true);
            TimerDelayScarico.SetTimer(20*UN_SECONDO);
            SetWorkPeriod(ID_CPU,30,true);
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: PROGRAMMO LO SCARICO TRA 20 Sec\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

        }
    }

    if(GetState(REC_ON_SENS))SetState(AVVIO_SCARICO,false);
    // Faccio partire lo scarico
    if(GetState(AVVIO_SCARICO) && TimerDelayScarico.IsElapsed() && Modem.GSMRegistrato)
    {

        SetState(AVVIO_SCARICO,false);
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO TRASFERIMENTO FILES\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);


        if(!GetState(FUORI_SERVIZIO) && !IsEggFtp())
        {
            if((Configuration.Config.modo_scarico==FTP_AUTO) ||
               (Configuration.Config.modo_di_funzionamento ==VA_RF))
            {
                 QFile file(REQFTP_AUTO);

                 if(file.exists())                  //se esiste file trasferisco sempre (è su comando
                 {
                     if(file.open(QFile::ReadOnly | QIODevice::Text))
                     {
                         QTextStream inStream(&file);

                         while (!inStream.atEnd())
                         {
                             snprintf(&FtpR.TDInit[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                             snprintf(&FtpR.THInit[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                             snprintf(&FtpR.TDFinish[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                             snprintf(&FtpR.THFinish[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                         }

                         file.flush();

                         file.close();

                         snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: read <%s><%s><%s><%s>\r\n",FtpR.TDInit,FtpR.THInit,FtpR.TDFinish,FtpR.THFinish);
                         SerialPrint.Flush(PRINT_DEBUG_ALL);


                         if(FTPtransf.SetDate(FtpR.TDInit,FtpR.THInit,FtpR.TDFinish,FtpR.THFinish))
                         {
                             SetWorkPeriod(ID_CPU,30,true);
                             FTPtransf.Ftp_sleep=false;
                             FtpR.start_req=false;
                             snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AUTO FTP START\r\n");
                             SerialPrint.Flush(PRINT_DEBUG_ALL);
                         }
                     }
                     else
                     {
                         snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AUTO FTP FAILED\r\n");
                         SerialPrint.Flush(PRINT_DEBUG_FUNC);
                     }
                 }
                 else
                 {


                     //SR: Trasferimento solo se registrato e atc != 0 1 2
                     if((Modem.GSMRegistrato) &&
                        (Modem.atc!=MODO_0ATC)&&
                        (Modem.atc!=MODO_1ATC)&&
                        (Modem.atc!=MODO_2ATC))
                     {
                         SetWorkPeriod(ID_CPU,30,true);
                         FTPtransf.Ftp_sleep=false;
                         FTPtransf.SetDate((char*)TRANSFERT_DATE_INIT,(char*)TRANSFERT_HOUR_INIT,(char*)TRANSFERT_DATE_FINISH,(char*)TRANSFERT_HOUR_FINISH);
                     }

                 }
            }
            else
            {
                    //if(FtpR.start_req)  //carico i valori dal file
                    //{
                        QFile Fileq(REQFTP_REQ);
                        if(Fileq.open(QIODevice::ReadOnly | QIODevice::Text)){

                        QTextStream inStream(&Fileq);

                        while (!inStream.atEnd())
                        {
                            snprintf(&FtpR.TDInit[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                            snprintf(&FtpR.THInit[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                            snprintf(&FtpR.TDFinish[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                            snprintf(&FtpR.THFinish[0],MAX_LEN_STR_FTP,"%s",inStream.readLine().toUtf8().constData());
                        }

                        Fileq.flush();

                        Fileq.close();

                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: read <%s><%s><%s><%s>\r\n",FtpR.TDInit,FtpR.THInit,FtpR.TDFinish,FtpR.THFinish);
                        SerialPrint.Flush(PRINT_DEBUG_ALL);


                        if(FTPtransf.SetDate(FtpR.TDInit,FtpR.THInit,FtpR.TDFinish,FtpR.THFinish))
                        {
                            SetWorkPeriod(ID_CPU,30,true);
                            //FTPtransf.UpgradeTransfer=true; //commnetato il 100616
                            FTPtransf.Ftp_sleep=false;
                            FtpR.start_req=false;
                            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: REQ FTP START\r\n");
                            SerialPrint.Flush(PRINT_DEBUG_ALL);
                        }
                    }
                    else
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: REQ FTP FAILED\r\n");
                        SerialPrint.Flush(PRINT_DEBUG_ALL);
                    }

            }

        }
    }

////////////
////////////
  //  if(!GetState(FUORI_SERVIZIO) && !IsEggFtp())
  //  {

//    }
//    else SetState(AVVIO_SCARICO,false);

     // Sends the datagram datagram
     // to the host address and at port.
     // qint64 QUdpSocket::writeDatagram(const QByteArray & datagram,
     //                      const QHostAddress & host, quint16 port)
    if(TimerT.IsElapsed())
    {
       // udpSocket->writeDatagram(Data,QHostAddress("88.149.185.225"), 7008);
        TimerT.SetTimer(UN_SECONDO*5);

    }



    if(GetOldState(DEBUG_COM)^GetState(DEBUG_COM))
    {
        if(GetState(DEBUG_COM))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG C ATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG C DISATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }

    }
    if(GetOldState(DEBUG_FUNC)^GetState(DEBUG_FUNC))
    {
        if(GetState(DEBUG_FUNC))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG F ATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG F DISATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }

    }
    if(GetOldState(DEBUG_TEL)^GetState(DEBUG_TEL))
    {
        if(GetState(DEBUG_TEL))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG T ATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG T DISATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }

    }
    //fusi begin
    if(GetOldState(DEBUG_GPS)^GetState(DEBUG_GPS))
    {
        if(GetState(DEBUG_GPS))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG G ATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: DEBUG G DISATTIVATA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }

    }
    //fusi end
    if(GetOldState(NETWORK_RESTART)^GetState(NETWORK_RESTART) && GetState(NETWORK_RESTART))
    {
        if(!GetState(MODEM_CONNECTED))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: NETWORK_RESTART\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            //system("/etc/init.d/networking restart");
            system("ifdown usb0");
            system("ifup usb0");

            SetState(MODEM_CONNECTED,true);

        }
        //system("dhclient -v usb0");
        SetState(NETWORK_RESTART,false);
    }



    if(GetState(REC_ON_SENS))SetState(REQ_CAM1_ON,true);

     //se era attivo ed ho cambiato la modalità di
    if((GetOldState(REC_ON_SENS)^GetState(REC_ON_SENS)) || (Multimedia.Update_mod_function) )
    {
        Multimedia.Update_mod_function=false;

        if(GetState(REC_ON_SENS))
        {
            Fram.invalidp();
            if(Configuration.Config.modo_di_funzionamento ==VA_RS)
            {
                if(!IsEggServerVideo() &&
                   (Configuration.Config.qualita_video_salvato!=0) &&
                   (Configuration.Config.qualita_video_stream!=0) &&
                   (strcasecmp(Configuration.Config.APN,STR_NO)!=0))
                 {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO REGISTRAZIONE E STREAMING\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    Modem.ECMReqStream=true;
                    strcpy(Modem.APNStream,Configuration.Config.APN);
                    if(Modem.cgactStatus)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: MODEM RESTART\r\n");
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        Modem.reStart=true;
                    }

                    Multimedia.Type(Configuration.Config.modo_di_funzionamento); //inserire param configurazione
                    Multimedia.Start();
                 }
                else  SetState(REC_ON_SENS,false);

            }
            else if(Configuration.Config.modo_di_funzionamento ==VA_R)
            {

                if((Configuration.Config.qualita_video_salvato!=0))
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO REGISTRAZIONE\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    Multimedia.Type(Configuration.Config.modo_di_funzionamento); //inserire param configurazione
                    Multimedia.Start();
                }
                else  SetState(REC_ON_SENS,false);

            }
            else if(Configuration.Config.modo_di_funzionamento ==VA_S)
            {
                if(!IsEggServerVideo() &&
                   (Configuration.Config.qualita_video_stream!=0)&&
                   (strcasecmp(Configuration.Config.APN,STR_NO)!=0))
                 {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO STREAMING\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    Modem.ECMReqStream=true;
                    strcpy(Modem.APNStream,Configuration.Config.APN);
                    if(Modem.cgactStatus)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: MODEM RESTART\r\n");
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        Modem.reStart=true;
                    }


                    Multimedia.Type(Configuration.Config.modo_di_funzionamento); //inserire param configurazione
                    Multimedia.Start();
                }
                else  SetState(REC_ON_SENS,false);
            }

            else if(Configuration.Config.modo_di_funzionamento ==VA_RF)
            {
                if(
                   (Configuration.Config.qualita_video_salvato!=0)&&
                   (strcasecmp(Configuration.Config.APN,STR_NO)!=0))
                 {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: AVVIO REGISTRAZIONE E FTP\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    if(Modem.cgactStatus)
                    {
                        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: MODEM RESTART\r\n");
                        SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        Modem.reStart=true;
                    }

                    Multimedia.Type(Configuration.Config.modo_di_funzionamento); //inserire param configurazione
                    Multimedia.Start();
                 }
                 else  SetState(REC_ON_SENS,false);
            }

            else
            {
              SetState(REC_ON_SENS,false);
            }

        }
        else
        {

            if(Configuration.Config.modo_di_funzionamento ==VA_RS)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: TERMINA REGISTRAZIONE E STREAMING\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else if(Configuration.Config.modo_di_funzionamento ==VA_R)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: TERMINA REGISTRAZIONE\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else if(Configuration.Config.modo_di_funzionamento ==VA_S)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: TERMINA STREAMING\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
            else if(Configuration.Config.modo_di_funzionamento ==VA_RF)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: TERMINA REGISTRAZIONE E FTP\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }

            Multimedia.Stop();

        }

    }

    if(IsEggServerVideo())
    {
       Modem.ECMReqStream=false;
    }


    if(GetState(ACCENSIONE_AVVENUTA) && !GetOldState(ACCENSIONE_AVVENUTA))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: ACCENSIONE AVVENUTA\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        SetSveglia(SV_HOUR,5);
    }

    // All'accensione e quando ho aggiornato lo stato con st
    if(GetState(AVVIO_AVVENUTO) && GetState(ST_STATE_UPDATED))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: COMUNICAZIONE CON ST OK\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        SetState(AVVIO_AVVENUTO,false);


        GestioneAvvisoAggiornamento();

        if(!GetState(FUORI_SERVIZIO) && !IsEggFtp())
        {
            SetState(AVVIO_SCARICO,true);
            TimerDelayScarico.SetTimer(20*UN_SECONDO);
            SetWorkPeriod(ID_CPU,30,true);
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GEV: PROGRAMMO LO SCARICO TRA 20 Sec\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

        }

    }




    for(i=0;i<STATE_LEN;i++)
    {
       old_stati[i]=stati[i];
    }
}



void GestioneEventi::PrintState()
{
    quint16 i;

    PrintListFunzioniAbilitate();

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* VIN: %2.1fV\r\n",GetVoltageSensor());
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* VOXCALIB: %d\r\n",VoxValueCalibration);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"* TEMPCPU: %dC\r\n",ValueTemperDec);
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"Sxx: NOW     OLD\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    for(i=0;i<STATE_LEN;i++)
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"S%02d: 0x%04x  0x%04x\r\n",i,stati[i],old_stati[i]);
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }


    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\n* STATI STM32:\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);

    if(GetState(MODEM_ON))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MODEM: ACCESO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MODEM: SPENTO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    if(GetState(MOVIMENTO_IN_ATTO))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MOVIMENTO: IN ATTO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MOVIMENTO: NO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(GetState(VOX_ATTIVO))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX: ATTIVO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX: NO ATTIVO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(GetState(ST_CALIBRAZIONE_VOX_ESEGUITA))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX: CALIBRATO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX: NO CALIBRATO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(GetState(ST_CALIBRAZIONE_IN_CORSO))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX: CALIBRAZIONE IN CORSO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\n* STATI ICORE:\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    if(GetState(VOX_ATTIVO_RITENUTO) )
    {
        if(GetState(VOX_ATTIVO))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX ATTIVO\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX RITENUTO\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);

        }
        TimerVoxRit.Info();


    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"VOX NON ATTIVO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(GetState(PARTITO) )
    {
        if(GetState(MOVIMENTO_IN_ATTO))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"PARTITO MOVIMENTO IN ATTO\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"PARTITO MOVIMENTO NON IN ATTO\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);

        }
        TimerSosta.Info();


    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"IN SOSTA\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }



    if(GetState(REQ_FAN_ON))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FAN ON\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FAN OFF\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    if(GetState(REC_ON_SENS))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"REC_ON_SENS ON\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"REC_ON_SENS OFF\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(GetState(TRANSCODE_RUN))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRANSCODE ON\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"TRANSCODE OFF\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    if(GetState(REQ_WIFI_ON))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WIFI ON\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"WIFI OFF\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }




    if(GetState(FTP_FAST_RUN))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTP FAST ON\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FTP FAST OFF\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(GetState(FASE_ATTIVA))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FASE ATTIVA\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SLEEP\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(GetState(FUORI_SERVIZIO))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FUORI SERVIZIO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"IN SERVIZIO\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }

    if(REMLINK.available)
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"REMLINK DISPONIBILE\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"REMLINK ATTENDERE\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
    }





}


void GestioneEventi::AvvisoAggiornamento(void)
{
    GestioneAvvisoAggiornamento();
}



void GestioneEventi::SetWorkPeriod(quint8 device,quint16 Time,bool fase_attiva)
{
    if(fase_attiva)
    {
        SetState(FASE_ATTIVA,true);
        SetState(REQ_SLEEP,false);
    }
  switch(device)
  {
    case ID_CPU:
      TimerSpegnimento.SetTimer(UN_SECONDO*Time);
    break;
    case ID_MODEM:
    break;
    default:
    break;
  }
}


void GestioneAvvisoAggiornamento()
{
    if(ReadInfoMMFromFile(&MMGestioneAvvisoAggiornamento,(char*) STR_NOME_FILE_INFO_UPGRADE))
    {
        if(IsTelephoneNumber(MMGestioneAvvisoAggiornamento.ConnectedNum))
        {
            sprintf(strGestioneAvvisoAggiornamento,"UPGRADE ESEGUITO A %s.%d",VERSIONE_FW,SUB_VERSIONE);
            MESSAGE.MessageUserMMPost(&MMGestioneAvvisoAggiornamento,(char *)&strGestioneAvvisoAggiornamento[0]);
            MMGestioneAvvisoAggiornamento.ConnectedNum[0]=0;
        }
    }

    if(ReadInfoMMFromFile(&MMGestioneAvvisoAggiornamento, (char*) STR_NOME_FILE_INFO_UPGRADEST))
    {
        if(IsTelephoneNumber(MMGestioneAvvisoAggiornamento.ConnectedNum))
        {
            sprintf(strGestioneAvvisoAggiornamento,"UPGRADST ESEGUITO A %s",InfoSt32.Fw_ST32);
            MESSAGE.MessageUserMMPost(&MMGestioneAvvisoAggiornamento,(char *)&strGestioneAvvisoAggiornamento[0]);
            MMGestioneAvvisoAggiornamento.ConnectedNum[0]=0;
        }
    }

    if(ReadInfoMMFromFile(&MMGestioneAvvisoAggiornamento, (char*) STR_NOME_FILE_INFO_DOWNLOAD))
    {
        if(IsTelephoneNumber(MMGestioneAvvisoAggiornamento.ConnectedNum))
        {
            sprintf(strGestioneAvvisoAggiornamento,"DOWNLOAD ESEGUITO");
            MESSAGE.MessageUserMMPost(&MMGestioneAvvisoAggiornamento,(char *)&strGestioneAvvisoAggiornamento[0]);
            MMGestioneAvvisoAggiornamento.ConnectedNum[0]=0;
        }
    }

}




