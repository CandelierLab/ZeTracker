#include "Vision.h"

// A note on time units
// --------------------
//
// Exposures are in microseconds
// Timestamps are in nanoseconds

Vision::Vision() {

    // Custom types registation
    qRegisterMetaType<Frame>();

    exposure = 5000;

    period_display = 40e6;   // 25Hz

    timer.start();
    tref_display = 0;

}

Vision::~Vision() { stopCamera(); }

/* ====================================================================== *
 *      CAMERA                                                            *
 * ====================================================================== */

void Vision::startCamera() {

    qInfo() << TITLE_2 << "Camera";

    camera = new Camera_FLIR(this);

    camera->sendInfo();
    camera->frameRate = fps;
    camera->offsetX = 0;
    camera->offsetY = 0;
    camera->width = 512;
    camera->height = 512;

    // Change camera thread
    tcam = new QThread;
    camera->moveToThread(tcam);

    // Connections
    connect(tcam, SIGNAL(started()), camera, SLOT(grab()));
    connect(camera, SIGNAL(sendExposure(double)), this, SIGNAL(sendExposure(double)));
    connect(camera, SIGNAL(newFrame(Frame)), this, SLOT(processFrame(Frame)));
    connect(tcam, &QThread::finished, camera, &QObject::deleteLater);

    // Start the camera
    tcam->start();

}

void Vision::stopCamera() {

    // NOT WORKING

    camera->grabState = false;
    tcam->quit();
    tcam->wait();
    delete camera;

}

/* ====================================================================== *
 *      FRAMES                                                            *
 * ====================================================================== */

void Vision::processFrame(Frame F) {

    // --- Compute FPS -----------------------------------------------------

    timestamps.push_back(F.timestamp);
    if (timestamps.size()>100) { timestamps.pop_front(); }
    double dt = 0;
    for(int i=0; i<timestamps.size()-1; i++) { dt += timestamps[i+1] - timestamps[i]; }
    emit updateFPS();
    // timestamps.size()/dt*1e9);

    // --- Process ---------------------------------------------------------

    // double m = mean(F.img)[0];

    // threshold(F.img, F.img, 60, 255, THRESH_BINARY);

    // adaptiveThreshold(F.img, F.img, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 21, 4);

    // --- Update display --------------------------------------------------

    if (timer.nsecsElapsed() >= tref_display + period_display) {
        emit newDisplay(F);
        tref_display += period_display;
    }


}
