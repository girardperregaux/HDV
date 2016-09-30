#include "GestioneMultimedia.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QDebug>
#include <string.h>
#include <qstring.h>
#include <QFile>
#include <qdir.h>
#include "mcdatetime.h"
#include "config.h"
#include "tstate.h"
#include "mctranscode.h"
#include "mcgestionefs.h"
#include "mcmodem.h"
#include "mcfspace.h"
#include "multisinkvidaudio.h"
#include "vidaudiorec.h"
#include "vidaudiostream.h"

extern TConfig Configuration;
extern McDateTime SystemDate;
extern MCSerial SerialPrint;
extern char buff_print[];
extern mctranscode transcode;
extern Mcgestionefs GestioneFs;
extern MCModem Modem;
extern mcfspace FSpace;
extern MultisinkVidAudio Mtv;
extern vidaudiostream Videoaudiostream;
extern vidaudiorec Videoaudio;



GestMultimedia::GestMultimedia(QObject *parent) :
    QObject(parent)
{

}

void GestMultimedia::Init(TimerMan *t)
{

    pREGTIMER=t;

    TimerMultimedia.Init((char*)"TimerMultimedia");

    pREGTIMER->RegisterTimer(&TimerMultimedia); //timer registrato


    Multimedia.setInterval(10); //era ste!!!!
    Multimedia.start();
    connect(&Multimedia, SIGNAL(timeout()), this, SLOT(Run())); //scatta ogni 100ms per macchina a stati

    GMState=STATE_MEDIA_NULL;

    mod_video=VA_RS;

    strcpy(mod_video_param,"0");
    Update_mod_function=false;
}

bool GestMultimedia::Type(quint8 mod)
{
    mod_video=mod;
    return (true);
}

void GestMultimedia::Start()
{
    Startgest=true;
}


void GestMultimedia::Stop()
{
    Startgest=false;
}

void GestMultimedia::Run()
{

    switch(GMState)
    {
        case STATE_MEDIA_NULL:

            if(Startgest)
            {
                GMState=STATE_MEDIA_INIT;
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: STATE_MEDIA_INIT\r\n");
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
            }
        break;

        case STATE_MEDIA_INIT:

            //lancio a seconda della modalità

            if(mod_video==VA_RS)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: AVVIO GSTREAMER VA_RS(%d)\r\n",mod_video);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                Mtv.SetVideoRec(Configuration.Config.qualita_video_salvato);
                Mtv.SetVideoStream(Configuration.Config.qualita_video_stream);
                Mtv.Start();
            }
            else if(mod_video==VA_R)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: AVVIO GSTREAMER VA_R(%d)\r\n",mod_video);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                Videoaudio.SetVideoRec(Configuration.Config.qualita_video_salvato);
                Videoaudio.Type(mod_video);
                Videoaudio.Start();
            }
            else if(mod_video==VA_S)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: AVVIO GSTREAMER VA_S(%d)\r\n",mod_video);
                SerialPrint.Flush(PRINT_DEBUG_FUNC);

                Videoaudiostream.SetVideoStream(Configuration.Config.qualita_video_stream);
                Videoaudiostream.Start();
            }
            else if(mod_video==VA_RF)
            {
                snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: AVVIO GSTREAMER VA_RF(%d)\r\n",mod_video); //rec + ftp
                SerialPrint.Flush(PRINT_DEBUG_FUNC);
                Videoaudio.SetVideoRec(Configuration.Config.qualita_video_salvato);
                Videoaudio.Type(mod_video);
                Videoaudio.Start();
            }




            GMState=STATE_MEDIA_WAIT;

        break;

        case STATE_MEDIA_WAIT:

            if(mod_video==VA_RS)
            {
                if(Mtv.StartProc)
                {
                    GMState=STATE_MEDIA_READY_REC_STREAM;
                }
            }
            else if(mod_video==VA_R)
            {
                if(Videoaudio.StartProc)
                {
                    GMState=STATE_MEDIA_READY_REC;
                }
            }
            else if(mod_video==VA_S)
            {
                if(Videoaudiostream.StartProc)
                {
                    GMState=STATE_MEDIA_READY_STREAM;
                }
            }
            else if(mod_video==VA_RF)
            {
                if(Videoaudio.StartProc)
                {
                    GMState=STATE_MEDIA_READY_REC_FTP;
                }
            }

        break;

        case STATE_MEDIA_READY_REC_STREAM:

            Mtv.Run(); //avvio macchina a stati rec video+audio stream video+audio

            if((Mtv.EndMultisinkVidAudio))
            {
                Mtv.EndMultisinkVidAudio=false;

                if(Configuration.Config.modo_di_funzionamento !=VA_RS) //se ho cambiato modalità
                {
                    Update_mod_function=true;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: TERMINA REGISTRAZIONE E STREAM e AVVIO NUOVA MODALITA\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_NULL;
                }
                else
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: TERMINA REGISTRAZIONE E STREAM\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_STOP;
                }
                Startgest=false;
            }

            if((!Startgest) || (Configuration.Config.modo_di_funzionamento !=VA_RS)) //aggiunto se ho cambiato modo di funzionamento
            {
                Mtv.Stop();
            }
            else
            {
                Mtv.Start();
            }

        break;

        case STATE_MEDIA_READY_REC:

            Videoaudio.Run();   //avvio macchina a stati rec video+audio

            if((Videoaudio.EndVidAudio))
            {
                Videoaudio.EndVidAudio=false;

                if(Configuration.Config.modo_di_funzionamento !=VA_R) //se ho cambiato modalità
                {
                    Update_mod_function=true;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: TERMINA REGISTRAZIONE e AVVIO NUOVA MODALITA\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_NULL;
                }
                else
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: TERMINA REGISTRAZIONE\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_STOP;
                }
                Startgest=false;

            }

            if((!Startgest) || (Configuration.Config.modo_di_funzionamento !=VA_R)) //aggiunto se ho cambiato modo di funzionamento
            {
                Videoaudio.Stop();
            }
            else
            {
                Videoaudio.Start();
            }


        break;
        case STATE_MEDIA_READY_STREAM:

            Videoaudiostream.Run();   //avvio macchina a stati stream video+audio

            if((Videoaudiostream.EndVidAudioStream))
            {
                Videoaudiostream.EndVidAudioStream=false;

                if(Configuration.Config.modo_di_funzionamento !=VA_S) //se ho cambiato modalità
                {
                    Update_mod_function=true;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: GM: TERMINA STREAM e AVVIO NUOVA MODALITA\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_NULL;
                }
                else
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: TERMINA STREAM\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_STOP;
                }
                Startgest=false;
            }

            if((!Startgest) || (Configuration.Config.modo_di_funzionamento !=VA_S)) //aggiunto se ho cambiato modo di funzionamento
            {
                Videoaudiostream.Stop();
            }
            else
            {
              Videoaudiostream.Start();
            }

        break;
        case STATE_MEDIA_READY_REC_FTP:

            Videoaudio.Run();   //avvio macchina a stati rec video+audio

            if((Videoaudio.EndVidAudio))
            {
                Videoaudio.EndVidAudio=false;

                if(Configuration.Config.modo_di_funzionamento !=VA_RF) //se ho cambiato modalità
                {
                    Update_mod_function=true;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: TERMINA REGISTRAZIONE e AVVIO NUOVA MODALITA\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_NULL;
                }
                else
                {
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: TERMINA REGISTRAZIONE\r\n");
                    SerialPrint.Flush(PRINT_DEBUG_FUNC);
                    GMState=STATE_MEDIA_STOP;
                }
                Startgest=false;
            }

            if((!Startgest) || (Configuration.Config.modo_di_funzionamento !=VA_RF)) //aggiunto se ho cambiato modo di funzionamento
            {
                Videoaudio.Stop();
            }
            else
            {
               Videoaudio.Start();
            }

        break;
        case STATE_MEDIA_STOP:

            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM: STATE_MEDIA_STOP\r\n");
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
            GMState=STATE_MEDIA_NULL;
            Startgest=false;

        break;

        default:
        break;
    }

}

void GestMultimedia::PrintObj()
{

    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM= PrintObj\r\n");
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM= *GMState :%d\r\n",GMState);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"GM= *Startgest :%d\r\n",Startgest);
    SerialPrint.Flush(PRINT_DEBUG_ALL);
}



