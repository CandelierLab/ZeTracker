#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>
#include <QThread>

#include "Motion.h"
#include "MsgHandler.h"

class Joystick : public QObject {

    Q_OBJECT


public:

    Joystick(class Motion*);
    ~Joystick();

    bool active;

public slots:

    void scan();
    void serialInput();

private:

    class Motion *Motion;
    QSerialPort *conn;

};

#endif // JOYSTICK_H
