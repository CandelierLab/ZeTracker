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
// #include "Interface.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:

public slots:

    // --- Motion
    void updatePeriods();
    void updateMotionState();
    void updatePosition();

    // --- Vision
    void updateExposure();
    void updateFPS();
    void updateProcessTime();
    void updateDisplay(QVector<UMat>);

    void processFrames(int);
    void saveBackground();
    void setThreshold();


private:

    // --- GUI
    Ui::MainWindow *ui;
    QShortcut *sExit;
    MsgHandler *MsgHandle;

    class Motion *Motion;
    class Vision *Vision;

};

#endif // MAINWINDOW_H
