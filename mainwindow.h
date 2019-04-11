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
    int maxLengthCurv;
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

    // --- Calibration

    void calibrate();
    void processCalibration(int);
    void updateCalibration();

    // --- Image processing

    void saveBackground();
    void setThreshold();
    void setMinBoutDelay();
    void processFrames(int);
    void updateProcessTime();
    void updateProcessStatus(int);

    void triggerBout();

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
