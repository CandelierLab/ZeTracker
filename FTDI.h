#ifndef FTDI_H
#define FTDI_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QVector>
#include <QThread>
#include <QElapsedTimer>
#include <QtMath>
#include <QDebug>

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

    FTDI_Device();
    ~FTDI_Device();

    // Open device
    unsigned int openDevice();

    // Device strings (and debug)
    QString strDeviceDescription();
    QString strOutputBuffer();
    QString strInputBuffer();

    // Digital I/O
    void setPin(int, bool);
    void sendOutput();

    // Interface
    void resetCounts();

    /* === PROPERTIES ======================== */

    int device_id;
    QString Type;
    QString SerialNumber;

    long int XloopPeriod, YloopPeriod;
    bool isRunningLoop;
    bool isRunningX, isRunningY, isRunningYL, isRunningYR;

public slots:

    void DataLoop();

signals:

    void inputRead(int);
    void enableState(bool);
    void updateCount(int, int);
    void uncheckPad();
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

    bool Enable = false;
    int Xcount = 0;
    int Ycount = 0;

};

#endif // FTDI_H
