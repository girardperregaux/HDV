#include "mcgpsnmea.h"
#include "GpsDef.h"
#include "HDV.h"
#include "utility.h"
#include "tstate.h"
#include "mcgpsrx.h"
#include "qmutex.h"

extern MCSerial SerialPrint;
extern MCGpsRx Gps;
extern Position CurrPosition;
extern Position TmpPosition;
extern char buff_print[];
extern QMutex MtxPosData;

MCGpsNmea::MCGpsNmea(QObject *parent) :
    QObject(parent)
{
    pWorkData = &Gps.WorkData;
    pPosition = &CurrPosition;

}

MCGpsNmea::MCGpsNmea(Position *pPos,TGPSData *pWData)
{
    pWorkData = pWData;
    pPosition = pPos;

}

//   ReadGpsPacket   -> Analizza il pacchetto NMEA fornito e aggiorna le strutture
//                       GPS di conseguenza
//   Parametri       -> Stringa contenente il pacchetto da analizzare
//   Result          -> Per semplicità, il valore di ritorno è compatibile con il risultato
//                      del metodo RunGpsRx dell'oggetto MCGpsRx, basato sulle costanti
//                      GPSRECRUN_. Il risultato contiene sempre GPSRECRUN_STATUS_WORKING.
//----------------------


quint16 MCGpsNmea::ReadGpsPacket(quint8 *pPack)
{
   quint16 iStatus;                        // Variabile per comporre il valore di ritorno
   quint16 idx;
   char strLat1[GPS_NMEA_FIELD_LEN+1];  // Variabili temporanee per i campi dei pacchetti NMEA
   char strLon1[GPS_NMEA_FIELD_LEN+1];  //
   char strOra1[GPS_NMEA_FIELD_LEN+1];  //
   char strTmp[GPS_NMEA_FIELD_LEN+1];   //

   // Scrive contenuto del pacchetto se richiesto
   if(GetState(DEBUG_GPS))
   {
      snprintf(&buff_print[0], MAX_LEN_BUFFER_PRINT, "%s\r\n", pPack);
      SerialPrint.Flush(PRINT_DEBUG_GPS);
   }

   // Sicuramente il valore di ritorno indicherà almeno l'elaborazione di dati
   iStatus = GPSRECRUN_STATUS_WORKING;

   if(!CheckSumNMEA(pPack))
   {
       return(iStatus);
   }

   // Se necessario, indica la ricezione di un probabile pacchetto valido
   if(strstr((char*)pPack, "$GP") || strstr((char*)pPack, "$GL") ||
      strstr((char*)pPack, "$GA") || strstr((char*)pPack, "$GB") ||
      strstr((char*)pPack, "$GN"))
   {
         iStatus |= GPSRECRUN_VALID_PACKET;
   }

   // Probabile pacchetto in formato GSV: GPS Satellites in View
   if(strstr((char*)pPack, "$GPGSV"))
   {
      // Legge numero di messaggio totale e corrente
      iStatus |= GPSRECRUN_VALID_SAT_PAK;

      if (ReadGpsSatPacket(pPack))
      {
         iStatus |= GPSRECRUN_ALL_SAT_PAK;
         SetNMEAGPSSatData(pWorkData);
      }
   }

   // Probabile pacchetto in formato GSA: GPS Satellites in View
   if(ReadGpsIsGSA(pPack))
   {
      // Legge numero di messaggio totale e corrente
      iStatus |= GPSRECRUN_VALID_DOP_PAK;

      if (ReadGpsDopPacket(pPack))
        SetNMEAGPSDopData(pWorkData);
   }
   // Probabile pacchetto in formato GGA: GPS Fix Data
   if(ReadGpsIsGGA(pPack))
   {
      // Indica la probabile ricezione di un pacchetto di posizione valido
      iStatus |= GPSRECRUN_VALID_POS_PAK;


      // Se campo "fix indicator" del pacchetto GGA non presente o diverso da "1"
      if (  (!ReadGpsField(pPack, GPS_GGAFIELD_FIX, (quint8*) strTmp))
         || ((strcmp(strTmp, "1") != 0) && (strcmp(strTmp, "2") != 0))
         )
      {
         //SetState(POSITION_GLONASS,false);
         NmeaCodec.bDoPosition = false;
         // Indica nessuna nuova posizione

      }

      //strTmp

      // Se era disponibile un pacchetto RMC
      if(NmeaCodec.bLastPacketType == GPS_NMEA_PAK_RMC)
      {
         // Indica che almeno una coppia di pacchetti è stata ricevuta
          if(GetState(DEBUG_GPS))
          {
              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  almeno una coppia di pacchetti ricevuta\r\n");
              SerialPrint.Flush(PRINT_DEBUG_GPS);
          }
         iStatus |= GPSRECRUN_TWIN_POS_PAK;
      }
      // Legge e verifica la disponibilità dei dati di posizione
      if(GetState(DEBUG_GPS))
      {
          snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  Inizio valutazione GGA -1-\r\n");
          SerialPrint.Flush(PRINT_DEBUG_GPS);
      }
      if((ReadGpsField(pPack, GPS_GGAFIELD_LATITUDE,(quint8*) strLat1))  &&
         (ReadGpsField(pPack, GPS_GGAFIELD_LONGITUDE,(quint8*) strLon1)) &&
         (ReadGpsField(pPack, GPS_GGAFIELD_HOUR,(quint8*) strOra1))      )
      {
          if(GetState(DEBUG_GPS))
          {
              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  RMC ok dati di posizione\r\n");
              SerialPrint.Flush(PRINT_DEBUG_GPS);
          }
         // Se è dispobibile un pacchetto per la sincronizzazione
         if(NmeaCodec.bLastPacketType == GPS_NMEA_PAK_RMC)
         {
             if(GetState(DEBUG_GPS))
             {
                 snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  Inizio sincro -2-\r\n");
                 SerialPrint.Flush(PRINT_DEBUG_GPS);
             }

             // Prova a sincronizzare i pacchetti
            if(ReadGpsField(NmeaCodec.strLastPosPak, GPS_RMCFIELD_LATITUDE,(quint8*) strTmp)  &&
               strncmp(strTmp, strLat1, mcmin(strlen(strTmp), strlen(strLat1))) == 0        &&
               ReadGpsField(NmeaCodec.strLastPosPak, GPS_RMCFIELD_LONGITUDE,(quint8*) strTmp) &&
               strncmp(strTmp, strLon1, mcmin(strlen(strTmp), strlen(strLon1))) == 0       &&
               ReadGpsField(NmeaCodec.strLastPosPak, GPS_RMCFIELD_HOUR,(quint8*) strTmp)      &&
               strncmp(strTmp, strOra1, mcmin(strlen(strTmp), strlen(strOra1))) == 0        )
            {
               // Indica che il pacchetto è stato sincronizzato
               iStatus |= GPSRECRUN_POS_DATA;

               if(GetState(DEBUG_GPS))
               {
                   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  -2- *****POS DATA*****\r\n");
                   SerialPrint.Flush(PRINT_DEBUG_GPS);
               }


               // Imposta i dati di posizione sulla base dei pacchetti ricevuti
               //  (pacchetto GGA, primo parametro, e pacchetto RMC, secondo parametro)
               if(SetNMEAGPSData(pPack, NmeaCodec.strLastPosPak, pWorkData))
               {
                  // Se c'è una nuova posizione, lo segnala
                  iStatus |= GPSRECRUN_POS_FIX;
                  if(GetState(DEBUG_GPS))
                  {
                      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  -2- *****POS FIX*****\r\n");
                      SerialPrint.Flush(PRINT_DEBUG_GPS);
                  }

               }
            }
         }
      }



      // Se il pacchetto ricevuto è già stato usato per la sincronizzazione, non lo
      //  riutilizza in futuro, altrimenti lo confronta sempre con il pacchetto successivo
      if(!(iStatus & GPSRECRUN_POS_DATA))
      {
         // Copia il pacchetto GGA nell'area temporanea e lo segnala
         strncpy((char*) NmeaCodec.strLastPosPak,(char*) pPack, GPS_NMEA_MSG_LEN);

         NmeaCodec.strLastPosPak[GPS_NMEA_MSG_LEN] = 0;
         NmeaCodec.bLastPacketType = GPS_NMEA_PAK_GGA;
      }
      else
      {
         // Non usa il vecchio pacchetto per altre sincronizzazioni
         NmeaCodec.bLastPacketType = GPS_NMEA_PAK_NONE;
      }
      // Imposta comunque i dati relativi al numero di satelliti in vista
      ReadGpsField(pPack, GPS_GGAFIELD_SATELLITES, (quint8*) strTmp);
      NmeaCodec.bSatellites = atoi(strTmp);
   }  //end if




   // Probabile pacchetto in formato RMC: Recommended Minimum Specific GPS/Transit Data
   if(ReadGpsIsRMC(pPack))
   {

      // Indica la probabile ricezione di un pacchetto di posizione valido
      iStatus |= GPSRECRUN_VALID_POS_PAK;

      // Se campo "status" del pacchetto RMC non presente o diverso da "A"
      if (  (!ReadGpsField(pPack, GPS_RMCFIELD_STATUS, (quint8*) strTmp))
         || ((strcmp(strTmp, "A") != 0) && (strcmp(strTmp, "D") != 0) )
         )
      {
         // Indica nessuna nuova posizione
         pWorkData->bDoPosition = false;
      }

      // Se era disponibile un pacchetto GGA
      if(NmeaCodec.bLastPacketType == GPS_NMEA_PAK_GGA)
      {
         // Indica che almeno una coppia di pacchetti è stata ricevuta
         iStatus |= GPSRECRUN_TWIN_POS_PAK;
      }

      if(GetState(DEBUG_GPS))
      {
          snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  Inizio valutazione monitorRMC -1-\r\n");
          SerialPrint.Flush(PRINT_DEBUG_GPS);
      }
      // Legge e verifica la disponibilità dei dati di posizione
      if((ReadGpsField(pPack, GPS_RMCFIELD_LATITUDE, (quint8*) strLat1))  &&
         (ReadGpsField(pPack, GPS_RMCFIELD_LONGITUDE, (quint8*) strLon1)) &&
         (ReadGpsField(pPack, GPS_RMCFIELD_HOUR, (quint8*) strOra1))      )
      {
          if(GetState(DEBUG_GPS))
          {
              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  *****OK RMC LAT LONG HOUR*****\r\n");
              SerialPrint.Flush(PRINT_DEBUG_GPS);
          }
          // Se è dispobibile un pacchetto per la sincronizzazione
         if(NmeaCodec.bLastPacketType == GPS_NMEA_PAK_GGA)
         {
             if(GetState(DEBUG_GPS))
             {
                 snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  Inizio sincro -1-\r\n");
                 SerialPrint.Flush(PRINT_DEBUG_GPS);
             }
             // Prova a sincronizzare i pacchetti

             if(ReadGpsField(NmeaCodec.strLastPosPak, GPS_GGAFIELD_LATITUDE, (quint8*) strTmp)  &&
               strncmp(strTmp, strLat1, mcmin(strlen(strTmp), strlen(strLat1))) == 0        &&
               ReadGpsField(NmeaCodec.strLastPosPak, GPS_GGAFIELD_LONGITUDE, (quint8*) strTmp) &&
               strncmp(strTmp, strLon1, mcmin(strlen(strTmp), strlen(strLon1))) == 0        &&
               ReadGpsField(NmeaCodec.strLastPosPak, GPS_GGAFIELD_HOUR, (quint8*)strTmp)      &&
               strncmp(strTmp, strOra1, mcmin(strlen(strTmp), strlen(strOra1))) == 0        )
             {
               // Indica che il pacchetto è stato sincronizzato
               iStatus |= GPSRECRUN_POS_DATA;

               if(GetState(DEBUG_GPS))
               {
                   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket() -1- *****POS DATA*****\r\n");
                   SerialPrint.Flush(PRINT_DEBUG_GPS);
               }
               // Imposta i dati di posizione sulla base dei pacchetti ricevuti
               //  (pacchetto GGA, primo parametro, e pacchetto RMC, secondo parametro)
               if(SetNMEAGPSData(NmeaCodec.strLastPosPak, pPack, pWorkData))
               {
                  // Se c'è una nuova posizione, lo segnala
                  iStatus |= GPSRECRUN_POS_FIX;
                  if(GetState(DEBUG_GPS))
                  {
                      snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket() -1- *****POS FIX*****\r\n");
                      SerialPrint.Flush(PRINT_DEBUG_GPS);
                  }
               }
            }//end if SynchronizePacketGGA()
         }
      }
      // Se il pacchetto ricevuto è già stato usato per la sincronizzazione, non lo
      //  riutilizza in futuro, altrimenti lo confronta sempre con il pacchetto successivo
      if(!(iStatus & GPSRECRUN_POS_DATA))
      {
         // Copia il pacchetto RMC nell'area temporanea e lo segnala

         //strncpy((char*)NmeaCodec.strLastPosPak,(char*) pPack, GPS_NMEA_MSG_LEN);
         for(idx = 0; idx < GPS_NMEA_MSG_LEN; idx++)
         {
            NmeaCodec.strLastPosPak[idx] = pPack[idx];
         }
         NmeaCodec.strLastPosPak[GPS_NMEA_MSG_LEN] = 0;

         NmeaCodec.strLastPosPak[GPS_NMEA_MSG_LEN] = 0;
         NmeaCodec.bLastPacketType = GPS_NMEA_PAK_RMC;
      }
      else
      {
         // Non usa il vecchio pacchetto per altre sincronizzazioni
          if(GetState(DEBUG_GPS))
          {
              snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsPacket()  Non usa il vecchio pacchetto\r\n");
              SerialPrint.Flush(PRINT_DEBUG_GPS);
          }
         NmeaCodec.bLastPacketType = GPS_NMEA_PAK_NONE;
      }
   }//end ReadGpsIsRMC()

   // Indica lo stato di ricezione dati attuale
   return(iStatus);
}




bool MCGpsNmea::CheckSumNMEA(quint8 *dest)
{
    quint16 len;
    quint8 checksum=0;
    quint16 i;
    quint8 checksumNmea;

    len=strlen((char*)dest);

    for (i=0;i<len;i++)
    {
        if(dest[i] == '*')
        {
            checksumNmea=(quint8)StrHex2Byte(dest[i+1],dest[i+2]);
            if(checksum==checksumNmea)
            {
                return(true);
            }
            else
            {
                return(false);
            }
        }
        if(dest[i] == '$');
        else checksum = checksum^dest[i];
    }
    return (false);
}


//***********SET***********************

// Imposta dati sui satelliti basandosi su i pacchetti GSV
bool MCGpsNmea::SetNMEAGPSSatData(TGPSData* pWorkData)
{
   quint8 i;

   if (pWorkData)
   {
       for (i=0; i<NmeaCodec.bSatNumber; i++)
       {
          pWorkData->Sats[i].bPRN = NmeaCodec.Sats[i].bPRN;
          pWorkData->Sats[i].bElevation = NmeaCodec.Sats[i].bElevation;
          pWorkData->Sats[i].iZenith = NmeaCodec.Sats[i].iZenith;
          pWorkData->Sats[i].bSNR = NmeaCodec.Sats[i].bSNR;
       }
       pWorkData->bSatNumber = NmeaCodec.bSatNumber;
       return(true);
   }
   return(false);
}
// Imposta dati sui Dop e satelliti basandosi su i pacchetti GSA
bool MCGpsNmea::SetNMEAGPSDopData(TGPSData* pWorkData)
{
   quint8 i;

   if (pWorkData)
   {
       for (i=0; i<NmeaCodec.bPRNNumber; i++)
          pWorkData->PRNs[i] = NmeaCodec.PRNs[i];
       pWorkData->bPRNNumber = NmeaCodec.bPRNNumber;

       pWorkData->PDOP = NmeaCodec.PDOP;
       pWorkData->VDOP = NmeaCodec.VDOP;
       return(true);
   }
   return(false);
}

// Imposta dati di posizione da un pacchetto GGA e un pacchetto RMC. Entrambi i
// pacchetti devono contenere almeno latitudine, longitudine e ora attuale.
// Restituisce True se è appena diventata disponibile una nuova posizione.
// Il contenuto degli ultimi pacchetti ricevuti viene aggiornato, ma solo in
// caso di nuova posizione le strutture dati relative sono effettivamente aggiornate.
bool MCGpsNmea::SetNMEAGPSData(quint8 *strGGAPacket, quint8 *strRMCPacket, TGPSData *pWorkData)
{
   char strTmp[GPS_NMEA_MSG_LEN+1];   // Variabile temporanea per i campi dei pacchetti NMEA
   char strWork[8+1];                   // Variabile temporanea per la costruzione di data, ora estesa e velocità
   char strTmpDest[64];
   double velocity;

   // Per prima cosa, memorizza sempre i pacchetti puri ricevuti
   // Per prima cosa, memorizza sempre i pacchetti puri ricevuti
   sprintf(strTmp,"%s",strGGAPacket);
   strncpy((char*)pWorkData->Msg.GGA, strTmp, GPS_NMEA_MSG_LEN);
   pWorkData->Msg.GGA[GPS_NMEA_MSG_LEN] = 0;
   sprintf(strTmp,"%s",strRMCPacket);
   strncpy((char*)pWorkData->Msg.RMC, strTmp, GPS_NMEA_MSG_LEN);
   pWorkData->Msg.RMC[GPS_NMEA_MSG_LEN] = 0;

   // Se almeno uno dei campi richiesti non è disponibile, esce subito
   if (  (!ReadGpsField(strGGAPacket, GPS_GGAFIELD_SATELLITES,(quint8*) strTmp))
      || (!ReadGpsField(strGGAPacket, GPS_GGAFIELD_HDOP,(quint8*) strTmp))
      || (!ReadGpsField(strRMCPacket, GPS_RMCFIELD_DATE,(quint8*) strTmp))
      || (!ReadGpsField(strRMCPacket, GPS_RMCFIELD_HOUR,(quint8*) strTmp))
      || (!ReadGpsField(strRMCPacket, GPS_RMCFIELD_VELOCITY,(quint8*) strTmp))
      )
   {
      // Indica nessuna nuova posizione
      return(false);
   }

   // Se campo "status" del pacchetto RMC non presente o diverso da "A" e diverso da "D"
   if (  (!ReadGpsField(strRMCPacket, GPS_RMCFIELD_STATUS,(quint8*) strTmp))
      ||  ((strcmp(strTmp, "A") != 0) && (strcmp(strTmp, "D") != 0)))
   {
      // Indica nessuna nuova posizione
      pWorkData->bDoPosition = false;
      return(false);
   }

   // Se campo "fix indicator" del pacchetto GGA non presente o diverso da "1" e diverso da "2"
   if (  (!ReadGpsField(strGGAPacket, GPS_GGAFIELD_FIX,(quint8*) strTmp))
         || ((strcmp(strTmp, "1") != 0) && (strcmp(strTmp, "2") != 0))
      )
   {

       //SetState(POSITION_GLONASS,false);

      // Indica nessuna nuova posizione
      pWorkData->bDoPosition = false;
      return(false);
   }

   //fusi -
   /*
   if( strTmp[1]=='A' || strTmp[1]=='D')
   {
       SetState(POSITION_GLONASS,true);
   }
   else
   {
       SetState(POSITION_GLONASS,false);
   }*/

   pWorkData->bDoPosition = true;

   // Compila la nuova posizione disponibile
   pPosition->Clear();


   // Latitudine
   memset(strTmpDest,0,sizeof(strTmpDest));
   ReadGpsField(strGGAPacket, GPS_GGAFIELD_LATITUDE,(quint8*) strTmp);
   // SetGpsField(pPosition->strGPSLatitude, strTmp, 4, 3);
   SetGpsField(strTmpDest, strTmp, 4, 3);
   //pPosition->SetGpsLatitude((quint8*) strDest);
   ReadGpsField(strGGAPacket, GPS_GGAFIELD_LATITUDESGN,(quint8*) strTmp);
   strcat(strTmpDest, strTmp);
   pPosition->SetGpsLatitude((quint8*) strTmpDest);


   // Longitudine
   memset(strTmpDest,0,sizeof(strTmpDest));
   ReadGpsField(strGGAPacket, GPS_GGAFIELD_LONGITUDE,(quint8*) strTmp);
   SetGpsField(strTmpDest, strTmp, 5, 3);
   //SetGpsField(pWorkData->pPosition->strGPSLongitude, strTmp, 5, 3);
   ReadGpsField(strGGAPacket, GPS_GGAFIELD_LONGITUDESGN,(quint8*) strTmp);
   strcat(strTmpDest, strTmp);
   pPosition->SetGpsLongitude((quint8*) strTmpDest);
   //strcat(pWorkData->pPosition->strGPSLongitude, strTmp);

   // Altitudine
   memset(strTmpDest,0,sizeof(strTmpDest));
   ReadGpsField(strGGAPacket, GPS_GGAFIELD_ALTITUDE,(quint8*) strTmp);
   //SetGpsField(pWorkData->pPosition->strAltitude, strTmp, 5, 0);
   SetGpsField(strTmpDest, strTmp, 5, 0);
   pPosition->SetGpsAltitude((quint8*)strTmpDest);

   // Data
   ReadGpsField(strRMCPacket, GPS_RMCFIELD_DATE,(quint8*) strTmp);
   SetGpsField(strWork, strTmp, 6, 0);
   memmove(&strWork[6], &strWork[4], 2);
   strWork[4] = '2';
   strWork[5] = '0';
   strWork[8] = 0x00;
   pPosition->SetDate((quint8*) strWork);


   // Ora
   ReadGpsField(strRMCPacket, GPS_RMCFIELD_HOUR,(quint8*) strTmp);
   SetGpsField(strWork, strTmp, 6, 0);
   pPosition->SetHour((quint8*) strWork);

   // Velocità  //21082007
   //Filtro velocità se la velocità è maggiore di 250 km/h
//   ReadGpsField(strRMCPacket, GPS_RMCFIELD_VELOCITY, strTmp);
//   SetGpsField(strWork, strTmp, 3, 2);
//   pWorkData->pPosition->Velocity = atoi(strWork) / 55;
//   if(pWorkData->pPosition->Velocity > 250)
//   {
      // Indica nessuna nuova posizione
//      pWorkData->bDoPosition = false;
//      return(false);
//   }
   //21082007


   // Velocità  //21082007

   ReadGpsField(strRMCPacket, GPS_RMCFIELD_VELOCITY,(quint8*) strTmp);
   SetGpsField(strWork, strTmp, 3, 2);
   velocity = (0.018520*atof(strWork));
   pPosition->SetVelocity((quint16)velocity);
   if(pPosition->GetVelocity() > 250)
       pPosition->SetVelocity(250); //21082007

   // Direzione
   ReadGpsField(strRMCPacket, GPS_RMCFIELD_DIRECTION,(quint8*) strTmp);
   SetGpsField(strWork, strTmp, 3, 0);
   pPosition->SetDirection(atoi(strWork));


   // Numero di satelliti utilizzati per la posizione
   ReadGpsField(strGGAPacket, GPS_GGAFIELD_SATELLITES,(quint8*) strTmp);
   pPosition->SetSatellites(atoi(strTmp));


   // HDOP
   ReadGpsField(strGGAPacket, GPS_GGAFIELD_HDOP,(quint8*) strTmp);
   pPosition->SetHDOP(atof(strTmp));

   // Dati già verificati in precedenza
   //pWorkData->pPosition->Fix = '1';
   //pWorkData->pPosition->Status = 'A';
   pPosition->SetFix('1');
   pPosition->SetStatus('A');



//   if((pWorkData->pPosition->Velocity >0)
//   && (!GetState(MOVEMENTACTIVITY)))
//   {
//      if(GetFlag(DEBUG_FUNC)) printf("POS : SCARTATA\r\n");
//      return(false);
//   }
//   else
//   {
      //MODIFICA FILTRO POS
      // Conferma le modifiche all'oggetto Position




    if(pPosition->Validate() == true)
    {
        MtxPosData.lock();
        pPosition->CopyPosition(&TmpPosition); //copia la posizione corrente in TmpPosition
        MtxPosData.unlock();
        if(GetState(DEBUG_GPS))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SetNMEAGPSData()  Validate TRUE\r\n");
            SerialPrint.Flush(PRINT_DEBUG_GPS);
        }

    }
    else
    {
        MtxPosData.lock();
        TmpPosition.SetValid(false);
        MtxPosData.unlock();
        if(GetState(DEBUG_GPS))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SetNMEAGPSData()  Validate FALSE\r\n");
            SerialPrint.Flush(PRINT_DEBUG_GPS);
        }

    }


    //TPositionObj(pWorkData->pPosition)->Validate();






//   }
   // Indica che la posizione è stata rilevata
   return(true);
}

//   SetGpsField     -> Imposta una stringa NMEA a seconda del numero di interi e di decimali desiderati
//   Parametri       ->
//   Result          -> true/false
//----------------------
void MCGpsNmea::SetGpsField(char *str_risultato, char *txt_field, unsigned int num_interi, unsigned int num_decimali)
{
   quint8 IntStart = 0;
   quint8 IntLen = 0;
   quint8 DecStart = 0;
   quint8 DecLen = 0;
   char *str = txt_field;

   memset(str_risultato,'0', num_interi+num_decimali);
   str_risultato[num_interi+num_decimali] = 0;

   if (str[0] == '-') str[0] = 0;
   if (str[0] == '+') str[0] = '0';
   str += strcspn(txt_field,STR_ALFANUM);
   if (str == txt_field)
   {
      IntLen = strspn(str,STR_ALFANUM);
      if (str[IntLen] == '.')
      {
         str += IntLen+1;
         DecStart = IntStart + IntLen+1;
         DecLen = strspn(str,STR_ALFANUM);
      }

      if (IntLen > num_interi)
      {
         IntStart += IntLen-num_interi;
         IntLen = num_interi;
      }
      if (DecLen > num_decimali)
         DecLen = num_decimali;
   }

   memcpy( &str_risultato[num_interi-IntLen],  &txt_field[IntStart], IntLen);
   memcpy( &str_risultato[num_interi],  &txt_field[DecStart], DecLen);
}





//**********************END SET


//**********************READ

//   ReadGpsPacket   -> Analizza un pacchetto NMEA GSV fornito e aggiorna le strutture
//                      GPS di conseguenza
//   Parametri       -> Stringa contenente il pacchetto da analizzare
//   Result          -> Indica se sono arrivati tutti i messaggi GSV costituenti l'informazione sui satelliti
//----------------------
bool MCGpsNmea::ReadGpsSatPacket(quint8 *pPack)
{
   quint8 bTotalMsgNumber = 0;
   quint8 bCurrMsgNumber = 0;
   quint8 bSatNumber = 0;
   quint8 bSat;
   quint16 i;
   char strTmp[GPS_NMEA_FIELD_LEN+1];   //


   if (ReadGpsField(pPack, GPS_GSVFIELD_TOTMSG, (quint8*)strTmp))
      bTotalMsgNumber = atoi(strTmp);
   if (ReadGpsField(pPack, GPS_GSVFIELD_NMSG, (quint8*)strTmp))
      bCurrMsgNumber = atoi(strTmp);
   if (ReadGpsField(pPack, GPS_GSVFIELD_NSAT, (quint8*)strTmp))
      bSatNumber = atoi(strTmp);

   if ( (bSatNumber) && (bTotalMsgNumber) && (bCurrMsgNumber) && (bCurrMsgNumber <= bTotalMsgNumber) )
   {

      // Se è il primo dei pacchetti GSV azzero i Sat Ricevuti
      if (bCurrMsgNumber == 1)   NmeaCodec.bSatNumber = 0;
      // Se il numero di Sat Ricevuti è differente dal pacchetto ricevuto ritorno fallendo
      if ((bCurrMsgNumber-1)*4 != NmeaCodec.bSatNumber)
         return(false);

      for (i=0; i<4; i++)
      {
         // Se ho raggiunto il numero di satelliti visti esco

         if (NmeaCodec.bSatNumber >= bSatNumber)
            return(true);

         switch (i)
         {
            case 0 : bSat = GPS_GSVFIELD_SAT1;
                     break;
            case 1 : bSat = GPS_GSVFIELD_SAT2;
                     break;
            case 2 : bSat = GPS_GSVFIELD_SAT3;
                     break;
            case 3 :
            default: bSat = GPS_GSVFIELD_SAT4;
                     break;
         }
         NmeaCodec.Sats[NmeaCodec.bSatNumber].bPRN = 0;
         NmeaCodec.Sats[NmeaCodec.bSatNumber].bElevation = 0;
         NmeaCodec.Sats[NmeaCodec.bSatNumber].iZenith = 0;
         NmeaCodec.Sats[NmeaCodec.bSatNumber].bSNR = 0;

         if (ReadGpsField(pPack, bSat+GPS_GSVOFFSET_PRN,(quint8*) strTmp))
             NmeaCodec.Sats[NmeaCodec.bSatNumber].bPRN = atoi(strTmp);
         if (ReadGpsField(pPack, bSat+GPS_GSVOFFSET_ELEVATION,(quint8*) strTmp))
             NmeaCodec.Sats[NmeaCodec.bSatNumber].bElevation = atoi(strTmp);
         if (ReadGpsField(pPack, bSat+GPS_GSVOFFSET_AZIMUTH,(quint8*) strTmp))
             NmeaCodec.Sats[NmeaCodec.bSatNumber].iZenith = atoi(strTmp);
         if (ReadGpsField(pPack, bSat+GPS_GSVOFFSET_SNR, (quint8*) strTmp))
             NmeaCodec.Sats[NmeaCodec.bSatNumber].bSNR = atoi(strTmp);

         NmeaCodec.bSatNumber++;
      }

      if (NmeaCodec.bSatNumber >= bSatNumber)
         return(true);
   }
   return(false);
}


//   ReadGpsPacket   -> Analizza un pacchetto NMEA GSA fornito e aggiorna le strutture
//                       GPS di conseguenza
//   Parametri       -> Stringa contenente il pacchetto da analizzare
//   Result          -> Indica se sono arrivati tutti i messaggi GSA costituenti l'informazione sui satelliti
//----------------------
bool MCGpsNmea::ReadGpsDopPacket(quint8 *pPack)
{
   //quint8 bMode = 0;
   //quint8 bCurrMode = 0;
   quint8 bPRNNumber = 0;
   quint8 bSat;
   char strTmp[GPS_NMEA_FIELD_LEN+1];   //

/*
   if (ReadGpsField(pPack, GPS_GSAFIELD_MODE,(quint8*) strTmp))
      bMode = atoi(strTmp);
   if (ReadGpsField(pPack, GPS_GSAFIELD_CURRMODE,(quint8*) strTmp))
      bCurrMode = atoi(strTmp);
*/
   for (bSat=0; bSat<GPS_GSASIZE_PRN; bSat++)
   {
      if (ReadGpsField(pPack, bSat+GPS_GSAFIELD_PRN,(quint8*) strTmp))
         NmeaCodec.PRNs[bPRNNumber] = atoi(strTmp);
      else
         NmeaCodec.PRNs[bPRNNumber] = 0;
      if (NmeaCodec.PRNs[bPRNNumber] != 0)
         bPRNNumber++;
   }
   NmeaCodec.bPRNNumber = bPRNNumber;


   if (ReadGpsField(pPack, GPS_GSAFIELD_PDOP,(quint8*) strTmp))
      NmeaCodec.PDOP = atof(strTmp);
   if (ReadGpsField(pPack, GPS_GSAFIELD_VDOP,(quint8*) strTmp))
      NmeaCodec.VDOP = atof(strTmp);

   return(true);
}

// Legge campo NMEA e restituisce True se campo non vuoto. In caso di errore,
// comunque il buffer txt_field viene impostato a stringa nulla.
bool MCGpsNmea::ReadGpsField(quint8 *NMEAMsg, quint16 num_field, quint8 *txt_field)
{
   quint16 i, num_virgola;
   quint16 indice = 0;
   quint16 inizio = 2;
   bool    Result = false;

   txt_field[0] = 0;

   // trovo l'inizio del messaggio: $
/*
   for (i=0 ; i<strlen((char*)NMEAMsg) ; i++)
   {
     if ( (NMEAMsg[i]== '$') && (NMEAMsg[i+1]== 'G') && ((NMEAMsg[i+2]== 'P') || (NMEAMsg[i+2]== 'N')) )
     {
        inizio = i;
        break;
     }
   }

   // verifico che sia all'inizio del buffer
   if (inizio != 0)
      return (false);
*/

   if((NMEAMsg[0]== '$') && (NMEAMsg[1]== 'G') && ((NMEAMsg[2]== 'P') || (NMEAMsg[2]== 'N')))
   {
       inizio = 0;
   }
   else
   {
       return (false);
   }

   // trovo l'inizio del campo ricercato
   num_virgola=0;
   for (i=inizio ; i<strlen((char*)NMEAMsg) ; i++)
   {
     if ( (NMEAMsg[i] == '*') || (NMEAMsg[i] == CR) ||  (NMEAMsg[i] == LF) )
        return (false);
     if (NMEAMsg[i] == ',' )
        num_virgola++;
     if (num_virgola == num_field)
     {
        inizio = i+1;
        break;
     }
   }

   if (num_virgola != num_field)
      return(false);


   // lettura del campo
   for (i=inizio ; i<strlen((char*)NMEAMsg) ; i++)
   {
     if ((NMEAMsg[i] == ',') || (NMEAMsg[i] == '*'))
     {
         /*
         if(GetState(DEBUG_GPS))
         {
             snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::ReadGpsField() -3- out: %s - state:%d\r\n",txt_field,Result);
             SerialPrint.Flush(PRINT_DEBUG_GPS);
         }*/
         return (Result);
     }

     txt_field[indice++] = NMEAMsg[i];
     txt_field[indice] = 0;
     Result = true;
   }


   return (false);
}

//   ReadGpsPacket   -> Analizza un pacchetto NMEA per stabilire se il pacchetto è di tipo RMC
//   Parametri       -> Stringa contenente il pacchetto da analizzare
//   Result          -> true se pacchetto di tipo RMC
//----------------------
bool MCGpsNmea::ReadGpsIsRMC(quint8 *pPack)
{
    if(strstr((char*) pPack, "$GPRMC") || strstr((char*) pPack, "$GLRMC") ||
       strstr((char*) pPack, "$GARMC") || strstr((char*) pPack, "$GBRMC") ||
       strstr((char*) pPack, "$GNRMC"))
    {
            return (true);
    }
    return (false);
}

//   ReadGpsPacket   -> Analizza un pacchetto NMEA per stabilire se il pacchetto è di tipo GSA
//   Parametri       -> Stringa contenente il pacchetto da analizzare
//   Result          -> true se pacchetto di tipo GSA
//----------------------
bool MCGpsNmea::ReadGpsIsGSA(quint8 *pPack)
{
    if( strstr((char*)pPack, "$GPGSA") || strstr((char*)pPack, "$GLGSA") ||
        strstr((char*)pPack, "$GAGSA") || strstr((char*)pPack, "$GBGSA") ||
        strstr((char*)pPack, "$GNGSA"))
    {
            return (true);
    }
    return (false);
}

//   ReadGpsPacket   -> Analizza un pacchetto NMEA per stabilire se il pacchetto è di tipo GNS
//   Parametri       -> Stringa contenente il pacchetto da analizzare
//   Result          -> true se pacchetto di tipo GNS
//----------------------
bool MCGpsNmea::ReadGpsIsGNS(quint8 *pPack)
{
    if(strstr((char*)pPack, "$GPGNS") || strstr((char*)pPack, "$GLGNS") ||
       strstr((char*)pPack, "$GAGNS") || strstr((char*)pPack, "$GBGNS") ||
       strstr((char*)pPack, "$GNGNS"))
    {
            return (true);
    }
    return (false);
}


//   ReadGpsPacket   -> Analizza un pacchetto NMEA per stabilire se il pacchetto è di tipo GGA
//   Parametri       -> Stringa contenente il pacchetto da analizzare
//   Result          -> true se pacchetto di tipo GGA
//----------------------
bool MCGpsNmea::ReadGpsIsGGA(quint8 *pPack)
{
    if(strstr((char*)pPack, "$GPGGA") || strstr((char*)pPack, "$GLGGA") ||
       strstr((char*)pPack, "$GAGGA") || strstr((char*)pPack, "$GBGGA") ||
       strstr((char*)pPack, "$GNGGA"))
    {
            return (true);
    }
    return (false);
}

bool MCGpsNmea::SynchronizePacketGGA(quint8 *pPack,char* pLat, char* pLong, char* pHour)
{
    quint16 len;
    int retscmp;
    bool ret;
    char strTmp[GPS_NMEA_FIELD_LEN+1];

    memset(strTmp,0,sizeof(strTmp));
    if(GetState(DEBUG_GPS))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  **start**\r\n");
        SerialPrint.Flush(PRINT_DEBUG_GPS);
    }

    if(GetState(DEBUG_GPS))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  pPack: %s\r\n",pPack);
        SerialPrint.Flush(PRINT_DEBUG_GPS);
    }

    ret = ReadGpsField(pPack, GPS_GGAFIELD_LATITUDE, (quint8*) strTmp);
    if(ret == false)
    {

        return (false);
    }


    if(GetState(DEBUG_GPS))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  strTmp: %s\r\n",strTmp);
        SerialPrint.Flush(PRINT_DEBUG_GPS);
    }

    len = mcmin(strlen(strTmp), strlen(pLat));
    if(GetState(DEBUG_GPS))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  MIN len: %d\r\n",len);
        SerialPrint.Flush(PRINT_DEBUG_GPS);
    }
    retscmp = strncmp(strTmp, pLat, len);
    if(GetState(DEBUG_GPS))
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  retscmp: %d\r\n",retscmp);
        SerialPrint.Flush(PRINT_DEBUG_GPS);
    }

    if((ret == true) && (retscmp == 0))
    {
        if(GetState(DEBUG_GPS))
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  ok -1-\r\n");
            SerialPrint.Flush(PRINT_DEBUG_GPS);
        }
        ret = ReadGpsField(pPack, GPS_GGAFIELD_LONGITUDE, (quint8*) strTmp);
        len = mcmin(strlen(strTmp), strlen(pLong));
        retscmp = strncmp(strTmp, pLong, len);
        if((ret == true) && (retscmp == 0))
        {
            if(GetState(DEBUG_GPS))
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  ok -2-\r\n");
                SerialPrint.Flush(PRINT_DEBUG_GPS);
            }
            ret = ReadGpsField(pPack, GPS_GGAFIELD_HOUR, (quint8*)strTmp);
            len = mcmin(strlen(strTmp), strlen(pHour));
            retscmp = strncmp(strTmp, pHour, len);
            if((ret == true) && (retscmp == 0))
            {
                if(GetState(DEBUG_GPS))
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"MCGpsNmea::SynchronizePacketGGA()  ok -3-\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_GPS);
                }
                return (true);
            }
            return (false);

        }
        return (false);
    }

   return(false);
}
