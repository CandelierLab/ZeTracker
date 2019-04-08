#include "Motion.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

Motion::Motion() {

    // === DEFINITIONS =====================================================

    loop_period_x = 1e6;        // Y loop period (ns) | max X speed @ 5e5
    loop_period_y = 5e6;        // Y loop period (ns) | max Y speed @ 5e5

    is_running_x = false;
    is_running_y = false;
    motion_state = false;
    mode = MODE_MANUAL;

    count_x = 0;
    count_y = 0;
    pos_x = 0;
    pos_y = 0;
    count2mm_x = 0.18;            // pos_x = count_x * count2mm_x
    count2mm_y = 0.18;            // pos_y = count_y * count2mm_y

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

void Motion::Move(bool b) {

    // Motion
    if (QObject::sender()->objectName()=="MOVE_DL" || QObject::sender()->objectName()=="MOVE_L" || QObject::sender()->objectName()=="MOVE_UL") {
        FTDI->setPin(0, true);
        is_running_x = b;
    }

    if (QObject::sender()->objectName()=="MOVE_DR" || QObject::sender()->objectName()=="MOVE_R" || QObject::sender()->objectName()=="MOVE_UR") {
        FTDI->setPin(0, false);
        is_running_x = b;
    }

    if (QObject::sender()->objectName()=="MOVE_U" || QObject::sender()->objectName()=="MOVE_UL" || QObject::sender()->objectName()=="MOVE_UR") {
        FTDI->setPin(2, true);
        FTDI->setPin(4, true);
        is_running_y = b;
    }

    if (QObject::sender()->objectName()=="MOVE_D"  || QObject::sender()->objectName()=="MOVE_DL" || QObject::sender()->objectName()=="MOVE_DR") {
        FTDI->setPin(2, false);
        FTDI->setPin(4, false);
        is_running_y = b;
    }

}

/* ====================================================================== *
 *      POINTER                                                           *
 * ====================================================================== */

void Motion::Pointer(bool b) {

    FTDI->setPin(6, b);
    FTDI->sendOutput();

}

/* ====================================================================== *
 *      INPUT                                                             *
 * ====================================================================== */

void Motion::switchTriggered(int SW) {

    switch(SW) {

    case SWITCH_XL:
        is_running_x = false;
        pad &= 0xD6;     // [11010110]
        break;

    case SWITCH_XR:
        is_running_x = false;
        pad &= 0x6B;     // [01101011]
        break;

    case SWITCH_YLF:
    case SWITCH_YRF:
        is_running_y = false;
        pad &= 0xF8;     // [11111000]
        break;

    case SWITCH_YLR:
    case SWITCH_YRR:
        is_running_y = false;
        pad &= 0x1F;     // [00011111]
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
