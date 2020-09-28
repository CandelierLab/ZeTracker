 #include "Interface.h"
#include "mainwindow.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

Interface::Interface(MainWindow *MW, class Motion *M, class Vision *V)
    :Main(MW), Motion(M), Vision(V) {

    // === INITIALIZATION ==================================================

    trajectoryFid = new QFile(trajectoryFile);
    boutsFid = new QFile(boutsFile);

    conn = new QTcpSocket(this);
    connectionToPyBeautySuccesful = false;

    version = QString("1.31");
    dataRoot = QString("/home/ljp/Science/ZeBeauty/Data/");
    lastBout.i = 0;

    // === CONNECTIONS =====================================================

    connect(Vision, SIGNAL(updateFish()), this, SLOT(updateFish()));
    connect(Vision, SIGNAL(newBout()), this, SLOT(newBout()));

    connect(conn, SIGNAL(connected()), Main, SLOT(setConnected()));
    connect(conn, SIGNAL(disconnected()), Main, SLOT(setDisconnected()));
}

/* ====================================================================== *
 *      FISH                                                              *
 * ====================================================================== */

void Interface::updateFish() {

    // --- Position

    Motion->dx = Vision->dx;
    Motion->dy = Vision->dy;    // natural coordinates
    pos_x = Motion->count_x*Motion->count2mm_x + Vision->dx;
    pos_y = Motion->count_y*Motion->count2mm_y + Vision->dy;

    // --- GUI

    // Update position
    emit updatePosition();

    // --- Send position update
    if (Main->isRunning && conn->isWritable()) {
        string command;
        switch (protocol) {
        case PROTOCOL_UNIFORM:
            command = "x=" + to_string(Vision->dx*Vision->pix2mm) + ";y=" + to_string(Vision->dy*Vision->pix2mm) + ";\n";
            break;
        case PROTOCOL_UNIFORM_RELATIVE:
            command = "x=" + to_string(Vision->dx*Vision->pix2mm) + ";y=" + to_string(Vision->dy*Vision->pix2mm) + ";\n";
            break;
        case PROTOCOL_SPLIT:
            command = "x=" + to_string(Vision->dx*Vision->pix2mm) + ";y=" + to_string(Vision->dy*Vision->pix2mm) + ";th=" + to_string(Vision->fish.head.theta) + ";\n";
            break;
        }
        conn->write(command.c_str());
        conn->flush();
    }


    // Update bout signal
    Main->pTime.append(Vision->time*1e-9);
    Main->pCurv.append(Vision->boutSignal);

    while (Main->pTime.size() > Main->maxLengthCurv) {
        Main->pTime.pop_front();
        Main->pCurv.pop_front();
    }

    // --- Save trajectory point

    if (Main->isRunning) {
        QDataStream F(trajectoryFid);
        F << Vision->time << double(Motion->count_x) << double(Motion->count_y)
          << Vision->dx << Vision->dy << pos_x << pos_y << Vision->fish.body.theta
          << Vision->fish.head.x << Vision->fish.head.y << Vision->fish.head.theta
          << Vision->fish.xc << Vision->fish.yc;
    }

}

void Interface::newBout() {

    // --- Update bout
    lastBout.i++;
    lastBout.t = Vision->time;
    lastBout.x = pos_x;
    lastBout.y = pos_y;

    // --- Save Bout
    if (Main->isRunning) {
        QTextStream F(boutsFid);
        F << lastBout.i << "\t" << lastBout.t << "\t" << lastBout.x << "\t" << lastBout.y << endl;
    }

    // --- Send bout
    if (Main->isRunning && conn->isWritable()) {
        switch (protocol) {
        case PROTOCOL_UNIFORM:
            conn->write("BOUT\n");
            conn->flush();
            break;
        case PROTOCOL_UNIFORM_RELATIVE:
            conn->write("BOUT\n");
            conn->flush();
            break;
        case PROTOCOL_SPLIT:
            conn->write("BOUT\n");
            conn->flush();
            break;
        }
    }

    // --- Update GUI
    emit updateBouts();

}

/* ====================================================================== *
 *      RUNS                                                              *
 * ====================================================================== */

void Interface::newRun() {

    // --- Run path -----------------------------

    // Create run path
    QDateTime D = QDateTime::currentDateTime();
    QString datePath = dataRoot + D.date().toString("yyyy-MM-dd");
    runPath = datePath + QString("/Run ") + D.time().toString("hh-mm-ss/");
    // runPath = datePath + QString("/Run test/");

    // Update run path
    Main->updateRunPath();

    // Create folders
    if (!QDir(datePath).exists()) { QDir().mkdir(datePath); }
    if (!QDir(runPath).exists()) { QDir().mkdir(runPath); }

    // --- Log ----------------------------------

    logFile = runPath + QString("Log.txt");

    // --- Parameters ---------------------------

    if (Main->metaParameters) {

        parametersFile = runPath + QString("Parameters.txt");

        QFile file(parametersFile);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream F(&file);
            F << "# ================== #" << endl;
            F << "#      ZeBeauty      #" << endl;
            F << "# ================== #" << endl << endl;

            F << "Version            " << version << endl << endl;

            F << "# --- Vision" << endl;
            F << "exposure           " << Vision->exposure << " us" << endl;
            F << "width              " << Vision->width << " pix" << endl;
            F << "height             " << Vision->height << " pix" << endl;
            F << "pix2mm             " << Vision->pix2mm << " mm" << endl;
            F << "thresholdFishHead      " << Vision->thresholdFishHead << " abs. pix val." << endl;
            F << "thresholdFishTail      " << Vision->thresholdFishTail << " abs. pix val." << endl;
            F << "eyesROIRadius       " << Vision->eyesROIRadius << " pix" << endl;
            F << "fishROIRadius       " << Vision->fishROIRadius  << " pix" << endl;
            F << "fishROIShift       " << Vision->fishROIShift << " pix" << endl;
            F << "nEyes       " << Vision->nEyes << " pix" << endl;
            F << "nHead       " << Vision->nHead << " pix" << endl;
            F << "thresholdBout " << Vision->thresholdBout << " (rel.)" << endl;
            F << "minimumBoutThreshold "  << Vision->minimumBoutThreshold << " val." << endl;
            F << "minBoutDelay       " << Vision->minBoutDelay << " ns" << endl;
            F << "prevBoutBufferMaxSize " << Vision->prevBoutBufferMaxSize << " (rel.)" << endl;
            F << "bufferDelay       " << Vision->bufferDelay << " ns" << endl;

            F << endl << "# --- Motion" << endl;
            F << "loop_period_x      " << Motion->loop_period_x << " ns" << endl;
            F << "loop_period_y      " << Motion->loop_period_y << " ns" << endl;
            F << "count2mm_x         " << Motion->count2mm_x << " mm" << endl;
            F << "count2mm_y         " << Motion->count2mm_y << " mm" << endl;

        }

        file.close();

        // Update GUI
        Main->updateMeta(META_PARAMETERS, true);

    }

    // --- Trajectories -------------------------

    trajectoryFile = runPath + QString("Trajectory.dat");
    boutsFile = runPath + QString("Bouts.txt");
    backgroundFile = runPath + QString("Background.pgm");
    imagesPath = runPath + QString("Images/");

}

void Interface::setRun(bool b) {

    if (b) {

        // --- Trajectory file
        if (Main->metaTrajectory) {
            QFileInfo tfile(trajectoryFile);
            if (!tfile.exists()) {
                trajectoryFid = new QFile(trajectoryFile);
                if (trajectoryFid->open(QIODevice::WriteOnly | QIODevice::Append)) {
                    QDataStream F(trajectoryFid);
                    F << version.toDouble();
                    Main->updateMeta(META_TRAJECTORY, true);
                } else { qWarning() << "Could not open trajectory file";}
            } else {
                Main->setRun(false);
                QMessageBox msgBox;
                msgBox.setText("The trajectory file already exists for this run. Please create a new run.");
                msgBox.exec();
                return;
            }
        }

        // --- Bout file
        if (Main->metaBouts) {
            boutsFid = new QFile(boutsFile);
            if (boutsFid->open(QIODevice::WriteOnly | QIODevice::Append)) {
                Main->updateMeta(META_BOUTS, true);
            } else { qWarning() << "Could not open bouts file";}
        }

        // --- Background
        if (Main->metaBackground) {
            QFile::copy(Vision->backgroundPath, backgroundFile);
            Main->updateMeta(META_BACKGROUND, true);
        }

        // --- Image folder
        if (Main->metaImages) {
            if (!QDir(imagesPath).exists()) { QDir().mkdir(imagesPath); }
            Main->updateMeta(META_IMAGES, true);
            connect(Vision, SIGNAL(updateDisplay(QVector<UMat>)), this, SLOT(saveDisplay(QVector<UMat>)));
        }

    } else {

        if (Main->metaTrajectory && trajectoryFid->isOpen()) { trajectoryFid->close(); }
        if (Main->metaBouts && boutsFid->isOpen()) { boutsFid->close(); }
    }

}

void Interface::saveDisplay(QVector<UMat> Img) {

    if (Main->isRunning && Main->metaImages) {

        QString iname = imagesPath + QString("Frame_%1.pgm").arg(Vision->frame, 10, 10, QLatin1Char('0'));
        cv::imwrite(iname.toStdString().c_str(), Img.at(0));

    }
}

/* ====================================================================== *
 *      CONNECTION                                                        *
 * ====================================================================== */

void Interface::manageConnection(bool b) {

    if (b) {
       // conn->connectToHost("localhost", 3231);
        conn->connectToHost("10.0.0.20", 3231);

    } else {
        conn->write("END\n");
        conn->flush();
        conn->close();
        connectionToPyBeautySuccesful = false;
    }

}

void Interface::sendRunInfo() {
    if (!Main->isRunning) {
        if (conn->state()==QAbstractSocket::ConnectedState) {
            if (connectionToPyBeautySuccesful) {
                // Reset the run in pyBeauty
                conn->write("END\n");
                conn->flush();

                // Reset success flag
                connectionToPyBeautySuccesful = false;
            }

            QString protocolCommand;

            switch (protocol) {
            case PROTOCOL_UNIFORM:
                protocolCommand = "SET_MODE_UNIFORM";
                break;
            case PROTOCOL_UNIFORM_RELATIVE:
                protocolCommand = "SET_MODE_UNIFORM_RELATIVE";
                break;
            case PROTOCOL_SPLIT:
                protocolCommand = "SET_MODE_SPLIT";
                break;
            }

            conn->write((protocolCommand + ";path=" + runPath).toStdString().c_str());
            conn->flush();
            qInfo() << "Sent run info.";

            // Wait for confirmation of success
            connect(conn, SIGNAL(readyRead()), this, SLOT(respondToConnectionConfirmation()));
            qInfo() << "Waiting for confirmation...";

        } else {
            // conn not in ConnectedState
            qInfo() << "Cannot send run info, no connection.";
            return;
        }
    } else {
        qInfo() << "Cannot send run info when the experiment is running.";
    }
}

void Interface::respondToConnectionConfirmation() {
    // Read response
    QByteArray responseBA = conn->readLine(128);
    QString receivedCommand = QString::fromStdString(responseBA.data());
    //qInfo() << receivedCommand;

    if (receivedCommand=="MODE_SET") {
        qInfo() << "Received confirmation of success.";
        connectionToPyBeautySuccesful = true;
        // Disconnect self
        disconnect(conn, SIGNAL(readyRead()), this, SLOT(respondToConnectionConfirmation()));
    } else {
        if (receivedCommand=="FILE_EXISTS") {
            qInfo() << ("File already exists.");
            QMessageBox msgBox;
            msgBox.setText("pyBeauty: The file already exists for this run. Please create a new run.");
            msgBox.exec();
            return;
        } else {
            qInfo() << ("Unknown message received into conn: " + receivedCommand);
        }
    }
}
