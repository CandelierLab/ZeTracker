#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMainWindow>
#include <QString>
#include <QDateTime>
#include <QDir>
#include <QDebug>

#include "MsgHandler.h"
#include "Motion.h"
#include "Vision.h"

// Forward declaration
class MainWindow;

struct Bout {
    int i;
    qint64 t;
    double x;
    double y;
};

class Interface : public QWidget {

    Q_OBJECT

public:

    explicit Interface(MainWindow*, class Motion*, class Vision*);

    QString version;

    // --- Fish
    double pos_x, pos_y;        // Absolute positions ( = camera position + relative position)
    Bout lastBout;

    // --- Runs
    QString dataRoot;
    QString runPath;

    QString logFile;
    QString parametersFile;
    QString trajectoryFile;
    QString boutsFile;
    QString backgroundFile;

signals:

    void updatePosition();
    void updateBouts();

public slots:

    // --- Fish
    void updateFish();
    void newBout();

    // --- Runs
    void newRun();
    void setRun(bool);



private:

    MainWindow *Main;
    class Motion *Motion;
    class Vision *Vision;

    QFile *trajectoryFid;
    QFile *boutsFid;


};

#endif // INTERFACE_H
