#ifndef VISION_H
#define VISION_H

#include <QElapsedTimer>

#include "FLIR.h"

// Forward declaration
class Camera_FLIR;
struct Frame;

/* #############################################################################################
   #                                                                                           #
   #                                        MOTION                                             #
   #                                                                                           #
   ############################################################################################# */

class Vision  : public QObject {

    Q_OBJECT

public:

    Vision();
    ~Vision();

    int exposure;
    double fps;

    void startCamera();
    void stopCamera();

public slots:

    void processFrame(Frame);

signals:

    void updateExposure();
    void updateFPS();
    void newDisplay(Frame);

private:

    // --- Camera;
    Camera_FLIR *camera;
    QThread *tcam;

    QElapsedTimer timer;
    qint64 period_display;
    qint64 tref_display;

    QVector<qint64> timestamps;


};

#endif // VISION_H
