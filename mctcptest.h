#ifndef MCTCPTEST_H
#define MCTCPTEST_H

#include <QObject>
#include <QtGlobal>
#include <HDV.h>
#include <QByteArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QProcess>
#include <QTimer>
#include <DefCom.h>
#include "timerman.h"
#include <QFile>
#include "config.h"

#define MAX_LEN_MCTCP_BUFFER 300
#define CMD_TCP_STRING_SIZE (8*MAX_LEN_CAMPO)+1


#define REMLINK_NO_READY 1
#define REMLINK_ERROR_DELAY 2
#define REMLINK_NO_APN 3


enum REMLINK_STATE {

    REMLINK_READY =0,
    REMLINK_REQ_ECM,
    REMLINK_WAIT_ECM,
    REMLINK_OPEN_CONNECTION,
    REMLINK_WAIT_OPEN_CONNECTION,

    RLK_STATE_INIT,
    TCP_STATE_DATE_HOUR,
    TCP_STATE_LOGIN,
    TCP_STATE_RUN,
    TCP_STATE_PSW,
    TCP_STATE_WAIT,
    TCP_STATE_CONNECT,
    TCP_STATE_CLOSE
};





class mctcptest : public QObject
{
    Q_OBJECT
public:
    explicit mctcptest(QObject *parent = 0);

    void Init(TimerMan *t);

    quint8 Start(char *ip, char *port,char *login,char *password,char *APN,char *NumTelSMS);
    void Write(char *str);

    void Abort();
    void PrintObj();
    quint8 GetResult();

    QTcpSocket socket;
    QTcpSocket* client;

    TConnectionParam ConnectionParam;

    AppTimer TimerConnect;
    AppTimer TimerLogin;
    AppTimer TimeOut;

    quint8 tcpState;
    bool TcpConnected;
    bool connectionTcpOpened;

    bool go;
    bool available;


signals:

public slots:
    void connected();
    void disconnected();
    void readyRead();
    void SClose();
    void Run();
    bool GetCharTcp(char *c);
    char SendCharTcp(char c);
    bool TcpGetDateHour(void);
    bool TcpGetLogin(void);
    bool TcpGetPsw(void);
    bool TcpGetWait(void);
    bool TcpGetLFailed(void);
    bool TcpGetConnected(void);

private:
  TimerMan *pREGTIMER;
  QTimer TimerTcp;
  char bufferTcp[MAX_LEN_MCTCP_BUFFER+1];
  char bufferTcpLogin[MAX_LEN_MCTCP_BUFFER+1];
  int Tcp_index_buffer_in;
  int Tcp_index_buffer_out;
  char Tcp_lastchar;
  quint8 Tcp_input_index;
  quint8 result;

};

#endif // MCTCPTEST_H
