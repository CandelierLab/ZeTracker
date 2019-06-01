#include "Motion.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

Motion::Motion() {

    // === DEFINITIONS =====================================================

    loop_period = 2e5;        // loop period (ns) | max speed @ 2e5 (20µs)

    mode = MODE_MANUAL;
    is_moving_x = false;
    is_moving_y = false;
    is_moving = false;

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

void Motion::Move(bool b) {

    // Motion
    if (QObject::sender()->objectName()=="MOVE_DL" || QObject::sender()->objectName()=="MOVE_L" || QObject::sender()->objectName()=="MOVE_UL") {
        FTDI->setPin(0, true);
        is_moving_x = b;
    }

    if (QObject::sender()->objectName()=="MOVE_DR" || QObject::sender()->objectName()=="MOVE_R" || QObject::sender()->objectName()=="MOVE_UR") {
        FTDI->setPin(0, false);
        is_moving_x = b;
    }

    if (QObject::sender()->objectName()=="MOVE_U" || QObject::sender()->objectName()=="MOVE_UL" || QObject::sender()->objectName()=="MOVE_UR") {
        FTDI->setPin(4, false);
        is_moving_y = b;
    }

    if (QObject::sender()->objectName()=="MOVE_D"  || QObject::sender()->objectName()=="MOVE_DL" || QObject::sender()->objectName()=="MOVE_DR") {
        FTDI->setPin(4, true);
        is_moving_y = b;
    }

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
        qDebug() << "X";
        // pad &= 0xD6;     // [11010110]
        break;

    case SWITCH_Y:
        is_moving_y = false;
        qDebug() << "Y";
        // pad &= 0xF8;     // [11111000]
        break;

    }

    emit setPad(pad);

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
