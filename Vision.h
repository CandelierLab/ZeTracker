#ifndef VISION_H
#define VISION_H

#include <QFileInfo>
#include <QElapsedTimer>

#include "opencv2/opencv.hpp"
#include "FLIR.h"

using namespace cv;

// Forward declaration
class Camera_FLIR;
struct Frame;

struct Ellipse {
    double x;
    double y;
    double theta;
    double major_length;
    double minor_length;
};

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
    int width;
    int height;

    bool process;
    long int process_time;
    bool save_background;
    double threshold;

    void startCamera();
    void stopCamera();

public slots:

    void processFrame(Frame);

signals:

    void updateExposure();
    void updateFPS();
    void updateProcessTime();
    void updateDisplay(QVector<UMat>);

private:

    // --- Camera;
    Camera_FLIR *camera;
    QThread *tcam;

    // --- Times
    QElapsedTimer timer;
    qint64 period_display;
    qint64 tref_display;
    QVector<qint64> timestamps;

    // --- Background
    const char *background_path;
    bool is_background;
    UMat Background;

    // --- Image processing
    Ellipse getEllipse(const UMat&);

};

#endif // VISION_H
