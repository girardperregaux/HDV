#ifndef GESTIONE_EVENTI_H
#define GESTIONE_EVENTI_H
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"
#include <QUdpSocket>



class GestioneEventi : public QObject
{
    Q_OBJECT

public:
    explicit GestioneEventi(QObject* parent=0);
    AppTimer TimerT;
    AppTimer TimerRead;
    AppTimer TimerSosta;
    AppTimer Temperatura_cpu;
    AppTimer TimerVoxRit;
    AppTimer TimerSpegnimento;
    AppTimer TimerSensoriAttivi;
    AppTimer TimerResetRitardato;
    AppTimer TimerDelayScarico;



    void Init(TimerMan *t);
    void PrintState();
    void SetWorkPeriod(quint8 device,quint16 Time,bool fase_attiva);
    void AvvisoAggiornamento(void);
    QUdpSocket  *udpSocket;
    qint32 ValueTemperDec;
public slots:
    void readPendingDatagrams();
    void Run();
    void xDatagrams();
    void ReadValueCpu();
    


private:
    void ReadTemperature();
    void GestioneMovimento();
    void GestionePosition(); //fusi+
    void GestioneVox();

    QTimer TimerRun;
    TimerMan *pREGTIMER;
    QByteArray datagrama;
    quint32 Temperature;

    qint32 ValueTemperature;
};





#endif // GESTIONE_EVENTI_H
