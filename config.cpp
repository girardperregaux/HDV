#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>
#include "utility.h"
#include "mcmodem.h"
#include "tstate.h"




extern MCSerial SerialPrint;
extern char buff_print[];

extern MCModem Modem;


//#define CHR_OVIZZATA 'v'

#define   SKIP                         2
#define STR_EMPTY	"<EMPTY>"


TConfig Configuration;

char strname[MAX_LEN_SMS+1];
char strvalue[MAX_LEN_CAMPO+1];



TConfig::TConfig()
{
    strcpy(Config.tipo_centrale1,    STR_NO);
    strcpy(Config.codice_centrale1,  STR_NO);
    strcpy(Config.codice_gruppo,     STR_NO);
    strcpy(Config.codice_periferica, STR_NO);

}


void TConfig::Init()
{
    quint8 op_result=OP_CONF1_KO;
    TConfiguration TmpConfig;

    switch(Read(&TmpConfig,(char*)STR_NOME_FILE_CNF_1))
    {
        case READ_RESULT_OK :
            op_result=OP_CONF1_OK;
        break;

        case READ_RESULT_FILE_NOT_FOUND :
        case READ_RESULT_CRC_ERROR :
        case READ_RESULT_NOT_OPEN :
            op_result=OP_CONF1_KO;
        break;
    }

    switch(Read(&TmpConfig,(char*)STR_NOME_FILE_CNF_2))
    {

        case READ_RESULT_OK :
            if(op_result==OP_CONF1_KO)op_result=OP_RECOVERY_FROM_CNF2;  //recupero CNF1 da CNF2
        break;
        case READ_RESULT_FILE_NOT_FOUND :
        case READ_RESULT_CRC_ERROR :
        case READ_RESULT_NOT_OPEN :
            if(op_result==OP_CONF1_OK) op_result=OP_RECOVERY_FROM_CNF1;  //recupero CNF2 da CNF1
            else op_result=OP_OVIZZA;
    }

    switch(op_result)
    {

        case OP_CONF1_OK :
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: OK\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
            memcpy((char *)&Config,(char *)&TmpConfig,sizeof(TConfiguration));
        break;
        case OP_CONF1_KO :
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: OP_CONF1_KO NON GESTITO !!!!!!!!!!\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        break;
        case OP_OVIZZA :
            Ovizza();
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: OP_OVIZZA\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);
            Read(&TmpConfig,(char*)STR_NOME_FILE_CNF_2);
            memcpy((char *)&Config,(char *)&TmpConfig,sizeof(TConfiguration));
            OvizzaState();

        break;
        case OP_RECOVERY_FROM_CNF1 :
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: RECUPERO CNF2 DA CNF1\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);

            Read(&ConfigMod,(char*)STR_NOME_FILE_CNF_1);
            Save();
        break;
        case OP_RECOVERY_FROM_CNF2 :
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: RECUPERO CNF1 DA CNF2\r\n");
            SerialPrint.Flush(PRINT_DEBUG_ALL);

            Read(&ConfigMod,(char*)STR_NOME_FILE_CNF_2);
            Save();
        break;

    }




}



void TConfig::InsMode()
{
    quint8 op_result=OP_CONF1_KO;

    switch(Read(&ConfigMod,(char*)STR_NOME_FILE_CNF_1))
    {
        case READ_RESULT_OK :
            op_result=OP_CONF1_OK;
        break;

        case READ_RESULT_FILE_NOT_FOUND :
        case READ_RESULT_CRC_ERROR :
        case READ_RESULT_NOT_OPEN :
            op_result=OP_CONF1_KO;
        break;
    }

    if(op_result==OP_CONF1_KO)
    {
        switch(Read(&ConfigMod,(char*)(char*)STR_NOME_FILE_CNF_2))
        {

            case READ_RESULT_OK :
                op_result=OP_CONF2_OK;
            break;
            case READ_RESULT_FILE_NOT_FOUND :
            case READ_RESULT_CRC_ERROR :
            case READ_RESULT_NOT_OPEN :
            break;
        }
    }
}




void TConfig::PrintObj()
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"->%s\r\n",Config.tipo_centrale1);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"->%s\r\n",Config.codice_centrale1);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"->%s\r\n",Config.codice_gruppo);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"->%s\r\n",Config.codice_periferica);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"->%d\r\n",Config.qualita_video_salvato);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"->%d\r\n",Config.qualita_video_stream);
    SerialPrint.Flush(PRINT_DEBUG_ALL);


}

bool TConfig::Save()
{
    QFile file1(STR_NOME_FILE_CNF_1);
    QFile file2(STR_NOME_FILE_CNF_2);

    quint8 *p;

    p=(quint8 *)&ConfigMod;


    if(ConfigMod.Ovizzata!=CHR_OVIZZATA)
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: NON OVIZZATA!!!!!\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        return(false);

    }



    ConfigMod.CRCc=CRCCCITT(0, CRC_FLAG_INREV | CRC_FLAG_OUTREV,(quint8 *)p, sizeof(ConfigMod)-sizeof(quint16));

    if(file1.open(QFile::WriteOnly))
    {
        file1.write( ( char * )( &ConfigMod ), sizeof(ConfigMod) );
    }
    else
    {

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: OVIZZA ERRORE OPEN FILE %s\r\n",STR_NOME_FILE_CNF_1);
        SerialPrint.Flush(PRINT_DEBUG_ALL);

    }
    file1.close();

    if(file2.open(QFile::WriteOnly))
    {
        file2.write( ( char * )( &ConfigMod ), sizeof(ConfigMod) );
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: OVIZZA ERRORE OPEN FILE %s\r\n",STR_NOME_FILE_CNF_2);
        SerialPrint.Flush(PRINT_DEBUG_ALL);

    }


    file2.close();
    Modem.ReStart();
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: SAVE!!!!!\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);


    SetOperation();
    return(true);

}


void TConfig::SetOperation(void)
{
    SetSveglia(SV_HOUR,5);
}
/*
 *
 *
 *
 *
 * result:
 *  READ_RESULT_OK
 *  READ_RESULT_FILE_NOT_FOUND
 *  READ_RESULT_CRC_ERROR
 *  READ_RESULT_NOT_OPEN
*/

quint8 TConfig::Read(TConfiguration *pCONFIGURATION, char *nome_file)
{
    QFile file(nome_file);
    char *p;
    p=(char *)pCONFIGURATION;
    quint16 Crc=0;

    if(!file.exists())return(READ_RESULT_FILE_NOT_FOUND);

    if(file.open(QFile::ReadOnly))
    {
        file.read(p, sizeof(TConfiguration));
    }
    else
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: READ ERRORE OPEN FILE %s\r\n",nome_file);
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        return(READ_RESULT_NOT_OPEN);
    }

    file.close();

    Crc=CRCCCITT(0, CRC_FLAG_INREV | CRC_FLAG_OUTREV,(quint8 *)p, sizeof(TConfiguration)-sizeof(quint16));

    if(pCONFIGURATION->CRCc==Crc)
    {
        if(strcmp(pCONFIGURATION->versione_configurazione,VERSIONE_FW)!=0)
        {
             return(READ_RESULT_CRC_ERROR);
        }
        else
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CNF: READ OK %c\r\n",pCONFIGURATION->Ovizzata);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);


            return(READ_RESULT_OK);
         }
    }
    else
    {
        return(READ_RESULT_CRC_ERROR);
    }

}



void TConfig::Ovizza()
{
    quint16 ID;

    ConfigMod.Ovizzata=CHR_OVIZZATA;

    strcpy(ConfigMod.versione_configurazione,VERSIONE_FW);

    for(ID=0;ID<=ID_MAX_PARAM;ID++)
    {
        ConfigParamDefault(ID);
    }

    Save();
}

char * TConfig::ConfigGetParamName(quint16 ID)
{
    switch (ID)
    {
        case ID_TIPO_CENTRALE1    : if (strcmp(STR_TIPO_CENTRALE1,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_TIPO_CENTRALE1,STR_TIPO_CENTRALE1);     break;
        case ID_CODICE_CENTRALE1  : if (strcmp(STR_CODICE_CENTRALE1,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_CODICE_CENTRALE1,STR_CODICE_CENTRALE1);     break;
        case ID_CODICE_GRUPPO     : if (strcmp(STR_CODICE_GRUPPO,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_CODICE_GRUPPO,STR_CODICE_GRUPPO);     break;
        case ID_CODICE_PERIFERICA : if (strcmp(STR_CODICE_PERIFERICA,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_CODICE_PERIFERICA,STR_CODICE_PERIFERICA);     break;
        case ID_NUM_DATI_CENTRALE1: if (strcmp(STR_NUM_DATI_CENTRALE1,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_NUM_DATI_CENTRALE1,STR_NUM_DATI_CENTRALE1);     break;
        case ID_NUM_SMS_CENTRALE1 : if (strcmp(STR_NUM_SMS_CENTRALE1,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_NUM_SMS_CENTRALE1,STR_NUM_SMS_CENTRALE1);     break;
////////
    case ID_NUM_TEL_USER1     : if (strcmp(STR_NUM_TEL_USER1,STR_NO)==0)strcpy(strname,STR_NO);
                                else sprintf (strname,"%03d-%s",ID_NUM_TEL_USER1,STR_NUM_TEL_USER1);     break;
        case ID_IP_APN        : if (strcmp(STR_IP_APN,STR_NO)==0)strcpy(strname,STR_NO);
                                else sprintf (strname,"%03d-%s",ID_IP_APN,STR_IP_APN);     break;
        case ID_NETWORK_SELECT: if (strcmp(STR_NETWORK_SELECT,STR_NO)==0)strcpy(strname,STR_NO);
                                else sprintf (strname,"%03d-%s",ID_NETWORK_SELECT,STR_NETWORK_SELECT);     break;


////////
        case ID_QUALITA_VIDEO_FILE: if (strcmp(STR_QUALITA_VIDEO_FILE,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_QUALITA_VIDEO_FILE,STR_QUALITA_VIDEO_FILE);     break;
        case ID_QUANT_VIDEO_FILE  : if (strcmp(STR_QUANT_VIDEO_FILE,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_QUANT_VIDEO_FILE,STR_QUANT_VIDEO_FILE);     break;
        case ID_FRAME_VIDEO_FILE  : if (strcmp(STR_FRAME_VIDEO_FILE,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_FRAME_VIDEO_FILE,STR_FRAME_VIDEO_FILE);     break;
        case ID_TRANSCODER        : if (strcmp(STR_TRANSCODER,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_TRANSCODER,STR_TRANSCODER);     break;
        case ID_LEN_REC           : if (strcmp(STR_LEN_REC,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_LEN_REC,STR_LEN_REC);     break;
        case ID_BITRATE_AUDIO     : if (strcmp(STR_BITRATE_AUDIO,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_BITRATE_AUDIO,STR_BITRATE_AUDIO);     break;



////////
        case ID_QUALITA_VIDEO_STREAM : if (strcmp(STR_QUALITA_VIDEO_STREAM,STR_NO)==0)strcpy(strname,STR_NO);
                                       else sprintf (strname,"%03d-%s",ID_QUALITA_VIDEO_STREAM,STR_QUALITA_VIDEO_STREAM);     break;
        case ID_QUANT_VIDEO_STREAM   : if (strcmp(STR_QUANT_VIDEO_STREAM,STR_NO)==0)strcpy(strname,STR_NO);
                                       else sprintf (strname,"%03d-%s",ID_QUANT_VIDEO_STREAM,STR_QUANT_VIDEO_STREAM);     break;

        case ID_FRAME_VIDEO_STREAM   : if (strcmp(STR_FRAME_VIDEO_STREAM,STR_NO)==0)strcpy(strname,STR_NO);
                                       else sprintf (strname,"%03d-%s",ID_FRAME_VIDEO_STREAM,STR_FRAME_VIDEO_STREAM);     break;

        case ID_MODO_DI_FUNZIONAMENTO: if (strcmp(STR_MODO_DI_FUNZIONAMENTO,STR_NO)==0)strcpy(strname,STR_NO);
                                       else sprintf (strname,"%03d-%s",ID_MODO_DI_FUNZIONAMENTO,STR_MODO_DI_FUNZIONAMENTO);     break;

        case ID_IP_SERVER_VIDEO      : if (strcmp(STR_IP_SERVER_VIDEO,STR_NO)==0)strcpy(strname,STR_NO);
                                       else sprintf (strname,"%03d-%s",ID_IP_SERVER_VIDEO,STR_IP_SERVER_VIDEO);     break;

        case ID_PORTA_SERVER_VIDEO   : if (strcmp(STR_PORTA_SERVER_VIDEO,STR_NO)==0)strcpy(strname,STR_NO);
                                       else sprintf (strname,"%03d-%s",ID_PORTA_SERVER_VIDEO,STR_PORTA_SERVER_VIDEO);     break;

        case ID_PORTA_SERVER_RTP     : if (strcmp(STR_PORTA_SERVER_RTP,STR_NO)==0)strcpy(strname,STR_NO);
                                       else sprintf (strname,"%03d-%s",ID_PORTA_SERVER_RTP,STR_PORTA_SERVER_RTP);     break;


////////
        case ID_IP_SERVER_FTP      : if (strcmp(STR_IP_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_IP_SERVER_FTP,STR_IP_SERVER_FTP);     break;

        case ID_LINK_SERVER_FTP    : if (strcmp(STR_LINK_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_LINK_SERVER_FTP,STR_LINK_SERVER_FTP);     break;

        case ID_USERNAME_SERVER_FTP: if (strcmp(STR_USERNAME_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_USERNAME_SERVER_FTP,STR_USERNAME_SERVER_FTP);     break;

        case ID_PASSWORD_SERVER_FTP: if (strcmp(STR_PASSWORD_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_PASSWORD_SERVER_FTP,STR_PASSWORD_SERVER_FTP);     break;

        case ID_ACCOUNT_SERVER_FTP : if (strcmp(STR_ACCOUNT_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_ACCOUNT_SERVER_FTP,STR_ACCOUNT_SERVER_FTP);     break;

        case ID_PORTA_SERVER_FTP   : if (strcmp(STR_PORTA_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_PORTA_SERVER_FTP,STR_PORTA_SERVER_FTP);     break;

        case ID_SSL_SERVER_FTP     : if (strcmp(STR_SSL_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_SSL_SERVER_FTP,STR_SSL_SERVER_FTP);     break;

        case ID_TIMEOUT_SERVER_FTP : if (strcmp(STR_TIMEOUT_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_TIMEOUT_SERVER_FTP,STR_TIMEOUT_SERVER_FTP);     break;

        case ID_FOLDER_SERVER_FTP  : if (strcmp(STR_FOLDER_SERVER_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_FOLDER_SERVER_FTP,STR_FOLDER_SERVER_FTP);     break;

        case ID_CANC_FILE_FTP      : if (strcmp(STR_CANC_FILE_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_CANC_FILE_FTP,STR_CANC_FILE_FTP);     break;

        case ID_ORDINEO_FTP        : if (strcmp(STR_ORDINEO_FTP,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_ORDINEO_FTP,STR_ORDINEO_FTP);     break;

////////

        case ID_TEMPO_RITENUTA_VOX: if (strcmp(STR_TEMPO_RITENUTA_VOX,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_TEMPO_RITENUTA_VOX,STR_TEMPO_RITENUTA_VOX);     break;

        case ID_SENSIBILITA_VOX   : if (strcmp(STR_SENSIBILITA_VOX,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_SENSIBILITA_VOX,STR_SENSIBILITA_VOX);     break;

        case ID_TEMPO_SOSTA       : if (strcmp(STR_TEMPO_SOSTA,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_TEMPO_SOSTA,STR_TEMPO_SOSTA);     break;

        case ID_SENSIBILITA_MOV   : if (strcmp(STR_SENSIBILITA_MOV,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_SENSIBILITA_MOV,STR_SENSIBILITA_MOV);     break;

        case ID_FUNZIONE_VOX      : if (strcmp(STR_FUNZIONE_VOX,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_FUNZIONE_VOX,STR_FUNZIONE_VOX);     break;

        case ID_FUNZIONE_MOV      : if (strcmp(STR_FUNZIONE_MOV,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_FUNZIONE_MOV,STR_FUNZIONE_MOV);     break;

        case ID_TEMP_FAN_ON       : if (strcmp(STR_TEMP_FAN_ON,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_TEMP_FAN_ON,STR_TEMP_FAN_ON);     break;

        case ID_TEMP_FAN_OFF      : if (strcmp(STR_TEMP_FAN_OFF,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_TEMP_FAN_OFF,STR_TEMP_FAN_OFF);     break;

        case ID_REMLINK_ADDRESS   : if (strcmp(STR_REMLINK_ADDRESS,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_REMLINK_ADDRESS,STR_REMLINK_ADDRESS);     break;

        case ID_REMLINK_PORT      : if (strcmp(STR_REMLINK_PORT,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_REMLINK_PORT,STR_REMLINK_PORT);     break;

        case ID_AUDIO_CHANNEL     : if (strcmp(STR_AUDIO_CHANNEL,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_AUDIO_CHANNEL,STR_AUDIO_CHANNEL);     break;

        case ID_VOLUME_AUDIO      : if (strcmp(STR_VOLUME_AUDIO,STR_NO)==0)strcpy(strname,STR_NO);
                                    else sprintf (strname,"%03d-%s",ID_VOLUME_AUDIO,STR_VOLUME_AUDIO);     break;

        case ID_MODO_SCARICO      : if (strcmp(STR_MODO_SCARICO,STR_NO)==0)strcpy(strname,STR_NO);
                                     else sprintf (strname,"%03d-%s",ID_MODO_SCARICO,STR_MODO_SCARICO);     break;

        //fusi begin
        case ID_GPS_FILTRO_POS_IN_SOSTA      : if (strcmp(STR_GPS_FILTRO_POS_IN_SOSTA,STR_NO)==0)strcpy(strname,STR_NO);
                                               else sprintf (strname,"%03d-%s",ID_GPS_FILTRO_POS_IN_SOSTA, STR_GPS_FILTRO_POS_IN_SOSTA);     break;

        case ID_GPS_ABILITA_GLONASS          : if (strcmp(STR_GPS_ABILITA_GLONASS,STR_NO)==0)strcpy(strname,STR_NO);
                                               else sprintf (strname,"%03d-%s",ID_GPS_ABILITA_GLONASS, STR_GPS_ABILITA_GLONASS);     break;

        case ID_GPS_ABILITA_EGNOS            : if (strcmp(STR_GPS_ABILITA_EGNOS,STR_NO)==0)strcpy(strname,STR_NO);
                                               else sprintf (strname,"%03d-%s",ID_GPS_ABILITA_EGNOS, STR_GPS_ABILITA_EGNOS);     break;

        case ID_GPS_MODALITA_AGPS            : if (strcmp(STR_GPS_MODALITA_AGPS,STR_NO)==0)strcpy(strname,STR_NO);
                                               else sprintf (strname,"%03d-%s",ID_GPS_MODALITA_AGPS, STR_GPS_MODALITA_AGPS);     break;

        //fusi end



        default:
            strcpy(strname,STR_NO);
        break;



    }
    return(strname);

}


quint16 TConfig::ConfigGetParamID(quint16 ID,quint8 verso)
{
  if(verso==SUCCESSIVO)
  {
    for (; ID <= ID_MAX_PARAM; ID++)
    if (strcmp(ConfigGetParamName(ID),STR_NO)!=0) break;
  }
  else
  {
    for (; ID > ID_NONE; ID--)
    if (strcmp(ConfigGetParamName(ID),STR_NO)!=0) break;

  }

  return(ID);
}

void TConfig::PrintHelpConfig(void)
{
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nModalita' CONFIG\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nDIGITARE:\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"? : per visualizza questo help\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"I : per modificare i parametri\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"O : per impostare la configurazione ai parametri di default\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"E : visualizzare la configurazione\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"<MONITOR> : per passare alla modalita' monitor\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nDURANTE L'INSERIMENTO DEI VALORI PREMERE:\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"<INVIO> : per confermare il valore del parametro e passare al successivo\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"<FRECCIA SU>: per passare al parametro precedente\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"<ESC> <ESC>: per abort configurazione\r\n\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
}

char * TConfig::ConfigGetParamValue(quint16 ID)
{
    strcpy(strvalue,STR_NO);
    switch(ID)
    {
        case ID_TIPO_CENTRALE1    : strcpy(strvalue,ConfigMod.tipo_centrale1);     break;
        case ID_CODICE_CENTRALE1  : strcpy(strvalue,ConfigMod.codice_centrale1);     break;
        case ID_CODICE_GRUPPO     : strcpy(strvalue,ConfigMod.codice_gruppo);     break;
        case ID_CODICE_PERIFERICA : strcpy(strvalue,ConfigMod.codice_periferica);     break;
        case ID_NUM_DATI_CENTRALE1: strcpy(strvalue,ConfigMod.num_dati_centrale1);     break;
        case ID_NUM_SMS_CENTRALE1 : strcpy(strvalue,ConfigMod.num_sms_centrale1);     break;

///////
        case ID_NUM_TEL_USER1 : strcpy(strvalue,ConfigMod.num_telefono_user1);     break;
        case ID_IP_APN        : strcpy(strvalue,ConfigMod.APN);     break;
        case ID_NETWORK_SELECT: sprintf(strvalue,"%d",ConfigMod.network_select);     break;

///////
        case ID_QUALITA_VIDEO_FILE: sprintf(strvalue,"%d",ConfigMod.qualita_video_salvato); break;
        case ID_QUANT_VIDEO_FILE  : sprintf(strvalue,"%d",ConfigMod.quant_video_file); break;
        case ID_FRAME_VIDEO_FILE  : sprintf(strvalue,"%d",ConfigMod.frame_video_file); break;

        case ID_TRANSCODER: sprintf(strvalue,"%d",ConfigMod.transcoder); break;

        case ID_LEN_REC      : sprintf(strvalue,"%d",ConfigMod.tempo_per_file_rec);break;
        case ID_BITRATE_AUDIO: sprintf(strvalue,"%d",ConfigMod.bitrate_audio); break;

///////
        case ID_QUALITA_VIDEO_STREAM : sprintf(strvalue,"%d",ConfigMod.qualita_video_stream);break;
        case ID_QUANT_VIDEO_STREAM   : sprintf(strvalue,"%d",ConfigMod.quant_video_stream);break;
        case ID_FRAME_VIDEO_STREAM   : sprintf(strvalue,"%d",ConfigMod.frame_video_stream);break;
        case ID_MODO_DI_FUNZIONAMENTO: sprintf(strvalue,"%d",ConfigMod.modo_di_funzionamento);break;
        case ID_IP_SERVER_VIDEO      : strcpy(strvalue,ConfigMod.ip_server_video);break;
        case ID_PORTA_SERVER_VIDEO   : strcpy(strvalue,ConfigMod.port_server_video);break;
        case ID_PORTA_SERVER_RTP     : strcpy(strvalue,ConfigMod.port_server_rtp);break;


///////


        case ID_IP_SERVER_FTP       : strcpy(strvalue,ConfigMod.ip_server_ftp);     break;
        case ID_LINK_SERVER_FTP     : strcpy(strvalue,ConfigMod.link_server_ftp);     break;
        case ID_USERNAME_SERVER_FTP : strcpy(strvalue,ConfigMod.username_server_ftp);     break;
        case ID_PASSWORD_SERVER_FTP : strcpy(strvalue,ConfigMod.password_server_ftp);     break;
        case ID_ACCOUNT_SERVER_FTP  : strcpy(strvalue,ConfigMod.account_server_ftp);     break;
        case ID_PORTA_SERVER_FTP    : strcpy(strvalue,ConfigMod.porta_server_ftp);     break;
        case ID_SSL_SERVER_FTP      : sprintf(strvalue,"%d",ConfigMod.SSL_server_ftp);     break;
        case ID_TIMEOUT_SERVER_FTP  : sprintf(strvalue,"%d",ConfigMod.timeout_server_ftp);     break;
        case ID_FOLDER_SERVER_FTP   : sprintf(strvalue,ConfigMod.folder_server_ftp);     break;
        case ID_CANC_FILE_FTP       : sprintf(strvalue,"%d",ConfigMod.canc_dopo_trasferimento);     break;
        case ID_ORDINEO_FTP         : sprintf(strvalue,"%d",ConfigMod.ordine_ottimizzato);     break;


//////
        case ID_TEMPO_RITENUTA_VOX: sprintf(strvalue,"%d",ConfigMod.tempo_ritenuta_vox);    break;
        case ID_SENSIBILITA_VOX   : sprintf(strvalue,"%d",ConfigMod.sensibilita_vox);  break;
        case ID_TEMPO_SOSTA       : sprintf(strvalue,"%d",ConfigMod.tempo_di_sosta);break;
        case ID_SENSIBILITA_MOV   : sprintf(strvalue,"%d",ConfigMod.sensibilita_mov);break;
        case ID_FUNZIONE_VOX      : sprintf(strvalue,"%d",ConfigMod.funzione_vox);break;
        case ID_FUNZIONE_MOV      : sprintf(strvalue,"%d",ConfigMod.funzione_mov);break;
        case ID_TEMP_FAN_ON       : sprintf(strvalue,"%d",ConfigMod.temp_fan_on);break;
        case ID_TEMP_FAN_OFF      : sprintf(strvalue,"%d",ConfigMod.temp_fan_off);break;



        case ID_REMLINK_ADDRESS: strcpy(strvalue,ConfigMod.remlink_address);     break;
        case ID_REMLINK_PORT   : strcpy(strvalue,ConfigMod.remlink_port);break;
        case ID_AUDIO_CHANNEL  : sprintf(strvalue,"%d",ConfigMod.audio_channel);break;
        case ID_VOLUME_AUDIO   : sprintf(strvalue,"%d",ConfigMod.volume_audio);break;
        case ID_MODO_SCARICO   : sprintf(strvalue,"%d",ConfigMod.modo_scarico);break;

        //fusi begin
        case ID_GPS_FILTRO_POS_IN_SOSTA      : sprintf(strvalue,"%d",ConfigMod.gps_filtro_pos_in_sosta);break;
        case ID_GPS_ABILITA_GLONASS          : sprintf(strvalue,"%d",ConfigMod.gps_abilita_glonass);break;
        case ID_GPS_ABILITA_EGNOS            : sprintf(strvalue,"%d",ConfigMod.gps_abilita_egnos);break;
        case ID_GPS_MODALITA_AGPS            : sprintf(strvalue,"%d",ConfigMod.gps_modalita_agps);break;

        //fusi end

        default:
        break;

    }
    return(strvalue);

}

char * TConfig::ConfigGetActualParamValue(quint16 ID)
{
    strcpy(strvalue,STR_NO);

    switch(ID)
    {
        case ID_TIPO_CENTRALE1    : strcpy(strvalue,Config.tipo_centrale1);     break;
        case ID_CODICE_CENTRALE1  : strcpy(strvalue,Config.codice_centrale1);     break;
        case ID_CODICE_GRUPPO     : strcpy(strvalue,Config.codice_gruppo);     break;
        case ID_CODICE_PERIFERICA : strcpy(strvalue,Config.codice_periferica);     break;
        case ID_NUM_DATI_CENTRALE1: strcpy(strvalue,Config.num_dati_centrale1);     break;
        case ID_NUM_SMS_CENTRALE1 : strcpy(strvalue,Config.num_sms_centrale1);     break;

    ///////
        case ID_NUM_TEL_USER1 : strcpy(strvalue,Config.num_telefono_user1);     break;
        case ID_IP_APN        : strcpy(strvalue,Config.APN);     break;
        case ID_NETWORK_SELECT: sprintf(strvalue,"%d",Config.network_select);     break;

    ///////
        case ID_QUALITA_VIDEO_FILE: sprintf(strvalue,"%d",Config.qualita_video_salvato); break;
        case ID_QUANT_VIDEO_FILE  : sprintf(strvalue,"%d",Config.quant_video_file); break;
        case ID_FRAME_VIDEO_FILE  : sprintf(strvalue,"%d",Config.frame_video_file); break;

        case ID_TRANSCODER: sprintf(strvalue,"%d",Config.transcoder); break;
        case ID_LEN_REC   : sprintf(strvalue,"%d",Config.tempo_per_file_rec);break;
    case ID_BITRATE_AUDIO : sprintf(strvalue,"%d",Config.bitrate_audio); break;

    ///////

        case ID_QUALITA_VIDEO_STREAM: sprintf(strvalue,"%d",Config.qualita_video_stream);break;
        case ID_QUANT_VIDEO_STREAM  : sprintf(strvalue,"%d",Config.quant_video_stream);break;
        case ID_FRAME_VIDEO_STREAM  : sprintf(strvalue,"%d",Config.frame_video_stream);break;

        case ID_MODO_DI_FUNZIONAMENTO: sprintf(strvalue,"%d",Config.modo_di_funzionamento);break;

        case ID_IP_SERVER_VIDEO   : strcpy(strvalue,Config.ip_server_video);break;
        case ID_PORTA_SERVER_VIDEO: strcpy(strvalue,Config.port_server_video);break;
        case ID_PORTA_SERVER_RTP  : strcpy(strvalue,Config.port_server_rtp);break;


    ///////


        case ID_IP_SERVER_FTP       : strcpy(strvalue,Config.ip_server_ftp);     break;
        case ID_LINK_SERVER_FTP     : strcpy(strvalue,Config.link_server_ftp);     break;
        case ID_USERNAME_SERVER_FTP : strcpy(strvalue,Config.username_server_ftp);     break;
        case ID_PASSWORD_SERVER_FTP : strcpy(strvalue,Config.password_server_ftp);     break;
        case ID_ACCOUNT_SERVER_FTP  : strcpy(strvalue,Config.account_server_ftp);     break;
        case ID_PORTA_SERVER_FTP    : strcpy(strvalue,Config.porta_server_ftp);     break;
        case ID_SSL_SERVER_FTP      : sprintf(strvalue,"%d",Config.SSL_server_ftp);     break;
        case ID_TIMEOUT_SERVER_FTP  : sprintf(strvalue,"%d",Config.timeout_server_ftp);     break;
        case ID_FOLDER_SERVER_FTP   : sprintf(strvalue,Config.folder_server_ftp);     break;
        case ID_CANC_FILE_FTP       : sprintf(strvalue,"%d",Config.canc_dopo_trasferimento);     break;
        case ID_ORDINEO_FTP         : sprintf(strvalue,"%d",Config.ordine_ottimizzato);     break;


    //////
        case ID_TEMPO_RITENUTA_VOX: sprintf(strvalue,"%d",Config.tempo_ritenuta_vox);    break;
        case ID_SENSIBILITA_VOX   : sprintf(strvalue,"%d",Config.sensibilita_vox);  break;
        case ID_TEMPO_SOSTA       : sprintf(strvalue,"%d",Config.tempo_di_sosta);break;
        case ID_SENSIBILITA_MOV   : sprintf(strvalue,"%d",Config.sensibilita_mov);break;
        case ID_FUNZIONE_VOX      : sprintf(strvalue,"%d",Config.funzione_vox);break;
        case ID_FUNZIONE_MOV      : sprintf(strvalue,"%d",Config.funzione_mov);break;
        case ID_TEMP_FAN_ON       : sprintf(strvalue,"%d",Config.temp_fan_on);break;
        case ID_TEMP_FAN_OFF      : sprintf(strvalue,"%d",Config.temp_fan_off);break;


        case ID_REMLINK_ADDRESS: strcpy(strvalue,Config.remlink_address);     break;
        case ID_REMLINK_PORT   : strcpy(strvalue,Config.remlink_port);break;
        case ID_AUDIO_CHANNEL  : sprintf(strvalue,"%d",Config.audio_channel);break;
        case ID_VOLUME_AUDIO   : sprintf(strvalue,"%d",Config.volume_audio);break;
        case ID_MODO_SCARICO   : sprintf(strvalue,"%d",Config.modo_scarico);break;

        //fusi begin
        case ID_GPS_FILTRO_POS_IN_SOSTA       : sprintf(strvalue,"%d",ConfigMod.gps_filtro_pos_in_sosta); break;
        case ID_GPS_ABILITA_GLONASS          : sprintf(strvalue,"%d",ConfigMod.gps_abilita_glonass); break;
        case ID_GPS_ABILITA_EGNOS            : sprintf(strvalue,"%d",ConfigMod.gps_abilita_egnos); break;
        case ID_GPS_MODALITA_AGPS            : sprintf(strvalue,"%d",ConfigMod.gps_modalita_agps); break;
        //fusi end

        default:
        break;

    }
    return(strvalue);

}


bool TConfig::ConfigWriteParamSet(quint16 ID, char *strvalue)
{
    InsMode();

    if(ConfigParamSet(ID,strvalue))
    {
        Save();
        Init();
       SetOperation();
        return(true);
    }

    return(false);
}



bool TConfig::ConfigParamSet(quint16 ID, char *strvalue)
{
    if(!ParamCheck (ID, strvalue))return(false);


    if(strcmp(strvalue,"*")==0)
    {
        ConfigParamDefault(ID);
        return(true);
    }
    else
    {
       switch (ID)
       {
        case ID_TIPO_CENTRALE1      :   strcpy (ConfigMod.tipo_centrale1,strvalue);     break;
        case ID_CODICE_CENTRALE1    :
                                        if(strcmp(strvalue,DEF_CODICE_CENTRALE1)==0)break;
                                        else strcpy (ConfigMod.codice_centrale1,strvalue);
                                        break;
        case ID_CODICE_GRUPPO       :
                                        if(strcmp(strvalue,DEF_CODICE_GRUPPO)==0)break;
                                        else strcpy (ConfigMod.codice_gruppo,strvalue);
                                        break;
        case ID_CODICE_PERIFERICA   :
                                        if(strcmp(strvalue,DEF_CODICE_PERIFERICA)==0)break;
                                        else strcpy (ConfigMod.codice_periferica,strvalue);
                                        break;

        case ID_NUM_DATI_CENTRALE1  :  strcpy(ConfigMod.num_dati_centrale1,strvalue);     break;
        case ID_NUM_SMS_CENTRALE1   : strcpy(ConfigMod.num_sms_centrale1,strvalue);     break;



///////
        case ID_NUM_TEL_USER1 : strcpy(ConfigMod.num_telefono_user1,strvalue);     break;
        case ID_IP_APN        : strcpy(ConfigMod.APN,strvalue);     break;
        case ID_NETWORK_SELECT: ConfigMod.network_select=atoi(strvalue);     break;

///////
        case ID_QUALITA_VIDEO_FILE: ConfigMod.qualita_video_salvato=atoi(strvalue); break;
        case ID_QUANT_VIDEO_FILE  : ConfigMod.quant_video_file=atoi(strvalue); break;

        case ID_FRAME_VIDEO_FILE  : ConfigMod.frame_video_file=atoi(strvalue);break;
        case ID_TRANSCODER        : ConfigMod.transcoder=atoi(strvalue);break;
        case ID_LEN_REC           : ConfigMod.tempo_per_file_rec=atoi(strvalue);break;
        case ID_BITRATE_AUDIO     : ConfigMod.bitrate_audio=atoi(strvalue);break;


///////
       case ID_QUALITA_VIDEO_STREAM : ConfigMod.qualita_video_stream=atoi(strvalue);break;
       case ID_QUANT_VIDEO_STREAM   : ConfigMod.quant_video_stream=atoi(strvalue);break;
       case ID_FRAME_VIDEO_STREAM   : ConfigMod.frame_video_stream=atoi(strvalue);break;
       case ID_MODO_DI_FUNZIONAMENTO: ConfigMod.modo_di_funzionamento=atoi(strvalue);break;

       case ID_IP_SERVER_VIDEO      : strcpy(ConfigMod.ip_server_video,strvalue);break;
       case ID_PORTA_SERVER_VIDEO   : strcpy(ConfigMod.port_server_video,strvalue);break;
       case ID_PORTA_SERVER_RTP     : strcpy(ConfigMod.port_server_rtp,strvalue);break;

///////
       case ID_IP_SERVER_FTP       : strcpy(ConfigMod.ip_server_ftp,strvalue);     break;
       case ID_LINK_SERVER_FTP     : strcpy(ConfigMod.link_server_ftp,strvalue);     break;
       case ID_USERNAME_SERVER_FTP : strcpy(ConfigMod.username_server_ftp,strvalue);     break;
       case ID_PASSWORD_SERVER_FTP : strcpy(ConfigMod.password_server_ftp,strvalue);     break;
       case ID_ACCOUNT_SERVER_FTP  : strcpy(ConfigMod.account_server_ftp,strvalue);     break;
       case ID_PORTA_SERVER_FTP    : strcpy(ConfigMod.porta_server_ftp,strvalue);     break;
       case ID_SSL_SERVER_FTP      : ConfigMod.SSL_server_ftp=atoi(strvalue);     break;
       case ID_TIMEOUT_SERVER_FTP  : ConfigMod.timeout_server_ftp=atoi(strvalue);     break;
       case ID_FOLDER_SERVER_FTP   : strcpy(ConfigMod.folder_server_ftp,strvalue);     break;
       case ID_CANC_FILE_FTP       : ConfigMod.canc_dopo_trasferimento=atoi(strvalue);     break;
       case ID_ORDINEO_FTP         : ConfigMod.ordine_ottimizzato=atoi(strvalue);     break;

//////
       case ID_TEMPO_RITENUTA_VOX: ConfigMod.tempo_ritenuta_vox=atoi(strvalue);    break;
       case ID_SENSIBILITA_VOX   : ConfigMod.sensibilita_vox=atoi(strvalue);  break;
       case ID_TEMPO_SOSTA       : ConfigMod.tempo_di_sosta=atoi(strvalue); break;
       case ID_SENSIBILITA_MOV   : ConfigMod.sensibilita_mov=atoi(strvalue);break;
       case ID_FUNZIONE_VOX      : ConfigMod.funzione_vox=atoi(strvalue); break;
       case ID_FUNZIONE_MOV      : ConfigMod.funzione_mov=atoi(strvalue); break;
       case ID_TEMP_FAN_ON       : ConfigMod.temp_fan_on=atoi(strvalue); break;
       case ID_TEMP_FAN_OFF      : ConfigMod.temp_fan_off=atoi(strvalue); break;

       case ID_REMLINK_ADDRESS   : strcpy(ConfigMod.remlink_address,strvalue);     break;
       case ID_REMLINK_PORT      : strcpy(ConfigMod.remlink_port,strvalue);break;

       case ID_AUDIO_CHANNEL     : ConfigMod.audio_channel=atoi(strvalue);break;
       case ID_VOLUME_AUDIO      : ConfigMod.volume_audio=atoi(strvalue);break;
       case ID_MODO_SCARICO      : ConfigMod.modo_scarico=atoi(strvalue);break;

       //fusi begin
       case ID_GPS_FILTRO_POS_IN_SOSTA      : ConfigMod.gps_filtro_pos_in_sosta = atoi(strvalue); break;
       case ID_GPS_ABILITA_GLONASS          : ConfigMod.gps_abilita_glonass = atoi(strvalue); break;
       case ID_GPS_ABILITA_EGNOS            : ConfigMod.gps_abilita_egnos = atoi(strvalue); break;
       case ID_GPS_MODALITA_AGPS            : ConfigMod.gps_modalita_agps = atoi(strvalue); break;
       //fusi end

      }
        return(true);
    }
    return(false);
}

bool TConfig:: ParamCheck(quint16 ID, char *strvalue)
{
    if(strcmp(strvalue,"*")==0)return(true);
//IsNumber(strvalue) && 1<=atoi(strvalue) &&  (atoi(strvalue)<=60)
    switch (ID)
    {

        case ID_TIPO_CENTRALE1      : if(strlen(strvalue)<=MAX_LEN_TIPO_CENTRALE1)return(true);
        case ID_NUM_SMS_CENTRALE1   : return((strlen(strvalue)<=MAX_LEN_CAMPO && IsTelephoneNumber(strvalue)) || (strcmp(strvalue,STR_NO)==0));     break;

///////
        case ID_CODICE_CENTRALE1    : return(strlen(strvalue)==6);
        case ID_CODICE_GRUPPO       : return(strlen(strvalue)==2);
        case ID_CODICE_PERIFERICA   : return(strlen(strvalue)==4);
        case ID_NUM_DATI_CENTRALE1  : return((strlen(strvalue)<=MAX_LEN_CAMPO && IsTelephoneNumber(strvalue)) || (strcmp(strvalue,STR_NO)==0));    break;
        case ID_NUM_TEL_USER1       : return((strlen(strvalue)<=MAX_LEN_CAMPO && IsTelephoneNumber(strvalue)) || (strcmp(strvalue,STR_NO)==0));      break;
        case ID_IP_APN              : return(strlen(strvalue)<=MAX_LEN_CAMPO);     break;
        case ID_NETWORK_SELECT      : return(MIN_NETWORK_SELECT<=atoi(strvalue) && atoi(strvalue)<=MAX_NETWORK_SELECT);     break;

///////
        case ID_QUALITA_VIDEO_FILE: return(MIN_QUALITA_VIDEO_FILE<=atoi(strvalue) && atoi(strvalue)<=MAX_QUALITA_VIDEO_FILE);
        case ID_QUANT_VIDEO_FILE  : return(MIN_QUANT_VIDEO_FILE<=atoi(strvalue) && atoi(strvalue)<=MAX_QUANT_VIDEO_FILE);
        case ID_FRAME_VIDEO_FILE  : return(MIN_FRAME_VIDEO_FILE<=atoi(strvalue) && atoi(strvalue)<=MAX_FRAME_VIDEO_FILE);
        case ID_TRANSCODER        : return(MIN_TRANSCODER<=atoi(strvalue) && atoi(strvalue)<=MAX_TRANSCODER);
        case ID_LEN_REC           : return(MIN_LEN_REC<=atoi(strvalue) && atoi(strvalue)<=MAX_LEN_REC);
        case ID_BITRATE_AUDIO     : return(MIN_BITRATE_AUDIO<=atoi(strvalue) && atoi(strvalue)<=MAX_BITRATE_AUDIO);

//////
        case ID_QUALITA_VIDEO_STREAM : return(MIN_QUALITA_VIDEO_STREAM<=atoi(strvalue) && atoi(strvalue)<=MAX_QUALITA_VIDEO_STREAM);
        case ID_QUANT_VIDEO_STREAM   : return(MIN_QUANT_VIDEO_STREAM<=atoi(strvalue) && atoi(strvalue)<=MAX_QUANT_VIDEO_STREAM);
        case ID_FRAME_VIDEO_STREAM   : return(MIN_FRAME_VIDEO_STREAM<=atoi(strvalue) && atoi(strvalue)<=MAX_FRAME_VIDEO_STREAM);
        case ID_MODO_DI_FUNZIONAMENTO: return(MIN_MODO_DI_FUNZIONAMENTO<=atoi(strvalue) && atoi(strvalue)<=MAX_MODO_DI_FUNZIONAMENTO);
        case ID_IP_SERVER_VIDEO      : return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_PORTA_SERVER_VIDEO   : return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_PORTA_SERVER_RTP     : return(strlen(strvalue)<=MAX_LEN_CAMPO);

//////

        case ID_IP_SERVER_FTP      : return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_LINK_SERVER_FTP    : return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_USERNAME_SERVER_FTP: return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_PASSWORD_SERVER_FTP: return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_ACCOUNT_SERVER_FTP : return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_PORTA_SERVER_FTP   : return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_SSL_SERVER_FTP     : return(MIN_SSL_SERVER_FTP<=atoi(strvalue) && atoi(strvalue)<=MAX_SSL_SERVER_FTP);
        case ID_TIMEOUT_SERVER_FTP : return( MIN_TIMEOUT_SERVER_FTP<=atoi(strvalue) && atoi(strvalue)<=MAX_TIMEOUT_SERVER_FTP);
        case ID_FOLDER_SERVER_FTP  : return(strlen(strvalue)<MAX_LEN_CAMPO);
        case ID_CANC_FILE_FTP      : return(MIN_CANC_FILE_FTP<=atoi(strvalue) && atoi(strvalue)<=MAX_CANC_FILE_FTP);
        case ID_ORDINEO_FTP        : return(MIN_ORDINEO_FTP<=atoi(strvalue) && atoi(strvalue)<=MAX_ORDINEO_FTP);

//////
        case ID_TEMPO_RITENUTA_VOX: return(IsNumber(strvalue) && MIN_TEMPO_RITENUTA_VOX<=atoi(strvalue) &&  (atoi(strvalue)<=MAX_TEMPO_RITENUTA_VOX));
        case ID_SENSIBILITA_VOX   : return(IsNumber(strvalue) && MIN_SENSIBILITA_VOX<=atoi(strvalue) && (atoi(strvalue)<=MAX_SENSIBILITA_VOX));
        case ID_TEMPO_SOSTA       : return(IsNumber(strvalue) && MIN_TEMPO_SOSTA<=atoi(strvalue) && (atoi(strvalue)<=MAX_TEMPO_SOSTA));
        case ID_SENSIBILITA_MOV   : return(IsNumber(strvalue) && MIN_SENSIBILITA_MOV<=atoi(strvalue) && (atoi(strvalue)<=MAX_SENSIBILITA_MOV));
        case ID_FUNZIONE_VOX      : return(IsNumber(strvalue) && MIN_FUNZIONE_VOX<=atoi(strvalue) && (atoi(strvalue)<=MAX_FUNZIONE_VOX));
        case ID_FUNZIONE_MOV      : return(IsNumber(strvalue) && MIN_FUNZIONE_MOV<=atoi(strvalue) && (atoi(strvalue)<=MAX_FUNZIONE_MOV));
        case ID_TEMP_FAN_ON       : return(IsNumber(strvalue) && MIN_TEMP_FAN_ON<=atoi(strvalue) && (atoi(strvalue)<=MAX_TEMP_FAN_ON));
        case ID_TEMP_FAN_OFF      : return(IsNumber(strvalue) && MIN_TEMP_FAN_OFF<=atoi(strvalue) && (atoi(strvalue)<=MAX_TEMP_FAN_OFF));

        case ID_REMLINK_ADDRESS: return(strlen(strvalue)<=MAX_LEN_CAMPO);
        case ID_REMLINK_PORT   : return(strlen(strvalue)<=MAX_LEN_CAMPO);

        case ID_AUDIO_CHANNEL: return(IsNumber(strvalue) && MIN_AUDIO_CHANNEL<=atoi(strvalue) && (atoi(strvalue)<=MAX_AUDIO_CHANNEL));
        case ID_VOLUME_AUDIO : return(IsNumber(strvalue) && MIN_VOLUME_AUDIO<=atoi(strvalue) && (atoi(strvalue)<=MAX_VOLUME_AUDIO));
        case ID_MODO_SCARICO : return(IsNumber(strvalue) && MIN_MODO_SCARICO<=atoi(strvalue) && (atoi(strvalue)<=MAX_MODO_SCARICO));

        //fusi begin
        case ID_GPS_FILTRO_POS_IN_SOSTA      : return(IsNumber(strvalue) && MIN_GPS_FILTRO_POS_IN_SOSTA<=atoi(strvalue) && (atoi(strvalue)<=MAX_GPS_FILTRO_POS_IN_SOSTA));
        case ID_GPS_ABILITA_GLONASS          : return(IsNumber(strvalue) && MIN_GPS_ABILITA_GLONASS<=atoi(strvalue) && (atoi(strvalue)<=MAX_GPS_ABILITA_GLONASS));
        case ID_GPS_ABILITA_EGNOS            : return(IsNumber(strvalue) && MIN_GPS_ABILITA_EGNOS<=atoi(strvalue) && (atoi(strvalue)<=MAX_GPS_ABILITA_EGNOS));
        case ID_GPS_MODALITA_AGPS            : return(IsNumber(strvalue) && MIN_GPS_MODALITA_AGPS<=atoi(strvalue) && (atoi(strvalue)<=MAX_GPS_MODALITA_AGPS));
        //fusi end

        default:
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"ParamCheck: ERRORE PARAMENTO %d NON TROVATO\r\n",ID);
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        break;

    }
    return(false);

}
void TConfig::ConfigParamDefault(quint16 ID)
{
    switch (ID)
    {
        case ID_TIPO_CENTRALE1   : strcpy (ConfigMod.tipo_centrale1,DEF_TIPO_CENTRALE1);     break;
        case ID_CODICE_CENTRALE1 : strcpy (ConfigMod.codice_centrale1,DEF_CODICE_CENTRALE1); break;
        case ID_CODICE_GRUPPO    : strcpy (ConfigMod.codice_gruppo,DEF_CODICE_GRUPPO);break;
        case ID_CODICE_PERIFERICA: strcpy (ConfigMod.codice_periferica,DEF_CODICE_PERIFERICA);

        case ID_NUM_DATI_CENTRALE1: strcpy(ConfigMod.num_dati_centrale1,DEF_NUM_DATI_CENTRALE1);     break;
        case ID_NUM_SMS_CENTRALE1 : strcpy(ConfigMod.num_sms_centrale1,DEF_NUM_SMS_CENTRALE1);     break;



        ///////
        case ID_NUM_TEL_USER1 : strcpy(ConfigMod.num_telefono_user1,DEF_NUM_TEL_USER1);     break;
        case ID_IP_APN        : strcpy(ConfigMod.APN,DEF_IP_APN);     break;
        case ID_NETWORK_SELECT: ConfigMod.network_select=DEF_NETWORK_SELECT;     break;

        ///////
        case ID_QUALITA_VIDEO_FILE: ConfigMod.qualita_video_salvato=DEF_QUALITA_VIDEO_FILE; break;
        case ID_QUANT_VIDEO_FILE  : ConfigMod.quant_video_file=DEF_QUANT_VIDEO_FILE; break;
        case ID_FRAME_VIDEO_FILE  : ConfigMod.frame_video_file=DEF_FRAME_VIDEO_FILE; break;

        case ID_TRANSCODER   : ConfigMod.transcoder=DEF_TRANSCODER; break;
        case ID_LEN_REC      : ConfigMod.tempo_per_file_rec=DEF_LEN_REC;break;
        case ID_BITRATE_AUDIO: ConfigMod.bitrate_audio=DEF_BITRATE_AUDIO;break;


        ///////
        case ID_QUALITA_VIDEO_STREAM : ConfigMod.qualita_video_stream=DEF_QUALITA_VIDEO_STREAM;break;
        case ID_QUANT_VIDEO_STREAM   : ConfigMod.quant_video_stream=DEF_QUANT_VIDEO_STREAM;break;
        case ID_FRAME_VIDEO_STREAM   : ConfigMod.frame_video_stream=DEF_FRAME_VIDEO_STREAM;break;

        case ID_MODO_DI_FUNZIONAMENTO: ConfigMod.modo_di_funzionamento=DEF_MODO_DI_FUNZIONAMENTO;break;

        case ID_IP_SERVER_VIDEO    : strcpy(ConfigMod.ip_server_video,DEF_IP_SERVER_VIDEO);break;
        case ID_PORTA_SERVER_VIDEO : strcpy(ConfigMod.port_server_video,DEF_PORTA_SERVER_VIDEO);break;
        case ID_PORTA_SERVER_RTP   : strcpy(ConfigMod.port_server_rtp,DEF_PORTA_SERVER_RTP);break;

        ///////
        case ID_IP_SERVER_FTP       : strcpy(ConfigMod.ip_server_ftp,DEF_IP_SERVER_FTP);     break;
        case ID_LINK_SERVER_FTP     : strcpy(ConfigMod.link_server_ftp,DEF_LINK_SERVER_FTP);     break;
        case ID_USERNAME_SERVER_FTP : strcpy(ConfigMod.username_server_ftp,DEF_USERNAME_SERVER_FTP);     break;
        case ID_PASSWORD_SERVER_FTP : strcpy(ConfigMod.password_server_ftp,DEF_PASSWORD_SERVER_FTP);     break;
        case ID_ACCOUNT_SERVER_FTP  : strcpy(ConfigMod.account_server_ftp,DEF_ACCOUNT_SERVER_FTP);     break;
        case ID_PORTA_SERVER_FTP    : strcpy(ConfigMod.porta_server_ftp,DEF_PORTA_SERVER_FTP);     break;
        case ID_SSL_SERVER_FTP      : ConfigMod.SSL_server_ftp=DEF_SSL_SERVER_FTP;     break;
        case ID_TIMEOUT_SERVER_FTP  : ConfigMod.timeout_server_ftp=DEF_TIMEOUT_SERVER_FTP;     break;
        case ID_FOLDER_SERVER_FTP   : strcpy(ConfigMod.folder_server_ftp,DEF_FOLDER_SERVER_FTP);     break;
        case ID_CANC_FILE_FTP       : ConfigMod.canc_dopo_trasferimento=DEF_CANC_FILE_FTP;     break;
        case ID_ORDINEO_FTP         : ConfigMod.ordine_ottimizzato=DEF_ORDINEO_FTP;     break;

        //////
        case ID_TEMPO_RITENUTA_VOX: ConfigMod.tempo_ritenuta_vox=DEF_TEMPO_RITENUTA_VOX;    break;
        case ID_SENSIBILITA_VOX   :  ConfigMod.sensibilita_vox=DEF_SENSIBILITA_VOX;  break;
        case ID_TEMPO_SOSTA       : ConfigMod.tempo_di_sosta=DEF_TEMPO_SOSTA; break;
        case ID_SENSIBILITA_MOV   : ConfigMod.sensibilita_mov=DEF_SENSIBILITA_MOV;break;
        case ID_FUNZIONE_VOX      : ConfigMod.funzione_vox=DEF_FUNZIONE_VOX; break;
        case ID_FUNZIONE_MOV      : ConfigMod.funzione_mov=DEF_FUNZIONE_MOV; break;
        case ID_TEMP_FAN_ON       : ConfigMod.temp_fan_on=DEF_TEMP_FAN_ON; break;
        case ID_TEMP_FAN_OFF      : ConfigMod.temp_fan_off=DEF_TEMP_FAN_OFF; break;

        case ID_REMLINK_ADDRESS: strcpy(ConfigMod.remlink_address,DEF_REMLINK_ADDRESS);   break;
    case ID_REMLINK_PORT       : strcpy(ConfigMod.remlink_port,DEF_REMLINK_PORT); break;

        case ID_AUDIO_CHANNEL: ConfigMod.audio_channel=DEF_AUDIO_CHANNEL; break;
        case ID_VOLUME_AUDIO : ConfigMod.volume_audio=DEF_VOLUME_AUDIO; break;
        case ID_MODO_SCARICO : ConfigMod.modo_scarico=DEF_MODO_SCARICO; break;

        //fusi begin
        case ID_GPS_FILTRO_POS_IN_SOSTA      : ConfigMod.gps_filtro_pos_in_sosta = DEF_GPS_FILTRO_POS_IN_SOSTA; break;
        case ID_GPS_ABILITA_GLONASS          : ConfigMod.gps_abilita_glonass = DEF_GPS_ABILITA_GLONASS; break;
        case ID_GPS_ABILITA_EGNOS            : ConfigMod.gps_abilita_egnos = DEF_GPS_ABILITA_EGNOS; break;
        case ID_GPS_MODALITA_AGPS            : ConfigMod.gps_modalita_agps = DEF_GPS_MODALITA_AGPS; break;
        //fusi end
    }
}

void TConfig::ConfigPrint()
{
  quint16 IDx=0;

  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"CONFIG:\r\n");
  SerialPrint.Flush(PRINT_DEBUG_ALL);

  // printf("Ov=%0x02\r\n",p->Ovizzata);

//  snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"\r\nModalita' CONFIG\r\n");
//  SerialPrint.Flush(PRINT_DEBUG_ALL);


//  printf("versione configurazione=%s\r\n",p->versione_configurazione);
//  printf("configurata da=%s\r\n",p->num_configuratore);
//  printf("configurata in data=%02d/%02d/20%02d\r\n",p->RTC_DataConfigurazione.RTC_Date,p->RTC_DataConfigurazione.RTC_Month,p->RTC_DataConfigurazione.RTC_Year);

    for(IDx=0;IDx<=ID_MAX_PARAM;IDx++)
    {
        if(strcmp(STR_NO,(char *) ConfigGetParamName(IDx))!=0 )
        {

            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"%s: %s\r\n", ConfigGetParamName(IDx), ConfigGetActualParamValue(IDx));
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }
    }
}


quint16 TConfig::ConfigGetNumParam(void)
{
    quint16 i;
    quint16 count=0;

    for(i=1;i<=ID_MAX_PARAM;i++)
    {
     if(strcmp(STR_NO,(char *) ConfigGetParamName(i))!=0 )count++;
    }
    return(count);
}

char * TConfig::ConfigGetIdGruppoDesc(quint16 ID)
{

//    sprintf (strname,"%03d/%s/%s",ID_TIPO_CENTRALE1,GR_TIPO_CENTRALE1,STR_TIPO_CENTRALE1);     break;
    //sprintf (strname,"%03d/%s/%s",ID,"A","B");

    // sprintf (strname,"%03d/%s/%s",ID_TIPO_CENTRALE1,GR_TIPO_CENTRALE1,STR_TIPO_CENTRALE1);

    switch(ID)
    {
    case ID_TIPO_CENTRALE1    : sprintf (strname,"%03d/%s/%s",ID_TIPO_CENTRALE1,GR_TIPO_CENTRALE1,STR_TIPO_CENTRALE1);     break;

    case ID_CODICE_CENTRALE1  : sprintf (strname,"%03d/%s/%s",ID_CODICE_CENTRALE1,GR_CODICE_CENTRALE1,STR_CODICE_CENTRALE1);       break;
    case ID_CODICE_GRUPPO     : sprintf (strname,"%03d/%s/%s",ID_CODICE_GRUPPO,GR_CODICE_GRUPPO,STR_CODICE_GRUPPO);       break;
    case ID_CODICE_PERIFERICA : sprintf (strname,"%03d/%s/%s",ID_CODICE_PERIFERICA,GR_CODICE_PERIFERICA,STR_CODICE_PERIFERICA);       break;
    case ID_NUM_DATI_CENTRALE1: sprintf (strname,"%03d/%s/%s",ID_NUM_DATI_CENTRALE1,GR_NUM_DATI_CENTRALE1,STR_NUM_DATI_CENTRALE1);     break;
    case ID_NUM_SMS_CENTRALE1 : sprintf (strname,"%03d/%s/%s",ID_NUM_SMS_CENTRALE1,GR_NUM_SMS_CENTRALE1,STR_NUM_SMS_CENTRALE1);     break;
////////
    case ID_NUM_TEL_USER1 : sprintf (strname,"%03d/%s/%s",ID_NUM_TEL_USER1,GR_NUM_TEL_USER1,STR_NUM_TEL_USER1);     break;
    case ID_IP_APN        : sprintf (strname,"%03d/%s/%s",ID_IP_APN,GR_IP_APN,STR_IP_APN);     break;
    case ID_NETWORK_SELECT: sprintf (strname,"%03d/%s/%s",ID_NETWORK_SELECT,GR_NETWORK_SELECT,STR_NETWORK_SELECT);     break;


////////
    case ID_QUALITA_VIDEO_FILE: sprintf (strname,"%03d/%s/%s",ID_QUALITA_VIDEO_FILE,GR_QUALITA_VIDEO_FILE,STR_QUALITA_VIDEO_FILE);     break;
    case ID_QUANT_VIDEO_FILE  : sprintf (strname,"%03d/%s/%s",ID_QUANT_VIDEO_FILE,GR_QUANT_VIDEO_FILE,STR_QUANT_VIDEO_FILE);    break;
    case ID_FRAME_VIDEO_FILE  : sprintf (strname,"%03d/%s/%s",ID_FRAME_VIDEO_FILE,GR_FRAME_VIDEO_FILE,STR_FRAME_VIDEO_FILE);     break;
    case ID_TRANSCODER        : sprintf (strname,"%03d/%s/%s",ID_TRANSCODER,GR_TRANSCODER,STR_TRANSCODER);     break;
    case ID_LEN_REC           : sprintf (strname,"%03d/%s/%s",ID_LEN_REC,GR_LEN_REC,STR_LEN_REC);    break;
    case ID_BITRATE_AUDIO     : sprintf (strname,"%03d/%s/%s",ID_BITRATE_AUDIO,GR_BITRATE_AUDIO,STR_BITRATE_AUDIO);     break;


////////
    case ID_QUALITA_VIDEO_STREAM : sprintf (strname,"%03d/%s/%s",ID_QUALITA_VIDEO_STREAM,GR_QUALITA_VIDEO_STREAM,STR_QUALITA_VIDEO_STREAM);     break;
    case ID_QUANT_VIDEO_STREAM   : sprintf (strname,"%03d/%s/%s",ID_QUANT_VIDEO_STREAM,GR_QUANT_VIDEO_STREAM,STR_QUANT_VIDEO_STREAM);    break;
    case ID_FRAME_VIDEO_STREAM   : sprintf (strname,"%03d/%s/%s",ID_FRAME_VIDEO_STREAM,GR_FRAME_VIDEO_STREAM,STR_FRAME_VIDEO_STREAM);     break;
    case ID_MODO_DI_FUNZIONAMENTO: sprintf (strname,"%03d/%s/%s",ID_MODO_DI_FUNZIONAMENTO,GR_MODO_DI_FUNZIONAMENTO,STR_MODO_DI_FUNZIONAMENTO);     break;
    case ID_IP_SERVER_VIDEO      : sprintf (strname,"%03d/%s/%s",ID_IP_SERVER_VIDEO,GR_IP_SERVER_VIDEO,STR_IP_SERVER_VIDEO);     break;
    case ID_PORTA_SERVER_VIDEO   : sprintf (strname,"%03d/%s/%s",ID_PORTA_SERVER_VIDEO,GR_PORTA_SERVER_VIDEO,STR_PORTA_SERVER_VIDEO);     break;
    case ID_PORTA_SERVER_RTP     : sprintf (strname,"%03d/%s/%s",ID_PORTA_SERVER_RTP,GR_PORTA_SERVER_RTP,STR_PORTA_SERVER_RTP);     break;

////////
    case ID_IP_SERVER_FTP      : sprintf (strname,"%03d/%s/%s",ID_IP_SERVER_FTP,GR_IP_SERVER_FTP,STR_IP_SERVER_FTP);    break;
    case ID_LINK_SERVER_FTP    : sprintf (strname,"%03d/%s/%s",ID_LINK_SERVER_FTP,GR_LINK_SERVER_FTP,STR_LINK_SERVER_FTP);     break;
    case ID_USERNAME_SERVER_FTP: sprintf (strname,"%03d/%s/%s",ID_USERNAME_SERVER_FTP,GR_USERNAME_SERVER_FTP,STR_USERNAME_SERVER_FTP);     break;
    case ID_PASSWORD_SERVER_FTP: sprintf (strname,"%03d/%s/%s",ID_PASSWORD_SERVER_FTP,GR_PASSWORD_SERVER_FTP,STR_PASSWORD_SERVER_FTP);     break;
    case ID_ACCOUNT_SERVER_FTP : sprintf (strname,"%03d/%s/%s",ID_ACCOUNT_SERVER_FTP,GR_ACCOUNT_SERVER_FTP,STR_ACCOUNT_SERVER_FTP);     break;
    case ID_PORTA_SERVER_FTP   : sprintf (strname,"%03d/%s/%s",ID_PORTA_SERVER_FTP,GR_PORTA_SERVER_FTP,STR_PORTA_SERVER_FTP);     break;
    case ID_SSL_SERVER_FTP     : sprintf (strname,"%03d/%s/%s",ID_SSL_SERVER_FTP,GR_SSL_SERVER_FTP,STR_SSL_SERVER_FTP);    break;
    case ID_TIMEOUT_SERVER_FTP : sprintf (strname,"%03d/%s/%s",ID_TIMEOUT_SERVER_FTP,GR_TIMEOUT_SERVER_FTP,STR_TIMEOUT_SERVER_FTP);     break;
    case ID_FOLDER_SERVER_FTP  : sprintf (strname,"%03d/%s/%s",ID_FOLDER_SERVER_FTP,GR_FOLDER_SERVER_FTP,STR_FOLDER_SERVER_FTP);    break;
    case ID_CANC_FILE_FTP      : sprintf (strname,"%03d/%s/%s",ID_CANC_FILE_FTP,GR_CANC_FILE_FTP,STR_CANC_FILE_FTP);     break;
    case ID_ORDINEO_FTP        : sprintf (strname,"%03d/%s/%s",ID_ORDINEO_FTP,GR_ORDINEO_FTP,STR_ORDINEO_FTP);    break;

////////

    case ID_TEMPO_RITENUTA_VOX: sprintf (strname,"%03d/%s/%s",ID_TEMPO_RITENUTA_VOX,GR_TEMPO_RITENUTA_VOX,STR_TEMPO_RITENUTA_VOX);    break;
    case ID_SENSIBILITA_VOX   : sprintf (strname,"%03d/%s/%s",ID_SENSIBILITA_VOX,GR_SENSIBILITA_VOX,STR_SENSIBILITA_VOX);    break;
    case ID_TEMPO_SOSTA       : sprintf (strname,"%03d/%s/%s",ID_TEMPO_SOSTA,GR_TEMPO_SOSTA,STR_TEMPO_SOSTA);    break;
    case ID_SENSIBILITA_MOV   : sprintf (strname,"%03d/%s/%s",ID_SENSIBILITA_MOV,GR_SENSIBILITA_MOV,STR_SENSIBILITA_MOV);     break;
    case ID_FUNZIONE_VOX      : sprintf (strname,"%03d/%s/%s",ID_FUNZIONE_VOX,GR_FUNZIONE_VOX,STR_FUNZIONE_VOX);     break;
    case ID_FUNZIONE_MOV      : sprintf (strname,"%03d/%s/%s",ID_FUNZIONE_MOV,GR_FUNZIONE_MOV,STR_FUNZIONE_MOV);     break;
    case ID_TEMP_FAN_ON       : sprintf (strname,"%03d/%s/%s",ID_TEMP_FAN_ON,GR_TEMP_FAN_ON,STR_TEMP_FAN_ON);     break;
    case ID_TEMP_FAN_OFF      : sprintf (strname,"%03d/%s/%s",ID_TEMP_FAN_OFF,GR_TEMP_FAN_OFF,STR_TEMP_FAN_OFF);     break;
    case ID_REMLINK_ADDRESS   : sprintf (strname,"%03d/%s/%s",ID_REMLINK_ADDRESS,GR_REMLINK_ADDRESS,STR_REMLINK_ADDRESS);    break;
    case ID_REMLINK_PORT      : sprintf (strname,"%03d/%s/%s",ID_REMLINK_PORT,GR_REMLINK_PORT,STR_REMLINK_PORT);     break;
    case ID_AUDIO_CHANNEL     : sprintf (strname,"%03d/%s/%s",ID_AUDIO_CHANNEL,GR_AUDIO_CHANNEL,STR_AUDIO_CHANNEL);     break;
    case ID_VOLUME_AUDIO      : sprintf (strname,"%03d/%s/%s",ID_VOLUME_AUDIO,GR_VOLUME_AUDIO,STR_VOLUME_AUDIO);     break;
    case ID_MODO_SCARICO      : sprintf (strname,"%03d/%s/%s",ID_MODO_SCARICO,GR_MODO_SCARICO,STR_MODO_SCARICO);     break;
////////
    //fusi begin
    case ID_GPS_FILTRO_POS_IN_SOSTA     : sprintf (strname,"%03d/%s/%s",ID_GPS_FILTRO_POS_IN_SOSTA,GR_GPS_FILTRO_POS_IN_SOSTA,STR_GPS_FILTRO_POS_IN_SOSTA);     break;
    case ID_GPS_ABILITA_GLONASS         : sprintf (strname,"%03d/%s/%s",ID_GPS_ABILITA_GLONASS,GR_GPS_ABILITA_GLONASS,STR_GPS_ABILITA_GLONASS);     break;
    case ID_GPS_ABILITA_EGNOS           : sprintf (strname,"%03d/%s/%s",ID_GPS_ABILITA_EGNOS,GR_GPS_ABILITA_EGNOS,STR_GPS_ABILITA_EGNOS);     break;
    case ID_GPS_MODALITA_AGPS           : sprintf (strname,"%03d/%s/%s",ID_GPS_MODALITA_AGPS,GR_GPS_MODALITA_AGPS,STR_GPS_MODALITA_AGPS);     break;
    //fusi end


///////

    default:
        return((char*)STR_NO);
    break;
    }


    return(strname);

}
char * TConfig::ConfigGetMin(quint16 ID)
{
        switch(ID)
        {
        case ID_TIPO_CENTRALE1      : sprintf (strname,"%d",MIN_TIPO_CENTRALE1);     break;

        case ID_CODICE_CENTRALE1    : sprintf (strname,"%d",MIN_CODICE_CENTRALE1);     break;
        case ID_CODICE_GRUPPO       : sprintf (strname,"%d",MIN_CODICE_GRUPPO);     break;
        case ID_CODICE_PERIFERICA   : sprintf (strname,"%d",MIN_CODICE_PERIFERICA);     break;
        case ID_NUM_DATI_CENTRALE1   : sprintf (strname,"%d",MIN_NUM_DATI_CENTRALE1);     break;
        case ID_NUM_SMS_CENTRALE1   : sprintf (strname,"%d",MIN_NUM_SMS_CENTRALE1);     break;
    ////////
        case ID_NUM_TEL_USER1   :   sprintf (strname,"%d",MIN_NUM_TEL_USER1);     break;
        case ID_IP_APN   :          sprintf (strname,"%d",MIN_IP_APN);     break;
        case ID_NETWORK_SELECT   :  sprintf (strname,"%d",MIN_NETWORK_SELECT);     break;


    ////////
        case ID_QUALITA_VIDEO_FILE   : sprintf (strname,"%d",MIN_QUALITA_VIDEO_FILE);     break;

        case ID_QUANT_VIDEO_FILE:
                                        sprintf (strname,"%d",MIN_QUANT_VIDEO_FILE);     break;

        case ID_FRAME_VIDEO_FILE:
                                        sprintf (strname,"%d",MIN_FRAME_VIDEO_FILE);     break;

        case ID_TRANSCODER:
                                        sprintf (strname,"%d",MIN_TRANSCODER);     break;

        case ID_LEN_REC:
                                        sprintf (strname,"%d",MIN_LEN_REC);     break;
        case ID_BITRATE_AUDIO:
                                        sprintf (strname,"%d",MIN_BITRATE_AUDIO);     break;


    ////////
        case ID_QUALITA_VIDEO_STREAM   :  sprintf (strname,"%d",MIN_QUALITA_VIDEO_STREAM);     break;

        case ID_QUANT_VIDEO_STREAM:
                                        sprintf (strname,"%d",MIN_QUANT_VIDEO_STREAM);     break;

        case ID_FRAME_VIDEO_STREAM:
                                       sprintf (strname,"%d",MIN_FRAME_VIDEO_STREAM);     break;

        case ID_MODO_DI_FUNZIONAMENTO:
                                       sprintf (strname,"%d",MIN_MODO_DI_FUNZIONAMENTO);     break;


        case ID_IP_SERVER_VIDEO   : sprintf (strname,"%d",MIN_IP_SERVER_VIDEO);     break;


        case ID_PORTA_SERVER_VIDEO   : sprintf (strname,"%d",MIN_PORTA_SERVER_VIDEO);     break;

        case ID_PORTA_SERVER_RTP   : sprintf (strname,"%d",MIN_PORTA_SERVER_RTP);     break;


    ////////
        case ID_IP_SERVER_FTP       :
                                   sprintf (strname,"%d",MIN_IP_SERVER_FTP);     break;

        case ID_LINK_SERVER_FTP     :
                                   sprintf (strname,"%d",MIN_LINK_SERVER_FTP);     break;

        case ID_USERNAME_SERVER_FTP :
                                   sprintf (strname,"%d",MIN_USERNAME_SERVER_FTP);     break;

        case ID_PASSWORD_SERVER_FTP :
                                   sprintf (strname,"%d",MIN_PASSWORD_SERVER_FTP);     break;

        case ID_ACCOUNT_SERVER_FTP  :
                                   sprintf (strname,"%d",MIN_ACCOUNT_SERVER_FTP);     break;
        case ID_PORTA_SERVER_FTP    :
                                   sprintf (strname,"%d",MIN_PORTA_SERVER_FTP);     break;
        case ID_SSL_SERVER_FTP      :
                                   sprintf (strname,"%d",MIN_SSL_SERVER_FTP);     break;
        case ID_TIMEOUT_SERVER_FTP  :
                                   sprintf (strname,"%d",MIN_TIMEOUT_SERVER_FTP);     break;
        case ID_FOLDER_SERVER_FTP  :
                                   sprintf (strname,"%d",MIN_FOLDER_SERVER_FTP);     break;
        case ID_CANC_FILE_FTP  :
                                   sprintf (strname,"%d",MIN_CANC_FILE_FTP);     break;
        case ID_ORDINEO_FTP  :
                                   sprintf (strname,"%d",MIN_ORDINEO_FTP);     break;

    ////////

        case ID_TEMPO_RITENUTA_VOX:
            sprintf (strname,"%d",MIN_TEMPO_RITENUTA_VOX);     break;
        case ID_SENSIBILITA_VOX:
            sprintf (strname,"%d",MIN_SENSIBILITA_VOX);     break;


        case ID_TEMPO_SOSTA:
            sprintf (strname,"%d",MIN_TEMPO_SOSTA);     break;

        case ID_SENSIBILITA_MOV:
            sprintf (strname,"%d",MIN_SENSIBILITA_MOV);     break;


        case ID_FUNZIONE_VOX:
            sprintf (strname,"%d",MIN_FUNZIONE_VOX);     break;

        case ID_FUNZIONE_MOV:
            sprintf (strname,"%d",MIN_FUNZIONE_MOV);     break;
        case ID_TEMP_FAN_ON:
            sprintf (strname,"%d",MIN_TEMP_FAN_OFF);     break;
        case ID_TEMP_FAN_OFF:
            sprintf (strname,"%d",MIN_TEMP_FAN_OFF);     break;

    ///////


        case ID_REMLINK_ADDRESS   : sprintf (strname,"%d",MIN_REMLINK_ADDRESS);     break;


    ////////
        case ID_REMLINK_PORT       :
                                   sprintf (strname,"%d",MIN_REMLINK_PORT);     break;

        case ID_AUDIO_CHANNEL       :
                                   sprintf (strname,"%d",MIN_AUDIO_CHANNEL);     break;

        case ID_VOLUME_AUDIO       :
                                   sprintf (strname,"%d",MIN_VOLUME_AUDIO);     break;


        case ID_MODO_SCARICO       :
                                   sprintf (strname,"%d",MIN_MODO_SCARICO);     break;

        //fusi begin
        case ID_GPS_FILTRO_POS_IN_SOSTA :
                                   sprintf (strname,"%d",MIN_GPS_FILTRO_POS_IN_SOSTA);     break;

        case ID_GPS_ABILITA_GLONASS:
                                   sprintf (strname,"%d",MIN_GPS_ABILITA_GLONASS);     break;

        case ID_GPS_ABILITA_EGNOS:
                                   sprintf (strname,"%d",MIN_GPS_ABILITA_EGNOS);     break;

        case ID_GPS_MODALITA_AGPS:
                                   sprintf (strname,"%d",MIN_GPS_MODALITA_AGPS);     break;

        //fusi end

        default:
            sprintf(strname,"%s",STR_NO);     break;
        break;
        }

    return(strname);

}

char * TConfig::ConfigGetMax(quint16 ID)
{
    switch(ID)
    {
    case ID_TIPO_CENTRALE1      : sprintf (strname,"%d",MAX_TIPO_CENTRALE1);     break;

    case ID_CODICE_CENTRALE1    : sprintf (strname,"%d",MAX_CODICE_CENTRALE1);     break;
    case ID_CODICE_GRUPPO       : sprintf (strname,"%d",MAX_CODICE_GRUPPO);     break;
    case ID_CODICE_PERIFERICA   : sprintf (strname,"%d",MAX_CODICE_PERIFERICA);     break;
    case ID_NUM_DATI_CENTRALE1   : sprintf (strname,"%d",MAX_NUM_DATI_CENTRALE1);     break;
    case ID_NUM_SMS_CENTRALE1   : sprintf (strname,"%d",MAX_NUM_SMS_CENTRALE1);     break;
////////
    case ID_NUM_TEL_USER1   :   sprintf (strname,"%d",MAX_NUM_TEL_USER1);     break;
    case ID_IP_APN   :          sprintf (strname,"%d",MAX_IP_APN);     break;
    case ID_NETWORK_SELECT   :  sprintf (strname,"%d",MAX_NETWORK_SELECT);     break;


////////
    case ID_QUALITA_VIDEO_FILE   : sprintf (strname,"%d",MAX_QUALITA_VIDEO_FILE);     break;

    case ID_QUANT_VIDEO_FILE:
                                    sprintf (strname,"%d",MAX_QUANT_VIDEO_FILE);     break;

    case ID_FRAME_VIDEO_FILE:
                                    sprintf (strname,"%d",MAX_FRAME_VIDEO_FILE);     break;

    case ID_TRANSCODER:
                                    sprintf (strname,"%d",MAX_TRANSCODER);     break;

    case ID_LEN_REC:
                                    sprintf (strname,"%d",MAX_LEN_REC);     break;
    case ID_BITRATE_AUDIO:
                                    sprintf (strname,"%d",MAX_BITRATE_AUDIO);     break;


////////
    case ID_QUALITA_VIDEO_STREAM   :  sprintf (strname,"%d",MAX_QUALITA_VIDEO_STREAM);     break;

    case ID_QUANT_VIDEO_STREAM:
                                    sprintf (strname,"%d",MAX_QUANT_VIDEO_STREAM);     break;

    case ID_FRAME_VIDEO_STREAM:
                                   sprintf (strname,"%d",MAX_FRAME_VIDEO_STREAM);     break;

    case ID_MODO_DI_FUNZIONAMENTO:
                                   sprintf (strname,"%d",MAX_MODO_DI_FUNZIONAMENTO);     break;

    case ID_IP_SERVER_VIDEO   : sprintf (strname,"%d",MAX_IP_SERVER_VIDEO);     break;


    case ID_PORTA_SERVER_VIDEO   : sprintf (strname,"%d",MAX_PORTA_SERVER_VIDEO);     break;

    case ID_PORTA_SERVER_RTP   : sprintf (strname,"%d",MAX_PORTA_SERVER_RTP);     break;


////////
    case ID_IP_SERVER_FTP       :
                               sprintf (strname,"%d",MAX_IP_SERVER_FTP);     break;

    case ID_LINK_SERVER_FTP     :
                               sprintf (strname,"%d",MAX_LINK_SERVER_FTP);     break;

    case ID_USERNAME_SERVER_FTP :
                               sprintf (strname,"%d",MAX_USERNAME_SERVER_FTP);     break;

    case ID_PASSWORD_SERVER_FTP :
                               sprintf (strname,"%d",MAX_PASSWORD_SERVER_FTP);     break;

    case ID_ACCOUNT_SERVER_FTP  :
                               sprintf (strname,"%d",MAX_ACCOUNT_SERVER_FTP);     break;
    case ID_PORTA_SERVER_FTP    :
                               sprintf (strname,"%d",MAX_PORTA_SERVER_FTP);     break;
    case ID_SSL_SERVER_FTP      :
                               sprintf (strname,"%d",MAX_SSL_SERVER_FTP);     break;
    case ID_TIMEOUT_SERVER_FTP  :
                               sprintf (strname,"%d",MAX_TIMEOUT_SERVER_FTP);     break;
    case ID_FOLDER_SERVER_FTP  :
                               sprintf (strname,"%d",MAX_FOLDER_SERVER_FTP);     break;
    case ID_CANC_FILE_FTP  :
                               sprintf (strname,"%d",MAX_CANC_FILE_FTP);     break;
    case ID_ORDINEO_FTP  :
                               sprintf (strname,"%d",MAX_ORDINEO_FTP);     break;

////////

    case ID_TEMPO_RITENUTA_VOX:
        sprintf (strname,"%d",MAX_TEMPO_RITENUTA_VOX);     break;
    case ID_SENSIBILITA_VOX:
        sprintf (strname,"%d",MAX_SENSIBILITA_VOX);     break;


    case ID_TEMPO_SOSTA:
        sprintf (strname,"%d",MAX_TEMPO_SOSTA);     break;

    case ID_SENSIBILITA_MOV:
        sprintf (strname,"%d",MAX_SENSIBILITA_MOV);     break;


    case ID_FUNZIONE_VOX:
        sprintf (strname,"%d",MAX_FUNZIONE_VOX);     break;

    case ID_FUNZIONE_MOV:
        sprintf (strname,"%d",MAX_FUNZIONE_MOV);     break;

    case ID_TEMP_FAN_ON:
        sprintf (strname,"%d",MAX_TEMP_FAN_ON);     break;
    case ID_TEMP_FAN_OFF:
        sprintf (strname,"%d",MAX_TEMP_FAN_OFF);     break;


    case ID_REMLINK_ADDRESS   : sprintf (strname,"%d",MAX_REMLINK_ADDRESS);     break;


////////
    case ID_REMLINK_PORT       :
                               sprintf (strname,"%d",MAX_REMLINK_PORT);     break;

    case ID_AUDIO_CHANNEL       :
                               sprintf (strname,"%d",MAX_AUDIO_CHANNEL);     break;

    case ID_VOLUME_AUDIO       :
                               sprintf (strname,"%d",MAX_VOLUME_AUDIO);     break;

    case ID_MODO_SCARICO       :
                               sprintf (strname,"%d",MAX_MODO_SCARICO);     break;

///////
    //fusi begin
    case ID_GPS_FILTRO_POS_IN_SOSTA :
                               sprintf (strname,"%d",MAX_GPS_FILTRO_POS_IN_SOSTA);     break;

    case ID_GPS_ABILITA_GLONASS:
                               sprintf (strname,"%d",MAX_GPS_ABILITA_GLONASS);     break;

    case ID_GPS_ABILITA_EGNOS:
                               sprintf (strname,"%d",MAX_GPS_ABILITA_EGNOS);     break;

    case ID_GPS_MODALITA_AGPS:
                               sprintf (strname,"%d",MAX_GPS_MODALITA_AGPS);     break;
    //fusi end

    default:
         sprintf (strname,"%s",STR_NO);     break;
    break;
    }

    return(strname);
}

char * TConfig::ConfigGetLen(quint16 ID)
{
    switch(ID)
    {
    case ID_TIPO_CENTRALE1      : sprintf (strname,"%d",LEN_TIPO_CENTRALE1);     break;

    case ID_CODICE_CENTRALE1    : sprintf (strname,"%d",LEN_CODICE_CENTRALE1);     break;
    case ID_CODICE_GRUPPO       : sprintf (strname,"%d",LEN_CODICE_GRUPPO);     break;
    case ID_CODICE_PERIFERICA   : sprintf (strname,"%d",LEN_CODICE_PERIFERICA);     break;
    case ID_NUM_DATI_CENTRALE1   : sprintf (strname,"%d",LEN_NUM_DATI_CENTRALE1);     break;
    case ID_NUM_SMS_CENTRALE1   : sprintf (strname,"%d",LEN_NUM_SMS_CENTRALE1);     break;
////////
    case ID_NUM_TEL_USER1   :   sprintf (strname,"%d",LEN_NUM_TEL_USER1);     break;
    case ID_IP_APN   :          sprintf (strname,"%d",LEN_IP_APN);     break;
    case ID_NETWORK_SELECT   :  sprintf (strname,"%d",LEN_NETWORK_SELECT);     break;


////////
    case ID_QUALITA_VIDEO_FILE   : sprintf (strname,"%d",LEN_QUALITA_VIDEO_FILE);     break;

    case ID_QUANT_VIDEO_FILE:
                                    sprintf (strname,"%d",LEN_QUANT_VIDEO_FILE);     break;

    case ID_FRAME_VIDEO_FILE:
                                    sprintf (strname,"%d",LEN_FRAME_VIDEO_FILE);     break;

    case ID_TRANSCODER:
                                    sprintf (strname,"%d",LEN_TRANSCODER);     break;


    case ID_LEN_REC:
                                    sprintf (strname,"%d",LEN_LEN_REC);     break;
    case ID_BITRATE_AUDIO:
                                    sprintf (strname,"%d",LEN_BITRATE_AUDIO);     break;


////////
    case ID_QUALITA_VIDEO_STREAM   :  sprintf (strname,"%d",LEN_QUALITA_VIDEO_STREAM);     break;

    case ID_QUANT_VIDEO_STREAM:
                                    sprintf (strname,"%d",LEN_QUANT_VIDEO_STREAM);     break;

    case ID_FRAME_VIDEO_STREAM:
                                   sprintf (strname,"%d",LEN_FRAME_VIDEO_STREAM);     break;

    case ID_MODO_DI_FUNZIONAMENTO:
                                   sprintf (strname,"%d",LEN_MODO_DI_FUNZIONAMENTO);     break;

    case ID_IP_SERVER_VIDEO   : sprintf (strname,"%d",LEN_IP_SERVER_VIDEO);     break;


    case ID_PORTA_SERVER_VIDEO   : sprintf (strname,"%d",LEN_PORTA_SERVER_VIDEO);     break;

    case ID_PORTA_SERVER_RTP   : sprintf (strname,"%d",LEN_PORTA_SERVER_RTP);     break;


////////
    case ID_IP_SERVER_FTP       :
                               sprintf (strname,"%d",LEN_IP_SERVER_FTP);     break;

    case ID_LINK_SERVER_FTP     :
                               sprintf (strname,"%d",LEN_LINK_SERVER_FTP);     break;

    case ID_USERNAME_SERVER_FTP :
                               sprintf (strname,"%d",LEN_USERNAME_SERVER_FTP);     break;

    case ID_PASSWORD_SERVER_FTP :
                               sprintf (strname,"%d",LEN_PASSWORD_SERVER_FTP);     break;

    case ID_ACCOUNT_SERVER_FTP  :
                               sprintf (strname,"%d",LEN_ACCOUNT_SERVER_FTP);     break;
    case ID_PORTA_SERVER_FTP    :
                               sprintf (strname,"%d",LEN_PORTA_SERVER_FTP);     break;
    case ID_SSL_SERVER_FTP      :
                               sprintf (strname,"%d",LEN_SSL_SERVER_FTP);     break;
    case ID_TIMEOUT_SERVER_FTP  :
                               sprintf (strname,"%d",LEN_TIMEOUT_SERVER_FTP);     break;
    case ID_FOLDER_SERVER_FTP  :
                               sprintf (strname,"%d",LEN_FOLDER_SERVER_FTP);     break;
    case ID_CANC_FILE_FTP  :
                               sprintf (strname,"%d",LEN_CANC_FILE_FTP);     break;
    case ID_ORDINEO_FTP  :
                               sprintf (strname,"%d",LEN_ORDINEO_FTP);     break;

////////

    case ID_TEMPO_RITENUTA_VOX:
        sprintf (strname,"%d",LEN_TEMPO_RITENUTA_VOX);     break;
    case ID_SENSIBILITA_VOX:
        sprintf (strname,"%d",LEN_SENSIBILITA_VOX);     break;


    case ID_TEMPO_SOSTA:
        sprintf (strname,"%d",LEN_TEMPO_SOSTA);     break;

    case ID_SENSIBILITA_MOV:
        sprintf (strname,"%d",LEN_SENSIBILITA_MOV);     break;


    case ID_FUNZIONE_VOX:
        sprintf (strname,"%d",LEN_FUNZIONE_VOX);     break;

    case ID_FUNZIONE_MOV:
        sprintf (strname,"%d",LEN_FUNZIONE_MOV);     break;
    case ID_TEMP_FAN_ON:
        sprintf (strname,"%d",LEN_TEMP_FAN_ON);     break;
    case ID_TEMP_FAN_OFF:
        sprintf (strname,"%d",LEN_TEMP_FAN_OFF);     break;


    case ID_REMLINK_ADDRESS   : sprintf (strname,"%d",LEN_REMLINK_ADDRESS);     break;


    case ID_REMLINK_PORT   : sprintf (strname,"%d",LEN_REMLINK_PORT);     break;


    case ID_AUDIO_CHANNEL   : sprintf (strname,"%d",LEN_AUDIO_CHANNEL);     break;

    case ID_VOLUME_AUDIO   : sprintf (strname,"%d",LEN_VOLUME_AUDIO);     break;

    case ID_MODO_SCARICO   : sprintf (strname,"%d",LEN_MODO_SCARICO);     break;

    //fusi begin
    case ID_GPS_FILTRO_POS_IN_SOSTA : sprintf (strname,"%d",LEN_GPS_FILTRO_POS_IN_SOSTA);     break;

    case ID_GPS_ABILITA_GLONASS: sprintf (strname,"%d",LEN_GPS_ABILITA_GLONASS);     break;

    case ID_GPS_ABILITA_EGNOS: sprintf (strname,"%d",LEN_GPS_ABILITA_EGNOS);     break;

    case ID_GPS_MODALITA_AGPS: sprintf (strname,"%d",LEN_GPS_MODALITA_AGPS);     break;
    //fusi end
///////

    default:
        sprintf (strname,"%s",STR_NO);     break;
    break;

    }
 return(strname);

}


quint8 TConfig::ConfigCheckParamSet(quint8 ID)
{

    switch(ID)
    {
    case ID_TIPO_CENTRALE1      : if (strcmp(STR_TIPO_CENTRALE1,STR_NO)==0)return(false);
    break;
    case ID_CODICE_CENTRALE1    : if (strcmp(STR_CODICE_CENTRALE1,STR_NO)==0)return(false);
    break;
    case ID_CODICE_GRUPPO       : if (strcmp(STR_CODICE_GRUPPO,STR_NO)==0)return(false);
    break;
    case ID_CODICE_PERIFERICA   : if (strcmp(STR_CODICE_PERIFERICA,STR_NO)==0)return(false);
    break;
    case ID_NUM_DATI_CENTRALE1   : if (strcmp(STR_NUM_DATI_CENTRALE1,STR_NO)==0)return(false);
    break;
    case ID_NUM_SMS_CENTRALE1   : if (strcmp(STR_NUM_SMS_CENTRALE1,STR_NO)==0)return(false);
    break;
////////
    case ID_NUM_TEL_USER1   :   if (strcmp(STR_NUM_TEL_USER1,STR_NO)==0)return(false);
    break;
    case ID_IP_APN   :          if (strcmp(STR_IP_APN,STR_NO)==0)return(false);
    break;
    case ID_NETWORK_SELECT   :  if (strcmp(STR_NETWORK_SELECT,STR_NO)==0)return(false);
    break;


////////
    case ID_QUALITA_VIDEO_FILE   : if (strcmp(STR_QUALITA_VIDEO_FILE,STR_NO)==0)return(false);
    break;

    case ID_QUANT_VIDEO_FILE:
                                    if (strcmp(STR_QUANT_VIDEO_FILE,STR_NO)==0)return(false);
    break;

    case ID_FRAME_VIDEO_FILE:
                                    if (strcmp(STR_FRAME_VIDEO_FILE,STR_NO)==0)return(false);
    break;

    case ID_TRANSCODER:
                                    if (strcmp(STR_TRANSCODER,STR_NO)==0)return(false);
    break;

    case ID_LEN_REC:
                                    if (strcmp(STR_LEN_REC,STR_NO)==0)return(false);
    break;
    case ID_BITRATE_AUDIO:
                                    if (strcmp(STR_BITRATE_AUDIO,STR_NO)==0)return(false);
    break;


////////
    case ID_QUALITA_VIDEO_STREAM   :  if (strcmp(STR_QUALITA_VIDEO_STREAM,STR_NO)==0)return(false);

    break;
    case ID_QUANT_VIDEO_STREAM:
                                    if (strcmp(STR_QUANT_VIDEO_STREAM,STR_NO)==0)return(false);
    break;

    case ID_FRAME_VIDEO_STREAM:
                                    if (strcmp(STR_FRAME_VIDEO_STREAM,STR_NO)==0)return(false);
    break;

    case ID_MODO_DI_FUNZIONAMENTO:
                                    if (strcmp(STR_MODO_DI_FUNZIONAMENTO,STR_NO)==0)return(false);
    break;


    case ID_IP_SERVER_VIDEO   : if (strcmp(STR_IP_SERVER_VIDEO,STR_NO)==0)return(false);
    break;


    case ID_PORTA_SERVER_VIDEO   : if (strcmp(STR_PORTA_SERVER_VIDEO,STR_NO)==0)return(false);
    break;

    case ID_PORTA_SERVER_RTP   : if (strcmp(STR_PORTA_SERVER_RTP,STR_NO)==0)return(false);
    break;


////////
    case ID_IP_SERVER_FTP       :
                               if (strcmp(STR_IP_SERVER_FTP,STR_NO)==0)return(false);

    break;
    case ID_LINK_SERVER_FTP     :
                               if (strcmp(STR_LINK_SERVER_FTP,STR_NO)==0)return(false);

    break;
    case ID_USERNAME_SERVER_FTP :
                               if (strcmp(STR_USERNAME_SERVER_FTP,STR_NO)==0)return(false);
    break;

    case ID_PASSWORD_SERVER_FTP :
                               if (strcmp(STR_PASSWORD_SERVER_FTP,STR_NO)==0)return(false);
    break;

    case ID_ACCOUNT_SERVER_FTP  :
                               if (strcmp(STR_ACCOUNT_SERVER_FTP,STR_NO)==0)return(false);
    break;
    case ID_PORTA_SERVER_FTP    :
                               if (strcmp(STR_PORTA_SERVER_FTP,STR_NO)==0)return(false);
    break;
    case ID_SSL_SERVER_FTP      :
                               if (strcmp(STR_SSL_SERVER_FTP,STR_NO)==0)return(false);
    break;
    case ID_TIMEOUT_SERVER_FTP  :
                               if (strcmp(STR_TIMEOUT_SERVER_FTP,STR_NO)==0)return(false);
    break;
    case ID_FOLDER_SERVER_FTP  :
                               if (strcmp(STR_FOLDER_SERVER_FTP,STR_NO)==0)return(false);
    break;
    case ID_CANC_FILE_FTP  :
                               if (strcmp(STR_CANC_FILE_FTP,STR_NO)==0)return(false);
    break;
    case ID_ORDINEO_FTP  :
                               if (strcmp(STR_ORDINEO_FTP,STR_NO)==0)return(false);
    break;

////////

    case ID_TEMPO_RITENUTA_VOX:
        if (strcmp(STR_TEMPO_RITENUTA_VOX,STR_NO)==0)return(false);
    break;
    case ID_SENSIBILITA_VOX:
        if (strcmp(STR_SENSIBILITA_VOX,STR_NO)==0)return(false);
    break;


    case ID_TEMPO_SOSTA:
        if (strcmp(STR_TEMPO_SOSTA,STR_NO)==0)return(false);
    break;

    case ID_SENSIBILITA_MOV:
        if (strcmp(STR_SENSIBILITA_MOV,STR_NO)==0)return(false);
    break;


    case ID_FUNZIONE_VOX:
        if (strcmp(STR_FUNZIONE_VOX,STR_NO)==0)return(false);
    break;

    case ID_FUNZIONE_MOV:
        if (strcmp(STR_FUNZIONE_MOV,STR_NO)==0)return(false);
    break;

    case ID_TEMP_FAN_ON:
        if (strcmp(STR_TEMP_FAN_ON,STR_NO)==0)return(false);
    break;
    case ID_TEMP_FAN_OFF:
        if (strcmp(STR_TEMP_FAN_OFF,STR_NO)==0)return(false);
    break;

    case ID_REMLINK_ADDRESS       :
                               if (strcmp(STR_REMLINK_ADDRESS,STR_NO)==0)return(false);
    break;

    case ID_REMLINK_PORT    :
                               if (strcmp(STR_REMLINK_PORT,STR_NO)==0)return(false);
    break;

    case ID_AUDIO_CHANNEL    :
                               if (strcmp(STR_AUDIO_CHANNEL,STR_NO)==0)return(false);
    break;

    case ID_VOLUME_AUDIO    :
                               if (strcmp(STR_VOLUME_AUDIO,STR_NO)==0)return(false);
    break;

    case ID_MODO_SCARICO    :
                               if (strcmp(STR_MODO_SCARICO,STR_NO)==0)return(false);
    break;

    //fusi begin
    case ID_GPS_FILTRO_POS_IN_SOSTA :
                              if (strcmp(STR_GPS_FILTRO_POS_IN_SOSTA,STR_NO)==0)return(false);
    break;
    case ID_GPS_ABILITA_GLONASS:
                              if (strcmp(STR_GPS_ABILITA_GLONASS,STR_NO)==0)return(false);
    break;
    case ID_GPS_ABILITA_EGNOS:
                              if (strcmp(STR_GPS_ABILITA_EGNOS,STR_NO)==0)return(false);
    break;
    case ID_GPS_MODALITA_AGPS:
                              if (strcmp(STR_GPS_MODALITA_AGPS,STR_NO)==0)return(false);
    break;

    //fusi end

///////

    default:
        return(false);
    break;

    }
    return(true);



}

