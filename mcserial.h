#ifndef MCSERIAL_H
#define MCSERIAL_H

#include <QTimer>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <DefCom.h>

#define PRINT_ALL 'A'
#define PRINT_MODEM 'M'
#define PRINT_FUNC  'F'


#define MAX_LEN_MCSERIAL_BUFFER     300
#define MCSERIAL_ID_SERIAL_NONE       0
#define MCSERIAL_ID_SERIAL_GPS        1
#define MCSERIAL_ID_SERIAL_MONITOR    2
#define MCSERIAL_ID_SERIAL_MODEM      3
#define MCSERIAL_ID_SERIAL_ST32       4

#define MCSERIAL_BAUD_115200  1
//todo aggiungere..


class MCSerial : public QObject
{
    Q_OBJECT

public:
    explicit MCSerial(QObject* parent=0);
    QSerialPort serial;
    quint8 Protocol;


public slots:

    void SerialIrqRx();

    void GestioneSerial();
    void Start(int time);

    void Init(int id_porta_mcserial);
    void Open(int id_porta_mcserial);
    void Close(int id_porta_mcserial);
    bool IsOpen();
    void SetBaud(int rete);                           // Imposta la velocità di trasmissione

    bool GetChar(char *c);                             // Ottiene un carattere
    char SendChar(char c);                               // Invia un carattere e se non può attende finche può

    void PrintObj();                                   // Stampa a Monitor lo Stato dell'oggetto

    void Flush(quint16 dest);



private:
    int m_a;
    int count;
    QTimer Serialtimer;
    QTimer *timer;

    QByteArray dato;
    char buff[100];
    char buff_rx[100];
    quint16 output_index;
    int pDato;

    int MCSerial_Id;
    char buffer[MAX_LEN_MCSERIAL_BUFFER+1];
    quint16 index_buffer_in;
    quint16 index_buffer_out;

    char PortName [MAX_LEN_CAMPO+1];
    int BaudRate;
    int DataBits;
    int Parity;
    int StopBits;
    int FlowControl;
    bool SerialIsOpen;


};

#endif // MCSERIAL_H
