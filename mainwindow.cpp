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

    // qInfo() << THREAD << "Main process lives in thread ";

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
    connect(Motion, SIGNAL(setMotionState(bool)), this, SLOT(setMotionState(bool)));

    // Displacement pad
    // connect(ui->Move_UL, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
    //        connect(ui->Move_U,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
    //        connect(ui->Move_UR, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
    //        connect(ui->Move_L,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
    //        connect(ui->Move_R,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
    //        connect(ui->Move_DL, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
    //        connect(ui->Move_D,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
    //        connect(ui->Move_DR, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));

    // Reset counts
    //        connect(ui->ResetCounts, SIGNAL(pressed()), this, SLOT(resetCounts()));

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

void MainWindow::setMotionState(bool b) {

    if (b) {
        ui->MOTION_STATE->setText(QString("ENABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background:#99ff99;"));
    } else {
        ui->MOTION_STATE->setText(QString("DISABLED"));
        ui->MOTION_STATE->setStyleSheet(QString("background:#ff5050;"));
    }

}


// === INPUTS

/* ====================================================================== *
 *      VISION                                                            *
 * ====================================================================== */



/* ====================================================================== *
 *      DESTRUCTOR                                                        *
 * ====================================================================== */

MainWindow::~MainWindow() {
    delete ui;
}
