#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMainWindow>
#include <QDebug>

#include "MsgHandler.h"
#include "Motion.h"
#include "Vision.h"

// Forward declaration
class MainWindow;

class Interface : public QWidget {

    Q_OBJECT

public:

    explicit Interface(MainWindow*, class Motion*, class Vision*);

signals:

    void updateDxy();

public slots:

    void updateFish();

private:

    MainWindow *Main;
    class Motion *Motion;
    class Vision *Vision;



};

#endif // INTERFACE_H
