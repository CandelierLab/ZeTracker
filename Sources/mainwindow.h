#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define META_LOG        0
#define META_PARAMETERS 1
#define META_TRAJECTORY 2
#define META_BOUTS      3
#define META_BACKGROUND 4
#define META_IMAGES     5

#include <QMainWindow>
#include <QStringList>
#include <QStringListModel>
#include <QItemSelection>
#include <QShortcut>
#include <QUrl>
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

    // --- Plots
    int maxLengthCurv;
    int maxLengthTraj;
    QVector<double> pTime, pCurv, pX, pY;

    // --- Runs
    bool isRunning;
    QElapsedTimer *runTimer;
    bool metaLog;
    bool metaParameters;
    bool metaTrajectory;
    bool metaBouts;
    bool metaBackground;
    bool metaImages;

signals:

public slots:

    // --- Motion

    void newPeriods();
    void updatePeriods();
    void updateMotionState();
    void updatePosition();
    void modeChanged(int);
    void moveFixed();
    void switchTriggeredSetManual();

    // --- Vision

    void updateExposure();
    void updateFPS();
    void updateDisplay(QVector<UMat>);

    // --- Calibration

    void calibrate();
    void processCalibration(int);
    void updateCalibration();

    // --- Image processing

    void saveBackground();
    void setNumPix();
    void setROIParameters();
    void setThreshold();
    void setBoutThreshold();
    void setMinBoutDelay();
    void processFrames(int);
    void updateProcessTime();
    void updateProcessStatus(int);

    // --- Plots
    void updateCurvature();
    void updateBouts();

    // --- Run
    void setMetas();
    void updateMeta(int, bool);
    void updateRunPath();
    void openFolder();
    void setRun(bool);
    void updateRunTime();

    // --- Interface
    void setConnected();
    void setDisconnected();
    void protocolChanged(int);
    void setData2send();

private:

    // --- GUI
    Ui::MainWindow *ui;
    QShortcut *sExit;
    MsgHandler *MsgHandle;

    // --- Objects
    class Motion *Motion;
    class Vision *Vision;
    class Interface *Interface;

    // --- Positions
    int posBufferSize;
    QVector<double> dx;
    QVector<double> dy;

    // --- Plots
    void initPlots();

    // --- Runs
    QTimer *runClock;

};

#endif // MAINWINDOW_H
