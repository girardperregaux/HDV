#ifndef FRAMING_H
#define FRAMING_H

#include <QObject>
#include <QtGlobal>
#include <HDV.h>
#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <DefCom.h>
#include "timerman.h"
#include <QFile>
#include <QUdpSocket>
#include "utility"
#include <QHostInfo>


#define MAX_SIZE_OGGPAGE 6000 //era 4096 ma attualmente supero questo valore
#define BUFF_OGG_MAX     8000
#define MAX_LEN_NAME_FILE 160
#define MAX_LEN_STR 160
//#define CHARNULL ' '
class Framing : public QObject
{
    Q_OBJECT


public:
    explicit Framing(QObject *parent = 0);

    AppTimer TimerT;

    bool stampa;
    short ready;
    quint16 count;


    quint8  il;

    QTcpServer  server;
    QTcpSocket* client;
    TimerMan *pREGTIMER;
    QTimer McFramingTimer;
    char Buffer_string[MAX_LEN_STR+1];
    char Buffer_ip[MAX_LEN_STR+1];
    char *pBuff;
    quint16 NPackCount;
    quint16 OldNPackCount;
    quint16 NCount;
    bool ip_to_string;
    bool start_upd;
signals:

public slots:
    void startRead();
    void RecFilesRun();
    void Init(TimerMan *t);
    void acceptConnection();
    void readPendingDatagrams();
    bool resolvip();
    void invalidp();
private:
    QUdpSocket *udpSocket;
    QUdpSocket *SendudpSocket;

    QByteArray EBMLHeader;
    QByteArray SegmentInformation;
    QByteArray SegmentInformation2;
    QByteArray SegmentInformation3;
    QByteArray SegmentInformation4;
    quint8  packetCount;
    bool sendHeader;

    quint16 sizeEBMLHeader;
    quint16 sizeSegmentInformation;
    quint16 sizeSegmentInformation2;
    quint16 sizeSegmentInformation3;
    quint16 sizeSegmentInformation4;
    QHostInfo Infoip;


};

#endif // FRAMING_H
