#ifndef GPSDEF_H
#define GPSDEF_H

#include "position.h"

#define  GPS_NMEA_MSG_LEN     149        // Lunghezza massima messaggio in formato NMEA
#define  GPS_NMEA_FIELD_LEN   25         // Lunghezza massima singolo campo di pacchetto NMEA

// Costanti di stato relative al metodo RunGPS dell'oggetto GPSReceiver
#define  GPSRECRUN_STATUS_MASK       0x0003   // Ultima operazione svolta dal gestore GPS
                                            //  (dalla meno prioritaria alla più prioritaria):
#define  GPSRECRUN_STATUS_IDLE       0x0000   //   Nessuna operazione o cambiamento eseguito
#define  GPSRECRUN_STATUS_WORKING    0x0001   //   Operazione normale, di solito ricezione
#define  GPSRECRUN_STATUS_MAINTSEQ   0x0002   //   Operazione di inizializzazione in corso
#define  GPSRECRUN_STATUS_FATALERR   0x0003   //   Incoerenza grave, spegnere e riaccendere

#define  GPSRECRUN_VALID_PACKET      0x0004   // Bit attivato se ricevuto qualsiasi pacchetto valido
#define  GPSRECRUN_VALID_POS_PAK     0x0008   // Bit attivato se ricevuto un pacchetto di posizione valido qualsiasi
#define  GPSRECRUN_TWIN_POS_PAK      0x0010   // Bit attivato se ricevuti pacchetti di posizione GGA e RMC (se possibile, sincronizzati)
#define  GPSRECRUN_POS_DATA          0x0020   // Bit attivato se ricevuti effettivi dati di posizione, anche non aggiornati
#define  GPSRECRUN_POS_FIX           0x0040   // Bit attivato se sono disponibili nuovi dati di posizione
#define  GPSRECRUN_TSIP_INFOPAK      0x0080   // Bit attivato se è stato ricevuto il pacchetto TSIP 0x4B
#define  GPSRECRUN_VALID_SAT_PAK     0x0100   // Bit attivato se è stato ricevuto il pacchetto SATELLITI
#define  GPSRECRUN_ALL_SAT_PAK       0x0200   // Bit attivato se è stato ricevuto tutti i pacchetti SATELLITI
#define  GPSRECRUN_VALID_DOP_PAK     0x0400   // Bit attivato se è stato ricevuto il pacchetto DOP e SATELLITI ATTIVI

// Costanti relative al pacchetto MMEA RMC
#define  GPS_RMCFIELD_HOUR         1
#define  GPS_RMCFIELD_STATUS       2
#define  GPS_RMCFIELD_LATITUDE     3
#define  GPS_RMCFIELD_LATITUDESGN  4
#define  GPS_RMCFIELD_LONGITUDE    5
#define  GPS_RMCFIELD_LONGITUDESGN 6
#define  GPS_RMCFIELD_VELOCITY     7
#define  GPS_RMCFIELD_DIRECTION    8
#define  GPS_RMCFIELD_DATE         9

// Costanti relative al pacchetto MMEA GGA
#define  GPS_GGAFIELD_HOUR         1
#define  GPS_GGAFIELD_LATITUDE     2
#define  GPS_GGAFIELD_LATITUDESGN  3
#define  GPS_GGAFIELD_LONGITUDE    4
#define  GPS_GGAFIELD_LONGITUDESGN 5
#define  GPS_GGAFIELD_FIX          6
#define  GPS_GGAFIELD_SATELLITES   7
#define  GPS_GGAFIELD_HDOP         8
#define  GPS_GGAFIELD_ALTITUDE     9

// Costanti relative al pacchetto MMEA GSV
#define  GPS_GSVFIELD_TOTMSG       1
#define  GPS_GSVFIELD_NMSG         2
#define  GPS_GSVFIELD_NSAT         3
#define  GPS_GSVFIELD_SAT1         4
#define  GPS_GSVFIELD_SAT2         8
#define  GPS_GSVFIELD_SAT3         12
#define  GPS_GSVFIELD_SAT4         16

#define  GPS_GSVOFFSET_PRN         0
#define  GPS_GSVOFFSET_ELEVATION   1
#define  GPS_GSVOFFSET_AZIMUTH     2
#define  GPS_GSVOFFSET_SNR         3

// Costanti relative al pacchetto MMEA GSA
#define  GPS_GSAFIELD_MODE         1
#define  GPS_GSAFIELD_CURRMODE     2
#define  GPS_GSAFIELD_PRN          3
#define  GPS_GSAFIELD_PDOP         15
#define  GPS_GSAFIELD_HDOP         16
#define  GPS_GSAFIELD_VDOP         17

#define  GPS_GSASIZE_PRN           12
// Costanti per le funzioni avanzate del ricevitore GPS

#define  GPS_ADV_EXT_DATA          1
//  Funzione: Richiesta attivazione, disattivazione o verifica modalità dati estesi GPS.
//             La condizione impostata permane anche attraverso le accensioni
//             e gli spegnimenti richiesti al dispositivo.
//  Parametro: Funzione richiesta, in base alle seguenti costanti
#define  GPS_ADV_EXT_DATA_OFF      0   // Disattivazione
#define  GPS_ADV_EXT_DATA_ON       1   // Attivazione
#define  GPS_ADV_EXT_DATA_CHECK    2   // Richiesta stato attuale
//  Valore di ritorno: Restituisce lo stato attuale o il nuovo stato di attivazione
//                      della modalità estesa, True (funzione attiva) oppure 0
//                      (funzione inattiva). Se, in seguito alla richiesta
//                      GPS_ADV_EXT_DATA_ON, il risultato è 0, significa che la
//                      modalità estesa non è supportata dal modello di GPS indicato.

#define  GPS_SAT_SIZE             12
// ---------------------------
//  Strutture Ad Uso Generale
// ---------------------------

// Informazioni sui pacchetti NMEA ricevuti, utilizzate dall'applicativo.
// KW: Verificarne l'uso in Message.c e MessageUser.c
typedef struct
{
  quint8 GGA[GPS_NMEA_MSG_LEN+1];   // Pacchetto NMEA GGA originale, non modificato, o stringa vuota
  quint8 RMC[GPS_NMEA_MSG_LEN+1];   // Pacchetto NMEA RMC originale, non modificato, o stringa vuota
} TGPSMsg;

typedef struct
{
  quint8     bPRN;              // Numero identificativo Satellite
  quint8     bElevation;        // Elevation sopra l'orizzonte del satellite (Gradi)
  quint16    iZenith;           // Zenith (Gradi)
  quint8     bSNR;              // Potenza Segnale (0..99DB)
} TGPSSat;

// Informazioni utilizzate dall'applicativo relative al sistema GPS
typedef struct
{
  TGPSMsg  Msg;          // Ultimi pacchetti NMEA coerenti e utili ricevuti, se applicabile


  quint8  strLastPosPak[GPS_NMEA_MSG_LEN+1]; // Ultimo pacchetto GGA o RMC ricevuto, non modificato //fusi +
  quint8  bLastPacketType;                   // Tipo di pacchetto memorizzato (enum GPS_PACKET_TYPE) //fusi +


  //Position *pPosition;    // Riferimento all'oggetto Position della periferica
  quint8  bSatellites;  // Numero di satelliti attualmente visibili dal ricevitore
  quint8  bDoPosition;  // Posizione in Atto

  // I seguenti dati sono aggiornati solo quando il GPS si trova in modalità dati
  //  estesi, impostata tramite il metodo SetExtendedMode dell'oggetto GPS
  quint8 bSatNumber;                // Numero di Satelliti Presenti
  TGPSSat Sats[GPS_SAT_SIZE];        // Satelliti

  quint8 bPRNNumber;                // Numero di Satelliti in uso
  quint8 PRNs[GPS_SAT_SIZE];        // PRN dei Satelliti usati

  float PDOP;
  float VDOP;

  quint8 bAlmanac;     // (esteso) True se almanacco presente
} TGPSData;




#endif // GPSDEF_H
