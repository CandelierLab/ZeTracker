#include "mainwindow.h"
#include "ui_mainwindow.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    // === DEFINITIONS =====================================================

    // --- User interface
    ui->setupUi(this);
    this->setWindowTitle("ZeTracker");
    move(0,0);

    // --- Shortcuts
    sExit = new QShortcut(Qt::Key_Escape, this);

    // --- Messages
    MsgHandle = new MsgHandler(ui->INFOS);

    qInfo() << THREAD << "Main lives in thread: " << QThread::currentThreadId();
    QThread::currentThread()->setPriority(QThread::NormalPriority);
    qInfo() << THREAD << "Priority:" << QThread::currentThread()->priority();

    qInfo() << TITLE_1 << "Initialization";

    // --- Plots
    initPlots();

    // --- Motion
    Motion = new class Motion();
    Joystick = new class Joystick(Motion);

    // --- Vision
    Vision = new class Vision();

    // --- Interface
    Interface = new class Interface(this, Motion, Vision);

    // === INITIALIZATIONS =================================================

    posBufferSize = 100;
    ui->VERSION->setText(Interface->version);

    // --- Motion

    Joystick->scan();

    // --- Vision

    ui->PIX2MM->setText(QString::number(Vision->pix2mm*1000, 'f', 1));
    Vision->minBoutDelay = long(ui->MIN_BOUT_DELAY->text().toDouble()*1e6);
    Vision->thresholdCurvature = ui->THRESH_CURV->text().toDouble();
    Vision->processCalibration = ui->PROCESS_CALIBRATION->isChecked();
    Vision->processFish = ui->PROCESS_VISION->isChecked();

    // Vision->startCamera();
    setThreshold();

    // --- Runs

    isRunning = false;
    setMetas();
    runClock = new QTimer(this);
    runTimer = new QElapsedTimer();

    // --- Connection
    setData2send();

    // === CONNECTIONS =====================================================

    // --- Shortcuts
    connect(sExit, SIGNAL(activated()), QApplication::instance(), SLOT(quit()));

    // --- Menu
    connect(ui->SCAN_CONTROLLER, SIGNAL(triggered(bool)), Motion, SLOT(initFTDI()));
    connect(ui->SCAN_JOYSTICK, SIGNAL(triggered(bool)), Joystick, SLOT(scan()));

    // --- Motion -------------------------------

    // State
    connect(Motion, SIGNAL(updatePad(unsigned char)), this, SLOT(updatePad(unsigned char)));
    connect(Motion, SIGNAL(updateMotionState()), this, SLOT(updateMotionState()));

    // Home
    connect(ui->HOME, SIGNAL(clicked(bool)), this, SLOT(home()));
    connect(Motion, SIGNAL(homed()), this, SLOT(homed()));

    // Displacements
    connect(ui->MOVE_UL, SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));
    connect(ui->MOVE_U,  SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));
    connect(ui->MOVE_UR, SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));
    connect(ui->MOVE_L,  SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));
    connect(ui->MOVE_R,  SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));
    connect(ui->MOVE_DL, SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));
    connect(ui->MOVE_D,  SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));
    connect(ui->MOVE_DR, SIGNAL(clicked(bool)), Motion, SLOT(movePad(bool)));

    connect(ui->MOVE_XY, SIGNAL(pressed()), this, SLOT(moveFixed()));

    // Position
    connect(ui->RESET_COUNTS, SIGNAL(pressed()), Motion, SLOT(resetCounts()));
    connect(Motion, SIGNAL(updatePosition()), this, SLOT(updatePosition()));

    // Pointer
    connect(ui->POINTER, SIGNAL(toggled(bool)), Motion, SLOT(Pointer(bool)));

    // Mode
    connect(ui->MOTION_MODE, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));

    // Test
    connect(ui->DEMO, SIGNAL(toggled(bool)), Motion, SLOT(demo(bool)));

    // --- Vision -------------------------------

    // Process ?
    connect(ui->PROCESS_VISION, SIGNAL(stateChanged(int)), this, SLOT(processFrames(int)));
    connect(ui->PROCESS_CALIBRATION, SIGNAL(stateChanged(int)), this, SLOT(processCalibration(int)));

    // Background
    connect(ui->BACKGROUND_CALIBRATION, SIGNAL(pressed()), this, SLOT(saveBackground()));
    connect(ui->BACKGROUND_VISION, SIGNAL(pressed()), this, SLOT(saveBackground()));

    // Thresholds
    connect(ui->THRESH_BW_SLIDER, SIGNAL(valueChanged(int)), this, SLOT(setThreshold()));
    connect(ui->THRESH_CH_SLIDER, SIGNAL(valueChanged(int)), this, SLOT(setThreshold()));
    connect(ui->THRESH_CURV, SIGNAL(editingFinished()), this, SLOT(setCurvatureThreshold()));

    // Parameters
    connect(ui->MIN_BOUT_DELAY, SIGNAL(editingFinished()), this, SLOT(setMinBoutDelay()));

    // Updates
    connect(Vision, SIGNAL(updateExposure()), this, SLOT(updateExposure()));
    connect(Vision, SIGNAL(updateFPS()), this, SLOT(updateFPS()));
    connect(Vision, SIGNAL(updateProcessTime()), this, SLOT(updateProcessTime()));
    connect(Vision, SIGNAL(updateDisplay(QVector<UMat>)), this, SLOT(updateDisplay(QVector<UMat>)));
    connect(Vision, SIGNAL(updateProcessStatus(int)), this, SLOT(updateProcessStatus(int)));
    connect(Vision, SIGNAL(updateCurvature()), this, SLOT(updateCurvature()));

    // Calibration
    connect(ui->CALIBRATE, SIGNAL(pressed()), this, SLOT(calibrate()));
    connect(Vision, SIGNAL(updateCalibration()), this, SLOT(updateCalibration()));

    // --- Interface ----------------------------

    connect(ui->CONNECT, SIGNAL(toggled(bool)), Interface, SLOT(manageConnection(bool)));
    connect(ui->SEND_RUN_INFO, SIGNAL(pressed()), Interface, SLOT(sendRunInfo()));
    connect(ui->SEND_POSITIONS, SIGNAL(toggled(bool)), this, SLOT(setData2send()));
    connect(ui->SEND_BOUTS, SIGNAL(toggled(bool)), this, SLOT(setData2send()));

    connect(Interface, SIGNAL(updatePosition()), this, SLOT(updatePosition()));
    connect(Interface, SIGNAL(updateBouts()), this, SLOT(updateBouts()));
    connect(ui->NEW_RUN, SIGNAL(pressed()), Interface, SLOT(newRun()));

    // --- Runs ---------------------------------

    connect(ui->RUN_LOG, SIGNAL(toggled(bool)), this, SLOT(setMetas()));
    connect(ui->RUN_PARAMETERS, SIGNAL(toggled(bool)), this, SLOT(setMetas()));
    connect(ui->RUN_TRAJECTORY, SIGNAL(toggled(bool)), this, SLOT(setMetas()));
    connect(ui->RUN_BOUTS, SIGNAL(toggled(bool)), this, SLOT(setMetas()));
    connect(ui->RUN_BACKGROUND, SIGNAL(toggled(bool)), this, SLOT(setMetas()));
    connect(ui->RUN_IMAGES, SIGNAL(toggled(bool)), this, SLOT(setMetas()));

    connect(ui->OPEN_FOLDER, SIGNAL(pressed()), this, SLOT(openFolder()));
    connect(ui->RUN, SIGNAL(toggled(bool)), this, SLOT(setRun(bool)));
    connect(runClock, SIGNAL(timeout()), this, SLOT(updateRunTime()));

    // --- TEST ---------------------------------

    connect(ui->TRIGGER_BOUT, SIGNAL(pressed()), Interface, SLOT(newBout()));

}

/* ====================================================================== *
 *      DEVICES MANAGEMENT                                                *
 * ====================================================================== */




/* ====================================================================== *
 *      MOTION                                                            *
 * ====================================================================== */

void MainWindow::updatePad(unsigned char p) {

    ui->MOVE_DL->setChecked(p & 0x01);
    ui->MOVE_D->setChecked(p>>1 & 0x01);
    ui->MOVE_DR->setChecked(p>>2 & 0x01);
    ui->MOVE_L->setChecked(p>>3 & 0x01);
    ui->MOVE_R->setChecked(p>>4 & 0x01);
    ui->MOVE_UL->setChecked(p>>5 & 0x01);
    ui->MOVE_U->setChecked(p>>6 & 0x01);
    ui->MOVE_UR->setChecked(p>>7 & 0x01);

}

void MainWindow::updateMotionState() {

    if (Motion->is_moving) {
        ui->MOTION_STATE->setText(QString("ENABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background: #abebc6;"));
    } else {
        ui->MOTION_STATE->setText(QString("DISABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background: #ddd;"));
    }

}

void MainWindow::updatePosition() {

    // Display counts
    ui->COUNT_X->setText(QString("%1").arg(Motion->count_x, 5, 10, QLatin1Char('0')));
    ui->COUNT_Y->setText(QString("%1").arg(Motion->count_y, 5, 10, QLatin1Char('0')));

    // Display positions
    ui->CAM_X->setText(QString::number(Motion->count_x*Motion->count2mm, 'f', 3));
    ui->CAM_Y->setText(QString::number(Motion->count_y*Motion->count2mm, 'f', 3));

    dx.append(Vision->dx);
    dy.append(Vision->dy);
    while (dx.size() > posBufferSize) { dx.pop_front(); }
    while (dy.size() > posBufferSize) { dy.pop_front(); }
    double dx_ = 0;
    double dy_ = 0;
    for (int i=0; i<dx.size(); i++) {
        dx_ += dx.at(i);
        dy_ += dy.at(i);
    }

    ui->DX->setText(QString::number(dx_/dx.size(), 'f', 3));
    ui->DY->setText(QString::number(dy_/dy.size(), 'f', 3));

    // ui->POS_X->setText(QString::number(Interface->pos_x, 'f', 3));
    // ui->POS_Y->setText(QString::number(Interface->pos_y, 'f', 3));

    ui->POS_X->setText(QString::number(Motion->count_x*Motion->count2mm + dx_/dx.size(), 'f', 3));
    ui->POS_Y->setText(QString::number(Motion->count_y*Motion->count2mm + dy_/dy.size(), 'f', 3));

}

void MainWindow::home() {

    Motion->target_x = round(ui->HOME_X->text().toDouble()/Motion->count2mm);
    Motion->target_y = round(ui->HOME_Y->text().toDouble()/Motion->count2mm);
    Motion->home();

}

void MainWindow::homed() {

    qInfo() << TITLE_2 << "Homed";
    ui->HOME->setStyleSheet(QString("color:black;"));

}

void MainWindow::modeChanged(int m) {

    Motion->mode = m;

}

void MainWindow::moveFixed() {

    Motion->target_x = Motion->count_x + ui->MOVE_X->text().toInt();
    Motion->target_y = Motion->count_y + ui->MOVE_Y->text().toInt();
    Motion->moveFixed();

}

/* ====================================================================== *
 *      VISION                                                            *
 * ====================================================================== */

/* --- PARAMETERS ------------------------------------------------------- */

void MainWindow::saveBackground() {

    Vision->saveBackground = true;

}

void MainWindow::setThreshold() {

    ui->THRESH_BW->setText(QString::number(double(ui->THRESH_BW_SLIDER->value())/1000, 'f', 3));
    Vision->thresholdFish = double(ui->THRESH_BW_SLIDER->value())/1000;

    ui->THRESH_CH->setText(QString::number(double(ui->THRESH_CH_SLIDER->value())/1000, 'f', 3));
    Vision->thresholdCalibration = double(ui->THRESH_CH_SLIDER->value())/1000;

}

void MainWindow::setCurvatureThreshold() {

    Vision->thresholdCurvature = ui->THRESH_CURV->text().toDouble();

}

void MainWindow::setMinBoutDelay() {

    Vision->minBoutDelay = ui->MIN_BOUT_DELAY->text().toLong()*long(1e6);

}

/* --- IMAGE PROCESING -------------------------------------------------- */

void MainWindow::processFrames(int b) {

    // Turn calibration off
    if (Vision->processCalibration && b>0) {
        Vision->processCalibration = false;
        ui->PROCESS_CALIBRATION->setChecked(false);
    }

    // Process fish
    Vision->processFish = b>0;

}

/* --- DISPLAY ---------------------------------------------------------- */

void MainWindow::updateExposure() {

    ui->EXPOSURE->setText(QString("%1").arg(double(Vision->exposure)/1000));

}

void MainWindow::updateFPS() {

    statusBar()->showMessage(QString::number(Vision->fps, 'f', 2) + QString(" FPS"));

}

void MainWindow::updateDisplay(QVector<UMat> D) {

    // --- IMAGE 1

    if (D.size()>0) {

        QImage Img1((uchar*)D.at(0).getMat(ACCESS_READ).data, D.at(0).cols, D.at(0).rows, QImage::Format_Indexed8);
        for (int i=0 ; i<=255; i++) { Img1.setColor(i, qRgb(i,i,i)); }

        // Crosshair ?
        if (ui->DISPLAY_CROSSHAIR->isChecked()) {
            for (int i=0 ; i<Vision->width; i++) {
                Img1.setPixel(i, Vision->width/2, 255);
                Img1.setPixel(Vision->width/2, i, 255);
            }
        }

        QPixmap pix1 = QPixmap::fromImage(Img1);
        ui->IMAGE_1->setPixmap(pix1.scaled(ui->IMAGE_1->width(), ui->IMAGE_1->height(), Qt::KeepAspectRatio));

    } else { ui->IMAGE_1->clear(); }

    // --- IMAGE 2

    if (D.size()>1) {

        QImage Img2((uchar*)D.at(1).getMat(ACCESS_READ).data, D.at(1).cols, D.at(1).rows, QImage::Format_RGB888);
        QPixmap pix2 = QPixmap::fromImage(Img2);
        ui->IMAGE_2->setPixmap(pix2.scaled(ui->IMAGE_2->width(), ui->IMAGE_2->height(), Qt::KeepAspectRatio));

    } else { ui->IMAGE_2->clear(); }

}


void MainWindow::updateProcessStatus(int S) {

    switch (S) {
    case PROCESS_INACTIVE:

        ui->CALIBRATION_PROCESS_STATE->setText(QString("NOT PROCESSING"));
        ui->CALIBRATION_PROCESS_STATE->setStyleSheet(QString("background: #eee;"));

        ui->VISION_PROCESS_STATE->setText(QString("NOT PROCESSING"));
        ui->VISION_PROCESS_STATE->setStyleSheet(QString("background: #eee;"));

        break;

    case PROCESS_NOBACK:

        if (Vision->processCalibration) {
            ui->CALIBRATION_PROCESS_STATE->setText(QString("NO BACKGROUND"));
            ui->CALIBRATION_PROCESS_STATE->setStyleSheet(QString("background: #eee;"));
        }

        if (Vision->processFish) {
            ui->VISION_PROCESS_STATE->setText(QString("NO BACKGROUND"));
            ui->VISION_PROCESS_STATE->setStyleSheet(QString("background: #eee;"));
        }

        break;

    case PROCESS_FAILED:

        if (Vision->processCalibration) {
            ui->CALIBRATION_PROCESS_STATE->setText(QString("PROCESS FAILED"));
            ui->CALIBRATION_PROCESS_STATE->setStyleSheet(QString("background: #ff5050;"));
        }

        if (Vision->processFish) {
            ui->VISION_PROCESS_STATE->setText(QString("PROCESS FAILED"));
            ui->VISION_PROCESS_STATE->setStyleSheet(QString("background: #ff5050;"));
        }

        break;

    case PROCESS_OK:

        if (Vision->processCalibration) {
            ui->CALIBRATION_PROCESS_STATE->setText(QString("PROCESS OK"));
            ui->CALIBRATION_PROCESS_STATE->setStyleSheet(QString("background: #abebc6;"));
        }

        if (Vision->processFish) {
            ui->VISION_PROCESS_STATE->setText(QString("PROCESS OK"));
            ui->VISION_PROCESS_STATE->setStyleSheet(QString("background: #abebc6;"));
        }

        break;

    }

}

void MainWindow::updateProcessTime() {

    ui->PROCESS_TIME->setText(QString::number(double(Vision->processTime)*1e-6, 'f', 3));

}

/* ====================================================================== *
 *      CALIBRATION                                                       *
 * ====================================================================== */

void MainWindow::processCalibration(int b) {

    // Turn calibration off
    if (Vision->processFish && b>0) {
        Vision->processFish = false;
        ui->PROCESS_VISION->setChecked(false);
    }

    // Process calibration
    Vision->processCalibration = b>0;

}

void MainWindow::calibrate() {

    Vision->crossLength = ui->CROSS_LENGTH->text().toDouble();
    Vision->calibrate = true;

}

void MainWindow::updateCalibration() {

    // Save new calibration
    QFile file(Vision->calibrationPath);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream F(&file);
        F << Vision->pix2mm;
    }
    file.close();

    // Display calibration coefficient
    ui->PIX2MM->setText(QString::number(Vision->pix2mm*1000, 'f', 1));

}

/* ====================================================================== *
 *      PLOTS                                                             *
 * ====================================================================== */

void MainWindow::initPlots() {

    maxLengthCurv = 2000;
    maxLengthTraj = 500;

    // === CURVATURE =======================================================

    // Create graph
    ui->PLOT_CURVATURE->addGraph();

    // Labels & box
    ui->PLOT_CURVATURE->xAxis->setLabel("Time (s)");
    ui->PLOT_CURVATURE->yAxis->setLabel("Curvature (1/mm)");
    ui->PLOT_CURVATURE->axisRect()->setupFullAxesBox();

    // Axis ranges
    ui->PLOT_CURVATURE->yAxis->setRange(-1, 1);


    // === TRAJECTORY ======================================================

    // Create graph
    ui->PLOT_TRAJ->addGraph();

    // Labels & box
    ui->PLOT_TRAJ->xAxis->setLabel("x (mm)");
    ui->PLOT_TRAJ->yAxis->setLabel("y (mm)");
    ui->PLOT_TRAJ->axisRect()->setupFullAxesBox();

    // Axis ranges
    ui->PLOT_TRAJ->xAxis->setRange(-500, 500);
    ui->PLOT_TRAJ->yAxis->setRange(-500, 500);

}

void MainWindow::updateCurvature() {

    ui->PLOT_CURVATURE->graph(0)->setData(pTime, pCurv, true);
    ui->PLOT_CURVATURE->xAxis->setRange(pTime.first(), max(pTime.last(), 10.));
    ui->PLOT_CURVATURE->replot();

}

void MainWindow::updateBouts() {

    // --- Plot bouts

    // Append bout
    pX.append(Interface->lastBout.x);
    pY.append(Interface->lastBout.y);

    // Trim if too long
    while (pX.size() > maxLengthTraj) {
        pX.pop_front();
        pY.pop_front();
    }

    // Plot
    ui->PLOT_TRAJ->graph(0)->setData(pX,pY,true);
    ui->PLOT_TRAJ->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::red, 1), QBrush(Qt::red), 3));
    ui->PLOT_TRAJ->graph(0)->setPen(QPen(QColor(120, 120, 120), 1));
    ui->PLOT_TRAJ->replot();

    // --- Run stats

    if (isRunning) {
        ui->RUN_NUM_BOUTS->setNum(ui->RUN_NUM_BOUTS->text().toInt()+1);
    }

}

/* ====================================================================== *
 *      RUNS                                                              *
 * ====================================================================== */

void MainWindow::setMetas() {

    metaLog = ui->RUN_LOG->isChecked();
    metaParameters = ui->RUN_PARAMETERS->isChecked();
    metaTrajectory = ui->RUN_TRAJECTORY->isChecked();
    metaBouts = ui->RUN_BOUTS->isChecked();
    metaBackground = ui->RUN_BACKGROUND->isChecked();
    metaImages = ui->RUN_IMAGES->isChecked();

}

void MainWindow::updateMeta(int param, bool b) {

    QString done = "font-weight:600; color:darkslategray;";
    QString base = "";

    switch (param) {
    case META_LOG:          ui->RUN_LOG->setStyleSheet(b ? done : base); break;
    case META_PARAMETERS:   ui->RUN_PARAMETERS->setStyleSheet(b ? done : base); break;
    case META_TRAJECTORY:   ui->RUN_TRAJECTORY->setStyleSheet(b ? done : base); break;
    case META_BOUTS:        ui->RUN_BOUTS->setStyleSheet(b ? done : base); break;
    case META_BACKGROUND:   ui->RUN_BACKGROUND->setStyleSheet(b ? done : base); break;
    case META_IMAGES:       ui->RUN_IMAGES->setStyleSheet(b ? done : base); break;
    }

}

void MainWindow::updateRunPath() {

    ui->RUN_PATH->setText(Interface->runPath);

}

void MainWindow::openFolder() {

    QDesktopServices::openUrl(QUrl::fromLocalFile(Interface->runPath));

}

void MainWindow::setRun(bool b) {

    // --- Checks
    if (b && Interface->runPath.isEmpty()) {

        ui->RUN->setChecked(false);
        QMessageBox msgBox;
        msgBox.setText("Please create a run folder before starting a run.");
        msgBox.exec();
        return;
    }

    // Check if trajectories exist

    // --- GUI appearance
    if (b) {

        ui->RUN->setChecked(true);
        ui->RUN->setStyleSheet(QString("Background:#099;"));
        ui->RUN->setText("RUNNING");

        // Reset
        ui->RUN_TIME->setText(QString("00:00:00"));
        ui->RUN_NUM_BOUTS->setNum(0);

    } else {
        ui->RUN->setChecked(false);
        ui->RUN->setStyleSheet(QString(""));
        ui->RUN->setText("RUN");
    }

    // --- Timer
    if (b) {
        runTimer->start();
        runClock->start(1000);
    } else {
        //runClock->stop();
    }

    // Interface
    isRunning = b;
    Interface->setRun(b);

}

void MainWindow::updateRunTime() {

    QTime t(0,0,0,0);
    t = t.addMSecs(runTimer->elapsed());
    ui->RUN_TIME->setText(t.toString("hh:mm:ss"));

}

/* ====================================================================== *
 *      INTERFACE                                                         *
 * ====================================================================== */

void MainWindow::setConnected() {

    ui->CONNECT->setStyleSheet(QString("background:#099;"));
    ui->CONNECT->setText(QString("CONNECTED"));

}

void MainWindow::setDisconnected() {

    ui->CONNECT->setStyleSheet(QString(""));
    ui->CONNECT->setText(QString("Connect"));

}

void MainWindow::setData2send() {

    Interface->sendPositions = ui->SEND_POSITIONS->isChecked();
    Interface->sendBouts = ui->SEND_BOUTS->isChecked();

}

/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

MainWindow::~MainWindow() {
    delete ui;
}
