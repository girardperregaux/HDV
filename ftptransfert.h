#ifndef FTPTRANSFER_H
#define FTPTRANSFER_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "mcdatetime.h"
#include <apptimer.h>
#include "timerman.h"
#include <qfile.h>
#include <QtNetwork>
#include "utility.h"
#include "DefCom.h"
#include <mcfind.h>
#include <QStringList>

#define STATE_FTP_TRANS_PYHTON_LAUNCH     0

#define STATE_FTP_TEST_PYHTON_1           1
#define STATE_FTP_TEST_PYHTON_2           2
#define STATE_FTP_TEST_PYHTON_3           3
#define STATE_FTP_TRANS_IS_CONNECT        4
#define STATE_FTP_TRANS_CONNECTED         5
#define STATE_FTP_TRANS_LOGIN             6
#define STATE_FTP_TRANS_CD                7
#define STATE_FTP_TRANS_SOURCE            8
#define STATE_FTP_TRANS_UPLOAD            9
#define STATE_FTP_TRANS_RENAME           10
#define STATE_FTP_TRANS_DISCONNECT       23
#define STATE_FTP_TRANS_DEL_RENAME_CONF  24
#define STATE_FTP_TRANS_MODEM_ON         25
#define STATE_FTP_TRANS_MODEM_CONNECT    26
#define STATE_FTP_TRANS_WAIT_KILL        27
#define STATE_FTP_TRANS_ERROR_NULL       28

#define STATE_FTP_TRANS_PYHTON_KILL       11
#define STATE_FTP_TRANS_PYHTON_INIT       12
#define STATE_FTP_TRANS_PYHTON_IS_CONNECT 13
#define STATE_FTP_TRANS_NULL          14
#define STATE_FTP_TRANS_INIT          15
#define STATE_FTP_TRANS_INIT_TO_LAST  16
#define STATE_FTP_TRANS_LAST_TO_INIT  17
#define STATE_FTP_TRANS_READY         18
#define STATE_FTP_TRANS_SEND          19
//#define STATE_FTP_TRANS_RENAME        20
#define STATE_FTP_TRANS_ERROR         21

#define STATE_FTP_TRANS_FINISH        22

class ftptransfert : public QObject
{
    Q_OBJECT
public:
    explicit ftptransfert(QObject *parent = 0);
    AppTimer Timertransfert;
    AppTimer ptimeout;
    short startTransfer;
    short UpgradeTransfer;
    short PythonConnected;
    short FtpConnected;
    short Ftp_sleep;
    short FtpTransfertRun;

    QTcpSocket socketpy;
    bool recheck_list;

    TDate FtpDate;

signals:

public slots:
    void Init(TimerMan *t);
    void Run();
    void connectedpy();
    void disconnectedpy();
    void readyReadpy();
    void SendCharTcpPy(char *c);

    quint8 TcpParampy(void);
    quint8 Tcploginpy(void);
    quint8 Tcpcdpy(void);
    quint8 Tcpsourcepy(void);

    bool TcpuploadOKpy(void);
    bool TcpuploadKOpy(void);
    bool TcpuploadRunning(void);
    bool TcpuploadTimeoutpy(void);

    quint8 TcpuploadRenamepy(void);
    quint8 TcpuploadDisconnectpy(void);
    bool TCcpNameFileServerpy(char *name_file);
    bool removefile(char * Tfile);

    void PrintObj();
    bool SetDate(char *dateinit,char *hourinit, char *datefinish,char *hourfinish);
    private:
    TimerMan *pREGTIMER;
    QTimer TTransfertRun;

private:
    #define STRING_TRANSC             100
    char StrBuffTra[STRING_TRANSC+1];
    char StrBuffTemp[STRING_TRANSC+1];
    char bufferTcpPy[STRING_TRANSC+1];

    char bufferFileServerPy[STRING_TRANSC+1];
    char NameFileTempPy[STRING_TRANSC+1];


    quint8 StateTransfert;
    MCfind MCFindTransfert;
    QStringList TranscStringList;
    quint8 indexTransfert;
    QProcess  *pytcp;
    quint16 pidpy;
    quint8 CommandRetry;

    char StrPidpy[PID_MAX+1];
};

#endif // FTPTRANSFER_H
