#include <mcserial.h>
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>
#include <QIODevice>
#include "Log.h"
#include "ConfigMobile.h"
#include <DefCom.h>
#include <tstate.h>
#include "mctcptest.h"


#define STR_SERIAL_GPS "/dev/ttymxc2"
#define STR_SERIAL_COPROCESSORE "/dev/ttymxc1"

#define STR_SERIAL_MODEM_HUAWEI "/dev/ttyUSB2"

#define STR_SERIAL_MODEM_UBOX "/dev/ttyACM0"

#define STR_SERIAL_MODEM STR_SERIAL_MODEM_HUAWEI

extern bool en_console;
extern char buff_print[];
extern TLog Log;
extern mctcptest REMLINK;


extern MCSerial SerialPrint;

MCSerial::MCSerial(QObject* parent)
    :QObject(parent)
{
    connect(&Serialtimer, SIGNAL(timeout()), this, SLOT(GestioneSerial()));
    count = 0;


//    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
//    {
//            qDebug() << "Name  :" << info.portName();
//            qDebug() << "Description  :" << info.description();
//            qDebug() << "Manufactuer :"  << info.manufacturer();
//    }


   // QSerialPort serial;
    serial.setPortName("");
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    index_buffer_in=0;
    index_buffer_out=0;
    SerialIsOpen=false;
    Protocol=PROTOCOL_NONE;


}


void MCSerial::SerialIrqRx()
{
     while(serial.bytesAvailable())
     {
         serial.read(&buffer[index_buffer_in++],1);
         if(index_buffer_in>=MAX_LEN_MCSERIAL_BUFFER)index_buffer_in=0;
     }
}




void MCSerial::Start(int time)
{
    Serialtimer.setInterval(time);
    Serialtimer.start();
    count = 0;
}

void MCSerial::GestioneSerial()
{
    if(serial.write("MCSerial: invio\r\n")==-1)
    {
 //       printf("MCSerial: Errore scrittura\r\n");
    }
}




void MCSerial::Init(int id_porta_mcserial)
{

        MCSerial_Id=id_porta_mcserial;
 //printf("INIT %d!!!!!\r\n",id_porta_mcserial);

        Protocol=PROTOCOL_NONE;


        switch(id_porta_mcserial)
        {
            case MCSERIAL_ID_SERIAL_GPS:
                //fusi begin
                /*
                // printf("McSerial     : MCSERIAL_ID_SERIAL_GPS\r\n");
                strcpy(PortName,STR_SERIAL_GPS);   //la  ttymxc1 è quella del gps
                BaudRate=9600;
                DataBits=8;
                Parity=0;
                StopBits=1;
                FlowControl=0;

                serial.setPortName(STR_SERIAL_GPS);  //la  ttymxc1 è quella del gps
                serial.setBaudRate(QSerialPort::Baud9600);
                serial.setDataBits(QSerialPort::Data8);
                serial.setParity(QSerialPort::NoParity);
                serial.setStopBits(QSerialPort::OneStop);
                serial.setFlowControl(QSerialPort::NoFlowControl);
                */
                //fusi end

            break;
            case MCSERIAL_ID_SERIAL_MONITOR:
                strcpy(PortName,"/dev/ttymxc3");
                BaudRate=115200;
                DataBits=8;
                Parity=0;
                StopBits=1;
                FlowControl=0;

                serial.setPortName("/dev/ttymxc3");
                serial.setBaudRate(QSerialPort::Baud115200);
                serial.setDataBits(QSerialPort::Data8);
                serial.setParity(QSerialPort::NoParity);
                serial.setStopBits(QSerialPort::OneStop);
                serial.setFlowControl(QSerialPort::NoFlowControl);
            break;

            case MCSERIAL_ID_SERIAL_MODEM:
                strcpy(PortName,STR_SERIAL_MODEM);
                BaudRate=115200;
                DataBits=8;
                Parity=0;
                StopBits=1;
                FlowControl=0;

                serial.setPortName(STR_SERIAL_MODEM);
                serial.setBaudRate(QSerialPort::Baud115200);
                serial.setDataBits(QSerialPort::Data8);
                serial.setParity(QSerialPort::NoParity);
                serial.setStopBits(QSerialPort::OneStop);
                serial.setFlowControl(QSerialPort::NoFlowControl);
            break;

            case MCSERIAL_ID_SERIAL_ST32 :
                strcpy(PortName,STR_SERIAL_COPROCESSORE);
                BaudRate=115200;
                //BaudRate=19200;
                DataBits=8;
                Parity=0;
                StopBits=1;
                FlowControl=0;

                serial.setPortName(STR_SERIAL_COPROCESSORE);
                serial.setBaudRate(QSerialPort::Baud115200);
                serial.setDataBits(QSerialPort::Data8);
                serial.setParity(QSerialPort::NoParity);
                serial.setStopBits(QSerialPort::OneStop);
                serial.setFlowControl(QSerialPort::NoFlowControl);
            break;




            default:
            break;
        }




}

bool MCSerial::IsOpen()
{
    return(SerialIsOpen);
}




void MCSerial::Open(int id_porta_mcserial)
{

    switch(id_porta_mcserial)
    {
        case MCSERIAL_ID_SERIAL_GPS:
            //fusi begin
            /*
            strcpy(PortName,STR_SERIAL_GPS);
            serial.setPortName(PortName);
            if(BaudRate==9600)
            {
                serial.setBaudRate(QSerialPort::Baud9600);
            }
            else if(BaudRate==19200)
            {
                serial.setBaudRate(QSerialPort::Baud19200);
            }
            if(DataBits==8)
            {
                serial.setDataBits(QSerialPort::Data8);
            }
            if(Parity==0)
            {
                serial.setParity(QSerialPort::NoParity);
            }
            if(StopBits==1)
            {
                serial.setStopBits(QSerialPort::OneStop);
            }
            if(FlowControl==0)
            {
                serial.setFlowControl(QSerialPort::NoFlowControl);
            }
            */
            //fusi end
        break;

        case MCSERIAL_ID_SERIAL_MONITOR:
            strcpy(PortName,"/dev/ttymxc3");

            serial.setPortName(PortName);
            if(BaudRate==115200)
            {
                serial.setBaudRate(QSerialPort::Baud115200);
            }
            else if(BaudRate==19200)
            {
                serial.setBaudRate(QSerialPort::Baud19200);
            }
            if(DataBits==8)
            {
                serial.setDataBits(QSerialPort::Data8);
            }
            if(Parity==0)
            {
                serial.setParity(QSerialPort::NoParity);
            }
            if(StopBits==1)
            {
                serial.setStopBits(QSerialPort::OneStop);
            }
            if(FlowControl==0)
            {
                serial.setFlowControl(QSerialPort::NoFlowControl);
            }

        break;
        case MCSERIAL_ID_SERIAL_MODEM:


            strcpy(PortName,STR_SERIAL_MODEM); //ttyacm????

            serial.setPortName(PortName);
            if(BaudRate==115200)
            {
                serial.setBaudRate(QSerialPort::Baud115200);
            }
            else if(BaudRate==19200)
            {
                serial.setBaudRate(QSerialPort::Baud19200);
            }
            if(DataBits==8)
            {
                serial.setDataBits(QSerialPort::Data8);
            }
            if(Parity==0)
            {
                serial.setParity(QSerialPort::NoParity);
            }
            if(StopBits==1)
            {
                serial.setStopBits(QSerialPort::OneStop);
            }
            if(FlowControl==0)
            {
                serial.setFlowControl(QSerialPort::NoFlowControl);
            }

        break;

    case MCSERIAL_ID_SERIAL_ST32:

        strcpy(PortName,STR_SERIAL_COPROCESSORE); //ttyacm????

        serial.setPortName(PortName);
        if(BaudRate==115200)
        {
            serial.setBaudRate(QSerialPort::Baud115200);
        }
        else if(BaudRate==19200)
        {
            serial.setBaudRate(QSerialPort::Baud19200);
        }
        if(DataBits==8)
        {
            serial.setDataBits(QSerialPort::Data8);
        }
        if(Parity==0)
        {
            serial.setParity(QSerialPort::NoParity);
        }
        if(StopBits==1)
        {
            serial.setStopBits(QSerialPort::OneStop);
        }
        if(FlowControl==0)
        {
            serial.setFlowControl(QSerialPort::NoFlowControl);
        }

    break;


        default:
        break;
    }
 //printf("VOGLIO APRIRE %s!!!!!\r\n",PortName);
    if(serial.open(QIODevice::ReadWrite))
    {
        SerialIsOpen=true;
        connect(&serial, SIGNAL(readyRead()),SLOT(SerialIrqRx()));
    }
    else
    {
 //printf("ERROR OPEN SERIAL!!!!!\r\n");
//    SerialIsOpen=true;
    }

}

void MCSerial::Close(int id_porta_mcserial)
{
    switch(id_porta_mcserial)
    {
        case MCSERIAL_ID_SERIAL_GPS:
            serial.close();
            SerialIsOpen=false;
        break;
        case MCSERIAL_ID_SERIAL_MONITOR:
            serial.close();
            SerialIsOpen=false;

        break;
        case MCSERIAL_ID_SERIAL_MODEM:
            SerialIsOpen=false;
            serial.close();
        break;
        case MCSERIAL_ID_SERIAL_ST32:
            SerialIsOpen=false;
            serial.close();
        break;

        default:
        break;
    }

}
void MCSerial::SetBaud(int rete)
{                           // Imposta la velocità di trasmissione
    BaudRate=rete;

}
bool MCSerial::GetChar(char *c)
{                             // Ottiene un carattere
    if(index_buffer_out!=index_buffer_in)
    {
        *c=buffer[index_buffer_out++];
        if(index_buffer_out>=MAX_LEN_MCSERIAL_BUFFER)index_buffer_out=0;
        return(true);
    }
    return(false);
}
char MCSerial::SendChar(char c)
{
   // const char *x=&c; //OK!!!
   // serial.write(x,1); //OK!!!

    // Invia un carattere e se non può attende finche può
    QByteArray tmparray;
    tmparray.append(c);

    serial.write(tmparray);
    return(c);
}
void MCSerial::PrintObj()
{                                   // Stampa a Monitor lo Stato dell'oggetto
}

void MCSerial::Flush(quint16 dest)
{
    if(en_console)
    {
        if((GetState(DEBUG_FUNC) && dest==PRINT_DEBUG_FUNC) ||
          (GetState(DEBUG_TEL) && dest==PRINT_DEBUG_TEL) ||
          (GetState(DEBUG_GPS) && dest==PRINT_DEBUG_GPS) ||
          (GetState(DEBUG_COM) && dest==PRINT_DEBUG_COM) ||

          (dest==PRINT_DEBUG_ALL) )printf("%s",buff_print);
    }


    else
    {
        if((GetState(DEBUG_FUNC) && dest==PRINT_DEBUG_FUNC) ||
          (GetState(DEBUG_TEL) && dest==PRINT_DEBUG_TEL) ||
          (GetState(DEBUG_GPS) && dest==PRINT_DEBUG_GPS) ||
          (GetState(DEBUG_COM) && dest==PRINT_DEBUG_COM) ||
          (dest==PRINT_DEBUG_ALL) )
        {
            if(GetState(TERMINAL_REMOTE_RUNNING))REMLINK.Write(buff_print);
            else serial.write(buff_print);
        }
    }

    if(dest!=PRINT_DEBUG_GPS)Log.Write(buff_print);


    buff_print[0]=0;
}











