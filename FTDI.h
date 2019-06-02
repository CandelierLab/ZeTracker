#ifndef FTDI_H
#define FTDI_H

#define SWITCH_X 0
#define SWITCH_Y 1

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QThread>
#include <QtMath>
#include <QList>
#include <QDebug>

#include "Motion.h"
#include "MsgHandler.h"

#ifdef __linux__
#include <../../ftd2xx_linux/ftd2xx.h>
#elif _WIN32
#include <../../ftd2xx_win/ftd2xx.h>
#else
#endif

/* #############################################################################################
   #                                                                                           #
   #                                     FTDI DEVICE                                           #
   #                                                                                           #
   ############################################################################################# */

class FTDI_Device : public QObject {

    Q_OBJECT

public:

    /* === METHODS =========================== */

    FTDI_Device(class Motion*);
    ~FTDI_Device();

    // Open device
    unsigned int openDevice();

    // Device strings (and debug)
    QString strDeviceDescription();
    QString strOutputBuffer();
    QString strInputBuffer();

    // Digital I/O
    bool getPin(int);
    void setPin(int, bool);
    void sendOutput();

    /* === PROPERTIES ======================== */

    int device_id;
    QString Type;
    QString SerialNumber;

    bool running;

public slots:

    void DataLoop();

signals:

    void homed();
    void enableState(bool);
    void setPad(int, bool);
    void switchTriggered(int);
    void finished();

private:

    /* === METHODS =========================== */

    // Low-level operation
    void _reset();
    void _add(BYTE);
    void _send();
    void _read();

    // Device routines
    void getEEPROMData();
    void setMPSSE();
    void setClock();

    // Data loops

    /* === PROPERTIES ======================== */

    FT_STATUS ftStatus;
    FT_HANDLE ftHandle;
    FT_PROGRAM_DATA Data;

    BYTE IN;
    BYTE OUT;
    BYTE InputBuffer[1024];
    BYTE OutputBuffer[1024];

    DWORD NumBytesToSend = 0;
    DWORD NumBytesToRead = 0;
    DWORD NumBytesRead = 0;
    DWORD NumBytesSent = 0;

    class Motion *Motion;

};

#endif // FTDI_H
