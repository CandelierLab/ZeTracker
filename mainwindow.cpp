#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);
    move(0,0);
    statusBar()->showMessage(QString("- FPS"));

}

MainWindow::~MainWindow() {
    delete ui;
}
