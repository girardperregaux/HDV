#include "mcvideoaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QDebug>
#include <string.h>
#include <qstring.h>
#include "terminal.h"



McVideoAudio::McVideoAudio(QObject* parent)
    :QObject(parent)
{

    gstreamer= new QProcess(this);


}


void McVideoAudio::Init(McDateTime *p,TimerMan *t)
{
    pDATETIME=p; //ottengo data e time li assegno
    pREGTIMER=t;

    TimerT.Init((char*)"McVideoAudioTimer");
    TimerT.SetTimer(UN_SECONDO*10);

    pREGTIMER->RegisterTimer(&TimerT);

    DefaultValue();

    McVideoAudiotimer.setInterval(100);
    McVideoAudiotimer.start();
    connect(&McVideoAudiotimer, SIGNAL(timeout()), this, SLOT(Run()));
    StateMachine=STATE_NULL;
    Stop();
}



void McVideoAudio::NullValue()
{
    strcpy (dimx,STRNULL);
    strcpy (dimy,STRNULL);
    strcpy (frame,STRNULL);
    strcpy (dimx_stream,STRNULL);
    strcpy (dimy_stream,STRNULL);
    strcpy (frame_stream,STRNULL);
    strcpy (ip,STRNULL);
    strcpy (portaudp,STRNULL);
    strcpy (pDATETIME->datename,STRNULL);
    strcpy (file_name_vid,STRNULL);
    strcpy (file_name_audio,STRNULL);
    strcpy (file_name_vid_audio,STRNULL);
}

void McVideoAudio::DefaultValue()
{

    ModVideoAudio=MCVIDAUDIO_ID_VIDEO_REC;
    strcpy(dimx,"640");
    strcpy(dimy,"480");
    strcpy(frame,"20");
    strcpy(dimx_stream,"640");
    strcpy(dimy_stream,"480");
    strcpy(frame_stream,"15");
    strcpy(ip,"192.168.1.37");
    strcpy(portaudp,"5000");

    strcpy(pDATETIME->datename,"data");

    SetfileSink();

}



void McVideoAudio::SetfileSink()
{
    //solo video

    strcpy(file_name_vid,STR_PATH);
    strcat(file_name_vid,dimx);
    strcat(file_name_vid,"X");
    strcat(file_name_vid,dimy);
    strcat(file_name_vid,"_");
    strcat(file_name_vid,frame);
    strcat(file_name_vid,"_");
    strcat(file_name_vid,pDATETIME->datename);
    strcat(file_name_vid,".h264");
    //video+audio
    strcpy(file_name_vid_audio,dimx);
    strcat(file_name_vid_audio,"X");
    strcat(file_name_vid_audio,dimy);
    strcat(file_name_vid_audio,"_");
    strcat(file_name_vid_audio,frame);
    strcat(file_name_vid_audio,"_audio_");
    strcat(file_name_vid_audio,pDATETIME->datename);
    strcat(file_name_vid_audio,".h264");
    //solo audio
    strcpy(file_name_audio,"audio_");
    strcat(file_name_audio,pDATETIME->datename);
    strcat(file_name_audio,".ogg");


}


void McVideoAudio::SetVideoRec(int mode)
{
    if(mode==1)
    {
        strcpy(dimx,"160");
        strcpy(dimy,"120");
    }
    else if(mode==2)
    {
        strcpy(dimx,"320");
        strcpy(dimy,"240");
    }
    else if(mode==3)
    {
        strcpy(dimx,"640");
        strcpy(dimy,"480");
    }
    else if(mode==4)
    {
        strcpy(dimx,"720");
        strcpy(dimy,"576");
    }
}
void McVideoAudio::SetVideoStream(int mode)
{
    if(mode==1)
    {
        strcpy(dimx_stream,"160");
        strcpy(dimy_stream,"120");
    }
    else if(mode==2)
    {
        strcpy(dimx_stream,"320");
        strcpy(dimy_stream,"240");
    }
    else if(mode==3)
    {
        strcpy(dimx_stream,"640");
        strcpy(dimy_stream,"480");
    }
    else if(mode==4)
    {
        strcpy(dimx_stream,"720");
        strcpy(dimy_stream,"576");
    }

}




void McVideoAudio::SetModVidAud(int mode)
{
    ModVideoAudio=mode;

    if(mode==MCVIDAUDIO_ID_VIDEO_REC)
    {
        printf ("*modalita            : 1 registra video\r\n");
    }
    else if(mode==MCVIDAUDIO_ID_VIDEO_STREAM)
    {
        printf ("*modalita            : 2 streaming video\r\n");
    }
    else if(mode==MCVIDAUDIO_ID_VIDEO_REC_VIDEO_STREAM)
    {
        printf ("*modalita            : 3 registra video e streaming video\r\n");
    }
    else if(mode==MCVIDAUDIO_ID_VIDEO_NO__AUDIO_REC)
    {
        printf ("*modalita            : 4 registra audio\r\n");
    }
    else if(mode==MCVIDAUDIO_ID_VIDEO_NO_AUDIO_STREAM)
    {
        printf ("*modalita            : 5 streaming audio\r\n");
    }
    else if(mode==MCVIDAUDIO_ID_VIDEO_REC_AUDIO_REC)
    {
        printf ("*modalita            : 6 registra video e registra audio\r\n");
    }
    else if(mode==MCVIDAUDIO_ID_VIDEO_REC_AUDIO_STREAM)
    {
        printf ("*modalita            : 7 registra video e streaming audio\r\n");
    }
    else if(mode==MCVIDAUDIO_ID_VIDEO_STREAM_AUDIO_STREAM)
    {
        printf ("*modalita            : 8 streaming video e streaming audio\r\n");
    }
}


void McVideoAudio::Start()
{
    RecRec=true;
    StateRec=false;
    Pid=0;
}

void McVideoAudio::Stop()
{
    RecRec=false;
    StateRec=false;
}

bool McVideoAudio::IsStart()
{
    return StateRec;
}



void McVideoAudio::Run()
{

    if(StateMachine==STATE_NULL)
    {
        StateRec=false;

        if(RecRec==true)
        {
            StateMachine=STATE_INIT;
        }
    }
    else if(StateMachine==STATE_INIT)
    {
        SetfileSink();

        char Buffer_string[1000]; //controllare dimensione max buffer
        char *pBuff;


        pBuff=&Buffer_string[0];



        switch(ModVideoAudio)
        {
            case MCVIDAUDIO_ID_VIDEO_REC:

                pBuff+=sprintf(pBuff," tvsrc ! mfw_ipucsc ");

                pBuff+=sprintf(pBuff,"! video/x-raw-yuv,width=%s,height=%s,framerate=%s/1,format=(fourcc)I420 ",&dimx[0],&dimy[0],&frame[0]);

                pBuff+=sprintf(pBuff,"! queue ! vpuenc codec=6 ! queue ! avimux name=mux ");

                pBuff+=sprintf(pBuff,"! filesink location=%s",&file_name_vid[0]);

                printf("%s\r\n",&Buffer_string[0]);

                printf("\r\n");

                break;

            case MCVIDAUDIO_ID_VIDEO_STREAM:

                pBuff+=sprintf(pBuff," tvsrc ! mfw_ipucsc ");

                pBuff+=sprintf(pBuff,"! video/x-raw-yuv,width=%s,height=%s,framerate=%s/1,format=(fourcc)I420 ",&dimx_stream[0],&dimy_stream[0],&frame_stream[0]);

                pBuff+=sprintf(pBuff,"! queue ! vpuenc codec=6 ! queue ");

                pBuff+=sprintf(pBuff,"! udpsink host=192.168.1.37 port=5000 ");

                printf("%s\r\n",&Buffer_string[0]);

                break;

            case MCVIDAUDIO_ID_VIDEO_REC_VIDEO_STREAM:

                pBuff+=sprintf(pBuff," tvsrc ! mfw_ipucsc ");

                pBuff+=sprintf(pBuff,"! tee name=divido ! queue ! videoscale ! video/x-raw-yuv,width=%s,height=%s ",&dimx_stream[0],&dimy_stream[0]);

                pBuff+=sprintf(pBuff,"! vpuenc codec=6 ! queue ! udpsink host=192.168.1.37 port=5000 divido. ");

                pBuff+=sprintf(pBuff,"! queue ! videoscale ! video/x-raw-yuv,width=%s,height=%s,framerate=%s/1 ",&dimx[0],&dimy[0],&frame[0]);

                pBuff+=sprintf(pBuff,"! vpuenc codec=6 ! queue ! avimux name=mux ! filesink location=%s ",&file_name_vid[0]);

                printf("%s\r\n",&Buffer_string[0]);

                break;


            case MCVIDAUDIO_ID_VIDEO_NO__AUDIO_REC:

                pBuff+=sprintf(pBuff," alsasrc device=\"hw:0,0\" ");

                pBuff+=sprintf(pBuff,"! audioconvert ! vorbisenc ! oggmux ! filesink location=%s",&file_name_audio[0]);

                printf("%s\r\n",&Buffer_string[0]);

                break;

            case MCVIDAUDIO_ID_VIDEO_NO_AUDIO_STREAM:

                //prova per stringa
                pBuff+=sprintf(pBuff," tvsrc ! mfw_ipucsc ");

                pBuff+=sprintf(pBuff,"! video/x-raw-yuv,width=640,height=480,framerate=20/1,format=(fourcc)I420");

                pBuff+=sprintf(pBuff,"! queue ! vpuenc codec=6 ! queue ! avimux name=mux ");

                pBuff+=sprintf(pBuff,"! filesink location=%s",&file_name_vid[0]);

                printf("%s\r\n",&Buffer_string[0]);
                break;


            case MCVIDAUDIO_ID_VIDEO_REC_AUDIO_REC:


                pBuff+=sprintf(pBuff," tvsrc ! mfw_ipucsc ");

                pBuff+=sprintf(pBuff,"! video/x-raw-yuv,width=%s,height=%s,framerate=%s/1,format=(fourcc)I420 ",&dimx[0],&dimy[0],&frame[0]);

                pBuff+=sprintf(pBuff,"! queue2 max-size-buffers=5000 max-size-bytes=0 max-size-time=0 ! ffmpegcolorspace ");

                pBuff+=sprintf(pBuff,"! vpuenc codec=0 name=venc  alsasrc device=\"hw:0,0\" num-buffers=2500 ");

                pBuff+=sprintf(pBuff,"! audio/x-raw-int,rate=32000,channels=1,depth=16 ! audioconvert ! queue2 max-size-buffers=5000 max-size-bytes=0 max-size-time=0 ");

                pBuff+=sprintf(pBuff,"! mfw_mp3encoder name=aenc matroskamux name=mux ! filesink location=%s aenc. ! mux. venc. ! mux. ",&file_name_vid_audio[0]);

                printf("%s\r\n",&Buffer_string[0]);

                break;

            case MCVIDAUDIO_ID_VIDEO_REC_AUDIO_STREAM:

                //todo
                break;

            case MCVIDAUDIO_ID_VIDEO_STREAM_AUDIO_STREAM:

                //todo
                break;

            default:
                break;

        }

        QStringList args=QString(Buffer_string).split(" ");

        qDebug() << args;

        gstreamer->start("gst-launch", args);


        Pid = gstreamer->pid();
        sprintf(StrPid," kill -9 %d",Pid);

        StateRec=true;
        StateMachine=STATE_READY;

    }
    else if(StateMachine==STATE_READY)
    {

        if(!gstreamer->pid()){
            printf("Process:  errore pid\r\n");
            StateMachine=STATE_NULL;
            RecRec=false;
        }
        if(IsStart()==false)
        {
            StateMachine=STATE_STOP;
            RecRec=false;
        }
    }

    else if(StateMachine==STATE_STOP)
    {
        system(StrPid);
        StateMachine=STATE_NULL;
    }
}



void McVideoAudio::PrintObj()
{
    printf ("\r\n");
    printf ("************************************\r\n");
    printf ("*********PrintObj*******************\r\n");
    printf ("************************************\r\n");


    SetModVidAud(ModVideoAudio);
    SetfileSink();


    printf("*StateMachine        : %d \r\n",StateMachine);
    if(strcasecmp(dimx,STRNULL)!=0)                printf("*dimx                : %s \r\n",dimx);
    if(strcasecmp(dimy,STRNULL)!=0)                printf("*dimy                : %s \r\n",dimy);
    if(strcasecmp(frame,STRNULL)!=0)               printf("*frame               : %s \r\n",frame);
    if(strcasecmp(dimx_stream,STRNULL)!=0)         printf("*dimx_stream         : %s \r\n",dimx_stream);
    if(strcasecmp(dimy_stream,STRNULL)!=0)         printf("*dimy_stream         : %s \r\n",dimy_stream);
    if(strcasecmp(frame_stream,STRNULL)!=0)        printf("*frame_stream        : %s \r\n",frame_stream);
    if(strcasecmp(ip,STRNULL)!=0)                  printf("*ip                  : %s \r\n",ip);
    if(strcasecmp(portaudp,STRNULL)!=0)            printf("*portaudp            : %s \r\n",portaudp);
    if(strcasecmp(pDATETIME->datename,STRNULL)!=0) printf("*data                : %s \r\n",pDATETIME->datename);

    if(strcasecmp(file_name_vid,STRNULL)!=0)       printf("*file_name_vid       : %s \r\n",file_name_vid);
    if(strcasecmp(file_name_audio,STRNULL)!=0)     printf("*file_name_audio     : %s \r\n",file_name_audio);
    if(strcasecmp(file_name_vid_audio,STRNULL)!=0) printf("*file_name_vid_audio : %s \r\n",file_name_vid_audio);
}

