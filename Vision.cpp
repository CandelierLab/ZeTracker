#include "Vision.h"

// A note on time units
// --------------------
//
// Exposures are in microseconds
// Timestamps are in nanoseconds

Vision::Vision() {

    // Custom types registation
    qRegisterMetaType<Frame>();

    process = true;

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
    width = 256;
    height = 256;

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
    UMat Res(F.img.size(), CV_8UC3, 0);

    if (process) {

        long int tref_process = timer.nsecsElapsed();

        if (is_background) {

            // Threshold
            subtract(Background, F.img, BW);
            cv::threshold(BW, BW, threshold*255, 255, cv::THRESH_BINARY);

            // Get contours
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy;
            findContours(BW, contours,hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

            // Keep only the largest contour
            BW = UMat(BW.size(), CV_8UC1, 0);
            int maxid = getMaxAreaContourId(contours);

            if (contours.size() > maxid) {

                outline = contours.at(maxid);
                drawContours(BW, contours, maxid, Scalar(255), FILLED);
                drawContours(Res, contours, maxid, Scalar(255, 255, 255), FILLED);

                // Compute body ellipse
                fish.body = getEllipse(BW);

                // Set the angle to point toward the head
                setHeadAngle();

                // Get the head and tail ellipses
                setHeadTail();

                // Get the head and tail ellipses
                setCurvature();

                // contours.at(maxid) = outline;
                // drawContours(Res, contours, maxid, Scalar(255, 255, 255));

                // Get (dx,dy)
                dx = fish.body.x - width/2;
                dy = height/2 - fish.body.y;
                emit updateDxy();

            }
        }

        process_time = timer.nsecsElapsed() - tref_process;

    }

    // --- Update display --------------------------------------------------

    if (timer.nsecsElapsed() >= tref_display + period_display) {

        QVector<UMat> display;
        display.push_back(F.img);

        if (process && is_background) {

            // Display ellipse
            if (!isnan(fish.body.x)) {
                // ellipse(Res, Point(fish.body.x,fish.body.y), Size(fish.body.major_length, fish.body.minor_length), -fish.body.theta*180/M_PI, 0, 360, Scalar(255,0,0), 1);
                // circle(Res, Point(fish.body.x+fish.body.major_length*cos(fish.body.theta), fish.body.y-fish.body.major_length*sin(fish.body.theta)), 2, Scalar(0,255,255));

                ellipse(Res, Point(fish.head.x,fish.head.y), Size(fish.head.major_length, fish.head.minor_length), fish.head.theta*180/M_PI, 0, 360, Scalar(255,0,255), 1);
                ellipse(Res, Point(fish.tail.x,fish.tail.y), Size(fish.tail.major_length, fish.tail.minor_length), fish.tail.theta*180/M_PI, 0, 360, Scalar(0,255,0), 1);

                circle(Res, Point(fish.xc, fish.yc), 2, Scalar(255,0,0), FILLED);
                circle(Res, Point(fish.xc, fish.yc), abs(1/fish.curvature), Scalar(255,0,0));
            }

            display.push_back(Res);

        }

        emit updateDisplay(display);
        emit updateProcessTime();
        tref_display += period_display;
    }

}

/* ====================================================================== *
 *      IMAGE PROCESSING                                                  *
 * ====================================================================== */

int Vision::getMaxAreaContourId(vector<vector<Point>> contours) {

    double maxArea = 0;
    int maxAreaContourId = -1;
    for (unsigned long j = 0; j < contours.size(); j++) {
        double newArea = contourArea(contours.at(j));
        if (newArea > maxArea) {
            maxArea = newArea;
            maxAreaContourId = int(j);
        }
    }
    return maxAreaContourId;
}

Ellipse Vision::getEllipse(const UMat &Img) {

    Ellipse E;

    Moments moment = moments(Img);
    E.x = moment.m10/moment.m00;
    E.y = moment.m01/moment.m00;

    E.theta = atan((2*moment.mu11)/(moment.mu20-moment.mu02))/2 + (moment.mu20<moment.mu02)*(M_PI/2);
    // E.theta += 2*M_PI*(E.theta<0);

    E.major_length = 2*pow( (((moment.mu20 + moment.mu02) + pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);
    E.minor_length = 2*pow( (((moment.mu20 + moment.mu02) - pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);

    return E;

}

void Vision::setHeadAngle() {

    // Check
    if (!outline.size()) { return; }

    // Compute skewness-like estimator
    double x, y, sk = 0;
    for (unsigned int i=0 ; i<outline.size() ; i++) {
        x = (outline.at(i).x-fish.body.x)*cos(fish.body.theta)  + (outline.at(i).y-fish.body.y)*sin(fish.body.theta);
        y = (outline.at(i).x-fish.body.x)*sin(-fish.body.theta) + (outline.at(i).y-fish.body.y)*cos(fish.body.theta);
        sk += pow(x*abs(y), 3);
    }

    // Angle correction
    fish.body.theta = M_PI - fish.body.theta + (sk>0 ? M_PI : 0);
    fish.body.theta -= fish.body.theta>2*M_PI ? 2*M_PI : 0;

    // qDebug() << fish.body.theta*180/M_PI;

}

void Vision::setHeadTail() {

    // Buffer image size
    int s = 64;

    // --- Define head and tail contours

    vector<Point> chead;
    vector<Point> ctail;

    // Minor axis
    double vx = cos(fish.body.theta);
    double vy = sin(fish.body.theta);
    Point p;

    // Compute cross product
    for (unsigned int i=0 ; i<outline.size() ; i++) {
        p.x = outline.at(i).x - fish.body.x + s/2;
        p.y = outline.at(i).y - fish.body.y + s/2;
        if ((outline.at(i).x - fish.body.x)*vx - (outline.at(i).y - fish.body.y)*vy > 0) { chead.push_back(p); }
        else { ctail.push_back(p); }
    }

    // --- Get head and tail ellipses

    vector<vector<Point>> V;
    V.push_back(chead);
    V.push_back(ctail);

    UMat Ihead(s, s, CV_8UC1, Scalar(0));
    drawContours(Ihead, V, 0, Scalar(255), FILLED);
    fish.head = getEllipse(Ihead);
    fish.head.x += fish.body.x - s/2;
    fish.head.y += fish.body.y - s/2;

    UMat Itail(s, s, CV_8UC1, Scalar(0));
    drawContours(Itail, V, 1, Scalar(255), FILLED);
    fish.tail = getEllipse(Itail);
    fish.tail.x += fish.body.x - s/2;
    fish.tail.y += fish.body.y - s/2;

}


void Vision::setCurvature() {

    double hx = sin(fish.head.theta);
    double hy = -cos(fish.head.theta);
    double tx = sin(fish.tail.theta);
    double ty = -cos(fish.tail.theta);

    fish.curvature = (hx*ty - hy*tx)/((fish.head.y - fish.tail.y)*tx - (fish.head.x - fish.tail.x)*ty);
    fish.xc = fish.head.x + hx/fish.curvature;
    fish.yc = fish.head.y + hy/fish.curvature;

}
