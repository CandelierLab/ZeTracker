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

    // --- Vision
    Vision = new class Vision();

    // --- Interface
    Interface = new class Interface(this, Motion, Vision);

    // === INITIALIZATIONS =================================================

    // --- Motion
    ui->TARGET_LOOP_X->setText(QString::number(Motion->loop_period_x));
    ui->TARGET_LOOP_Y->setText(QString::number(Motion->loop_period_y));

    // --- Vision

    Vision->processCalibration = ui->PROCESS_CALIBRATION->isChecked();
    Vision->processFish = ui->PROCESS_VISION->isChecked();
    Vision->startCamera();
    setThreshold();

    // === CONNECTIONS =====================================================

    // --- Shortcuts
    connect(sExit, SIGNAL(activated()), QApplication::instance(), SLOT(quit()));

    // --- Menu
    connect(ui->SCAN_CONTROLLER, SIGNAL(triggered(bool)), Motion, SLOT(initFTDI()));
    // connect(ui->SCAN_JOYSTICK, SIGNAL(triggered(bool)), this, SLOT(ScanJoystick()));

    // --- Motion -------------------------------

    // Periods
    connect(Motion, SIGNAL(updatePeriods()), this, SLOT(updatePeriods()));

    // State
    connect(Motion, SIGNAL(updateMotionState()), this, SLOT(updateMotionState()));

    // Displacement pad
    connect(ui->MOVE_UL, SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));
    connect(ui->MOVE_U,  SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));
    connect(ui->MOVE_UR, SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));
    connect(ui->MOVE_L,  SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));
    connect(ui->MOVE_R,  SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));
    connect(ui->MOVE_DL, SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));
    connect(ui->MOVE_D,  SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));
    connect(ui->MOVE_DR, SIGNAL(toggled(bool)), Motion, SLOT(Move(bool)));

    // Reset counts
    connect(ui->RESET_COUNTS, SIGNAL(pressed()), Motion, SLOT(resetCounts()));
    connect(Motion, SIGNAL(updatePosition()), this, SLOT(updatePosition()));

    // Pointer
    connect(ui->POINTER, SIGNAL(toggled(bool)), Motion, SLOT(Pointer(bool)));

    // Mode
    connect(ui->MOTION_MODE, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));

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

    // Updates
    connect(Vision, SIGNAL(updateExposure()), this, SLOT(updateExposure()));
    connect(Vision, SIGNAL(updateFPS()), this, SLOT(updateFPS()));
    connect(Vision, SIGNAL(updateProcessTime()), this, SLOT(updateProcessTime()));
    connect(Vision, SIGNAL(updateDisplay(QVector<UMat>)), this, SLOT(updateDisplay(QVector<UMat>)));
    connect(Vision, SIGNAL(updateCurvature()), this, SLOT(updateCurvature()));
    connect(Vision, SIGNAL(updateProcessStatus(int)), this, SLOT(updateProcessStatus(int)));

    // Calibration
    connect(ui->CALIBRATE, SIGNAL(pressed()), this, SLOT(calibrate()));
    connect(Vision, SIGNAL(updateCalibration()), this, SLOT(updateCalibration()));

    // --- Interface ----------------------------

    connect(Interface, SIGNAL(updateDxy()), this, SLOT(updateDxy()));

}

/* ====================================================================== *
 *      DEVICES MANAGEMENT                                                *
 * ====================================================================== */




/* ====================================================================== *
 *      MOTION                                                            *
 * ====================================================================== */

void MainWindow::updatePeriods() {

    ui->PERIOD_LOOP_X->setNum(int(Motion->period_x));
    ui->PERIOD_LOOP_Y->setNum(int(Motion->period_y));

}

void MainWindow::updateMotionState() {

    if (Motion->motion_state) {
        ui->MOTION_STATE->setText(QString("ENABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background: #abebc6;"));
    } else {
        ui->MOTION_STATE->setText(QString("DISABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background: #ddd;"));
    }

}

void MainWindow::updatePosition() {

    // Update positions
    Motion->pos_x = Motion->count_x*Motion->count2mm_x;
    Motion->pos_y = Motion->count_y*Motion->count2mm_y;

    // Display counts
    ui->COUNT_X->setText(QString("%1").arg(Motion->count_x, 5, 10, QLatin1Char('0')));
    ui->COUNT_Y->setText(QString("%1").arg(Motion->count_y, 5, 10, QLatin1Char('0')));

    // Display positions
    ui->POS_X->setText(QString::number(Motion->pos_x, 'f', 2));
    ui->POS_Y->setText(QString::number(Motion->pos_y, 'f', 2));

}

void MainWindow::modeChanged(int m) {

    Motion->mode = m;

}

/* ====================================================================== *
 *      VISION                                                            *
 * ====================================================================== */

/* --- PARAMETERS ------------------------------------------------------- */

void MainWindow::saveBackground() {

    Vision->save_background = true;

}


void MainWindow::setThreshold() {

    ui->THRESH_BW->setText(QString::number(double(ui->THRESH_BW_SLIDER->value())/1000, 'f', 3));
    Vision->thresholdFish = double(ui->THRESH_BW_SLIDER->value())/1000;

    ui->THRESH_CH->setText(QString::number(double(ui->THRESH_CH_SLIDER->value())/1000, 'f', 3));
    Vision->thresholdCalibration = double(ui->THRESH_CH_SLIDER->value())/1000;

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

void MainWindow::updateDxy() {

    Motion->dx = Vision->dx;
    Motion->dy = Vision->dy;
    ui->DX->setText(QString::number(Vision->dx, 'f', 3));
    ui->DY->setText(QString::number(Vision->dy, 'f', 3));

}

void MainWindow::updateCurvature() {

    /*
    ui->PLOT_CURVATURE->graph(0)->setData(pTime, pCurv);
    ui->PLOT_CURVATURE->xAxis->setRange(pTime.first(), max(pTime.last(), 10.));
    ui->PLOT_CURVATURE->replot();
    */

    /*
    curvatureChart->removeSeries(curvatureSeries);
    curvatureSeries->append(Vision->time/1e9, Vision->fish.curvature);
    curvatureChart->addSeries(curvatureSeries);
    curvatureChartView->update();
    */
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

    ui->PROCESS_TIME->setText(QString("%1").arg(int(double(Vision->processTime)/1e3), 6, 10, QLatin1Char('0')));

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

    Vision->cross_length = ui->CROSS_LENGTH->text().toDouble();
    Vision->calibrate = true;

}

void MainWindow::updateCalibration() {

    ui->PIX2MM->setText(QString::number(Vision->pix2mm, 'f', 3));

}


/* ====================================================================== *
 *      PLOTS                                                             *
 * ====================================================================== */

void MainWindow::initPlots() {

    maxLentghCurv = 2000;
    maxLengthTraj = 500;

    // Create graph
    ui->PLOT_CURVATURE->addGraph();

    // Labels
    ui->PLOT_CURVATURE->xAxis->setLabel("Time (s)");
    ui->PLOT_CURVATURE->yAxis->setLabel("Curvature (1/pix)");

    // Axis ranges
    ui->PLOT_CURVATURE->yAxis->setRange(-0.1, 0.1);

}


/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

MainWindow::~MainWindow() {
    delete ui;
}
