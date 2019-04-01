#include "mainwindow.h"
#include "ui_mainwindow.h"

/* ====================================================================== *
 *      CONSTRUCTOR                                                       *
 * ====================================================================== */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    // --- User interface -----------------------

        ui->setupUi(this);
        this->setWindowTitle("Controller");
        move(0,0);


        // --- Vision -------------------------------


        // --- Shortcuts ----------------------------

        sExit = new QShortcut(Qt::Key_Escape, this);

        // --- Connections --------------------------

        // Shortcuts
         connect(sExit, SIGNAL(activated()), QApplication::instance(), SLOT(quit()));

        // Menu
        // connect(ui->actionScanJoystick, SIGNAL(triggered(bool)), this, SLOT(ScanJoystick()));

        // Displacement pad
        // connect(ui->Move_UL, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
//        connect(ui->Move_U,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
//        connect(ui->Move_UR, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
//        connect(ui->Move_L,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
//        connect(ui->Move_R,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
//        connect(ui->Move_DL, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
//        connect(ui->Move_D,  SIGNAL(toggled(bool)), this, SLOT(Move(bool)));
//        connect(ui->Move_DR, SIGNAL(toggled(bool)), this, SLOT(Move(bool)));

//        connect(ui->ResetCounts, SIGNAL(pressed()), this, SLOT(resetCounts()));

        // Pointer
//        connect(ui->Pointer,  SIGNAL(toggled(bool)), this, SLOT(Pointer(bool)));

        // --- Camera

        // --- Scan controller -------------------------------------------------
}

/* ====================================================================== *
 *      DEVICES MANAGEMENT                                                *
 * ====================================================================== */

// void MainWindow::ScanController() { FTDI->ScanDevices(); }

/* ====================================================================== *
 *      MOTION                                                            *
 * ====================================================================== */

// === POINTER

//void MainWindow::Pointer(bool b) {

//    FTDI->Dev[FTDI->cdi]->setPin(6, b);
//    FTDI->Dev[FTDI->cdi]->sendOutput();

//}

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
