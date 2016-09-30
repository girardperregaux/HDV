#ifndef LIS331DLH_H
#define LIS331DLH_H

#include <QTimer>
#include <QObject>
#include <QProcess>
#include <mcserial.h>
#include "apptimer.h"
#include <DefCom.h>
#include "timerman.h"
#include <linux/i2c-dev.h>
#include <qfile.h>

#define WHO_AM_I               0x0F
#define CTRL_REG1              0x20
#define CTRL_REG2              0x21
#define CTRL_REG3              0x22
#define CTRL_REG4              0x23
#define CTRL_REG5              0x24
#define HP_FILTER_RESET        0x25
#define REFERENCE              0x26
#define STATUS_REG             0x27
#define OUT_X_L                0x28
#define OUT_X_H                0x29
#define OUT_Y_L                0x2A
#define OUT_Y_H                0x2B
#define OUT_Z_L                0x2C
#define OUT_Z_H                0x2D
#define INT1_CFG               0x30
#define INT1_SOURCE            0x31
#define INT1_THS               0x32
#define INT1_DURATION          0x33
#define INT2_CFG               0x34
#define INT2_SOURCE            0x35
#define INT2_THS               0x36
#define INT2_DURATION          0x37

#define MIN_VAL_PULSEMOV       0x00
#define MIN_VAL_PULSEMOV_AMB   0x01
#define MEMS_DURATION_025S     0x10
#define MEMS_DURATION_0063S    0x04
#define MEMS_DURATION_05S      0x14

#define ID_LIS331DLM           0x12
#define READ_ERROR                2


class lis331dlh : public QObject
{
    Q_OBJECT

public:
    explicit lis331dlh(QObject* parent=0);
    AppTimer TimerT;
public slots:

    void init(TimerMan *t);

    void exit_on_error (const char *s);

private:

    int ReadAcceleration();

    TimerMan *pREGTIMER;

    void ReadI2c(quint8 reg);

    void ReadI2cProva(quint8 reg);

    void WriteI2c(quint8 reg,quint8 value);


    typedef struct
    {
       signed short Ax;
       signed short Ay;
       signed short Az;
       float fAX;
       float fAY;
       float fAZ;

    } TAcceleration;


    typedef struct
    {
       TAcceleration  Acceleration;

       float deltaAx;
       float deltaAy;
       float deltaAz;

       int bAX;
       int bAY;
       int bAZ;

       int init;
       int state;
       int command;

    } TLIS331DLH;

    TLIS331DLH LIS331DLH;
    TAcceleration ACCE;

};

#endif // LIS331DLH_H
