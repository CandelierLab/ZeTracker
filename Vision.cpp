#include "Vision.h"

// A note on time units
// --------------------
//
// Exposures are in microseconds
// Timestamps are in nanoseconds

Vision::Vision() {

    // Custom types registation
    qRegisterMetaType<Frame>();

    // Times
    exposure = 5000;
    period_display = 40e6;   // 25Hz

    timer.start();
    tref_display = 0;

    // Background
    background_path = "Background.pgm";
    save_background = false;

    QFileInfo bkg(background_path);
    if (bkg.exists() && bkg.isFile()) {
        Background = imread(background_path, IMREAD_GRAYSCALE).getUMat(ACCESS_READ);
        is_background = true;
    } else { is_background = false; }

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
    connect(camera, SIGNAL(newFrame(Frame)), this, SLOT(processFrame(Frame)));
    connect(tcam, &QThread::finished, camera, &QObject::deleteLater);

    // Start the camera
    tcam->start();
    tcam->setPriority(QThread::HighestPriority);
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

    long int tref_process = timer.nsecsElapsed();

    // --- Compute FPS -----------------------------------------------------

    timestamps.push_back(F.timestamp);
    if (timestamps.size()>100) { timestamps.pop_front(); }
    double dt = 0;
    for (int i=0; i<timestamps.size()-1; i++) { dt += timestamps[i+1] - timestamps[i]; }
    fps = timestamps.size()/dt*1e9;
    emit updateFPS();

    // --- Save background -------------------------------------------------

    if (save_background) {

        // Update backround
        Background = F.img.clone();
        is_background = true;

        // Update file
        imwrite(background_path, F.img);

        qInfo() << "Background saved";
        save_background = false;

    }

    // --- Process ---------------------------------------------------------

    UMat BW;

    if (is_background) {

        absdiff(F.img, Background, BW);
        // cv::threshold(BW, BW, threshold*255, 255, cv::THRESH_BINARY);

        // Close object
        //morphologyEx(BW, BW, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(10,10)));

    }

    process_time = timer.nsecsElapsed() - tref_process;

    // --- Update display --------------------------------------------------

    if (timer.nsecsElapsed() >= tref_display + period_display) {

        QVector<UMat> display;
        display.push_back(F.img);
/*
        if (is_background) {

            // Compute ellipse
            Ellipse E = getEllipse(BW);

            // Display ellipse
            UMat Res(BW.size(), CV_8UC3);
            cvtColor(BW, Res, COLOR_GRAY2RGB);
            ellipse(Res, Point(E.x,E.y), Size(E.major_length, E.minor_length), E.theta*180/M_PI, 0, 360, Scalar(255,0,0), 2);

            display.push_back(Res);

        }
/**/
        emit updateDisplay(display);
        emit updateProcessTime();
        tref_display += period_display;
    }

/**/
}

/* ====================================================================== *
 *      IMAGE PROCESSING                                                  *
 * ====================================================================== */

Ellipse Vision::getEllipse(const UMat &Img) {

    Ellipse E;

    Moments moment = moments(Img);
    E.x = moment.m10/moment.m00;
    E.y = moment.m01/moment.m00;

    double i = moment.mu20;
    double j = moment.mu11;
    double k = moment.mu02;

    E.theta = atan((2*moment.mu11)/(moment.mu20-moment.mu02))/2 + (moment.mu20<moment.mu02)*(M_PI/2);
    E.theta += 2*M_PI*(E.theta<0);

    E.major_length = 2*pow( (((moment.mu20 + moment.mu02) + pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);
    E.minor_length = 2*pow( (((moment.mu20 + moment.mu02) - pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);

    return E;

}
