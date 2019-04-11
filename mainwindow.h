#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QStringListModel>
#include <QItemSelection>
#include <QShortcut>
#include <QDebug>

#include "MsgHandler.h"
#include "Motion.h"
#include "Vision.h"
#include "Interface.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Plots
    int maxLentghCurv;
    int maxLengthTraj;
    QVector<double> pTime, pCurv, pX, pY;

signals:

public slots:

    // --- Motion

    void updatePeriods();    
    void updateMotionState();
    void updatePosition();
    void modeChanged(int);

    // --- Vision

    void updateExposure();
    void updateFPS();
    void updateDisplay(QVector<UMat>);
    void updateDxy();
    void updateCurvature();

    // --- Image processing

    void saveBackground();
    void calibrate();

    void setThreshold();
    void processCalibration(int);
    void updateCalibration();
    void processFrames(int);
    void updateProcessTime();
    void updateProcessStatus(int);

private:

    // --- GUI
    Ui::MainWindow *ui;
    QShortcut *sExit;
    MsgHandler *MsgHandle;

    // --- Plots
    void initPlots();

    class Motion *Motion;
    class Vision *Vision;
    class Interface *Interface;

};

#endif // MAINWINDOW_H
