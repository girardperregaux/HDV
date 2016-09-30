#ifndef DEFCOM_H
#define DEFCOM_H

#define mcmin(a,b) (((a) < (b)) ? (a) : (b))
#define mcmax(a,b) (((a)>(b))?(a):(b))

#define MAX_LEN_CAMPO 50
// era #define MAX_LEN_SMS 160
#define MAX_LEN_SMS (4*MAX_LEN_CAMPO)

#define STR_ALFANUM "0123456789"
#define STRNULL " "

#define MAX_LEN_SN 5
#define MAX_LEN_PASSWORD 6


#define    CRC_FLAG             0      // Nessun Flag
#define    CRC_FLAG_INREV       1      // Il carattere entrante interpretato ribaltato MSB -> LSB e LSB -> MSB
#define    CRC_FLAG_OUTREV      2      // Il CRC uscente generato ribaltato MSB -> LSB e LSB -> MSB
#define    CRC_FLAG_OUTNEG      4      // Il CRC uscente generato viene Negato(Modulo 1)


#define UN_OTTAVO                 1
#define UN_QUARTO      (UN_OTTAVO*2)
#define UN_MEZZO       (UN_QUARTO*2)
#define UN_SECONDO      (UN_MEZZO*2)
#define UN_MINUTO    (UN_SECONDO*60)
#define UN_ORA        (UN_MINUTO*60)
#define UN_GIORNO        (UN_ORA*24)
#define STR_NO        ""

//#define UN_QUARTO      2
//#define UN_MEZZO       4
//#define UN_SECONDO     8
//#define UN_MINUTO      480
//#define UN_ORA        UN_MINUTO*60
//#define UN_GIORNO        UN_ORA*24

#define PRINT_DEBUG_NO  0
#define PRINT_DEBUG_FUNC 0x0001
#define PRINT_DEBUG_TEL  0x0002
#define PRINT_DEBUG_GPS 0x0004
#define PRINT_DEBUG_COM 0x0008
#define PRINT_DEBUG_ALL 0xFFFF

#define MAX_LEN_BUFFER_PRINT 1000

#define ATFIELD_LEN MAX_LEN_CAMPO


#define 	   CTRLZ    0x1A

#define STR_CANALE_NO    ""
#define STR_CANALE_SMS   "SMS"
#define STR_CANALE_DATI  "DATI"
#define STR_CANALE_UDP   "UDP"
#define STR_CANALE_TCP   "TCP"

#define ID_CANALE_NONE   0
#define ID_CANALE_SMS   1
#define ID_CANALE_DATI  2
#define ID_CANALE_UDP   3
#define ID_CANALE_TCP   4


#define     VREF              3.3
#define     VOFFSET						0.38
#define     R1ATD             40.2
#define     R3ATD             3.24
#define 		VINMAX						40

#define PROTOCOL_NONE 0
#define PROTOCOL_XMODEM 1



#define DEF_ADD_FTP_UPGRADE "ftp.mcsistemi.it"
#define DEF_USER_FTP_UPGRADE "1928272@aruba.it"
#define DEF_PW_FTP_UPGRADE "o5chad6z"
#define DEF_PORT_FTP_UPGRADE "21"
#define DEF_FOLDER_FTP_UPGRADE_ST "/mcsistemi.it/fwREMHDV/st/"
#define DEF_FOLDER_FTP_UPGRADE_IC "/mcsistemi.it/fwREMHDV/"
#define CHR_OVIZZATA 'v'

#define MAX_LEN_STR_FOLDER 6
#define MAX_LEN_STR_HOUR_FOLDER 31
#define MAX_LEN_STR_DATE_HOUR 12
#define MAX_LEN_STR_DATE 6
#define MAX_LEN_STR_HOUR 6

#define MAX_LEN_STR_FTP 7

#define MODO_2ATC 2
#define MODO_1ATC 1
#define MODO_0ATC 0


#define TYPE_NULL 0
#define TYPE_DIR  1
#define TYPE_MP4  2
#define TYPE_MPX  3
#define TYPE_MPW  4
#define TYPE_TW   5
#define TYPE_TX   6
#define TYPE_TS   7
#define TYPE_MPZ  8
#define TYPE_ALL  9
#define TYPE_TZ  10
#define TYPE_TY  11


#define SPACE_FS_1        1
#define SPACE_FS_2        2
#define SPACE_FS_OK       3




#define TRANSFERT_DATE_INIT    "010101"
#define TRANSFERT_HOUR_INIT    "000000"
#define TRANSFERT_DATE_FINISH  "990101"
#define TRANSFERT_HOUR_FINISH  "235959"


#define CHARNULL ' '

#define READ_RESULT_OK 1
#define READ_RESULT_FILE_NOT_FOUND 2
#define READ_RESULT_CRC_ERROR 3
#define READ_RESULT_NOT_OPEN 4
#define READ_RESULT_FILE_ERROR 5

#define TIPO_APPARATO         'R'

#define MOBILE_PERIFERICA_TIPO_APPARATO   'V' // todo per video mettere 'V'
#define MOBILE_TABLET_TIPO_APPARATO   'M'

#define PATH_FILE_MAX 50
#define FILE_NAME_MAX 20


#define FOLDER_UPDATE_ST "/home/root/upgrade_pro/st/"
#define NAME_FILE_ST "/home/root/upgrade_pro/st/updatest.bin"

#define FOLDER_UPDATE "/home/root/upgrade_pro/"

#define ID_CPU   1
#define ID_MODEM 2

#define ASCII_ESC 27

#define PID_MAX  20
#define SCAN_DIR     "/mnt/msata/videoaudio/"
#define STRING_LINUX 200

#define STRING_IP_MAX                     20

#define STR_PATH "/mnt/msata/videoaudio/"
#define EXTENSION  ".ts"

#define STR_CD_VIDEO "videoaudio"
#define STR_CD_LOGS "logs"

#define STR_FILE_TR_COUNT "/mnt/msata/file_transfert.txt"


#define SENSORI_SEMPRE_ATTIVI 2


#define STR_NOME_FILE_CNFUPG_1 "/home/root/run_pro/cnfupg.txt"
#define STR_NAME_FILE_BIT0 "/home/root/run_pro/state0.rem"
#define STR_NAME_FILE_BIT1 "/home/root/run_pro/state1.rem"
#define STR_PYTHON_PROG "python /home/root/run_pro/ttcprx.py"
#define STR_NOME_FILE_CNF_1 "/home/root/run_pro/cnf1.txt"
#define STR_NOME_FILE_CNF_2 "/home/root/run_pro/cnf2.txt"
#define STR_NOME_FILE_POS "/home/root/run_pro/position.pos" //fusi+
#define STR_NOME_FILE_POSTEST "/home/root/run_pro/testpos.pos" //fusi+

#define STR_PYTHON_COUNT "python /home/root/run_pro/count_file.py"
#define STR_PY_FAST_PROG "python /home/root/run_pro/ftpfast.py"
#define   HOME "/mnt/msata/"
#define STR_NOME_FILE_INFO_UPGRADE "/home/root/run_pro/infoupgres.txt"
#define STR_NOME_FILE_INFO_UPGRADEST "/home/root/run_pro/infoupgresst.txt"
#define STR_NOME_FILE_INFO_DOWNLOAD "/home/root/run_pro/download.txt"

#define RESULT_NULL          0
#define RESULT_OK            1
#define RESULT_ERROR         2

#define VA_RS   1
#define VA_R    2
#define VA_S    3
#define VA_RF   4

#define PORT_PY_PROG 7001
#define PORT_PY_FAST 7050
#define PORT_PY_COUNT 7051

// PORTA 7501 PER GSTREAMER
#define PORT_UDP 7500

#define VALUE_OK   1
#define VALUE_KO   2
#define VALUE_NULL 3

#define N_RETRY 3
#define STRING_FILENAME_MAX              100

#define PATH_REM_HDV            "/home/root/"
#define PATH_REM_HDV_RUN        "/home/root/run_pro/"


#define PATH_REM_PYFAST        "/home/root/run_pro/ftpfast.py"
#define PATH_REM_PYFTP         "/home/root/run_pro/ttcprx.py"
#define   MAX_CAMPI  5

#define MONO 1
#define STEREO 2

#define FTP_AUTO 1
#define FTP_REQ  2
#define REQFTP_REQ "/mnt/msata/ftp_req.txt"
#define REQFTP_AUTO "/mnt/msata/ftp_auto.txt"

#define SIZE_TRANSF_MAX 1000
#define MAX_LEN_STR_DATA_E_ORA 12

#define IMEI 15

#define STRING_F_MAX  100
#define WPA_SUPPLICANT  "/etc/wpa_supplicant.conf"
#define STR_MIN_VERSIONE_IMMAGINE "1.00"


#define MCFINDTRANSODE 1
#define MCFINDTRANSFERT 2
#define MCFINDSPACEFS 3
#define MCFINDSPACEINFO 4
#define MCFINDERTRANSFERFAST 5
#define MCFINDERFTPTRANSCODE 6

#define   FORMAT_USER     0x00
#define   FORMAT_GOOGLE   0x03

#endif // DEFCOM_H


