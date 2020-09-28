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
#include "anglebuffer.h"

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
    qint64 time;

    // Calibration
    double thresholdCalibration;
    double crossLength;
    bool calibrate;
    bool processCalibration;
    double pix2mm;  // mm/pix
    QString calibrationPath;

    // Image processing
    const char *backgroundPath;
    bool saveBackground;
    bool processFish;
    int nEyes;
    int nHead;
    int fishROIRadius;
    int fishROIShift;
    int eyesROIRadius;
    int thresholdFishHead;
    int thresholdFishTail;
    double boutSignal;
    double minimumBoutThreshold;
    long int processTime;
    double dx;
    double dy;

    Fish fish;

    // --- Bout processing
    double thresholdBout;
    long int minBoutDelay;
    int prevBoutBufferMaxSize;
    int bufferDelay;

    void startCamera();
    void stopCamera();
    void resetBoutProcessing();


signals:

    void updateExposure();
    void updateFPS();
    void updateCalibration();
    void updateProcessStatus(int);
    void updateProcessTime();
    void updateDisplay(QVector<UMat>);
    void updateCurvature();
    void updateFish();
    void newBout();

public slots:

    void processFrame(Frame);

private:

    // --- Camera;
    Camera_FLIR *camera;
    QThread *tcam;

    // --- Times
    QElapsedTimer timer;
    qint64 periodDisplay;
    qint64 trefDisplay;
    qint64 trefBout;
    QVector<qint64> timestamps;

    // --- Background
    bool isBackground;
    UMat Background;

    // --- Image processing
    vector<Point> fishpix;
    vector<Point> outline;
    double vx;
    double vy;
    double cmxTorso;
    double cmyTorso;
    double cmxEyes;
    double cmyEyes;

    int getMaxAreaContourId(vector<vector<Point>>);
    Ellipse getEllipse(const UMat&);
    Ellipse getEllipse(const vector<Point>&);
    Moments getContourMoments(const vector<Point>&);
    double angleDifference(double , double );
    void setEllipseAngles();
    void setHeadTail();
    void setCurvature();
    void setCurvatureBodyEllipse();

    // --- Bout processing
    AngleBuffer *PrevBoutBuffer;
    QVector<double> MeanDelayBuffer;
    QVector<double> StdDelayBuffer;
    QVector<int> PositiveSignalBuffer;
    double prevBoutBufferMean;
    double prevBoutBufferStd;

};

#endif // VISION_H
