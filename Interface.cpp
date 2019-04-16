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

    version = QString("1.0");
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
    Motion->dy = Vision->dy;
    pos_x = Motion->count_x*Motion->count2mm_x + Vision->dx;
    pos_y = Motion->count_y*Motion->count2mm_y + Vision->dy;

    // --- GUI

    // Update position
    emit updatePosition();

    // Update curvature
    Main->pTime.append(Vision->time*1e-9);
    Main->pCurv.append(Vision->fish.curvature/Vision->pix2mm);

    while (Main->pTime.size() > Main->maxLengthCurv) {
        Main->pTime.pop_front();
        Main->pCurv.pop_front();
    }

    // --- Save Bout

    if (Main->isRunning) {
        QDataStream F(trajectoryFid);
        F << Vision->time << double(Motion->count_x) << double(Motion->count_y)
          << Vision->dx << Vision->dy << pos_x << pos_y << Vision->fish.body.theta
          << Vision->fish.head.x << Vision->fish.head.y << Vision->fish.head.theta
          << Vision->fish.tail.x << Vision->fish.tail.y << Vision->fish.tail.theta
          << Vision->fish.xc << Vision->fish.yc << Vision->fish.curvature;
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
    if (Main->isRunning && sendBouts && conn->isWritable()) {
        conn->write("bout");
        conn->flush();
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
    // runPath = datePath + QString("/Run ") + D.time().toString("hh-mm-ss/");
    runPath = datePath + QString("/Run test/");

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
            F << "thresholdFish      " << Vision->thresholdFish << " %" << endl;
            F << "thresholdCurvature " << Vision->thresholdCurvature << " 1/mm" << endl;
            F << "minBoutDelay       " << Vision->minBoutDelay << " ns" << endl;

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

}

void Interface::setRun(bool b) {

    if (b) {

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

        if (Main->metaBouts) {
            boutsFid = new QFile(boutsFile);
            if (boutsFid->open(QIODevice::WriteOnly | QIODevice::Append)) {
                Main->updateMeta(META_BOUTS, true);
            } else { qWarning() << "Could not open bouts file";}
        }

    } else {

        if (Main->metaTrajectory && trajectoryFid->isOpen()) { trajectoryFid->close(); }
        if (Main->metaBouts && boutsFid->isOpen()) { boutsFid->close(); }
    }

}

/* ====================================================================== *
 *      CONNECTION                                                        *
 * ====================================================================== */

void Interface::manageConnection(bool b) {

    if (b) {
        conn->connectToHost("localhost", 3231);
    } else {
        conn->write("quit");
        conn->flush();
        conn->close();
    }

}

void Interface::sendRunInfo() {

    conn->write(("path=" + runPath).toStdString().c_str());
    conn->flush();

}
