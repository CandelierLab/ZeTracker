#include "Motion.h"
#include "FTDI.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

Motion::Motion() {

    // === DEFINITIONS =====================================================

    loop_period = 2e5;        // loop period (ns) | max speed @ 2e5 (20µs)

    mode = MODE_MANUAL;
    ishomed = false;
    is_moving_x = false;
    is_moving_y = false;
    is_moving = false;

    dividor_x = 1;
    dividor_y = 1;

    count2mm = 0.028;            // cam_x = count_x * count2mm_x
    count_x = 0;
    count_y = 0;
    target_x = 0;
    target_y = 0;

    // === INITIALIZATIONS =================================================

    initFTDI();
    pad = 0x00;

    // Wait 1ms to avoid display overlap
    QThread::msleep(1);

}

/* ====================================================================== *
 *      DEVICES                                                           *
 * ====================================================================== */

void Motion::initFTDI() {

    // -------------------------------------
    // NB: Close existing devices ?
    // -------------------------------------

    FTDI = new FTDI_Device(this);
    qInfo() << TITLE_2 << "Motion";

    unsigned int status = FTDI->openDevice();

    switch (status) {
    case 0:

        // --- Display info
        qInfo().noquote() << FTDI->strDeviceDescription() << "opened";

        // --- Thread management
        tFTDI = new QThread();
        FTDI->moveToThread(tFTDI);
        connect(tFTDI, SIGNAL (started()), FTDI, SLOT (DataLoop()));
        connect(FTDI, SIGNAL (finished()), tFTDI, SLOT (quit()));
        connect(FTDI, SIGNAL (finished()), FTDI, SLOT (deleteLater()));
        connect(tFTDI, SIGNAL (finished()), tFTDI, SLOT (deleteLater()));
        tFTDI->start();
        tFTDI->setPriority(QThread::TimeCriticalPriority);

        // --- Connections
        connect(FTDI, SIGNAL(switchTriggered(int)), this, SLOT(switchTriggered(int)));
        connect(FTDI, SIGNAL(homed()), this, SIGNAL(homed()));

        break;

    case 2:
        qWarning() << "FTDI Device is probably not plugged";
        break;
    default:
        qCritical() << "FTDI Device could not be opened with unkown error code" << status;
        break;

    }
}

/* ====================================================================== *
 *      DISPLACEMENTS                                                     *
 * ====================================================================== */

void Motion::moveFixed() { mode = MODE_FIXED; }

void Motion::movePad(bool b) { move(QObject::sender()->objectName(), b); }

void Motion::move(QString dir, bool b) {

    unsigned char pad = 0;

    if (dir=="MOVE_DL") {
        FTDI->setPin(0, false);
        FTDI->setPin(4, false);
        is_moving_x = b;
        is_moving_y = b;
        pad = 0x01;
    }

    if (dir=="MOVE_D") {
        FTDI->setPin(4, false);
        is_moving_x = false;
        is_moving_y = b;
        pad = 0x02;
    }

    if (dir=="MOVE_DR") {
        FTDI->setPin(0, true);
        FTDI->setPin(4, false);
        is_moving_x = b;
        is_moving_y = b;
        pad = 0x04;
    }

    if (dir=="MOVE_L") {
        FTDI->setPin(0, false);
        is_moving_x = b;
        is_moving_y = false;
        pad = 0x08;
    }

    if (dir=="MOVE_R") {
        FTDI->setPin(0, true);
        is_moving_x = b;
        is_moving_y = false;
        pad = 0x10;
    }

    if (dir=="MOVE_UL") {
        FTDI->setPin(0, false);
        FTDI->setPin(4, true);
        is_moving_x = b;
        is_moving_y = b;
        pad = 0x20;
    }

    if (dir=="MOVE_U") {
        FTDI->setPin(4, true);
        is_moving_x = false;
        is_moving_y = b;
        pad = 0x40;
    }

    if (dir=="MOVE_UR") {
        FTDI->setPin(0, true);
        FTDI->setPin(4, true);
        is_moving_x = b;
        is_moving_y = b;
        pad = 0x80;
    }

    if (dir=="MOVE_OFF") {
        is_moving_x = false;
        is_moving_y = false;
    }

    emit updatePad(b?pad:0);
}

void Motion::home() {

    mode = MODE_HOME;
    FTDI->setPin(0, false);
    FTDI->setPin(4, false);
    is_moving_x = true;
    is_moving_y = true;

}

/* ====================================================================== *
 *      POINTER                                                           *
 * ====================================================================== */

void Motion::Pointer(bool b) {

    FTDI->setPin(7, b);
    FTDI->sendOutput();

}

/* ====================================================================== *
 *      INPUT                                                             *
 * ====================================================================== */

void Motion::switchTriggered(int SW) {

    switch(SW) {

    case SWITCH_X:
        is_moving_x = false;
        break;

    case SWITCH_Y:
        is_moving_y = false;
        break;

    }

    emit updatePad(0);

}

void Motion::resetCounts() {

    count_x = 0;
    count_y = 0;
    emit updatePosition();

}

/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

Motion::~Motion() {}
