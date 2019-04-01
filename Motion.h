#ifndef MOTION_H
#define MOTION_H

#define AXIS_X 0
#define AXIS_Y 1
#define MOVE_NEG -1
#define MOVE_STOP 0
#define MOVE_POS  1

#include <QObject>
#include <QThread>

#include "FTDI.h"

/* #############################################################################################
   #                                                                                           #
   #                                        MOTION                                             #
   #                                                                                           #
   ############################################################################################# */

class Motion : public QObject {

    Q_OBJECT

public:
    Motion();
    ~Motion();

signals:

    void setMotionState(bool);
    void setCounts(int, int);
    void uncheckPad();

public slots:

    // FTDI Device
    void initFTDI();

    // Displacements
    void Move(int, int);

    // Pointer
    void Pointer(bool);

    // Input
    void inputRead(int);
    void resetCounts();

private:

    // FTDI Device
    FTDI_Device *FTDI;
    QThread *tFTDI;

    // low-level motor communication
    long int loop_period_x, loop_period_y;
    bool is_running_x, is_running_y, motion_state;
    int count_x, count_y;

};

#endif // MOTION_H
