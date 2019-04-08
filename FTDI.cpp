#include "FTDI.h"

/* #############################################################################################
   #                                                                                           #
   #                                        DEVICES                                            #
   #                                                                                           #
   ############################################################################################# */

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

FTDI_Device::FTDI_Device(class Motion* M)
    :Motion(M) {

    // === DEFINITIONS =====================================================

    device_id = 0;

}

/* ====================================================================== *
 *      INITIALIZATION                                                    *
 * ====================================================================== */

unsigned int FTDI_Device::openDevice() {

    ftStatus = FT_Open(device_id, &ftHandle);

    if (ftStatus == FT_OK) {

        // Initialization routines
        _reset();

        getEEPROMData();    // Populate Data structure
        setMPSSE();         // Set MPSSE mode
        setClock();         // Set clock to 200kHz

        // Set all pins to down, purge read buffer
        IN = 0;
        OUT = 0;
        sendOutput();

    }

    return ftStatus;
}

/* ====================================================================== *
 *      LOW-LEVEL OPERATION                                               *
 * ====================================================================== */

void FTDI_Device::_reset() { NumBytesToSend = 0; }

void FTDI_Device::_add(BYTE B) { OutputBuffer[NumBytesToSend++] = B; }

void FTDI_Device::_send() {

    ftStatus = FT_Write(ftHandle, OutputBuffer, NumBytesToSend, &NumBytesSent);
    if (ftStatus != FT_OK) { qWarning() << "Error while sending" << ftStatus; }
    _reset();

}

void FTDI_Device::_read() {

    // Get number of available bytes
    ftStatus = FT_GetQueueStatus(ftHandle, &NumBytesToRead);

    // Read out data
    if ((ftStatus == FT_OK) && (NumBytesToRead > 0)) {
        FT_Read(ftHandle, &InputBuffer, NumBytesToRead, &NumBytesRead);
        if (ftStatus != FT_OK) { qWarning() << "Error while reading" << ftStatus; }
    }

    if (ftStatus != FT_OK) { qWarning() << "Error while getting number of available bytes" << ftStatus; }
}

/* ====================================================================== *
 *      DEVICE ROUTINES                                                   *
 * ====================================================================== */

void FTDI_Device::getEEPROMData() {

    Data.Signature1 = 0x00000000;
    Data.Signature2 = 0xffffffff;
    Data.Manufacturer = static_cast<char *>(malloc(256));
    Data.ManufacturerId = static_cast<char *>(malloc(256));
    Data.Description = static_cast<char *>(malloc(256));
    Data.SerialNumber = static_cast<char *>(malloc(256));

    ftStatus = FT_EE_Read(ftHandle, &Data);
    if(ftStatus != FT_OK) { qWarning() << "FT_EE_Read failed"; }

}

void FTDI_Device::setMPSSE() {

    ftStatus = FT_OK;

    // Purge USB receive buffer first by reading out all old data from FT2232H receive buffer
    ftStatus |= FT_ResetDevice(ftHandle);

    // Get the number of bytes in the FT2232H
    ftStatus |= FT_GetQueueStatus(ftHandle, &NumBytesToRead);

    // Read out the data from FT2232H receive buffer
    if ((ftStatus == FT_OK) && (NumBytesToRead > 0)) {
        FT_Read(ftHandle, &InputBuffer, NumBytesToRead, &NumBytesRead);
    }

    // Set USB request transfer sizes to 64K
    ftStatus |= FT_SetUSBParameters(ftHandle, 65536, 65535);

    // Disable event and error characters
    ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0);

    // Sets the read and write timeouts in milliseconds
    ftStatus |= FT_SetTimeouts(ftHandle, 0, 5000);

    // Set the latency timer to 1mS (default is 16mS)
    ftStatus |= FT_SetLatencyTimer(ftHandle, 1);

    // Turn on flow control to synchronize IN requests
    ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0x00, 0x00);

    // Reset controller
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x00);

    // Enable MPSSE mode
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x02);
    if (ftStatus != FT_OK) {
        qDebug() << "Error in initializing the MPSSE mode" << ftStatus;
    }

    // Wait for all the USB stuff to complete and work
    QThread::msleep(20);

}

void FTDI_Device::setClock() {

/*  The clock dividor sets the clock frequency/period with:
 *      TCK Frequency = 60/( 2*(1+Value) ) (MHz)
 *      TCK Period = ( 2*(1+Value) )/60    (µs)
 *
 *  A value of 74 (0x4A) gives a period of 2.5µs (400kHz)
 *  A value of  8 (0x08) gives a period of 300ns (3.33MHz)
 */

    _add(0x8A);     // [ DISABLE CLOCK DIVIDE BY 5 ]
    _add(0x97);     // [ TURN OFF ADAPTATIVE CLOCKING ]

    _add(0x86);     // [ CLOCK DIVIDOR ] (8)
    _add(0x08);     // + Low byte  (8)
    _add(0x00);     // + High byte (0)

    _send();

}

/* ====================================================================== *
 *      OUTPUT                                                            *
 * ====================================================================== */

void FTDI_Device::setPin(int pin, bool state) {

    switch (pin) {
        case 0: if (state) { OUT |= 0x01; } else { OUT &= 0xFE; } break;
        case 1: if (state) { OUT |= 0x02; } else { OUT &= 0xFD; } break;
        case 2: if (state) { OUT |= 0x04; } else { OUT &= 0xFB; } break;
        case 3: if (state) { OUT |= 0x08; } else { OUT &= 0xF7; } break;
        case 4: if (state) { OUT |= 0x10; } else { OUT &= 0xEF; } break;
        case 5: if (state) { OUT |= 0x20; } else { OUT &= 0xDF; } break;
        case 6: if (state) { OUT |= 0x40; } else { OUT &= 0xBF; } break;
        case 7: if (state) { OUT |= 0x80; } else { OUT &= 0x7F; } break;
    }

}

void FTDI_Device::sendOutput() {

    _add(0x82);         // [ HIGH BYTE SETUP ]
    _add(OUT);          // + Value     (OUT)
    _add(0xFF);         // + Direction
    _send();

}

/* ====================================================================== *
 *      DATA LOOP                                                         *
 * ====================================================================== */

void FTDI_Device::DataLoop() {

    qInfo().nospace() << THREAD << "[Dev" << device_id << "] loop thread: " << QThread::currentThreadId();
    qInfo() << THREAD << "Priority:" << QThread::currentThread()->priority();
    qInfo() << "Starting data loop";

    // Define time references
    long int now;
    long int tref_x = 0;
    long int tref_y = 0;
    Motion->timer.start();

    // Booleans
    running = true;
    bool send;
    bool state_x = false;
    bool state_y = false;

    while (running) {

        send = false;

        now = Motion->timer.nsecsElapsed();

        // --- Mode

        if (Motion->mode) {
            double dr = sqrt(Motion->dx*Motion->dx + Motion->dy*Motion->dy);

            if (Motion->dx<-10) {
                setPin(0, false);
                Motion->is_running_x = true;
            } else if (Motion->dx>10) {
                setPin(0, true);
                Motion->is_running_x = true;
            } else {
                Motion->is_running_x = false;
            }

            if (Motion->dy>10) {
                setPin(2, false);
                setPin(4, false);
                Motion->is_running_y = true;
            } else if (Motion->dy<-10) {
                setPin(2, true);
                setPin(4, true);
                Motion->is_running_y = true;
            } else {
                Motion->is_running_y = false;
            }

        }

        // --- Loops

        if (now-tref_x >= Motion->loop_period_x) {

            Motion->period_x = now - tref_x;
            emit Motion->updatePeriods();

            // --- Input read

            _read();
            if (NumBytesRead) {

                // Switches
                if (Motion->is_running_x &  (OUT & 0x01) & !(InputBuffer[0] & 0x01))        { emit switchTriggered(SWITCH_XL); }
                if (Motion->is_running_x & !(OUT & 0x01) & !(InputBuffer[0]>>1 & 0x01))     { emit switchTriggered(SWITCH_XR); }
                if (Motion->is_running_y & !(OUT>>2 & 0x01) & !(InputBuffer[0]>>2 & 0x01))  { emit switchTriggered(SWITCH_YLF); }
                if (Motion->is_running_y &  (OUT>>2 & 0x01) & !(InputBuffer[0]>>3 & 0x01))  { emit switchTriggered(SWITCH_YLR); }
                if (Motion->is_running_y & !(OUT>>2 & 0x01) & !(InputBuffer[0]>>4 & 0x01))  { emit switchTriggered(SWITCH_YRF); }
                if (Motion->is_running_y &  (OUT>>2 & 0x01) & !(InputBuffer[0]>>5 & 0x01))  { emit switchTriggered(SWITCH_YRR); }

            }

            // Prepare read for next iteration
            _add(0x81);          // [ LOW BYTE READOUT ]

            // --- Motor output

            if (Motion->is_running_x)  {
                setPin(1, state_x);
                state_x = !state_x;
                if (state_x) {
                    if (OUT & 0x01) { Motion->count_x++; } else { Motion->count_x--; }
                    // Motion->is_running_x = false;
                    emit Motion->updatePosition();
                }
            }

            tref_x += Motion->loop_period_x;
            send = true;

        }

        if (now-tref_y >= Motion->loop_period_y) {

            Motion->period_y = now - tref_y;
            emit Motion->updatePeriods();

            // --- Motor output

            if (Motion->is_running_y) {
                setPin(3, state_y);
                setPin(5, state_y);
                state_y = !state_y;
                if (state_y) {
                    if (OUT>>2 & 0x01) { Motion->count_y++; } else { Motion->count_y--; }
                    // Motion->is_running_y = false;
                    emit Motion->updatePosition();
                }
            }

            tref_y += Motion->loop_period_y;
            send = true;

        }

        if (send) {

            // Enable state
            if (Motion->motion_state != Motion->is_running_x || Motion->is_running_y) {
                Motion->motion_state = Motion->is_running_x || Motion->is_running_y;
                emit Motion->updateMotionState();
            }

            setPin(7, !Motion->motion_state);

            sendOutput();
        }

    }

    emit finished();
    return;

}

/* ====================================================================== *
 *      DEVICE STRINGS (+DEBUG)                                           *
 * ====================================================================== */

QString FTDI_Device::strDeviceDescription() {

    QString out;
    out += "[Dev" + QString("%1").arg(device_id) + "] " + Data.Description + " (" + Data.SerialNumber + ")";
    return out;
}

QString FTDI_Device::strInputBuffer() {

    QString str;
    for (DWORD i=0 ; i<NumBytesRead ; i++) {
        if (i) { str += ","; }
        str += QString("%1").arg(InputBuffer[i]);
    }
    return str;
}

QString FTDI_Device::strOutputBuffer() {

    QString str;
    for (DWORD i=0 ; i<NumBytesToSend ; i++) {
        if (i) { str += ","; }
        str += QString("%1").arg(OutputBuffer[i]);
    }
    return str;
}

/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

FTDI_Device::~FTDI_Device() {

    qDebug() << "--- Destroying" << this->device_id;

    FT_Close(ftHandle);

}
