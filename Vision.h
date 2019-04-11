#ifndef VISION_H
#define VISION_H

#define PROCESS_INACTIVE -1
#define PROCESS_NOBACK   0
#define PROCESS_OK       1
#define PROCESS_FAILED   2

#include <QFileInfo>
#include <QElapsedTimer>

#include "opencv2/opencv.hpp"
#include "FLIR.h"

using namespace cv;
using namespace std;

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

struct Fish {
    Ellipse body;
    Ellipse head;
    Ellipse tail;
    double xc;
    double yc;
    double curvature;
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

    // Camera
    int exposure;
    double fps;
    int width;
    int height;

    // Time
    long int frame;
    double time;

    // Image processing
    bool save_background;
    bool calibrate;
    double cross_length;
    bool processCalibration, processFish;
    double thresholdCalibration;
    double thresholdFish;
    long int processTime;
    double pix2mm;
    double dx;
    double dy;
    Fish fish;

    void startCamera();
    void stopCamera();

public slots:

    void processFrame(Frame);

signals:

    void updateExposure();
    void updateFPS();
    void updateCalibration();
    void updateProcessStatus(int);
    void updateProcessTime();
    void updateDisplay(QVector<UMat>);
    void updateCurvature();
    void updateFish();

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
    vector<Point> outline;
    int getMaxAreaContourId(vector<vector<Point>>);
    Ellipse getEllipse(const UMat&);
    void setHeadAngle();
    void setHeadTail();
    void setCurvature();

};

#endif // VISION_H
