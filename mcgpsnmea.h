#ifndef MCGPSNMEA_H
#define MCGPSNMEA_H

#include <QObject>
#include <GpsDef.h>
#include <position.h>


// Tipi di pacchetti salvati nell'area temporanea
enum GPS_NMEA_PAK_ {
   GPS_NMEA_PAK_NONE = 0,     // Nessun pacchetto presente (valore predefinito)
   GPS_NMEA_PAK_GGA,          // Pacchetto GGA
   GPS_NMEA_PAK_RMC           // Pacchetto RMC
};

class MCGpsNmea : public QObject
{
    Q_OBJECT
public:
    explicit MCGpsNmea(QObject *parent = 0);
    explicit MCGpsNmea(Position *pPos,TGPSData *pWData);
    quint16  ReadGpsPacket(quint8 *strPacket);
signals:

public slots:

private:
    bool CheckSumNMEA(quint8 *dest);
    bool ReadGpsField(quint8 *NMEAMsg, quint16 num_field, quint8 *txt_field);
    bool ReadGpsSatPacket(quint8 *pPack);
    bool ReadGpsDopPacket(quint8 *pPack);
    bool SetNMEAGPSSatData(TGPSData *pWorkData);
    bool SetNMEAGPSDopData(TGPSData *pWorkData);
    bool SetNMEAGPSData(quint8 *strGGAPacket, quint8 *strRMCPacket, TGPSData *pWorkData);
    void SetGpsField(char *str_risultato, char *txt_field, unsigned int num_interi, unsigned int num_decimali);
    bool ReadGpsIsRMC(quint8 *pPack);
    bool ReadGpsIsGSA(quint8 *pPack);
    bool ReadGpsIsGNS(quint8 *pPack);
    bool ReadGpsIsGGA(quint8 *pPack);
    bool SynchronizePacketGGA(quint8 *pPack,char* pLat, char* pLong, char* pHour);
    TGPSData *pWorkData;
    TGPSData  NmeaCodec;
    Position *pPosition;    // Riferimento all'oggetto Position della periferica

protected:

};

#endif // MCGPSNMEA_H
