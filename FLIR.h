#ifndef CAMERA_FLIR_H
#define CAMERA_FLIR_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QDebug>
#include <math.h>

#include <QElapsedTimer>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "Vision.h"
#include "MsgHandler.h"
#include "opencv2/opencv.hpp"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace cv;
using namespace std;

struct Frame {
    qint64 frameId;
    qint64 timestamp;
    UMat img;
};

Q_DECLARE_METATYPE(Frame)

/* =================================================================== *\
|    LowLevel_FLIR Class                                                |
\* =================================================================== */

class Camera_FLIR : public QObject {

    Q_OBJECT

public:

    // Constructor and destructor
    Camera_FLIR(class Vision*);
    ~Camera_FLIR();

    // Camera parameters
    QString camName;
    double frameRate;
    int64_t offsetX;
    int64_t offsetY;
    int width;
    int height;

    bool grabState;

public slots:

    void sendInfo();
    void grab();

signals:

    void newFrame(Frame);

private:

    class Vision *Vision;

    // Internal FLIR properties
    SystemPtr FLIR_system;
    CameraList FLIR_camList;
    CameraPtr pCam;

    QElapsedTimer timer;

};

#endif
