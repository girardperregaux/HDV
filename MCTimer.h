#ifndef MCTIMER_H
#define MCTIMER_H
#include <QTimer>
#include <QObject>
#include <QProcess>


class MCTimer : public QObject
{
    Q_OBJECT

public:
    explicit MCTimer(QObject* parent=0);


public slots:
    void GestioneTimer();
    void Start(int time);
    void update();
    void PrintTimer();

private:
    int stato_timer;
    int count;
    QTimer m_timer;
    QTimer *timer;

};

#endif // MCTIMER_H


