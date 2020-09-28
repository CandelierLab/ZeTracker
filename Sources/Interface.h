#ifndef INTERFACE_H
#define INTERFACE_H

#define PROTOCOL_UNIFORM            0
#define PROTOCOL_SPLIT              1
#define PROTOCOL_UNIFORM_RELATIVE   2


#include <QMainWindow>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QDateTime>
#include <QDir>
#include <QTcpSocket>
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
    QString imagesPath;


    // --- Connection
    int protocol;
    bool connectionToPyBeautySuccesful;
    bool sendPositions;
    bool sendBouts;

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
    void saveDisplay(QVector<UMat>);

    // --- Connection
    void manageConnection(bool);
    void sendRunInfo();
    void respondToConnectionConfirmation();


private:

    MainWindow *Main;
    class Motion *Motion;
    class Vision *Vision;

    QTcpSocket *conn = nullptr;

    QFile *trajectoryFid;
    QFile *boutsFid;


};

#endif // INTERFACE_H
