#include "AccessPointWifi.h"
#include "ConfigMobile.h"
#include <stdlib.h>
#include <stdio.h>
#include <DefCom.h>
#include <qdebug.h>
#include <qfile.h>
#include "tstate.h"
#include "HDV.h"
#include "config.h"
#include "utility.h"
#include "stm32.h"
#include "mctcptest.h"





extern MCSerial SerialPrint;
extern char buff_print[];
extern TConfig Configuration;
extern TInfoSt32 InfoSt32;
extern mctcptest REMLINK;






ConfigMobile::ConfigMobile(QObject* parent)
    :QObject(parent)
{

}



void ConfigMobile::Init(TimerMan *t)
{
    pREGTIMER=t;

    Delay.Init((char*)"CnfMDelay");
    pREGTIMER->RegisterTimer(&Delay);

    LiveTimer.Init((char*)"CnfMLiveTimer");
    pREGTIMER->RegisterTimer(&LiveTimer);

    TimeOut.Init((char*)"CnfMTimeOut");
    pREGTIMER->RegisterTimer(&TimeOut);




    TimerRun.setInterval(100);
    TimerRun.start();
    stato=INIT;

    connect(&TimerRun, SIGNAL(timeout()), this, SLOT(Run()));
   // fsmState=ACCESSPOINTWIFI_INIT;

}


bool ConfigMobile::Start(char *ip, char *porta,char *login,char *password,char *APN,char *NumTelSMS)
{
    if(stato!=READY) return(false);
    strncpy(InfoCon.ip,ip,MAX_LEN_CAMPO);
    strncpy(InfoCon.port,porta,MAX_LEN_CAMPO);
    strncpy(InfoCon.login,login,MAX_LEN_CAMPO);
    strncpy(InfoCon.password,password,MAX_LEN_CAMPO);
    strncpy(InfoCon.APN,APN,MAX_LEN_CAMPO);
    strncpy(InfoCon.NumTelSMS,NumTelSMS,MAX_LEN_CAMPO);
    go=true;
    return(true);
}

void ConfigMobile::Abort()
{
    strncpy(InfoCon.ip,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.port,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.login,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.password,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.APN,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.NumTelSMS,STR_NO,MAX_LEN_CAMPO);
    go=false;
}




void ConfigMobile::Run()
{
    //quint8 protocolEnable = false;
    quint8 AckRicevuto=false;
    char strtmp[MAX_LEN_SMS+1];
    quint16 ParamId=0;
    quint8 index=0;
    quint8 OkScrittura;
    ConfigEnable=false;

    switch(stato)
    {
        case INIT:
            ConfigEnable=false;
            stato=READY;

            Abort();
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: READY\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

        break;
        case READY:
            ConfigEnable=false;

            if(go)
            {
                stato=REQ_REMLINK;
            }
        break;
        case REQ_REMLINK:
            ConfigEnable=false;

            if(!go)
            {
                Abort();
                stato=READY;
                break;
            }

            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: AVVIO REMLINK\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            REMLINK.Start(&InfoCon.ip[0],&InfoCon.port[0],&InfoCon.login[0],&InfoCon.password[0],&InfoCon.APN[0],&InfoCon.NumTelSMS[0]);

            TimeOut.SetTimer(60*UN_SECONDO);
            stato=WAIT_REQ_CONNECTED;
        break;
        case WAIT_REQ_CONNECTED:
            ConfigEnable=false;
            if(REMLINK.TcpConnected)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: CONNESSO AL CONFIGURATORE REMOTO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

               LiveTimer.SetTimer(1*UN_MINUTO);
               stato=WAIT_COMMAND;
               break;
            }

            if(TimeOut.IsElapsed())
            {
                REMLINK.Abort();
                stato=INIT;
                break;
            }
        break;
        case WAIT_COMMAND:
            if(!REMLINK.TcpConnected)
            {
                REMLINK.Abort();
                stato=INIT;
            }
            ConfigEnable=true;

            if(LiveTimer.IsElapsed())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: TERMINA PER INATTIVITA'\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                REMLINK.Abort();
                stato=INIT;
                return;
            }
        break;

    }




      if(ConfigEnable)
      {
        ConfigMobileMessageInCreate(&MobileMessageIn);

        if(ConfigMobileGetInputStr())
        {

          SetState(CONFIGURAZIONE_AVVENUTA,false);

          if(ConfigMobileGetMessageIn(&MobileMessageIn))
          {
            strcpy(ConfigMobileLineIn,STR_NO);
            LiveTimer.SetTimer(60*UN_SECONDO);

            switch(MobileMessageIn.TipoPacchetto)
            {

              case MOBILE_PING:

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: SP(%03d)\r\n",MobileMessageIn.Progressivo);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);


                ProgIn=MobileMessageIn.Progressivo;
                FillStatoPosizione(&MobileMessageOut);
                ConfigMobilePutString(&MobileMessageOut.Pacchetto[0]);
              break;

              case MOBILE_RICHIESTA_GENERALITA:
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: RICHIESTA GENERALITA'\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                FillGeneralita(&MobileMessageOut);
                ConfigMobilePutString(&MobileMessageOut.Pacchetto[0]);
              break;
              case ACK_PARAMETRI_CONFIGURAZIONE:

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: ACK(%03d)\r\n",MobileMessageIn.Progressivo);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                AckRicevuto=true;
                ProgAckRicevuto=MobileMessageIn.Progressivo;
              break;
              case MOBILE_RICHIESTA_CONFIGURAZIONE:

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: RICHIESTA LETTURA CONFIGURAZIONE\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                FillRichiestaConfigurazione(&MobileMessageOut);
                ConfigMobilePutString(&MobileMessageOut.Pacchetto[0]);
                State=WAIT_START_SEND_PARAM;
                Delay.SetTimer(5*UN_SECONDO);
              break;
              case MOBILE_START_INVIO_CONFIGURAZIONE:

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: RICHIESTA SCRITTURA CONFIGURAZIONE\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                NumParam=Configuration.ConfigGetNumParam();
                strncpy(strtmp,&MobileMessageIn.Pacchetto[20],4);
                if(NumParam==atoi(strtmp))OkScrittura=true;
                else OkScrittura=false;

                FillRichiestaScritturaConfigurazione(&MobileMessageOut,OkScrittura);
                ConfigMobilePutString(&MobileMessageOut.Pacchetto[0]);

                if(OkScrittura)
                {
                  ParamID = ID_IDENTIFICAZIONE;
                  State=WAIT_PARAM;
                  Configuration.InsMode();
                }

                Delay.SetTimer(30*UN_SECONDO);
              break;
              default:
              break;


            }


          }


        }


          switch(State)
          {
            case WAIT_COMMAND:

            break;

            case POST_ACK_CONFIGURAZIONE:
              ConfigMobilePutString(ConfigMobileLineOut);
              State=WAIT_START_SEND_PARAM;
              Delay.SetTimer(5*UN_SECONDO);


            break;

            case WAIT_START_SEND_PARAM:

              if(!Delay.IsElapsed())break;

              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: INIZIO INVIO PARAMETRI\r\n");
              SerialPrint.Flush(PRINT_DEBUG_FUNC);

              Configuration.InsMode();

              ParamID = ID_IDENTIFICAZIONE;
              NumParam=Configuration.ConfigGetNumParam();
              State=GET_INFO_PARAMETRO;


            break;

            case GET_INFO_PARAMETRO:

              if(ParamID>ID_MAX_PARAM)
              {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: FINE INVIO PARAMETRI\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                State=WAIT_COMMAND;

                break;
              }



              if(strcmp(STR_NO,(char *) Configuration.ConfigGetParamName(ParamID))!=0)
              {

                FillParametro(&MobileMessageFsmOut,ParamID);
                State=SEND_INFO_PARAMETRO;
                ParamID++;
              }
              else ParamID++;


            break;

            case SEND_INFO_PARAMETRO:

              Delay.SetTimer(5*UN_SECONDO);
              ConfigMobilePutString(&MobileMessageFsmOut.Pacchetto[0]);
              State=WAIT_ACK_INFO_PARAMETRO;

            break;

            case WAIT_ACK_INFO_PARAMETRO:

              if(AckRicevuto)
              {
                if(ProgAckAtteso==ProgAckRicevuto)
                {
                  State=GET_INFO_PARAMETRO;
                  break;
                }
              }

              if(Delay.IsElapsed())
              {
                State=SEND_INFO_PARAMETRO;
              }

            break;

            case WAIT_PARAM:

              if(NumParam==0)
              {


                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"       CONFIGURAZIONE DA MOBILE AVVENUTA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                ParamID = 0;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nMEMORIZZAZIONE IN CORSO.....\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                Configuration.Save();


                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nCONFIGURAZIONE AVVENUTA\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                Configuration.Init();
                SetState(CONFIGURAZIONE_AVVENUTA,true);

                //todo ConfigSetOperation();

                // configurazione Eseguita
                FillStatoPosizioneSpont(&MobileMessageFsmOut);
                ConfigMobilePutString(&MobileMessageFsmOut.Pacchetto[0]);
                State=WAIT_COMMAND;
                break;

              }


              if(MobileMessageIn.TipoPacchetto==MOBILE_INVIO_PARAMETRO)
              {

                ParamId=atoi(&MobileMessageIn.Pacchetto[20]);

                if(Configuration.ConfigCheckParamSet(ParamId))
                {
                  //printf("PARAMID %d\r\n",ParamId);



                  index = strcspn(&MobileMessageIn.Pacchetto[0],"/");
               //   printf("PARAMETRO %s\r\n",&MobileMessageIn.Pacchetto[index+1]);



                  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: PARAMETRO Id=%d, NUOVO VALORE=\"%s\"\r\n",ParamId,&MobileMessageIn.Pacchetto[index+1]);
                  SerialPrint.Flush(PRINT_DEBUG_FUNC);


                  if(Configuration.ConfigParamSet(ParamId,(char *)&MobileMessageIn.Pacchetto[index+1]))
                  {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: PARAMETRO IMPOSTATO\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    NumParam--;

                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: RESTANO %d PARAMETRI DA CONFIGURARE\r\n",NumParam);
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);

                    FillAckParametro(&MobileMessageFsmOut);
                    ConfigMobilePutString(&MobileMessageFsmOut.Pacchetto[0]);
                  }

                }
                else
                {

                  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: PARAMETRO NON MODIFICABILE\r\n");
                  SerialPrint.Flush(PRINT_DEBUG_FUNC);

                }


              }


            break;

            default:
            break;

          }
        }

}




////////////////////////////////////da  qqqqqqqqqqqqq


/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigMobile::ConfigMobileMessageInCreate(TMobileMessageIn *MobileMessageIn)
{

  strcpy(MobileMessageIn->Pacchetto,STR_NO);

  MobileMessageIn->TipoPacchetto=0;
  MobileMessageIn->Sender=0;
  MobileMessageIn->ProtocolVer=0;

  strcpy(MobileMessageIn->CC,STR_NO);
  strcpy(MobileMessageIn->CG,STR_NO);
  strcpy(MobileMessageIn->CP,STR_NO);

  MobileMessageIn->Progressivo=0;
  MobileMessageIn->RichiestaRisposta='N';

  return(true);
}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigMobile::ConfigMobileGetMessageIn(TMobileMessageIn *MobileMessageIn)
{
  if(!CheckCRC(ConfigMobileLineIn))return(false);

  if(ConfigMobileLineIn[MOBILE_INDEX_SENDER]!= MOBILE_TABLET_TIPO_APPARATO)return(false);

  if( (ConfigMobileLineIn[MOBILE_INDEX_TIPO_PACCHETTO]!=MOBILE_PING) &&
      (ConfigMobileLineIn[MOBILE_INDEX_TIPO_PACCHETTO]!=MOBILE_RICHIESTA_GENERALITA) &&
      (ConfigMobileLineIn[MOBILE_INDEX_TIPO_PACCHETTO]!=MOBILE_RICHIESTA_CONFIGURAZIONE) &&
      (ConfigMobileLineIn[MOBILE_INDEX_TIPO_PACCHETTO]!=MOBILE_PARAMETRI_CONFIGURAZIONE) &&
      (ConfigMobileLineIn[MOBILE_INDEX_TIPO_PACCHETTO]!=MOBILE_START_INVIO_CONFIGURAZIONE) &&
      (ConfigMobileLineIn[MOBILE_INDEX_TIPO_PACCHETTO]!=MOBILE_INVIO_PARAMETRO)) return(false);

  MobileMessageIn->TipoPacchetto=0;

  // mi salvo il pacchetto!!
  //strncpy(MobileMessageIn->Pacchetto,ConfigMobileLineIn,MAX_LEN_PCK_TCP);//fusi-
  //MobileMessageIn->Pacchetto[MAX_LEN_PCK_TCP]=0;//fusi- MAX_LEN_PCK_TCP non è dimensione di pacchetto
  strncpy(MobileMessageIn->Pacchetto,ConfigMobileLineIn,MAX_LEN_SMS);
  MobileMessageIn->Pacchetto[MAX_LEN_SMS]=0;


  MobileMessageIn->Sender=MobileMessageIn->Pacchetto[MOBILE_INDEX_SENDER];

  if(MobileMessageIn->Sender!=MOBILE_TABLET_TIPO_APPARATO)
  {

   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: TIPO MOBILE NON RICONOSCIUTO\r\n");
   SerialPrint.Flush(PRINT_DEBUG_FUNC);



   return(false);
  }


  MobileMessageIn->TipoPacchetto=MobileMessageIn->Pacchetto[MOBILE_INDEX_TIPO_PACCHETTO];

  MobileMessageIn->ProtocolVer=MobileMessageIn->Pacchetto[MOBILE_INDEX_VER_PROTOCOL];

  ConnectedProtocolVer=MobileMessageIn->ProtocolVer;

  strncpy(ConnectedCC,&MobileMessageIn->Pacchetto[MOBILE_INDEX_CODICECENTRALE],MAX_STR_LEN_CC);
  ConnectedCC[MAX_STR_LEN_CC]=0;

  strncpy(ConnectedCG,&MobileMessageIn->Pacchetto[MOBILE_INDEX_CODICEGRUPPO],MAX_STR_LEN_CG);
  ConnectedCG[MAX_STR_LEN_CG]=0;

  strncpy(ConnectedCP,&MobileMessageIn->Pacchetto[MOBILE_INDEX_CODICEPRIFERICA],MAX_STR_LEN_CP);
  ConnectedCP[MAX_STR_LEN_CP]=0;

  strncpy(MobileMessageIn->CC,&MobileMessageIn->Pacchetto[MOBILE_INDEX_CODICECENTRALE],MAX_STR_LEN_CC);
  MobileMessageIn->CC[MAX_STR_LEN_CC]=0;

  strncpy(MobileMessageIn->CG,&MobileMessageIn->Pacchetto[MOBILE_INDEX_CODICEGRUPPO],MAX_STR_LEN_CG);
  MobileMessageIn->CG[MAX_STR_LEN_CG]=0;

  strncpy(MobileMessageIn->CP,&MobileMessageIn->Pacchetto[MOBILE_INDEX_CODICEPRIFERICA],MAX_STR_LEN_CP);
  MobileMessageIn->CP[MAX_STR_LEN_CP]=0;


  MobileMessageIn->Progressivo=atoi(&MobileMessageIn->Pacchetto[MOBILE_INDEX_PROGRESSIVO]);
  MobileMessageIn->RichiestaRisposta=MobileMessageIn->Pacchetto[MOBILE_INDEX_ACK_RICHIESTO];


  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: PACCHETTO OK\r\n");
  SerialPrint.Flush(PRINT_DEBUG_FUNC);



  ConfigMobileLineIn[0]=0;

  return(true);
}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigMobile::FillGeneralita(TMobileMessageOut *MobileMessageOut)
{
  char *pMess=&MobileMessageOut->Pacchetto[0];
  //char *pVerBoot=(char *)0x8000200;
 // char strboot[5];

 // sprintf(&strboot[0],"%c%c%c%c",pVerBoot[0],pVerBoot[1],pVerBoot[2],pVerBoot[3]);


  pMess += sprintf(pMess,"%c",MOBILE_RICHIESTA_GENERALITA);
  pMess += sprintf(pMess,"%c",MOBILE_PERIFERICA_TIPO_APPARATO);

  pMess += sprintf(pMess,"%c",MobileMessageIn.ProtocolVer);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CC);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CG);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CP);
  pMess += sprintf(pMess,"%03d", MobileMessageIn.Progressivo);

  pMess += sprintf(pMess,"%c",'N');
  pMess += sprintf(pMess,"%c",'S');

  //if(!IsHwWithGPSConnetedViaHw())pMess += sprintf(pMess,"%s.0.0/",pConfig->versione_configurazione);
  //else pMess += sprintf(pMess,"%s.0.1/",pConfig->versione_configurazione);
  pMess += sprintf(pMess,"%s.0.1/",Configuration.Config.versione_configurazione);
  pMess += sprintf(pMess,"%s.%01d/",VERSIONE_FW,SUB_VERSIONE);
  pMess += sprintf(pMess,"0000/");
  pMess += sprintf(pMess,"%s/",Configuration.Config.tipo_centrale1);
  pMess += sprintf(pMess,"%s",InfoSt32.Sn_ST32);

  InsertCRC(&MobileMessageOut->Pacchetto[0]);
  pMess += 4;
  pMess += sprintf(pMess,"\r\n");

}


/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigMobile::FillRichiestaConfigurazione(TMobileMessageOut *MobileMessageOut)
{

  char *pMess=&MobileMessageOut->Pacchetto[0];

  pMess += sprintf(pMess,"%c",MOBILE_RICHIESTA_CONFIGURAZIONE);
  pMess += sprintf(pMess,"%c",MOBILE_PERIFERICA_TIPO_APPARATO);

  pMess += sprintf(pMess,"%c",MobileMessageIn.ProtocolVer);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CC);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CG);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CP);
  pMess += sprintf(pMess,"%03d", MobileMessageIn.Progressivo);

  pMess += sprintf(pMess,"%c",'N');
  pMess += sprintf(pMess,"%c",'S');

  pMess += sprintf(pMess,"%04d",Configuration.ConfigGetNumParam());

  InsertCRC(&MobileMessageOut->Pacchetto[0]);
  pMess += 4;
  pMess += sprintf(pMess,"\r\n");

}


/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigMobile:: FillRichiestaScritturaConfigurazione(TMobileMessageOut *MobileMessageOut,quint8 ok)
{

  char *pMess=&MobileMessageOut->Pacchetto[0];

  pMess += sprintf(pMess,"%c",MOBILE_START_INVIO_CONFIGURAZIONE);
  pMess += sprintf(pMess,"%c",MOBILE_PERIFERICA_TIPO_APPARATO);

  pMess += sprintf(pMess,"%c",MobileMessageIn.ProtocolVer);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CC);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CG);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CP);
  pMess += sprintf(pMess,"%03d", MobileMessageIn.Progressivo);

  pMess += sprintf(pMess,"%c",'N');
  pMess += sprintf(pMess,"%c",'S');

  if(ok)pMess += sprintf(pMess,"S");
  else  pMess += sprintf(pMess,"N");

  InsertCRC(&MobileMessageOut->Pacchetto[0]);
  pMess += 4;
  pMess += sprintf(pMess,"\r\n");

}




void ConfigMobile::FillParametro(TMobileMessageOut *MobileMessageOut,quint16 ID)
{
  char *pMess=&MobileMessageOut->Pacchetto[0];

  pMess += sprintf(pMess,"%c",MOBILE_PARAMETRI_CONFIGURAZIONE);
  pMess += sprintf(pMess,"%c",MOBILE_PERIFERICA_TIPO_APPARATO);

  pMess += sprintf(pMess,"%c",ConnectedProtocolVer);
  pMess += sprintf(pMess,"%s",ConnectedCC);
  pMess += sprintf(pMess,"%s",ConnectedCG);
  pMess += sprintf(pMess,"%s",ConnectedCP);
  pMess += sprintf(pMess,"%03d",ProgOut);

  pMess += sprintf(pMess,"%c",'S');
  pMess += sprintf(pMess,"%c",'N');

  pMess += sprintf(pMess,"%s/",Configuration.ConfigGetIdGruppoDesc(ID));
  pMess += sprintf(pMess,"%s/",Configuration.ConfigGetMin(ID));
  pMess += sprintf(pMess,"%s/",Configuration.ConfigGetMax(ID));
  pMess += sprintf(pMess,"%s/",Configuration.ConfigGetParamValue(ID));

  pMess += sprintf(pMess,"%s",Configuration.ConfigGetLen(ID));

  InsertCRC(&MobileMessageOut->Pacchetto[0]);
  pMess += 4;

  pMess += sprintf(pMess,"\r\n");

  ProgAckAtteso= ProgOut;
  ProgOut++;

}


void ConfigMobile:: FillAckParametro(TMobileMessageOut *MobileMessageOut)
{
 char *pMess=&MobileMessageOut->Pacchetto[0];

  pMess += sprintf(pMess,"%c",MOBILE_INVIO_PARAMETRO);
  pMess += sprintf(pMess,"%c",MOBILE_PERIFERICA_TIPO_APPARATO);

  pMess += sprintf(pMess,"%c",MobileMessageIn.ProtocolVer);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CC);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CG);
  pMess += sprintf(pMess,"%s",MobileMessageIn.CP);
  pMess += sprintf(pMess,"%03d",MobileMessageIn.Progressivo);

  pMess += sprintf(pMess,"%c",'N');
  pMess += sprintf(pMess,"%c",'S');

  InsertCRC(&MobileMessageOut->Pacchetto[0]);
  pMess += 4;

  pMess += sprintf(pMess,"\r\n");

}


void ConfigMobile::FillStatoPosizione(TMobileMessageOut *MobileMessageOut)
{
  char *pMess=&MobileMessageOut->Pacchetto[0];

  pMess += sprintf(pMess,"%c",MOBILE_PING);
  pMess += sprintf(pMess,"%c",MOBILE_PERIFERICA_TIPO_APPARATO);

  pMess += sprintf(pMess,"%c",ConnectedProtocolVer);
  pMess += sprintf(pMess,"%s",ConnectedCC);
  pMess += sprintf(pMess,"%s",ConnectedCG);
  pMess += sprintf(pMess,"%s",ConnectedCP);
  pMess += sprintf(pMess,"%03d",ProgIn);

  pMess += sprintf(pMess,"%c",'N');
  pMess += sprintf(pMess,"%c",'S');

//  pMess += sprintf(pMess,pPOSITION->strDate);         //(20..27) DATA POSIZIONE
  pMess += sprintf(pMess,"00000000");         //(20..27) DATA POSIZIONE
//  pMess += sprintf(pMess,pPOSITION->strHour);         //(28..33) ORA POSIZIONE
  pMess += sprintf(pMess,"000000");         //(28..33) ORA POSIZIONE
//  pMess += sprintf(pMess,pPOSITION->strLatitude);          //(34..41) LATITUDINE
  pMess += sprintf(pMess,"00000000");          //(34..41) LATITUDINE
//  pMess += sprintf(pMess,pPOSITION->strLongitude);         //(42..50) LONGITUDINE
  pMess += sprintf(pMess,"000000000");         //(42..50) LONGITUDINE

//  pMess += sprintf(pMess,pPOSITION->strAltitude);          //(51..55) ALTITUDINE
  pMess += sprintf(pMess,"00000");          //(51..55) ALTITUDINE

//  pMess += sprintf(pMess,"%03u",pPOSITION->Velocity);      //(56..58) VELOCITA
  pMess += sprintf(pMess,"000");      //(56..58) VELOCITA

//  pMess += sprintf(pMess,"%03d",pPOSITION->Direction);     //(59..61) direzione di spostamento in gradi (3 caratteri)
  pMess += sprintf(pMess,"000");     //(59..61) direzione di spostamento in gradi (3 caratteri)
//  pMess += sprintf(pMess,"%02d",pPOSITION->Satellites);    //(62..63) numero di satelliti utilizzati (2 caratteri)
  pMess += sprintf(pMess,"00");    //(62..63) numero di satelliti utilizzati (2 caratteri)
//  pMess += sprintf(pMess,pSTATE->strDate);         //(64..71) DATA PERIFERICA (ATTUALE)
    pMess += sprintf(pMess,"00000000");
//  pMess += sprintf(pMess,pSTATE->strHour);         //(72..77) ORA PERIFERICA (ATTUALE)
    pMess += sprintf(pMess,"000000");         //(72..77) ORA PERIFERICA (ATTUALE)

//  pMess += sprintf(pMess,"%02x",pMODEMGSM->GSMFieldIntensity); //(78,79) Campo GSM (8 bit)
    pMess += sprintf(pMess,"00"); //(78,79) Campo GSM (8 bit)

  //  pMess += sprintf(pMess,"%01d000",pMODEMGSM->pMYVIRTUALMODEM->ureg); //(80,83) Campo UREG (8 bit)
    pMess += sprintf(pMess,"0000"); //(80,83) Campo UREG (8 bit)

   pMess += sprintf(pMess,"%04x",GetStatoPeriferica());     //(90..93) STATO PERIFERICA (16 sensori)


  strupper(MobileMessageOut->Pacchetto);

  InsertCRC(&MobileMessageOut->Pacchetto[0]);
  pMess += 4;

  pMess += sprintf(pMess,"\r\n");

  //printf("FillStatoPosizione: %s\r\n",MobileMessageOut->Pacchetto);

}




void ConfigMobile::FillStatoPosizioneSpont(TMobileMessageOut *MobileMessageOut)
{
  char *pMess=&MobileMessageOut->Pacchetto[0];

  pMess += sprintf(pMess,"%c",MOBILE_PING);
  pMess += sprintf(pMess,"%c",MOBILE_PERIFERICA_TIPO_APPARATO);

  pMess += sprintf(pMess,"%c",ConnectedProtocolVer);
  pMess += sprintf(pMess,"%s",ConnectedCC);
  pMess += sprintf(pMess,"%s",ConnectedCG);
  pMess += sprintf(pMess,"%s",ConnectedCP);
  pMess += sprintf(pMess,"%03d",ProgOut);

  pMess += sprintf(pMess,"%c",'N');
  pMess += sprintf(pMess,"%c",'S');

//  pMess += sprintf(pMess,pPOSITION->strDate);         //(20..27) DATA POSIZIONE
  pMess += sprintf(pMess,"00000000");         //(20..27) DATA POSIZIONE
//  pMess += sprintf(pMess,pPOSITION->strHour);         //(28..33) ORA POSIZIONE
  pMess += sprintf(pMess,"000000");         //(28..33) ORA POSIZIONE
//  pMess += sprintf(pMess,pPOSITION->strLatitude);          //(34..41) LATITUDINE
  pMess += sprintf(pMess,"00000000");          //(34..41) LATITUDINE
//  pMess += sprintf(pMess,pPOSITION->strLongitude);         //(42..50) LONGITUDINE
  pMess += sprintf(pMess,"000000000");         //(42..50) LONGITUDINE

//  pMess += sprintf(pMess,pPOSITION->strAltitude);          //(51..55) ALTITUDINE
  pMess += sprintf(pMess,"00000");          //(51..55) ALTITUDINE

//  pMess += sprintf(pMess,"%03u",pPOSITION->Velocity);      //(56..58) VELOCITA
  pMess += sprintf(pMess,"000");      //(56..58) VELOCITA

//  pMess += sprintf(pMess,"%03d",pPOSITION->Direction);     //(59..61) direzione di spostamento in gradi (3 caratteri)
  pMess += sprintf(pMess,"000");     //(59..61) direzione di spostamento in gradi (3 caratteri)
//  pMess += sprintf(pMess,"%02d",pPOSITION->Satellites);    //(62..63) numero di satelliti utilizzati (2 caratteri)
  pMess += sprintf(pMess,"00");    //(62..63) numero di satelliti utilizzati (2 caratteri)
//  pMess += sprintf(pMess,pSTATE->strDate);         //(64..71) DATA PERIFERICA (ATTUALE)
    pMess += sprintf(pMess,"00000000");
//  pMess += sprintf(pMess,pSTATE->strHour);         //(72..77) ORA PERIFERICA (ATTUALE)
    pMess += sprintf(pMess,"000000");         //(72..77) ORA PERIFERICA (ATTUALE)

//  pMess += sprintf(pMess,"%02x",pMODEMGSM->GSMFieldIntensity); //(78,79) Campo GSM (8 bit)
    pMess += sprintf(pMess,"00"); //(78,79) Campo GSM (8 bit)

  //  pMess += sprintf(pMess,"%01d000",pMODEMGSM->pMYVIRTUALMODEM->ureg); //(80,83) Campo UREG (8 bit)
    pMess += sprintf(pMess,"0000"); //(80,83) Campo UREG (8 bit)

   pMess += sprintf(pMess,"%04x",GetStatoPeriferica());     //(90..93) STATO PERIFERICA (16 sensori)

  strupper(MobileMessageOut->Pacchetto);

  InsertCRC(&MobileMessageOut->Pacchetto[0]);
  pMess += 4;

  pMess += sprintf(pMess,"\r\n");


  //printf("FillStatoPosizione: %s\r\n",MobileMessageOut->Pacchetto);

}




quint8 ConfigMobile::ConfigMobileGetInputStr(void)
{
    char c = 0;

    do
    {
    if(REMLINK.GetCharTcp(&c)==false)return(false);


        if (c == '\r')break;

        if (ConfigMobileCount  >= MAX_LEN_PCK_TCP)
        {
            ConfigMobileCount = 0;
            continue;
        }

        if (c >= 0x20 && c <= 0x7E)
        {
            ConfigMobileLineIn[ConfigMobileCount++] = c;
        }

    }
    while(true);

    ConfigMobileLineIn[ConfigMobileCount]  = '\0';
    ConfigMobileCount=0;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: [Rx]=\"%s\"\r\n",ConfigMobileLineIn);
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    LiveTimer.SetTimer(3*UN_MINUTO);
    return(true);
}



void ConfigMobile::ConfigMobilePutString(char *s)
{
    if(!ConfigEnable)return;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFM: [Tx]=\"%s\"\r\n",s);
    SerialPrint.Flush(PRINT_DEBUG_FUNC);


    REMLINK.socket.write(s);
}



