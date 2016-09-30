#include "lis331dlh.h"
#include <stdlib.h>
#include <stdio.h>
#include <QProcess>
#include <QList>
#include <QString>
#include <QDebug>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <string.h>



lis331dlh::lis331dlh(QObject* parent)
    :QObject(parent)
{

    printf("lis331dlh      : costruttore\r\n");
}

void lis331dlh::init(TimerMan *t)
{
    pREGTIMER=t;
    TimerT.Init("MEMSREAD");

    pREGTIMER->RegisterTimer(&TimerT);

    LIS331DLH.state=0;// todo
    //ReadAcceleration();

    ReadI2c(2);
    ReadI2cProva(0x11);
    WriteI2c(0x02,0x01);
    WriteI2c(0x03,0x02);
    WriteI2c(0x04,0x03);
    WriteI2c(0x05,0x04);
    ReadI2c(2);

}


void lis331dlh::exit_on_error (const char *s)	// Exit and print error code
{
    perror(s);
    abort();
}

void lis331dlh::WriteI2c(quint8 reg,quint8 value)
{
    int fd;
    int adapter_nr = 2; /* probably dynamically determined */
    char filename[20];
    quint8  buffer[2];



    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    fd = open(filename, O_RDWR);
    if (fd < 0) {
      /* ERROR HANDLING; you can check errno to see what went wrong */
      exit(1);
    }

    int addr = 0x20; /* The I2C address */

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
      /* ERROR HANDLING; you can check errno to see what went wrong */
      exit(1);
    }

    buffer[0] = reg;     //registro
    buffer[1] = value;   //valore
    write(fd,buffer,2);  //scrivo

    if (write(fd,buffer,2) != 2) exit_on_error ("errore nella scrittura \r\n");

    close(fd);
}

void lis331dlh::ReadI2c(quint8 reg)
{
    char buffer[1];
    int fd;
    #define I2C_ADDR 0x20


    fd = open("/dev/i2c-2", O_RDWR);

    if (fd < 0) {
            printf("Error opening file: %s\n", strerror(errno));
          //  return false;
    }

    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
            printf("ioctl error: %s\n", strerror(errno));
          //  return false;
    }

    //buffer[0]=0x11;
    //write(fd, buffer, 1);

    read(fd, buffer, 1);
    printf("0x%02X\n", buffer[0]);


}

void lis331dlh::ReadI2cProva(quint8 reg)
{

    i2c_rdwr_ioctl_data data;
    i2c_msg msgs[2];
    __u8 buf[sizeof(__u8) * 1] = {static_cast<__u8>(reg)};
    int r;
    int _fd;

    msgs[0].addr = static_cast<__u16>(0x20);
    msgs[0].buf = buf;
    msgs[0].flags = 2;
    msgs[0].len = static_cast<__u16>(sizeof(unsigned char));
    msgs[1].addr = static_cast<__u16>(0x20);
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = static_cast<__u8>(sizeof(unsigned char));
    msgs[1].buf = buf;
    data.msgs = msgs;
    data.nmsgs = 2;


    r =  ioctl(_fd, I2C_RDWR, &data);

    printf("leggo=====%d\r\n",r);

   // return (-1 == r) ? r : buf[0];
}

int lis331dlh::ReadAcceleration()
{
  int status;
  signed short Ax=0;
  signed short Ay=0;
  signed short Az=0;
  quint8  buffer[2];
  quint16    data;
  //nota accellerazione è in complemento a 2
  //MSB è OUT_?_H  LSB è OUT_?_L

  // 1 read STATUS_REG

  //status = //LIS331DLHReadReg(STATUS_REG);
  printf("status=%d\r\n",status);

}
