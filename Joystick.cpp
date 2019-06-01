#include "Joystick.h"

Joystick::Joystick(class Motion* M):Motion(M) {

    active = false;

}

void Joystick::scan() {

    qInfo() << TITLE_2 << "Joystick";

    // --- Get available ports

    const QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    qInfo() << infos.length() << "serial connections detected";

    // --- Search for the joystick
    for (int i=0; i<infos.length(); i++) {

        // --- Checks

        // Skip non-ACM connections
        if (infos[i].portName().left(6)!="ttyACM") { continue; }

        // --- Open connection and ask for id

        conn = new QSerialPort(this);
        conn->setPortName(infos[i].portName());
        conn->setBaudRate(115200);
        conn->setDataBits(QSerialPort::Data8);
        conn->setParity(QSerialPort::NoParity);
        conn->setStopBits(QSerialPort::OneStop);
        conn->setFlowControl(QSerialPort::NoFlowControl);

        if (conn->open(QIODevice::ReadWrite)) {

            // --- Check serial id

            // Read response
            conn->waitForReadyRead(2000);
            QByteArray readData = conn->readAll();
            while (conn->waitForReadyRead(5)) { readData.append(conn->readAll()); }

            // Convert the response to an identifier
            QString identifier(readData);
            if (identifier=="ZeJoystick") {

                active = true;
                qInfo() << conn->portName() << "is" << identifier;

                // Establish signal/slot connection
                connect(conn, SIGNAL(readyRead()), this, SLOT(serialInput()));
                break;

            } else {
                conn->close();
            }

        } else {
            qWarning() << "Failed to open port" << conn->portName();
        }

    }

    if (!active) { qInfo() << "No joystick detected"; }

}

Joystick::~Joystick() {

    if (conn->isOpen()) { conn->close(); }

}

void Joystick::serialInput() {

    // --- Read & convert input

    QByteArray readData = conn->readAll();
    while (conn->waitForReadyRead(5)) {
        readData.append(conn->readAll());
    }
    QString input(readData);

    // --- Parse and process

    if (input.left(5)=="MOVE_") {
        Motion->move(input, true);
    }

    // qDebug() << input;

}

