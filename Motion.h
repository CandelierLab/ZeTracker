#ifndef MOTION_H
#define MOTION_H

#define AXIS_X 0
#define AXIS_Y 1
#define MOVE_NEG -1
#define MOVE_STOP 0
#define MOVE_POS  1

#define PAD_DL  0
#define PAD_D   1
#define PAD_DR  2
#define PAD_L   3
#define PAD_R   4
#define PAD_UL  5
#define PAD_U   6
#define PAD_UR  7

#include <QObject>
#include <QThread>

#include "FTDI.h"

// Forward declaration
class FTDI_Device;

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

    // Timing
    long int loop_period_x;
    long int loop_period_y;

    // Motion state
    bool is_running_x;
    bool is_running_y;
    bool motion_state;

    // Positions
    int count_x;
    int count_y;
    double pos_x;
    double pos_y;
    double count2mm_x;
    double count2mm_y;

signals:

    void updateMotionState();
    void updatePosition();
    void setPad(unsigned char);

public slots:

    // FTDI Device
    void initFTDI();

    // Displacements
    void Move(bool);

    // Pointer
    void Pointer(bool);

    // Input
    void switchTriggered(int);
    void resetCounts();

private:

    // FTDI Device
    FTDI_Device *FTDI;
    QThread *tFTDI;

    // GUI
    unsigned char pad;

};

#endif // MOTION_H
