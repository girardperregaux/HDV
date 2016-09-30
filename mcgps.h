#ifndef MCGPS_H
#define MCGPS_H

#include <QObject>
#include <QTimer>
#include <timerman.h>


class MCGps : public QObject
{
    Q_OBJECT
public:
    explicit MCGps(QObject *parent = 0);

signals:

public slots:
    void GpsRun(void);
    void Start(int time);
    void Init(TimerMan *t); //per passare timer
private:
    QTimer GpsT;
    AppTimer GpsTimer;

    quint8 GpsState;
    bool restart;

};

#endif // MCGPS_H
