#include "framing.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QDebug>
#include <qstring.h>
#include <QFile>
#include "mcserial.h"
#include <QCryptographicHash>
#include "config.h"
#include "tstate.h"


extern TConfig Configuration;
extern MCSerial SerialPrint;
extern char buff_print[];
#define MAX_TEST_BUFF 70
char strCommentDateHour[12+1];

Framing::Framing(QObject *parent) :
    QObject(parent)
{
//costruttore

}

void Framing::Init(TimerMan *t)
{

    pREGTIMER=t;

    TimerT.Init((char*)"McFramingTimer");


    pREGTIMER->RegisterTimer(&TimerT);

    McFramingTimer.setInterval(50);
    McFramingTimer.start();
    connect(&McFramingTimer, SIGNAL(timeout()), this, SLOT(RecFilesRun()));



    ready=false;
    count=0;
    stampa=false;
    il=0;
    packetCount=0;
    sendHeader=false;

    SendudpSocket= new QUdpSocket(this);
    udpSocket = new QUdpSocket(this);
    //udpSocket->bind(QHostAddress::LocalHost, 7500);
    udpSocket->bind(PORT_UDP);

    // printf("Framing::Init!!!!!!!!!!!!\r\n");

    connect(udpSocket, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams()));

    NPackCount=0;
    NCount=0;
    OldNPackCount=0;
    ip_to_string=false;
    start_upd=false;

}

void Framing::invalidp()
{
    ip_to_string=false;
}


bool Framing::resolvip()
{

    if(!ip_to_string && GetState(MODEM_CONNECTED))
    {
        ip_to_string=true;
        QHostInfo info = QHostInfo::fromName(Configuration.Config.ip_server_video);

        QList<QHostAddress> l= info.addresses();

        for(int i=0; i<l.count(); i++) {

            snprintf(&Buffer_ip[0],MAX_LEN_BUFFER_PRINT,"%s",l[i].toString().toUtf8().constData());
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FMR: (%s)->(%s)\r\n",Configuration.Config.ip_server_video,Buffer_ip);
            SerialPrint.Flush(PRINT_DEBUG_FUNC);
        }

        if(info.error()==0)
        {
            start_upd=true;
            return (true);
        }

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"*FMR=Error ip dns\r\n");
        SerialPrint.Flush(PRINT_DEBUG_ALL);
        start_upd=false;
        return (false);
    }
    return (false); //fusi+
}

void Framing::readPendingDatagrams()
{
    quint32 dimSp;
    QByteArray Tmp;
    QHostAddress sender;
    quint16 senderPort;
    //char tmpstr[(MAX_LEN_CAMPO*2)];
    QCryptographicHash krypo(QCryptographicHash::Sha256);

//    memset(Buffer_string,CHARNULL,MAX_LEN_STR);
//    pBuff=&Buffer_string[0];

    if(GetState(MODEM_CONNECTED))
    {
        resolvip();
    }
    if(!start_upd)
    {

        while (udpSocket->hasPendingDatagrams())
        {
            Tmp.resize(udpSocket->pendingDatagramSize());
            udpSocket->readDatagram(Tmp.data(), Tmp.size(),&sender, &senderPort);
        }
        return;
    }

    while (udpSocket->hasPendingDatagrams())
    {

            if(strcmp(Configuration.Config.port_server_rtp,STR_NO)==0)
            {
                Tmp.resize(udpSocket->pendingDatagramSize());
                udpSocket->readDatagram(Tmp.data(), Tmp.size(),&sender, &senderPort);

                dimSp=Tmp.size();




                //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"dimSp=%d\r\n",dimSp);
                //SerialPrint.Flush(PRINT_DEBUG_ALL);

                if(start_upd)
                {
                    NCount++;

                    if(NCount>50)
                    {
                     //   snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"FRM: STREAMING IN CORSO...\r\n");
                     //   SerialPrint.Flush(PRINT_DEBUG_FUNC);
                        NCount=0;
                    }

                    SendudpSocket->writeDatagram(Tmp,QHostAddress(Buffer_ip),atol(Configuration.Config.port_server_video)); //FACCIO ECHO SU PORTA 5001
                }

            }
            else
            {
                QByteArray datagram;

                NCount++;

                if(NCount>99)
                {
                    NPackCount++;
                    //NCount=0;
                    if(NPackCount>99)NPackCount=0;
                }

                // Salvo in Tmp il contenuto del pacchetto udp
                Tmp.resize(udpSocket->pendingDatagramSize());

                udpSocket->readDatagram(Tmp.data(), Tmp.size(),&sender, &senderPort);

                dimSp=Tmp.size();
                //snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"%ld ",dimSp);
                //SerialPrint.Flush(PRINT_DEBUG_ALL);



                // Preparo header
                memset(&Buffer_string[0],CHARNULL,MAX_LEN_STR);
                pBuff=&Buffer_string[0];


                //pBuff+=sprintf(pBuff,"1V011234567%04d%06d",dimSp);
                pBuff+=sprintf(pBuff,"1V");
                pBuff+=sprintf(pBuff,"01");

                pBuff+=sprintf(pBuff,"%s",Configuration.Config.codice_gruppo);//codice gruppo
                pBuff+=sprintf(pBuff,"%s",Configuration.Config.codice_periferica);//codice_periferica

                pBuff+=sprintf(pBuff,"%02d",NPackCount);//NPackCount
                pBuff+=sprintf(pBuff,"%04d",NCount);//NCount


                //pBuff+=sprintf(pBuff,"%05d",atol(Configuration.Config.port_server_rtp));// PORT fusi-
                //pBuff+=sprintf(pBuff,"%05ld",atol(Configuration.Config.port_server_rtp));// PORT fusi+
                pBuff+=sprintf(pBuff,"%06d",dimSp);//DIM
                //pBuff+=sprintf(pBuff,"%06d",strlen("PIPPO"));//DIM
                *pBuff=0;


                if(OldNPackCount!=NPackCount)
                {
                    OldNPackCount=NPackCount;
                    snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"test: %s\r\n",&Buffer_string[0]);
                    SerialPrint.Flush(PRINT_DEBUG_ALL);
                    NCount=0;

                }

                datagram.append(Buffer_string);
                datagram.append(Tmp);
                dimSp=datagram.size();
                krypo.reset();
                krypo.addData(datagram.data(),dimSp);

                QByteArray hash = krypo.result();

                //const char *str;

                datagram.append(hash.toHex());
                dimSp=datagram.size();

                if(start_upd)
                {
                    SendudpSocket->writeDatagram(datagram,QHostAddress(Buffer_ip),atol(Configuration.Config.port_server_video)); //FACCIO ECHO SU PORTA 5001
                }
            }
    }

}



void Framing::acceptConnection()
{
    client = server.nextPendingConnection();
    packetCount=0;
   // sizeEBMLHeader=0;
   // sizeSegmentInformation=0;

    connect(client, SIGNAL(readyRead()),this, SLOT(startRead()));
}



void Framing::startRead()  //nb ricevo pagine oggs complete ad ogni signal
{
    QByteArray tmpRx;

    return;

    if(packetCount==4)
    {
        TimerT.SetTimer(UN_SECONDO*15);
        sendHeader=true;
        SegmentInformation4.append(client->readAll());
        sizeSegmentInformation4 = SegmentInformation4.size();
        stampa=true;

        udpSocket->writeDatagram(SegmentInformation4.data(),sizeSegmentInformation4,QHostAddress("192.168.1.37"), 7008);
    }
    else if(packetCount==3)
    {
        SegmentInformation3.append(client->readAll());
        sizeSegmentInformation3 = SegmentInformation3.size();
        udpSocket->writeDatagram(SegmentInformation3.data(),sizeSegmentInformation3,QHostAddress("192.168.1.37"), 7008);
    }

    else if(packetCount==2)
    {
        //TimerT.SetTimer(UN_SECONDO*30);
        //sendHeader=true;
        SegmentInformation2.append(client->readAll());
        sizeSegmentInformation2 = SegmentInformation2.size();
        //stampa=true;

        udpSocket->writeDatagram(SegmentInformation2.data(),sizeSegmentInformation2,QHostAddress("192.168.1.37"), 7008);
    }

    else if(packetCount==1)
    {
       // TimerT.SetTimer(UN_SECONDO*30);
       // sendHeader=true;
        SegmentInformation.append(client->readAll());
        sizeSegmentInformation = SegmentInformation.size();
//        stampa=true;

        udpSocket->writeDatagram(SegmentInformation.data(),sizeSegmentInformation,QHostAddress("192.168.1.37"), 7008);
    }
    else if(packetCount==0)
    {
        sendHeader=false;
        EBMLHeader.append(client->readAll());
        sizeEBMLHeader = EBMLHeader.size();
   //     udpSocket->writeDatagram("22222222222222222222222222222222222222222222222222222222222222222222",QHostAddress("192.168.1.37"), 7008);

        udpSocket->writeDatagram(EBMLHeader.data(),sizeEBMLHeader,QHostAddress("192.168.1.37"), 7008);
    }
    else
    {
       tmpRx.append(client->readAll());
       udpSocket->writeDatagram(tmpRx.data(),tmpRx.size(),QHostAddress("192.168.1.37"), 7008);
       tmpRx.clear();
    }

    packetCount++;
    if(packetCount>100)packetCount=5;


}
void Framing::RecFilesRun() //ogni 100 ms macchina a stati
{

    quint32 i;

    if(TimerT.IsElapsed() && sendHeader==true)
    {
        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SEND!!!!!");
        SerialPrint.Flush(PRINT_DEBUG_ALL);

       udpSocket->writeDatagram(EBMLHeader.data(),sizeEBMLHeader,QHostAddress("192.168.1.37"), 7008);
       udpSocket->writeDatagram(SegmentInformation.data(),sizeSegmentInformation,QHostAddress("192.168.1.37"), 7008);
     //  udpSocket->writeDatagram(SegmentInformation2.data(),sizeSegmentInformation2,QHostAddress("192.168.1.37"), 7008);
      // udpSocket->writeDatagram(SegmentInformation3.data(),sizeSegmentInformation3,QHostAddress("192.168.1.37"), 7008);
      // udpSocket->writeDatagram(SegmentInformation4.data(),sizeSegmentInformation4,QHostAddress("192.168.1.37"), 7008);
       TimerT.SetTimer(UN_SECONDO*5);
    }


    if(stampa)
    {
        stampa=false;

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"EBMLHeader(%d)=",sizeEBMLHeader);
        SerialPrint.Flush(PRINT_DEBUG_ALL);

     //   udpSocket->writeDatagram(SegmentInformation.data(),sizeSegmentInformation,QHostAddress("192.168.1.37"), 7008);
     //   udpSocket->writeDatagram(EBMLHeader.data(),sizeEBMLHeader,QHostAddress("192.168.1.37"), 7008);


        for(i=0;i<sizeEBMLHeader;i++)
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"%02x ",EBMLHeader.at(i));
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }

        EBMLHeader.clear();

        snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"SegmentInformation(%d)=",sizeSegmentInformation);
        SerialPrint.Flush(PRINT_DEBUG_ALL);

        for(i=0;i<sizeSegmentInformation;i++)
        {
            snprintf(&buff_print[0],MAX_LEN_BUFFER_PRINT,"%02x ",SegmentInformation.at(i));
            SerialPrint.Flush(PRINT_DEBUG_ALL);
        }

        SegmentInformation.clear();

    }

}










