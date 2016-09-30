#ifndef LOG_H
#define LOG_H
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"
#include <qfile.h>
#include <QTextStream>






class TLog : public QObject
{
    Q_OBJECT

public:
    explicit TLog(QObject* parent=0);

    void Init(TimerMan *t);
    void Write(char * str);
    void DelFolder();

    AppTimer TimerT;
    char log_path[100];
    QFile file;
    QTextStream outStream;
    bool fileAperto;





public slots:
    void Run();

private:

    QTimer TimerRun;
    TimerMan *pREGTIMER;


};

#endif // TLOG_H
