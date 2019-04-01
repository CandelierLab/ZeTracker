#include "Motion.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

Motion::Motion() {

    // === DEFINITIONS =====================================================

    loop_period_x = 5e5;
    loop_period_y = 5e6;

    is_running_x = false;
    is_running_y = false;
    motion_state = false;
    count_x = 0;
    count_y = 0;

    // === INITIALIZATIONS =================================================

    initFTDI();

}

/* ====================================================================== *
 *      DEVICES                                                           *
 * ====================================================================== */

void Motion::initFTDI() {

    FTDI = new FTDI_Device();

    unsigned int status = FTDI->openDevice();

    switch (status) {
    case 0:

        qInfo() << "FTDI Device" << FTDI->device_id << "successfully opened";

        // Initialize properties

        // ...

        // Thread management
        tFTDI = new QThread();
        FTDI->moveToThread(tFTDI);
        connect(tFTDI, SIGNAL (started()), FTDI, SLOT (DataLoop()));
        connect(FTDI, SIGNAL (finished()), tFTDI, SLOT (quit()));
        connect(FTDI, SIGNAL (finished()), FTDI, SLOT (deleteLater()));
        connect(tFTDI, SIGNAL (finished()), tFTDI, SLOT (deleteLater()));

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

void Motion::Move(int, int) {

}

/* ====================================================================== *
 *      POINTER                                                           *
 * ====================================================================== */

void Motion::Pointer(bool b) {

//    FTDI->setPin(6, b);
//    FTDI->sendOutput();

}

/* ====================================================================== *
 *      INPUT                                                             *
 * ====================================================================== */

void Motion::inputRead(int) {}

void Motion::resetCounts() {}

/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

Motion::~Motion() {}
