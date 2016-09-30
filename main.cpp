#include <QCoreApplication>
#include "mcdatetime.h"
#include "mcvideoaudio.h"
#include <MCTimer.h>
#include "mcmodem.h"
#include <mcserial.h>
#include <terminal.h>
#include "timerman.h"
#include <stdlib.h>
#include <stdio.h>
#include "mcwifi.h"

#include <linux/i2c-dev.h>
#include <lis331dlh.h>
#include "tstate.h"
#include <mcgpsrx.h>
#include "config.h"
#include "AccessPointWifi.h"
#include"ConfigMobile.h"
#include "GestioneEventi.h"
#include "Log.h"
#include "multisinkvidaudio.h"
#include "message.h"
#include "framing.h"

#include <iostream>
#include "tupgrade.h"
#include "stm32.h"
#include "mcgestionedate.h"
#include "xmodem.h"

#include "mcgestionefs.h"
#include "mctcptest.h"
#include "mcfind.h"
#include "mctranscode.h"
#include "ftptransfert.h"
#include "mcfspace.h"
#include "mcspaceinfo.h"
#include "upgradest.h"
#include "GestioneMultimedia.h"
#include "vidaudiorec.h"
#include "vidaudiostream.h"
#include "ftpfast.h"
#include "ftptranscode.h"
#include "ftprun.h"
#include "Gestionewifi.h"

#include "ConfigRem.h"
#include "mcountfile.h"
#include "download.h"

extern TConfig Configuration;
extern char buff_print[];
extern Terminal *pTERMINAL;
bool en_console;



MCSerial SerialPrint;

McDateTime SystemDate;
AccessPointWifi APWifi;
ConfigMobile CONFIGMOBILE;
GestioneEventi GEventi;
TLog Log;
Framing Fram;
tUpgrade Upgrade;
Message MESSAGE;
Stm32 STM32;
MCModem Modem;
MCGpsRx Gps; //fusi+
Position CurrPosition; //fusi + posizione corrente
Position LastPosition; //fusi + last position
Position TmpPosition; //fusi +
QMutex MtxPosData;
mcgestionedate Gdate;
XModem XMODEM;
Mcgestionefs GestioneFs;
mctcptest REMLINK;
mctranscode transcode;
ftptransfert FTPtransf;
mcfspace FSpace;

mcspaceinfo SpaceInfo;
UpgradeSt UPGRADEST;

//gestione video
GestMultimedia Multimedia;
vidaudiorec Videoaudio;
MultisinkVidAudio Mtv;
vidaudiostream Videoaudiostream;
ftpfast FtpFastRec;

ftptranscode FtpTranscodeRun;

ftprun FTPTEST;

WifiClient clientwifi;



ConfigRem CONFIGREM;
TDownload DOWNLOAD;

MCountfile Countfile;

int main(int argc, char *argv[])
{

    QCoreApplication a(argc, argv);


    if(argc>1 && strcasecmp(argv[1],"2")==0) en_console=true;


    TimerMan Timerman;



    McVideoAudio McVideoAudioRun;



    //Mcwifi wifi;
    //wifi.init(&Timerman);


    XMODEM.Init(&Timerman);
    Terminal TerminalRun;
    TerminalRun.Init(&SystemDate,&McVideoAudioRun,&Timerman,&Modem);
    pTERMINAL=&TerminalRun;

    ConfigUpgradeinit();
    Configuration.Init();
    InitState();
    InitFunzioniAbilitate();


    // DA QUI E' POSSIBILIE IMPOSTARE GLI STATI !!!!!

    SetState(REQ_MODEM_ON,true);


    //MCGpsRx Gps; //fusi-
    Gps.Init(&Timerman);
    Gps.Start(100);

    CurrPosition.Init(&Timerman); //fusi+
    CurrPosition.Start(250);

    Modem.Init(&Timerman);
    Modem.Start(100);

    STM32.Init(&Timerman);
    STM32.Start(100);

    Gdate.Init(&Timerman);
    Gdate.Start(250);


    TerminalRun.PrintVer(false);

    APWifi.Init(&Timerman);
    CONFIGMOBILE.Init(&Timerman);
    CONFIGREM.Init(&Timerman);

    GEventi.Init(&Timerman);
    MESSAGE.Init(&Timerman);
    MESSAGE.Start(200);
    Log.Init(&Timerman);
    SystemDate.Init();
    Mtv.Init(&Timerman);
    Fram.Init(&Timerman);

    Upgrade.Init(&Timerman);
    UPGRADEST.Init(&Timerman);

    GestioneFs.Init(&Timerman);
    REMLINK.Init(&Timerman);


    //GestioneFind.Init(&Timerman);
    transcode.Init(&Timerman);
    FTPtransf.Init(&Timerman);
    FSpace.Init(&Timerman);

    SpaceInfo.Init(&Timerman);

    Multimedia.Init(&Timerman);
    Videoaudio.Init(&Timerman);
    Videoaudiostream.Init(&Timerman);

    FtpFastRec.Init(&Timerman);
    FtpTranscodeRun.Init(&Timerman);
    FTPTEST.Init(&Timerman);

    clientwifi.Init(&Timerman);

    Countfile.Init(&Timerman);

    DOWNLOAD.Init(&Timerman);

    SetState(AVVIO_AVVENUTO,true);

    SetWorkPeriod(ID_CPU,90,true);
    return a.exec();

}
