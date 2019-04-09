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

    // --- Motion
    Motion = new class Motion();

    ui->TARGET_LOOP_X->setText(QString::number(Motion->loop_period_x));
    ui->TARGET_LOOP_Y->setText(QString::number(Motion->loop_period_y));

    // --- Vision
    Vision = new class Vision();

    // === INITIALIZATIONS =================================================

    // --- Vision

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
    connect(ui->PROCESS, SIGNAL(stateChanged(int)), this, SLOT(processFrames(int)));

    // Background
    connect(ui->BACKGROUND, SIGNAL(pressed()), this, SLOT(saveBackground()));

    // Threshold
    connect(ui->THRESHOLD_SLIDER, SIGNAL(valueChanged(int)), this, SLOT(setThreshold()));

    // Updates
    connect(Vision, SIGNAL(updateExposure()), this, SLOT(updateExposure()));
    connect(Vision, SIGNAL(updateFPS()), this, SLOT(updateFPS()));
    connect(Vision, SIGNAL(updateProcessTime()), this, SLOT(updateProcessTime()));
    connect(Vision, SIGNAL(updateDisplay(QVector<UMat>)), this, SLOT(updateDisplay(QVector<UMat>)));
    connect(Vision, SIGNAL(updateDxy()), this, SLOT(updateDxy()));

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

void MainWindow::updateExposure() {

    ui->EXPOSURE->setText(QString("%1").arg(double(Vision->exposure)/1000));

}

void MainWindow::updateFPS() {

    statusBar()->showMessage(QString::number(Vision->fps, 'f', 2) + QString(" FPS"));

}

void MainWindow::updateProcessTime() {

    ui->PROCESS_TIME->setText(QString("%1").arg(int(double(Vision->process_time)/1e3), 6, 10, QLatin1Char('0')));

}

void MainWindow::updateDisplay(QVector<UMat> D) {

    // --- IMAGE 1

    if (D.size()>0) {

        QImage Img1((uchar*)D.at(0).getMat(ACCESS_READ).data, D.at(0).cols, D.at(0).rows, QImage::Format_Indexed8);
        for (int i=0 ; i<=255; i++) { Img1.setColor(i, qRgb(i,i,i)); }
        QPixmap pix1 = QPixmap::fromImage(Img1);
        ui->IMAGE_1->setPixmap(pix1.scaled(ui->IMAGE_1->width(), ui->IMAGE_1->height(), Qt::KeepAspectRatio));

    }

    // --- IMAGE 2

    if (D.size()>1) {

        QImage Img2((uchar*)D.at(1).getMat(ACCESS_READ).data, D.at(1).cols, D.at(1).rows, QImage::Format_RGB888);
        // for (int i=0 ; i<=255; i++) { Img2.setColor(i, qRgb(i,i,i)); }
        QPixmap pix2 = QPixmap::fromImage(Img2);
        ui->IMAGE_2->setPixmap(pix2.scaled(ui->IMAGE_2->width(), ui->IMAGE_2->height(), Qt::KeepAspectRatio));

    }

}

void MainWindow::updateDxy() {

    Motion->dx = Vision->dx;
    Motion->dy = Vision->dy;
    ui->DX->setText(QString::number(Vision->dx, 'f', 3));
    ui->DY->setText(QString::number(Vision->dy, 'f', 3));

}


void MainWindow::processFrames(int b) {

    Vision->process = b>0;

}

void MainWindow::saveBackground() {

    Vision->save_background = true;

}


void MainWindow::setThreshold() {

    ui->THRESHOLD->setText(QString::number(double(ui->THRESHOLD_SLIDER->value())/1000, 'f', 3));
    Vision->threshold = double(ui->THRESHOLD_SLIDER->value())/1000;

}

/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

MainWindow::~MainWindow() {
    delete ui;
}
