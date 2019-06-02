#ifndef MOTION_H
#define MOTION_H

#define MODE_MANUAL 0
#define MODE_HOME   1
#define MODE_FIXED  2
#define MODE_AUTO   3
#define MODE_DEMO   99

#include <QObject>
#include <QThread>
#include <QElapsedTimer>

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
    QElapsedTimer timer;
    long int loop_period;

    // Motion
    void move(QString, bool);
    bool ishomed;
    bool is_moving_x;
    bool is_moving_y;
    bool is_moving;

    // Positions
    unsigned int percent_x;
    unsigned int percent_y;
    double count2mm;
    int count_x;
    int count_y;
    int target_x;
    int target_y;

    // Feedback
    int mode;
    double dx;
    double dy;

    // Debug
    long int demo_tref;

signals:

    void homed();
    void updateMotionState();
    void updatePosition();
    void updatePad(unsigned char);

public slots:

    // FTDI Device
    void initFTDI();

    // Displacements
    void home();
    void movePad(bool);
    void moveFixed();

    // Pointer
    void Pointer(bool);

    // Input
    void switchTriggered(int);
    void resetCounts();

    // Debug
    void demo(bool);

private:

    // FTDI Device
    FTDI_Device *FTDI;
    QThread *tFTDI;

    // GUI
    unsigned char pad;

};

#endif // MOTION_H
