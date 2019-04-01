#include "FTDI.h"

/* #############################################################################################
   #                                                                                           #
   #                                        DEVICES                                            #
   #                                                                                           #
   ############################################################################################# */

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

FTDI_Device::FTDI_Device() {

    // === DEFINITIONS =====================================================

    device_id = 0;
    isRunningLoop = false;
    isRunningX = false;
    isRunningY = false;
    isRunningYL = false;
    isRunningYR = false;

    XloopPeriod = 1e6;          // X loop period (ns) | max X speed @ 5e5
    YloopPeriod = 5e6;          // Y loop period (ns) | max Y speed @ 2e6

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

    isRunningLoop = true;

    qreal tRefX = 0;
    qreal tRefY = 0;
    QElapsedTimer timer;
    timer.start();

    bool send;
    bool stateX = false;
    bool stateY = false;

    while (isRunningLoop) {

        send = false;

        if (timer.nsecsElapsed()-tRefX >= XloopPeriod) {

            // --- Input read

            _read();
            if (NumBytesRead) {

                // Switches
                if (isRunningX &  (OUT & 0x01) & !(InputBuffer[0] & 0x01)) { isRunningX = false; emit uncheckPad(); }
                if (isRunningX & !(OUT & 0x01) & !(InputBuffer[0]>>1 & 0x01)) { isRunningX= false; emit uncheckPad(); }
                if (isRunningY &  (OUT>>2 & 0x01) & (!(InputBuffer[0]>>3 & 0x01) | !(InputBuffer[0]>>5 & 0x01))) { isRunningY = false; emit uncheckPad(); }
                if (isRunningY & !(OUT>>2 & 0x01) & (!(InputBuffer[0]>>2 & 0x01) | !(InputBuffer[0]>>4 & 0x01))) { isRunningY = false; emit uncheckPad(); }

                // Update user interface
                emit inputRead(InputBuffer[0]);

            }

            // Prepare read for next iteration
            _add(0x81);          // [ LOW BYTE READOUT ]

            // --- Motor output

            if (isRunningX)  {
                setPin(1, stateX);
                stateX = !stateX;
                if (stateX) {
                    if (OUT & 0x01) { Xcount++; } else { Xcount--; }
                    emit updateCount(Xcount, Ycount);
                }
            }

            tRefX += XloopPeriod;
            send = true;

        }

        if (timer.nsecsElapsed()-tRefY >= YloopPeriod) {

            // --- Motor output

            if (isRunningY) {
                setPin(3, stateY);
                setPin(5, stateY);
                stateY = !stateY;
                if (stateY) {
                    if (OUT>>2 & 0x01) { Ycount++; } else { Ycount--; }
                    emit updateCount(Xcount, Ycount);
                }
            }

            tRefY += YloopPeriod;
            send = true;

        }

        if (send) {

            // Enable state
            if (Enable != isRunningX || isRunningY) {
                Enable = isRunningX || isRunningY;
                emit enableState(Enable);
            }
            setPin(7, !Enable);

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

void FTDI_Device::resetCounts() {

    Xcount = 0;
    Ycount = 0;
    emit updateCount(Xcount, Ycount);

}

/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

FTDI_Device::~FTDI_Device() {

    qDebug() << "--- Destroying" << this->device_id;

    FT_Close(ftHandle);

}
