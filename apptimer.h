#ifndef APPTIMER_H
#define APPTIMER_H
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <DefCom.h>

#define MAX_LEN_STR_TIMER  30

class AppTimer : public QObject
{
    Q_OBJECT

public:
    explicit AppTimer(QObject* parent=0);
    char Name [MAX_LEN_STR_TIMER+1];

public slots:
    void Init(char *str);
    void Run();
    void SetTimer(int period);
    bool IsElapsed();
    void PrintObj();
    void Info();

private:
    QTimer timer;
    int count;
    int TempInSec;
    int TempInSec2;
    int TempInmSec;
    bool Elapsed;
    bool Running;

};

#endif // APPTIMER_H

