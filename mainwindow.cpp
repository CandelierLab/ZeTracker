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
    this->setWindowTitle("Controller");
    move(0,0);

    // --- Shortcuts
    sExit = new QShortcut(Qt::Key_Escape, this);

    // --- Messages
    MsgHandle = new MsgHandler(ui->INFOS);

    //qInfo() << THREAD << "Main process lives in thread " << ;

    // --- Motion
    Motion = new class Motion();

    // --- Vision

    // === INITIALIZATIONS =================================================



    // === CONNECTIONS =====================================================

    // --- Shortcuts
    connect(sExit, SIGNAL(activated()), QApplication::instance(), SLOT(quit()));

    // --- Menu
    // connect(ui->actionScanJoystick, SIGNAL(triggered(bool)), this, SLOT(ScanJoystick()));

    // --- Motion -------------------------------

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
    //        connect(ui->Pointer,  SIGNAL(toggled(bool)), this, SLOT(Pointer(bool)));

    // --- Visions ------------------------------

}

/* ====================================================================== *
 *      DEVICES MANAGEMENT                                                *
 * ====================================================================== */

// void MainWindow::ScanController() { FTDI->ScanDevices(); }

/* ====================================================================== *
 *      MOTION                                                            *
 * ====================================================================== */

void MainWindow::updateMotionState() {

    if (Motion->motion_state) {
        ui->MOTION_STATE->setText(QString("ENABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background:#99ff99;"));
    } else {
        ui->MOTION_STATE->setText(QString("DISABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background:#ff5050;"));
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

/* ====================================================================== *
 *      VISION                                                            *
 * ====================================================================== */



/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

MainWindow::~MainWindow() {
    delete ui;
}
