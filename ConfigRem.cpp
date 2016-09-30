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
#include "ConfigRem.h"



extern MCSerial SerialPrint;
extern char buff_print[];
extern TConfig Configuration;
extern TInfoSt32 InfoSt32;
extern mctcptest REMLINK;


enum CONFIGREM_STATE {

    CONFIGREM_INIT =0,
    CONFIGREM_READY,
    CONFIGREM_REQ_REMLINK,
    CONFIGREM_WAIT_REQ_CONNECTED,
    CONFIGREM_CONNECTED
};

enum GESTUSER_STATE {
   CONFIGREM_GESTUSER_INIT=0,
   CONFIGREM_WAIT_COMMAND,
   CONFIGREM_WAIT_VALUE,
   CONFIGREM_WAIT_CONFERMA_EXIT,
   CONFIGREM_GESTUSER_EXIT,
   CONFIGREM_CNF_INS_WAIT_VALUE
};


char CONFIGREM_CAMPO1 [MAX_LEN_CAMPO+1];
char CONFIGREM_CAMPO2 [MAX_LEN_CAMPO+1];
char CONFIGREM_CAMPO3 [MAX_LEN_CAMPO+1];


ConfigRem::ConfigRem(QObject* parent)
    :QObject(parent)
{

}



void ConfigRem::Init(TimerMan *t)
{
    pREGTIMER=t;

    Delay.Init((char*)"CnfMDelay");
    pREGTIMER->RegisterTimer(&Delay);

    LiveTimer.Init((char*)"CnfRLiveTimer");
    pREGTIMER->RegisterTimer(&LiveTimer);

    TimeOut.Init((char*)"CnfMTimeOut");
    pREGTIMER->RegisterTimer(&TimeOut);

    TimerRun.setInterval(100);
    TimerRun.start();
    stato=INIT;

    connect(&TimerRun, SIGNAL(timeout()), this, SLOT(Run()));
}


bool ConfigRem::Start(char *ip, char *porta,char *login,char *password,char *APN,char *NumTelSMS)
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

void ConfigRem::Abort()
{
    strncpy(InfoCon.ip,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.port,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.login,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.password,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.APN,STR_NO,MAX_LEN_CAMPO);
    strncpy(InfoCon.NumTelSMS,STR_NO,MAX_LEN_CAMPO);
    go=false;
}




void ConfigRem::Run()
{
    switch(stato)
    {
        case CONFIGREM_INIT:
            ConfigEnable=false;
            stato=CONFIGREM_READY;

            Abort();
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: READY\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);

        break;
        case CONFIGREM_READY:
            ConfigEnable=false;

            if(go)
            {
                stato=CONFIGREM_REQ_REMLINK;
            }
        break;
        case CONFIGREM_REQ_REMLINK:
            ConfigEnable=false;

            if(!go)
            {
                Abort();
                stato=CONFIGREM_READY;
                break;
            }

            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: AVVIO REMLINK\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            REMLINK.Start(&InfoCon.ip[0],&InfoCon.port[0],&InfoCon.login[0],&InfoCon.password[0],&InfoCon.APN[0],&InfoCon.NumTelSMS[0]);

            TimeOut.SetTimer(60*UN_SECONDO);
            stato=CONFIGREM_WAIT_REQ_CONNECTED;

        break;

        case CONFIGREM_WAIT_REQ_CONNECTED:

            ConfigEnable=false;
            if(REMLINK.TcpConnected)
            {
                ConfigEnable=true;
                ConfigChanged=false;
                LiveTimer.SetTimer(3*UN_MINUTO);

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: AVVIO IL CONFIGURATORE REMOTO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: TIMEOUT 3 MINUTI\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);


                ConfigRemotaPrintMenu();
                StartConfigRemota();
                stato=CONFIGREM_CONNECTED;
                break;
            }

            if(TimeOut.IsElapsed())
            {
                REMLINK.Abort();
                stato=CONFIGREM_INIT;
                break;
            }
        break;
        case CONFIGREM_CONNECTED:
            if(!REMLINK.TcpConnected)
            {
                REMLINK.Abort();
                stato=CONFIGREM_INIT;
                ConfigEnable=false;
                return;

            }
            ConfigEnable=true;

            if(LiveTimer.IsElapsed())
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: TERMINA PER INATTIVITA'\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                REMLINK.Abort();
                stato=CONFIGREM_INIT;
                ConfigEnable=false;
                return;
            }
        break;

    }


    if(ConfigEnable)
    {
// da qui 1111
        if(ConfigRemotaGetInputStr())
        {

          ConfigCommand();

          switch(StateGestUser)
          {

              case CONFIGREM_WAIT_COMMAND:


                if(IsNumber(CONFIGREM_CAMPO1))
                {
                  StateGestUser=CONFIGREM_WAIT_VALUE;
                  ParamID =atoi(CONFIGREM_CAMPO1);

                  ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                  ConfigRemotaPutString(ConfigRemotaLineOut);
                  ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                  ConfigRemotaPutString(ConfigRemotaLineOut);

                  ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"DIGITARE IL NUOVO VALORE PER IL PARAMETRO:\r\n");
                  ConfigRemotaPutString(ConfigRemotaLineOut);

                  if(!ConfigRemotaShowParam(ParamID))ConfigRemotaPrintMenu();
                }

                if (strcasecmp(CONFIGREM_CAMPO1,"S")==0)                                                             // Inizio Configurazione
                {
                  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: SALVO\r\n");
                  SerialPrint.Flush(PRINT_DEBUG_FUNC);

                  StateGestUser=CONFIGREM_WAIT_COMMAND;
                  ConfigRemotaSave();
                  ConfigChanged=false;

                }


                if (strcasecmp(CONFIGREM_CAMPO1,"I")==0)                                                             // Inizio Configurazione
                {

                  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: INSERIMENTO\r\n");
                  SerialPrint.Flush(PRINT_DEBUG_FUNC);

                  StateGestUser=CONFIGREM_CNF_INS_WAIT_VALUE;
                  ParamID = ID_IDENTIFICAZIONE;
                  ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                  ConfigRemotaPutString(ConfigRemotaLineOut);
                  ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                  ConfigRemotaPutString(ConfigRemotaLineOut);

                  ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"DIGITARE IL NUOVO VALORE PER IL PARAMETRO (INVIO=SUCCESSIVO):\r\n");
                  ConfigRemotaPutString(ConfigRemotaLineOut);

                  ConfigRemotaInsShowParam(true,SUCCESSIVO);

                }


          break;


          case CONFIGREM_CNF_INS_WAIT_VALUE:


            if(strcasecmp(CONFIGREM_CAMPO1,"C")==0)
            {


              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: CANCELLO IL PARAMETRO\r\n");
              SerialPrint.Flush(PRINT_DEBUG_FUNC);


              strcpy(CONFIGREM_CAMPO1,STR_NO);
              if( (ParamID==ID_NUM_TEL_USER1) ||
                  (ParamID==ID_NUM_SMS_CENTRALE1) ||
                  (ParamID==ID_NUM_DATI_CENTRALE1))
                  {
                     strcpy(CONFIGREM_CAMPO1,"*");
                  }


              if(!Configuration.ConfigParamSet(ParamID,(char *)CONFIGREM_CAMPO1))
              {

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"QUESTO PARAMETRO NON PUO' ESSERE CANCELLATO!!!\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"DIGITARE IL NUOVO VALORE PER IL PARAMETRO (INVIO=SUCCESSIVO):\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);

              }
              else
              {

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"NUOVO VALORE PER IL PARAMETRO:\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigChanged=true;

                ConfigRemotaShowParam(ParamID);
                break;

              }


              if(ConfigRemotaInsShowParam(true,SUCCESSIVO)==false)
              {
                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"FINE PARAMETRI\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);

               // printf("CFR: RAGGIUNTA FINE CONFIGURAZIONE\r\n"); //continuare da qui!!!!!!!

                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: RAGGIUNTA FINE CONFIGURAZIONE\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);


                ConfigRemotaPrintSubMenu();
                StateGestUser=CONFIGREM_WAIT_COMMAND;
              }

              break;
            }





            if(strcmp(CONFIGREM_CAMPO1,STR_NO)==0)
            {
              ParamID++;

             // printf("CFR: INSERIMENTO PASSO AL PARAM SUCCESSIVO\r\n");
              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: INSERIMENTO PASSO AL PARAM SUCCESSIVO\r\n");
              SerialPrint.Flush(PRINT_DEBUG_FUNC);


              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
              ConfigRemotaPutString(ConfigRemotaLineOut);
              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
              ConfigRemotaPutString(ConfigRemotaLineOut);



              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"DIGITARE IL NUOVO VALORE PER IL PARAMETRO (INVIO=SUCCESSIVO):\r\n");
              ConfigRemotaPutString(ConfigRemotaLineOut);

              if(ConfigRemotaInsShowParam(true,SUCCESSIVO)==false)
              {

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"FINE PARAMETRI\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);

                //printf("CFR: RAGGIUNTA FINE CONFIGURAZIONE\r\n");
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: RAGGIUNTA FINE CONFIGURAZIONE\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);


                ConfigRemotaPrintSubMenu();
                StateGestUser=CONFIGREM_WAIT_COMMAND;
              }


              break;
            }



            if(!Configuration.ConfigParamSet(ParamID,(char *)CONFIGREM_CAMPO1))
            {

              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"VALORE ERRATO\r\n");
              ConfigRemotaPutString(ConfigRemotaLineOut);
            }
            else
            {
              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
              ConfigRemotaPutString(ConfigRemotaLineOut);
              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
              ConfigRemotaPutString(ConfigRemotaLineOut);

              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"NUOVO VALORE PER IL PARAMETRO:\r\n");
              ConfigRemotaPutString(ConfigRemotaLineOut);
              ConfigChanged=true;

              ConfigRemotaShowParam(ParamID);

     //         printf("INC2\r\n");



            }

          break;


          case CONFIGREM_WAIT_VALUE:



            if(strcasecmp(CONFIGREM_CAMPO1,"C")==0)
            {

              //if(GetState(DEBUG_FUNC))printf("CFR: CANCELLO IL PARAMETRO\r\n");
              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: CANCELLO IL PARAMETRO\r\n");
              SerialPrint.Flush(PRINT_DEBUG_FUNC);


              strcpy(CONFIGREM_CAMPO1,STR_NO);
              if( (ParamID==ID_NUM_TEL_USER1) ||
                  (ParamID==ID_NUM_SMS_CENTRALE1) ||
                  (ParamID==ID_NUM_DATI_CENTRALE1))
                  {
                     strcpy(CONFIGREM_CAMPO1,"*");
                  }




              if(!Configuration.ConfigParamSet(ParamID,(char *)CONFIGREM_CAMPO1))
              {

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"QUESTO PARAMETRO NON PUO' ESSERE CANCELLATO!!!\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"DIGITARE IL NUOVO VALORE PER IL PARAMETRO:\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigRemotaShowParam(ParamID);


              }
              else
              {

                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
                ConfigRemotaPutString(ConfigRemotaLineOut);


                ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"NUOVO VALORE PER IL PARAMETRO:\r\n");
                ConfigRemotaPutString(ConfigRemotaLineOut);
                ConfigChanged=true;
                ConfigRemotaShowParam(ParamID);
                StateGestUser=CONFIGREM_WAIT_COMMAND;
                ParamID=0;

                ConfigRemotaPrintSubMenu();
                break;

              }

              break;
            }


            if(strcasecmp(CONFIGREM_CAMPO1,STR_NO)==0)
            {
              ConfigRemotaShowParam(ParamID);
              StateGestUser=CONFIGREM_WAIT_COMMAND;
              ParamID=0;
              ConfigRemotaPrintSubMenu();
              break;
            }




             if(!Configuration.ConfigParamSet(ParamID,(char *)CONFIGREM_CAMPO1))
             {
              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"VALORE ERRATO\r\n");
              ConfigRemotaPutString(ConfigRemotaLineOut);
              StateGestUser=CONFIGREM_WAIT_COMMAND;
              ParamID=0;
             }
             else
             {
             // ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"NUOVO VALORE PER IL PARAMETRO:\r\n");
             // ConfigRemotaPutString(ConfigRemotaLineOut);
              ConfigChanged=true;

              ConfigRemotaShowParam(ParamID);
              StateGestUser=CONFIGREM_WAIT_COMMAND;
              ParamID=0;

             }
             ConfigRemotaPrintSubMenu();



          break;


          case CONFIGREM_WAIT_CONFERMA_EXIT:

            if (strcasecmp(CONFIGREM_CAMPO1,"S")==0)                                                             // Inizio Configurazione
            {
              //if(GetState(DEBUG_FUNC))printf("CFR: NON SALVO ED ESCO!!!\r\n");
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: NON SALVO ED ESCO!!!\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);


              StateGestUser=CONFIGREM_GESTUSER_EXIT;
              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
              ConfigRemotaPutString(ConfigRemotaLineOut);

              ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"CONNESSIONE TERMINATA DISCONNETTERE IL TERMINALE !!!\r\n");
              ConfigRemotaPutString(ConfigRemotaLineOut);
              Abort();


            }
            if (strcasecmp(CONFIGREM_CAMPO1,"N")==0)                                                             // Inizio Configurazione
            {
              //if(GetState(DEBUG_FUNC))printf("CFR: NON ESCO\r\n");
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: NON ESCO\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                StateGestUser=CONFIGREM_WAIT_COMMAND;
                ConfigRemotaPrintSubMenu();
            }


          break;

          case CONFIGREM_GESTUSER_EXIT:

            ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
            ConfigRemotaPutString(ConfigRemotaLineOut);

            ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"CONNESSIONE TERMINATA ....DISCONNETTERE IL TERMINALE !!!\r\n");
            ConfigRemotaPutString(ConfigRemotaLineOut);
            Abort();

          break;



          }
          strcpy(CONFIGREM_CAMPO1,STR_NO);
          strcpy(CONFIGREM_CAMPO2,STR_NO);
          strcpy(CONFIGREM_CAMPO3,STR_NO);

        }


// a qui 1111



    }

}









/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigRem::ConfigRemotaGetInputStr(void)// 1 pri
{
    char c = 0;

    do
    {
        if(REMLINK.GetCharTcp(&c)==false)return(false);


        if (c == '\r')break;

        if (ConfigRemotaLineInCount  >= MAX_LEN_SMS)
        {
            ConfigRemotaLineInCount = 0;
            continue;
        }

        if (c >= 0x20 && c <= 0x7E)
        {
            ConfigRemotaLineIn[ConfigRemotaLineInCount++] = c;
        }

    }
    while(true);

    ConfigRemotaLineIn[ConfigRemotaLineInCount]  = '\0';
    ConfigRemotaLineInCount=0;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: [Rx]=\"%s\"\r\n",ConfigRemotaLineIn);
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    LiveTimer.SetTimer(3*UN_MINUTO);
    REMLINK.TimerConnect.SetTimer(3*UN_MINUTO);
    return(true);

}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigRem::ConfigRemotaPutString(char *s) //2 pri
{
    if(!ConfigEnable)return;
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: [Tx]=\"%s\"\r\n",s);
    SerialPrint.Flush(PRINT_DEBUG_FUNC);
    REMLINK.socket.write(s);
}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigRem::StartConfigRemota(void) //3 pri
{
    ParamID = ID_IDENTIFICAZIONE;
    StateGestUser = CONFIGREM_WAIT_COMMAND;
    Configuration.InsMode();
    return(true);
}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigRem::AbortConfigRemota(void) //4 pri
{
    return (true);
}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigRem::ConfigRemotaPrintMenu(void) //5 pri
{
    StateGestUser=CONFIGREM_WAIT_COMMAND;

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[H", ASCII_ESC);
    ConfigRemotaPutString(ConfigRemotaLineOut);



    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"* %s(%s) V%s.%d\r\n",Configuration.Config.tipo_centrale1,NOME_APPARATO,VERSIONE_FW,SUB_VERSIONE);
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);


    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"MENU' DI CONFIGURAZIONE:\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<A><INVIO>               ANNULLA L'OPERAZIONE IN CORSO\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);
    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<I><INVIO>               INSERIMENTO CONFIGURAZIONE\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<L><INVIO>               LETTURA CONFIGURAZIONE\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);
    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<M><INVIO>               VISUALIZZA QUESTO MENU'\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<S><INVIO>               SALVA LA CONFIGURAZIONE\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<R><INVIO>               RICARICA L'ULTIMA CONFIGURAZIONE SALVATA\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);


    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<E><INVIO>               ESCE\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);


    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<NUM><INVIO>             MODIFICA PARAMETRO\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);

}


/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigRem::ConfigRemotaPrintSubMenu(void) //6 pri
{
    StateGestUser=CONFIGREM_WAIT_COMMAND;

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);
    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"<M><INVIO>               VISUALIZZA IL MENU\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);


}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigRem::ConfigCommand(void) //7 pri
{
    char *str =&ConfigRemotaLineIn[0];
    char *strs =&ConfigRemotaLineIn[0];



     if (strcasecmp(str,"L")==0)                                                             // Inizio Configurazione
     {

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: STAMPO LA CNF\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);
        ConfigRemotaPrint(&Configuration.ConfigMod);
        return;
     }

    if (strcasecmp(str,"A")==0)                                                             // Inizio Configurazione
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: OPERAZIONE ANNULLATA\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);

        StateGestUser=CONFIGREM_WAIT_COMMAND;
        ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"OPERAZIONE ANNULLATA\r\n");
        ConfigRemotaPutString(ConfigRemotaLineOut);
        ConfigRemotaPrintSubMenu();
        return;
    }

    if (strcasecmp(str,"R")==0)                                                             // Inizio Configurazione
    {

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: RICARICO LA CNF\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);

        ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
        ConfigRemotaPutString(ConfigRemotaLineOut);

        StateGestUser=CONFIGREM_WAIT_COMMAND;
        ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"ATTENDERE, STO CARICANDO LA CONFIGURAZIONE.\r\n");
        ConfigRemotaPutString(ConfigRemotaLineOut);
        Configuration.InsMode(); //I2CConfigRead(pWorkingConfig,0);

        ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"CONFIGURAZIONE CARICATA.....\r\n");
        ConfigRemotaPutString(ConfigRemotaLineOut);
        ConfigChanged=false;

        ConfigRemotaPrintSubMenu();

        return;
     }

     if (strcasecmp(str,"E")==0)                                                             // Inizio Configurazione
     {

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CFR: ESCO DALLA CNF?\r\n");
        SerialPrint.Flush(PRINT_DEBUG_FUNC);

        if(ConfigChanged)
        {

          StateGestUser=CONFIGREM_WAIT_CONFERMA_EXIT;

          ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
          ConfigRemotaPutString(ConfigRemotaLineOut);


          ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"USCIRE SENZA SALVARE (S=Si,N=No,A=ANNULLA)?\r\n");
          ConfigRemotaPutString(ConfigRemotaLineOut);

        }
        else StateGestUser=CONFIGREM_GESTUSER_EXIT;

        return;
     }



    if (strcasecmp(str,"M")==0)                                                             // Inizio Configurazione
    {
        ConfigRemotaPrintMenu();
        return;
    }
    strcpy(CONFIGREM_CAMPO1,strs);




}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
void ConfigRem::ConfigRemotaPrint(TConfiguration *p) //8 pri
{
    quint8 ID;

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"CONFIGURAZIONE :\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);

    for(ID=1;ID<=ID_MAX_PARAM;ID++)
    {

        if(strcmp(STR_NO,(char *) Configuration.ConfigGetParamName(ID))!=0 )
        {
            ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%s: %s\r\n", Configuration.ConfigGetParamName(ID), Configuration.ConfigGetParamValue(ID));
            ConfigRemotaPutString(ConfigRemotaLineOut);
        }
    }

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);
    ConfigRemotaPrintSubMenu();


}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigRem::ConfigRemotaShowParam(quint8 ID) //9 pri
{
    if(strcmp(STR_NO,(char *) Configuration.ConfigGetParamName(ID))!=0 )
    {

        ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%s: %s\r\n", Configuration.ConfigGetParamName(ID), Configuration.ConfigGetParamValue(ID));
        ConfigRemotaPutString(ConfigRemotaLineOut);
        return(true);

    }
    return(false);
}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigRem::ConfigRemotaInsShowParam(quint8 newID,quint8 verso) //10 pri
{

    if (newID)ParamID = Configuration.ConfigGetParamID(ParamID,verso);


    if(ParamID==0)
    {
      ParamID =ID_IDENTIFICAZIONE;
      ParamID = Configuration.ConfigGetParamID(ParamID,SUCCESSIVO);
    }

    if (ParamID > ID_MAX_PARAM)return (false);

    ConfigRemotaShowParam(ParamID);

    return(true);


}

/**
  * @brief
  *
  * @param
  * @note
  * @retval
  */
quint8 ConfigRem::ConfigRemotaSave(void) //11 pri
{
    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"%c[2J", ASCII_ESC);
    ConfigRemotaPutString(ConfigRemotaLineOut);

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"ATTENDERE, SCRITTURA CONFIGURAZIONE IN CORSO.....\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);
    Configuration.Save();
    Configuration.Init();
    Configuration.SetOperation();

    ConfigRemotaLineOutCount=sprintf(ConfigRemotaLineOut,"CONFIGURAZIONE AVVENUTA\r\n");
    ConfigRemotaPutString(ConfigRemotaLineOut);
    ConfigRemotaPrintSubMenu();

    return(true);
}
