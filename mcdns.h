#ifndef MCDNS_H
#define MCDNS_H

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


class mcdns : public QObject
{
    Q_OBJECT


public:
    explicit mcdns(QObject *parent = 0);

    AppTimer TimerT;


    TimerMan *pREGTIMER;

signals:

public slots:
    void startRead();
    void Run();
    void Init(TimerMan *t);
    void acceptConnection();
    void readPendingDatagrams();

private:


};

#endif // MCDNS_H
