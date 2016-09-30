#ifndef HDV_H
#define HDV_H



#define VERSIONE_FW "1.02"
#define SUB_VERSIONE 1
#define NOME_APPARATO "REMHDV"

//fusi begin
#define SYN     0x16
#define SOH     0x01
#define ETX     0x03
#define CR      0x0D
#define LF      0x0A
#define CTRLZ   0x1A


//fusi end



// BIT_STATO  0x00..0x0F
// BIT_STATO  0x10..0x1F

#define BIT_SALVATI_EE0   0 // questi stati vengono salvati in ee
#define BIT_SALVATI_EE1   1 // questi stati vengono salvati in ee
#define STATI_RICORDATI_LEN 2

#define DEF_BIT_SALVATI_EE0  0x01
#define DEF_BIT_SALVATI_EE1  0x05

//BIT_SALVATI_EE0  0x00..0x0F (VEDERE TELECOMANDI F1_B0_TELECOMANDO_0..F1_B0_TELECOMANDO_15)
#define FUORI_SERVIZIO    0x00
//#define                 0x01
//#define                 0x02
//#define                 0x03
//#define                 0x04
//#define                 0x05
//#define                 0x06
//#define                 0x07
//#define                 0x08
//#define                 0x09
//#define                 0x0A
//#define                 0x0B
//#define                 0x0C
//#define                 0x0D
//#define                 0x0E
//#define                 0x0F


// BIT_SALVATI_EE1  0x10..0x1F
#define  REQ_LOG_DEBUG 0x10
#define  DEBUG_ON      0x11
#define  DEL_TS        0x12
//#define   0x12




// BIT_STATO_DEBUG  0x20..0x2F
#define DEBUG_FUNC                    0x20
#define DEBUG_TEL                     0x21
#define LOG_DEBUG                     0x22
#define DEBUG_REMOTO_ATTIVO           0x23
#define DEBUG_COM                     0x24
#define DEBUG_GPS                     0x25
#define AVVIO_AVVENUTO                0x26
#define TEST_ALARM_1                  0x27
#define PRINT_TEMP                    0x28
#define INTERVALLO_REGISTRAZIONE      0x29
#define ST_STATE_UPDATED              0x2A
#define FASE_ATTIVA                   0x2B
#define SENS_ON                       0x2C
#define SENS_PULSE                    0x2D
#define SENSORI_ATTIVI_A_TEMPO        0x2E



#define NETWORK_RESTART               0x30
//#define REC_ON                        0x31
#define MODEM_CONNECTED               0x32
#define MODEM_ASSENTE                 0x33
#define PARTITO                       0x34
#define VOX_ATTIVO_RITENUTO           0x35
#define REC_ON_SENS                   0x36
#define FTP_FAST_RUN                  0x37
#define TEST_UDP                      0x38
//#define           0x39
#define CONFIGURAZIONE_AVVENUTA       0x4A
#define TRANSCODE_RUN                 0x4B
#define REBOOT                        0x4C
#define RESET_RITARDATO               0x4D


// da 0x40 a 0x7F richieste
#define REQ_MODEM_ON                  0x40
#define REQ_CALIBRA_VOX               0x41
#define REQ_FAN_ON                    0x42
#define REQ_CAM1_ON                   0x43
#define REQ_XMODEM_ON                 0x44
#define REQ_RESET                     0x45
#define REQ_SLEEP                     0x46
#define REQ_HRESET                    0x47
#define REQ_GPS_ON                    0x48
#define REQ_WIFI_ON                   0x49


//#define                0x50

//#define                0x60

//#define                0x70

// da 0x80 a 0xBF stati ST0_15 .....
#define MODEM_ON            0x80
#define MOVIMENTO_IN_ATTO   0x81
#define VOX_ATTIVO          0x82
#define ST_CALIBRAZIONE_VOX_ESEGUITA  0x83
#define ST_CALIBRAZIONE_IN_CORSO    0x84
#define REQ_OFF             0x85
#define IN_SLEEP            0x86
#define ACCENSIONE_AVVENUTA 0x87
#define GPS_ON              0x88  //fusi +

//#define             0x90


//#define             0xA0

//#define             0xB0

// SVEGLIE  0xE0..0xEF    // solo sveglie
#define START_SV    0xE0
#define SV_TRANSFER START_SV
#define SV_MODEM    0xE1
#define SV_HOUR     0xE2
#define END_SV    0xEF



#define TERMINAL_REMOTE_RUNNING 0xD0
#define AVVIO_SCARICO 0xD1
#define COUNT_READY 0xD2
#define INIT_STATE  0xD3
#define JAMMING_GPS 0xD4 //fusi+
#define POSITION_RUN_STATE 0xD5//fusi+




#define SENSOR_LEN 16
#define SENS_DISABLED 0xFF



// SENSORI
#define     SENS_1      PARTITO
#define     SENS_2      SENS_DISABLED
#define     SENS_3      SENS_DISABLED
#define     SENS_4      VOX_ATTIVO
#define     SENS_5      SENS_DISABLED
#define     SENS_6      MOVIMENTO_IN_ATTO
#define     SENS_7      SENS_DISABLED
#define     SENS_8      SENS_DISABLED
#define     SENS_9      SENS_DISABLED
#define     SENS_10     CONFIGURAZIONE_AVVENUTA
#define     SENS_11     SENS_DISABLED
#define     SENS_12     SENS_DISABLED
#define     SENS_13     SENS_DISABLED
#define     SENS_14     SENS_DISABLED
#define     SENS_15     SENS_DISABLED
#define     SENS_16     SENS_DISABLED
#define     SENSORI_HISTORY  0xFFFF


// FUNZIONI CHE DIPENDONO DALL'IMMAGINE

#define FUNZIONE_NONE 0x00000000
#define FUNZIONE_CLOCKOVERLAY_PRESENTE 0x00000001
#define FUNZIONE_WIFI_PRESENTE 0x00000002

#define STR_FILENAME_VERSIONE_IMMAGINE "/mnt/ramdisk/img/versione.txt"
#define STR_FILENAME_FUNZIONE_CLOCKOVERLAY  "/mnt/ramdisk/img/fun/clockoverlay.txt"
#define STR_FILENAME_FUNZIONE_WIFI  "/mnt/ramdisk/img/fun/wifi.txt"

#define CHAR_FUNZIONE_CLOCKOVERLAY 'C'
#define CHAR_FUNZIONE_WIFI 'W'



#endif // HDV_H
