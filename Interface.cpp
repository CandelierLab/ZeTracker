#include "Interface.h"
#include "mainwindow.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

Interface::Interface(MainWindow *MW, class Motion *M, class Vision *V)
    :Main(MW), Motion(M), Vision(V) {

    // === INITIALIZATION ==================================================

    version = QString("1.0");
    dataRoot = QString("/home/ljp/Science/ZeBeauty/Data/");

    // === CONNECTIONS =====================================================

    connect(Vision, SIGNAL(updateFish()), this, SLOT(updateFish()));
    connect(Vision, SIGNAL(newBout()), this, SLOT(newBout()));
}

/* ====================================================================== *
 *      FISH                                                              *
 * ====================================================================== */

void Interface::updateFish() {

    // --- GUI -------------------------------------------------------------

    // Update position

    Motion->dx = Vision->dx;
    Motion->dy = Vision->dy;

    pos_x = Motion->count_x*Motion->count2mm_x + Vision->dx;
    pos_y = Motion->count_y*Motion->count2mm_y + Vision->dy;
    emit updatePosition();

    // Update curvature
    Main->pTime.append(Vision->time*1e-9);
    Main->pCurv.append(Vision->fish.curvature/Vision->pix2mm);

    while (Main->pTime.size() > Main->maxLengthCurv) {
        Main->pTime.pop_front();
        Main->pCurv.pop_front();
    }

}

void Interface::newBout() {

    // --- Update GUI

    lastBout.t = Vision->time;
    lastBout.x = pos_x;
    lastBout.y = pos_y;
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

    trajectoryFile = runPath + QString("Trajectory.txt");
    boutsFile = runPath + QString("Bouts.txt");

}

void Interface::setRun(bool b) {

    if (b) {

        if (Main->metaTrajectory) {
            trajectoryFid = new QFile(trajectoryFile);
            if (!trajectoryFid->open(QIODevice::ReadWrite)) { qWarning() << "Could not open trajectory file";}
        }

        if (Main->metaBouts) {
            boutsFid = new QFile(boutsFile);
            if (!boutsFid->open(QIODevice::ReadWrite)) { qWarning() << "Could not open bouts file";}
        }

    } else {

        if (Main->metaTrajectory) { trajectoryFid->close(); }
        if (Main->metaBouts) { boutsFid->close(); }

    }

}

/* ====================================================================== *
 *      REMOTE                                                            *
 * ====================================================================== */
