#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QStringListModel>
#include <QItemSelection>
#include <QShortcut>
#include <QDebug>

// #include "MsgHandler.h"
#include "Motion.h"
// #include "Vision.h"
// #include "Interface.h"


namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    // --- GUI
    Ui::MainWindow *ui;
    QShortcut *sExit;


};

#endif // MAINWINDOW_H
