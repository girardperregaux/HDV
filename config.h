#ifndef CONFIG_H
#define CONFIG_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"
#include <QtGlobal>
#include <QFile>


#define SUCCESSIVO    1
#define PRECEDENTE    2

//#define STR_NOME_FILE_CNF_1 "/home/root/stefano/cnf1.txt"
//#define STR_NOME_FILE_CNF_2 "/home/root/stefano/cnf2.txt"



#define OP_CONF1_KO             0
#define OP_CONF1_OK             1
#define OP_OVIZZA               2
#define OP_RECOVERY_FROM_CNF1   3
#define OP_RECOVERY_FROM_CNF2   4
#define OP_CONF2_OK             5


//#define   STR_TIPO_CENTRALE1           "ID (max 10 caratteri, senza spazi)"









#define MAX_LEN_TIPO_CENTRALE1     MAX_LEN_CAMPO
#define MAX_LEN_CODICE_CENTRALE1   6
#define MAX_LEN_CODICE_GRUPPO     2
#define MAX_LEN_CODICE_PERIFERICA 4
#define MAX_LEN_VERSIONE_PROTOCOLLO 4



#define FUN_APPARATO_COMBINATO    'C'
#define FUN_APPARATO_AMBIENTALE   'A'
#define DEF_FUN_APPARATO FUN_APPARATO_COMBINATO



/*

#define ID_NONE                         0
#define ID_IDENTIFICAZIONE              1
#define ID_QUALITA_VIDEO_STREAM         ID_IDENTIFICAZIONE+7
#define ID_QUANT_VIDEO_STREAM           ID_IDENTIFICAZIONE+8
#define ID_FRAME_VIDEO_STREAM           ID_IDENTIFICAZIONE+9

#define ID_IP_SERVER_VIDEO              ID_IDENTIFICAZIONE+10
#define ID_PORTA_SERVER_VIDEO           ID_IDENTIFICAZIONE+11
#define ID_NETWORK_SELECT               ID_IDENTIFICAZIONE+12

#define ID_BITRATE_AUDIO                ID_IDENTIFICAZIONE+14
#define ID_PORTA_SERVER_RTP             ID_IDENTIFICAZIONE+15

#define ID_TEMPO_RITENUTA_VOX           ID_IDENTIFICAZIONE+16
#define ID_SENSIBILITA_VOX              ID_IDENTIFICAZIONE+17
#define ID_TEMPO_SOSTA                  ID_IDENTIFICAZIONE+18
#define ID_SENSIBILITA_MOV              ID_IDENTIFICAZIONE+19
#define ID_FUNZIONE_VOX                 ID_IDENTIFICAZIONE+20
#define ID_FUNZIONE_MOV                 ID_IDENTIFICAZIONE+21



#define ID_IDENTIFICAZIONE_SIZE         22
#define ID_MAX_PARAM                    ID_FUNZIONE_MOV

*/


#define   SKIP                         2
#define STR_EMPTY	"<EMPTY>"


#define   STR_TIPO_CENTRALE1           "ID (max 10 caratteri, senza spazi)"
#define   STR_CODICE_CENTRALE1         "CODICE CENTRALE (6 CIFRE)"
#define   STR_CODICE_GRUPPO            "CODICE DI GRUPPO (2 CIFRE)"
#define   STR_CODICE_PERIFERICA        "CODICE DISPOSITIVO (4 CIFRE)"
#define   STR_NUM_DATI_CENTRALE1       STR_NO
#define   STR_NUM_SMS_CENTRALE1        STR_NO

#define   STR_NUM_TEL_USER1             "NUM TEL UTENTE"
#define   STR_IP_APN                    "ACCESS POINT GPRS (es. web.omnitel.it ibox.tim.it altri...)"
#define   STR_NETWORK_SELECT            "ACCESS RADIO TECH (0=AUTO,2=UMTS,3=LTE)"


#define STR_QUALITA_VIDEO_FILE           "RISOLUZIONE VIDEO SALVATO (1-4)"
#define STR_QUANT_VIDEO_FILE             "DATA-ORA VIDEO ABILITATA (0=NO,1=SI)"
#define STR_FRAME_VIDEO_FILE             "FRAMES VIDEO SALVATO(10-25)"
#define STR_TRANSCODER                   STR_NO

#define STR_LEN_REC                      "REC DURATA MAX FILE (1-60)min"
#define STR_BITRATE_AUDIO                "AUDIO BITRATE(64Kbps-160Kbps)"

#define STR_QUALITA_VIDEO_STREAM         "RISOLUZIONE VIDEO STREAMING (1-4)"
#define STR_QUANT_VIDEO_STREAM           STR_NO
#define STR_FRAME_VIDEO_STREAM           "FRAMES VIDEO STREAMING (1-25)"
#define STR_MODO_DI_FUNZIONAMENTO        "MODALITA' FUNZIONAMENTO (1-4)"
#define STR_IP_SERVER_VIDEO              "INDIRIZZO IP SERVER VIDEO"
#define STR_PORTA_SERVER_VIDEO           "PORTA SERVER VIDEO"
#define STR_PORTA_SERVER_RTP             "PORTA SERVER RTP"


#define   STR_IP_SERVER_FTP             "FTP INDIRIZZO IP DEL SERVER"
#define   STR_LINK_SERVER_FTP           STR_NO //"FTP URL SERVER"
#define   STR_USERNAME_SERVER_FTP       "FTP USERNAME"
#define   STR_PASSWORD_SERVER_FTP       "FTP PASSWORD"
#define   STR_ACCOUNT_SERVER_FTP        "FTP ACCOUNT"
#define   STR_PORTA_SERVER_FTP          "FTP PORTA DEL SERVER"
#define   STR_SSL_SERVER_FTP            "FTP VERIFICA FILES TRASFERITI(0=NO,1=SI,2=SHA256)"
#define   STR_TIMEOUT_SERVER_FTP        "FTP TIMEOUT DI CONNESSIONE"
#define   STR_FOLDER_SERVER_FTP         "FTP CARTELLA DI SCARICO (per root: .. oppure aaaa\\\\bbbb (max 30 car.))"
//#define   STR_CANC_FILE_FTP           "CANCELLA I FILE DOPO AVERLI TRASFERITI(0=NO,1=SI)"
#define   STR_CANC_FILE_FTP             STR_NO
#define   STR_ORDINEO_FTP               "ORDINE TRASF FILE DURANTE LA REGISTRAZIONE(1=DAL PIU' VECCHIO, 2=SEMPRE L'ULTIMO)"


#define   STR_TEMPO_RITENUTA_VOX        "VOX TEMPO DI RITENUTA (1-60)min"
#define   STR_SENSIBILITA_VOX           "VOX SENSIBILITA' (1=MAX 8=MIN)"
#define   STR_TEMPO_SOSTA               "MOV TEMPO PER ENTRARE IN SOSTA (1-60)min"
#define   STR_SENSIBILITA_MOV           "MOV SENSIBILITA' (1=MAX 5=MIN)"
#define   STR_FUNZIONE_VOX              "VOX ABILITATO (0=NO,1=SI,2=SEMPRE ATTIVO)"
#define   STR_FUNZIONE_MOV              "MOV ABILITATO (0=NO,1=SI)"
#define   STR_TEMP_FAN_ON               "TEMP. ACCENSIONE VENTOLA(21,119)C"
#define   STR_TEMP_FAN_OFF              "TEMP. SPEGNIMENTO VENTOLA(21,119)C"
#define STR_REMLINK_ADDRESS             "INDIRIZZO IP REMLINK"
#define STR_REMLINK_PORT                "PORTA REMLINK"
#define STR_AUDIO_CHANNEL               "MODALITA' AUDIO (1=MONO,2=STEREO)"
#define STR_VOLUME_AUDIO                "VOLUME AUDIO (1=MIN,10=MAX)"
#define STR_MODO_SCARICO                "FTP MODALITA' SCARICO (1=AUTO,2=REQ)"

//fusi begin
#define STR_GPS_FILTRO_POS_IN_SOSTA      "VELOCITA' MAX PER MANTENIMENTO POS. GPS DA FERMO (0=NESSUN FILTRO) cm/sec"
#define STR_GPS_ABILITA_GLONASS          "ABILITA GLONASS (0=NO 1=SI)"
#define STR_GPS_ABILITA_EGNOS            "GPS ABILITA EGNOS(0=NO 1=SI)"
#define STR_GPS_MODALITA_AGPS            "GPS MODALITA' AGPS(0=NO 1=SI)"
//fusi end

/*
#define STR_GPS_TEMPO_ATTESA_ACK_PING   "REGISTRA PERCORSO(0=NO,1=SI)"
#define STR_GPS_IP_ADD_CENTRAL          "INDIRIZZO IP CENTRALE"
#define STR_GPS_PORT_CENTRAL            "PORTA CENTRALE"
#define STR_GPS_HISTORY_DIR             "ANGOLO DI DEVIAZIONE(0-360)gradi"
#define STR_GPS_HISTORY_DIST            "DISTANZA MASSIMA(0-50000)m"
#define STR_GPS_HISTORY_VF              "VELOCITA' DI FERMO(0-200)kmh"
#define STR_GPS_INTERVALLO_RISVEGLI     "RISVEGLI GPS PER POSIZIONE (0-9999)min"
#define STR_GPS_TEMPO_GPS_ON            "TEMPO ACCENSIONE GPS PER POSIZIONE (10-9999)sec"
#define STR_GPS_TX_IN_MOV               STR_NO
#define STR_GPS_TX_IN_SOSTA             STR_NO
#define STR_GPS_FILTRO_IN_SOSTA         "FILTRO POS. GPS DA FERMO (0=NESSUN FILTRO)cms"
#define STR_GPS_PDOP                    STR_NO
#define STR_GPS_PACC                    STR_NO
#define STR_GPS_NAV_STATICA             STR_NO
#define STR_GPS_AGPS                    "GPS MODALITA' AGPS(0=NO 1=SI)"
#define STR_GPS_EGNOS                   "GPS ABILITA EGNOS(0=NO 1=SI)"
#define STR_GPS_STATO_MODEM_SBY         STR_NO
*/

//1
#define ID_NONE                         0
#define ID_IDENTIFICAZIONE              1
#define ID_TIPO_CENTRALE1               ID_IDENTIFICAZIONE+0
#define ID_CODICE_CENTRALE1             ID_IDENTIFICAZIONE+1
#define ID_CODICE_GRUPPO                ID_IDENTIFICAZIONE+2
#define ID_CODICE_PERIFERICA            ID_IDENTIFICAZIONE+3
#define ID_NUM_DATI_CENTRALE1           ID_IDENTIFICAZIONE+4
#define ID_NUM_SMS_CENTRALE1            ID_IDENTIFICAZIONE+5
#define ID_IDENTIFICAZIONE_SIZE         6


#define ID_USER                         ID_IDENTIFICAZIONE+ID_IDENTIFICAZIONE_SIZE
#define ID_NUM_TEL_USER1                ID_USER+0
#define ID_IP_APN                       ID_USER+1
#define ID_NETWORK_SELECT               ID_USER+2
#define ID_USER_SIZE                    3


#define ID_REC                          ID_USER+ID_USER_SIZE
#define ID_QUALITA_VIDEO_FILE           ID_REC+0
#define ID_QUANT_VIDEO_FILE             ID_REC+1
#define ID_FRAME_VIDEO_FILE             ID_REC+2
#define ID_TRANSCODER                   ID_REC+3
#define ID_LEN_REC                      ID_REC+4
#define ID_BITRATE_AUDIO                ID_REC+5
#define ID_REC_SIZE                     6


#define ID_STREAM                       ID_REC+ID_REC_SIZE
#define ID_QUALITA_VIDEO_STREAM         ID_STREAM+0
#define ID_QUANT_VIDEO_STREAM           ID_STREAM+1
#define ID_FRAME_VIDEO_STREAM           ID_STREAM+2
#define ID_MODO_DI_FUNZIONAMENTO        ID_STREAM+3
#define ID_IP_SERVER_VIDEO              ID_STREAM+4
#define ID_PORTA_SERVER_VIDEO           ID_STREAM+5
#define ID_PORTA_SERVER_RTP             ID_STREAM+6
#define ID_STREAM_SIZE                  7



#define ID_SERVER_FTP                   ID_STREAM+ID_STREAM_SIZE
#define ID_IP_SERVER_FTP                ID_SERVER_FTP+0
#define ID_LINK_SERVER_FTP              ID_SERVER_FTP+1
#define ID_USERNAME_SERVER_FTP          ID_SERVER_FTP+2
#define ID_PASSWORD_SERVER_FTP          ID_SERVER_FTP+3
#define ID_ACCOUNT_SERVER_FTP           ID_SERVER_FTP+4
#define ID_PORTA_SERVER_FTP             ID_SERVER_FTP+5
#define ID_SSL_SERVER_FTP               ID_SERVER_FTP+6
#define ID_TIMEOUT_SERVER_FTP           ID_SERVER_FTP+7
#define ID_FOLDER_SERVER_FTP            ID_SERVER_FTP+8
#define ID_CANC_FILE_FTP                ID_SERVER_FTP+9
#define ID_ORDINEO_FTP                  ID_SERVER_FTP+10
#define ID_SERVER_FTP_SIZE              11


#define ID_SENS                         ID_SERVER_FTP+ID_SERVER_FTP_SIZE
#define ID_TEMPO_RITENUTA_VOX           ID_SENS+0
#define ID_SENSIBILITA_VOX              ID_SENS+1
#define ID_TEMPO_SOSTA                  ID_SENS+2
#define ID_SENSIBILITA_MOV              ID_SENS+3
#define ID_FUNZIONE_VOX                 ID_SENS+4
#define ID_FUNZIONE_MOV                 ID_SENS+5
#define ID_TEMP_FAN_ON                  ID_SENS+6
#define ID_TEMP_FAN_OFF                 ID_SENS+7

#define ID_REMLINK_ADDRESS              ID_SENS+8
#define ID_REMLINK_PORT                 ID_SENS+9

#define ID_AUDIO_CHANNEL                ID_SENS+10 //ultimo param
#define ID_VOLUME_AUDIO                 ID_SENS+11
#define ID_MODO_SCARICO                 ID_SENS+12
#define ID_SENS_SIZE                    13

//fusi begin
#define ID_GPS                          ID_SENS+ID_SENS_SIZE
#define ID_GPS_FILTRO_POS_IN_SOSTA      ID_GPS+0
#define ID_GPS_ABILITA_GLONASS          ID_GPS+1
#define ID_GPS_ABILITA_EGNOS            ID_GPS+2
#define ID_GPS_MODALITA_AGPS            ID_GPS+3
#define ID_GPS_SIZE                     4
//fus end

//#define ID_MAX_PARAM                ID_REMLINK_PORT
#define ID_MAX_PARAM                    ID_GPS_MODALITA_AGPS



#define DEF_STR_GRUPPO_PARAM	"PARAMETRI"

#define DEF_TIPO_CENTRALE1   "REMHDV"
#define GR_TIPO_CENTRALE1   DEF_STR_GRUPPO_PARAM
#define MAX_TIPO_CENTRALE1 0
#define MIN_TIPO_CENTRALE1 0
#define LEN_TIPO_CENTRALE1 MAX_LEN_TIPO_CENTRALE1

#define DEF_CODICE_CENTRALE1  "******"
#define GR_CODICE_CENTRALE1 DEF_STR_GRUPPO_PARAM
#define MAX_CODICE_CENTRALE1 0
#define MIN_CODICE_CENTRALE1 0
#define LEN_CODICE_CENTRALE1 MAX_LEN_CODICE_CENTRALE1

#define DEF_CODICE_GRUPPO     "**"
#define GR_CODICE_GRUPPO DEF_STR_GRUPPO_PARAM
#define MAX_CODICE_GRUPPO 0
#define MIN_CODICE_GRUPPO 0
#define LEN_CODICE_GRUPPO MAX_LEN_CODICE_GRUPPO

#define DEF_CODICE_PERIFERICA  "****"
#define GR_CODICE_PERIFERICA DEF_STR_GRUPPO_PARAM
#define MAX_CODICE_PERIFERICA 0
#define MIN_CODICE_PERIFERICA 0
#define LEN_CODICE_PERIFERICA MAX_LEN_CODICE_PERIFERICA

#define DEF_NUM_DATI_CENTRALE1  STR_NO
#define GR_NUM_DATI_CENTRALE1 DEF_STR_GRUPPO_PARAM
#define MAX_NUM_DATI_CENTRALE1 0
#define MIN_NUM_DATI_CENTRALE1 0
#define LEN_NUM_DATI_CENTRALE1 MAX_LEN_CAMPO

#define DEF_NUM_SMS_CENTRALE1   STR_NO
#define GR_NUM_SMS_CENTRALE1 DEF_STR_GRUPPO_PARAM
#define MAX_NUM_SMS_CENTRALE1 0
#define MIN_NUM_SMS_CENTRALE1 0
#define LEN_NUM_SMS_CENTRALE1 MAX_LEN_CAMPO

////
#define DEF_NUM_TEL_USER1  STR_NO
#define GR_NUM_TEL_USER1 DEF_STR_GRUPPO_PARAM
#define MAX_NUM_TEL_USER1 0
#define MIN_NUM_TEL_USER1 0
#define LEN_NUM_TEL_USER1 MAX_LEN_CAMPO

#define DEF_IP_APN           STR_NO
#define GR_IP_APN DEF_STR_GRUPPO_PARAM
#define MAX_IP_APN 0
#define MIN_IP_APN 0
#define LEN_IP_APN MAX_LEN_CAMPO

#define DEF_NETWORK_SELECT 0
#define GR_NETWORK_SELECT DEF_STR_GRUPPO_PARAM
#define MAX_NETWORK_SELECT 3
#define MIN_NETWORK_SELECT 0
#define LEN_NETWORK_SELECT 0

////
#define DEF_QUALITA_VIDEO_FILE 3
#define GR_QUALITA_VIDEO_FILE DEF_STR_GRUPPO_PARAM
#define MAX_QUALITA_VIDEO_FILE 4
#define MIN_QUALITA_VIDEO_FILE 1
#define LEN_QUALITA_VIDEO_FILE 0

#define DEF_QUANT_VIDEO_FILE 1
#define GR_QUANT_VIDEO_FILE DEF_STR_GRUPPO_PARAM
#define MAX_QUANT_VIDEO_FILE 1
#define MIN_QUANT_VIDEO_FILE 0
#define LEN_QUANT_VIDEO_FILE 0

#define DEF_FRAME_VIDEO_FILE 10
#define GR_FRAME_VIDEO_FILE DEF_STR_GRUPPO_PARAM
#define MAX_FRAME_VIDEO_FILE 25
#define MIN_FRAME_VIDEO_FILE 10
#define LEN_FRAME_VIDEO_FILE 0


#define DEF_TRANSCODER 1
#define GR_TRANSCODER DEF_STR_GRUPPO_PARAM
#define MAX_TRANSCODER 1
#define MIN_TRANSCODER 0
#define LEN_TRANSCODER 0



#define DEF_LEN_REC  5
#define GR_LEN_REC DEF_STR_GRUPPO_PARAM
#define MAX_LEN_REC 60
#define MIN_LEN_REC 1
#define LEN_LEN_REC 0

#define DEF_BITRATE_AUDIO 96
#define GR_BITRATE_AUDIO DEF_STR_GRUPPO_PARAM
#define MAX_BITRATE_AUDIO 160
#define MIN_BITRATE_AUDIO 64
#define LEN_BITRATE_AUDIO 0



#define DEF_QUALITA_VIDEO_STREAM 2
#define GR_QUALITA_VIDEO_STREAM DEF_STR_GRUPPO_PARAM
#define MAX_QUALITA_VIDEO_STREAM 4
#define MIN_QUALITA_VIDEO_STREAM 1
#define LEN_QUALITA_VIDEO_STREAM 0

#define DEF_QUANT_VIDEO_STREAM 5
#define GR_QUANT_VIDEO_STREAM DEF_STR_GRUPPO_PARAM
#define MAX_QUANT_VIDEO_STREAM 5
#define MIN_QUANT_VIDEO_STREAM 1
#define LEN_QUANT_VIDEO_STREAM 0

#define DEF_FRAME_VIDEO_STREAM  10
#define GR_FRAME_VIDEO_STREAM DEF_STR_GRUPPO_PARAM
#define MAX_FRAME_VIDEO_STREAM 25
#define MIN_FRAME_VIDEO_STREAM 1
#define LEN_FRAME_VIDEO_STREAM 0


#define DEF_MODO_DI_FUNZIONAMENTO  1
#define GR_MODO_DI_FUNZIONAMENTO DEF_STR_GRUPPO_PARAM
#define MAX_MODO_DI_FUNZIONAMENTO 4
#define MIN_MODO_DI_FUNZIONAMENTO 1
#define LEN_MODO_DI_FUNZIONAMENTO 0


#define DEF_IP_SERVER_VIDEO STR_NO
#define GR_IP_SERVER_VIDEO DEF_STR_GRUPPO_PARAM
#define MAX_IP_SERVER_VIDEO 0
#define MIN_IP_SERVER_VIDEO 0
#define LEN_IP_SERVER_VIDEO MAX_LEN_CAMPO

#define DEF_PORTA_SERVER_VIDEO STR_NO
#define GR_PORTA_SERVER_VIDEO DEF_STR_GRUPPO_PARAM
#define MAX_PORTA_SERVER_VIDEO 0
#define MIN_PORTA_SERVER_VIDEO 0
#define LEN_PORTA_SERVER_VIDEO MAX_LEN_CAMPO

#define DEF_PORTA_SERVER_RTP STR_NO
#define GR_PORTA_SERVER_RTP DEF_STR_GRUPPO_PARAM
#define MAX_PORTA_SERVER_RTP 0
#define MIN_PORTA_SERVER_RTP 0
#define LEN_PORTA_SERVER_RTP MAX_LEN_CAMPO







#define DEF_IP_SERVER_FTP STR_NO
#define GR_IP_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_IP_SERVER_FTP 0
#define MIN_IP_SERVER_FTP 0
#define LEN_IP_SERVER_FTP MAX_LEN_CAMPO

#define DEF_LINK_SERVER_FTP STR_NO
#define GR_LINK_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_LINK_SERVER_FTP 0
#define MIN_LINK_SERVER_FTP 0
#define LEN_LINK_SERVER_FTP MAX_LEN_CAMPO

#define DEF_USERNAME_SERVER_FTP STR_NO
#define GR_USERNAME_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_USERNAME_SERVER_FTP 0
#define MIN_USERNAME_SERVER_FTP 0
#define LEN_USERNAME_SERVER_FTP MAX_LEN_CAMPO

#define DEF_PASSWORD_SERVER_FTP STR_NO
#define GR_PASSWORD_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_PASSWORD_SERVER_FTP 0
#define MIN_PASSWORD_SERVER_FTP 0
#define LEN_PASSWORD_SERVER_FTP MAX_LEN_CAMPO

#define DEF_ACCOUNT_SERVER_FTP STR_NO
#define GR_ACCOUNT_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_ACCOUNT_SERVER_FTP 0
#define MIN_ACCOUNT_SERVER_FTP 0
#define LEN_ACCOUNT_SERVER_FTP MAX_LEN_CAMPO

#define DEF_PORTA_SERVER_FTP "21"
#define GR_PORTA_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_PORTA_SERVER_FTP 0
#define MIN_PORTA_SERVER_FTP 0
#define LEN_PORTA_SERVER_FTP MAX_LEN_CAMPO

#define DEF_SSL_SERVER_FTP 1
#define GR_SSL_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_SSL_SERVER_FTP 1
#define MIN_SSL_SERVER_FTP 0
#define LEN_SSL_SERVER_FTP 0

#define DEF_TIMEOUT_SERVER_FTP 30
#define GR_TIMEOUT_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_TIMEOUT_SERVER_FTP 60
#define MIN_TIMEOUT_SERVER_FTP 0
#define LEN_TIMEOUT_SERVER_FTP 0

#define DEF_FOLDER_SERVER_FTP ".."
#define GR_FOLDER_SERVER_FTP DEF_STR_GRUPPO_PARAM
#define MAX_FOLDER_SERVER_FTP 0
#define MIN_FOLDER_SERVER_FTP 0
#define LEN_FOLDER_SERVER_FTP MAX_LEN_CAMPO

#define DEF_CANC_FILE_FTP 1
#define GR_CANC_FILE_FTP DEF_STR_GRUPPO_PARAM
#define MAX_CANC_FILE_FTP 1
#define MIN_CANC_FILE_FTP 0
#define LEN_CANC_FILE_FTP 0

#define DEF_ORDINEO_FTP 1
#define GR_ORDINEO_FTP DEF_STR_GRUPPO_PARAM
#define MAX_ORDINEO_FTP 2
#define MIN_ORDINEO_FTP 0
#define LEN_ORDINEO_FTP 0




#define DEF_TEMPO_RITENUTA_VOX  3
#define GR_TEMPO_RITENUTA_VOX DEF_STR_GRUPPO_PARAM
#define MAX_TEMPO_RITENUTA_VOX 60
#define MIN_TEMPO_RITENUTA_VOX  1
#define LEN_TEMPO_RITENUTA_VOX  0

#define DEF_SENSIBILITA_VOX 4
#define GR_SENSIBILITA_VOX DEF_STR_GRUPPO_PARAM
#define MAX_SENSIBILITA_VOX 8
#define MIN_SENSIBILITA_VOX 1
#define LEN_SENSIBILITA_VOX 0

#define DEF_TEMPO_SOSTA  5
#define GR_TEMPO_SOSTA DEF_STR_GRUPPO_PARAM
#define MAX_TEMPO_SOSTA 60
#define MIN_TEMPO_SOSTA 1
#define LEN_TEMPO_SOSTA 0

#define DEF_SENSIBILITA_MOV 3
#define GR_SENSIBILITA_MOV DEF_STR_GRUPPO_PARAM
#define MAX_SENSIBILITA_MOV 5
#define MIN_SENSIBILITA_MOV 1

#define LEN_SENSIBILITA_MOV 0

#define DEF_FUNZIONE_VOX 0
#define GR_FUNZIONE_VOX DEF_STR_GRUPPO_PARAM
#define MAX_FUNZIONE_VOX 2
#define MIN_FUNZIONE_VOX 0
#define LEN_FUNZIONE_VOX 0

#define DEF_FUNZIONE_MOV 0
#define GR_FUNZIONE_MOV DEF_STR_GRUPPO_PARAM
#define MAX_FUNZIONE_MOV 1
#define MIN_FUNZIONE_MOV 0
#define LEN_FUNZIONE_MOV 0

#define DEF_TEMP_FAN_ON 40
#define GR_TEMP_FAN_ON DEF_STR_GRUPPO_PARAM
#define MAX_TEMP_FAN_ON 119
#define MIN_TEMP_FAN_ON 20
#define LEN_TEMP_FAN_ON 0

#define DEF_TEMP_FAN_OFF 35
#define GR_TEMP_FAN_OFF DEF_STR_GRUPPO_PARAM
#define MAX_TEMP_FAN_OFF 119
#define MIN_TEMP_FAN_OFF 20
#define LEN_TEMP_FAN_OFF 0


//todo

#define DEF_REMLINK_ADDRESS STR_NO
#define GR_REMLINK_ADDRESS DEF_STR_GRUPPO_PARAM
#define MAX_REMLINK_ADDRESS 0
#define MIN_REMLINK_ADDRESS 0
#define LEN_REMLINK_ADDRESS MAX_LEN_CAMPO


#define DEF_REMLINK_PORT STR_NO
#define GR_REMLINK_PORT DEF_STR_GRUPPO_PARAM
#define MAX_REMLINK_PORT 0
#define MIN_REMLINK_PORT 0
#define LEN_REMLINK_PORT MAX_LEN_CAMPO


#define DEF_AUDIO_CHANNEL 1
#define GR_AUDIO_CHANNEL DEF_STR_GRUPPO_PARAM
#define MAX_AUDIO_CHANNEL 2
#define MIN_AUDIO_CHANNEL 1
#define LEN_AUDIO_CHANNEL 0

#define DEF_VOLUME_AUDIO 10
#define GR_VOLUME_AUDIO DEF_STR_GRUPPO_PARAM
#define MAX_VOLUME_AUDIO 10
#define MIN_VOLUME_AUDIO 1
#define LEN_VOLUME_AUDIO 0


#define DEF_MODO_SCARICO 1
#define GR_MODO_SCARICO DEF_STR_GRUPPO_PARAM
#define MAX_MODO_SCARICO 2
#define MIN_MODO_SCARICO 1
#define LEN_MODO_SCARICO 0


//fusi begin
#define DEF_GPS_FILTRO_POS_IN_SOSTA 80
#define GR_GPS_FILTRO_POS_IN_SOSTA  DEF_STR_GRUPPO_PARAM
#define MAX_GPS_FILTRO_POS_IN_SOSTA 255
#define MIN_GPS_FILTRO_POS_IN_SOSTA 0
#define LEN_GPS_FILTRO_POS_IN_SOSTA 0

#define DEF_GPS_ABILITA_GLONASS 0
#define GR_GPS_ABILITA_GLONASS DEF_STR_GRUPPO_PARAM
#define MAX_GPS_ABILITA_GLONASS 1
#define MIN_GPS_ABILITA_GLONASS 0
#define LEN_GPS_ABILITA_GLONASS 0


#define DEF_GPS_ABILITA_EGNOS 0
#define GR_GPS_ABILITA_EGNOS DEF_STR_GRUPPO_PARAM
#define MAX_GPS_ABILITA_EGNOS 1
#define MIN_GPS_ABILITA_EGNOS 0
#define LEN_GPS_ABILITA_EGNOS 0


#define DEF_GPS_MODALITA_AGPS 0
#define GR_GPS_MODALITA_AGPS  DEF_STR_GRUPPO_PARAM
#define MAX_GPS_MODALITA_AGPS 1
#define MIN_GPS_MODALITA_AGPS 0
#define LEN_GPS_MODALITA_AGPS 0

//fusi end






typedef struct
{    
  char ip[MAX_LEN_CAMPO+1];
  char port[MAX_LEN_CAMPO+1];
  char login[MAX_LEN_CAMPO+1];
  char password[MAX_LEN_CAMPO+1];
  char APN[MAX_LEN_CAMPO+1];
  char NumTelSMS[MAX_LEN_CAMPO+1];
} TConnectionParam;


typedef struct
{

    // Gestione configurazione
    quint8 Ovizzata;
    char versione_configurazione[MAX_LEN_VERSIONE_PROTOCOLLO+1];
    quint8 versione_protocollo;

    //ID
    char tipo_centrale1[MAX_LEN_TIPO_CENTRALE1+1];
    char codice_centrale1[MAX_LEN_CODICE_CENTRALE1+1];
    char codice_gruppo[MAX_LEN_CODICE_GRUPPO+1];
    char codice_periferica[MAX_LEN_CODICE_PERIFERICA+1];
    char num_dati_centrale1[MAX_LEN_CAMPO+1];
    char num_sms_centrale1[MAX_LEN_CAMPO+1];


    //USER
    char num_telefono_user1[MAX_LEN_CAMPO+1];
    char APN[MAX_LEN_CAMPO+1];
    quint8 network_select;
    //REC
    quint8 qualita_video_salvato;
    quint8 quant_video_file;
    quint8 frame_video_file;
    quint8 transcoder;
    quint16 tempo_per_file_rec;
    quint8 bitrate_audio;

    //STREAM
    quint8 qualita_video_stream;
    quint8 quant_video_stream;
    quint8 frame_video_stream;
    quint8 modo_di_funzionamento;
    char ip_server_video[MAX_LEN_CAMPO+1];
    char port_server_video[MAX_LEN_CAMPO+1];
    char port_server_rtp[MAX_LEN_CAMPO+1];


    //FTP
    char ip_server_ftp[MAX_LEN_CAMPO+1];
    char link_server_ftp[MAX_LEN_CAMPO+1];
    char username_server_ftp[MAX_LEN_CAMPO+1];
    char password_server_ftp[MAX_LEN_CAMPO+1];
    char account_server_ftp[MAX_LEN_CAMPO+1];
    char porta_server_ftp[MAX_LEN_CAMPO+1];
    quint8 SSL_server_ftp;
    quint16 timeout_server_ftp;
    char folder_server_ftp[MAX_LEN_CAMPO+1];
    quint16 canc_dopo_trasferimento;
    quint8 ordine_ottimizzato;


    //SENS
    quint8 tempo_ritenuta_vox;
    quint8 sensibilita_vox;
    quint8 tempo_di_sosta;
    quint8 sensibilita_mov;
    quint8 funzione_vox;
    quint8 funzione_mov;
    quint8 temp_fan_on;
    quint8 temp_fan_off;

    char remlink_address[MAX_LEN_CAMPO+1];
    char remlink_port[MAX_LEN_CAMPO+1];

    quint8 audio_channel;
    quint8 volume_audio;
    quint8 modo_scarico;


    // GPS
    //fusi begin
    quint8 gps_filtro_pos_in_sosta;
    quint8 gps_abilita_glonass;
    quint8 gps_abilita_egnos;
    quint8 gps_modalita_agps;
    //fusi end

    quint16 CRCc;



} TConfiguration;


/* Exported types ------------------------------------------------------------*/

class TConfig
{

public:
    TConfig();
    TConfiguration Config;
    TConfiguration ConfigMod;
    char strname[MAX_LEN_SMS+1];

    void Init(); //per passare timer
    void PrintObj();
    bool Save();
    void PrintHelpConfig(void);
    void InsMode(void);
    void ConfigPrint(void);
    void SetOperation(void);
    bool ConfigWriteParamSet(quint16 ID, char *strvalue);

    quint8 Read(TConfiguration *pCONFIGURATION, char *nome_file);
    quint16 ConfigGetParamID(quint16 ID,quint8 verso);
    char * ConfigGetParamName(quint16 ID);
    char * ConfigGetParamValue(quint16 ID);
    bool ConfigParamSet(quint16 ID, char *strvalue);
    bool ParamCheck(quint16 ID, char *strvalue);
    void ConfigParamDefault(quint16 ID);
    char * ConfigGetActualParamValue(quint16 ID); //todo

    quint16 ConfigGetNumParam(void);
    char * ConfigGetIdGruppoDesc(quint16 ID);
    char * ConfigGetMin(quint16 ID);
    char * ConfigGetMax(quint16 ID);
    char * ConfigGetLen(quint16 ID);
    quint8 ConfigSID(quint16 SID, char *strvalue);
    quint8 ConfigCheckParamSet(quint8 ID);


    void Ovizza();

private:

};

#endif // MCMODEM_H
