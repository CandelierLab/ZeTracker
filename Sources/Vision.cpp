#include "Vision.h"

// A note on time units
// --------------------
//
// Exposures are in microseconds
// Timestamps are in nanoseconds

Vision::Vision() {

    // Custom types registation
    qRegisterMetaType<Frame>();

    // --- Times

    frame = 0;
    time = 0;

    exposure = 5000;
    periodDisplay = 40e6;   // 25Hz

    timer.start();
    trefDisplay = 0;
    trefBout = 0;

    // --- Background

    saveBackground = false;
    backgroundPath = "Background.pgm";
    QFileInfo bkg(backgroundPath);
    if (bkg.exists() && bkg.isFile()) {
        Background = imread(backgroundPath, IMREAD_GRAYSCALE).getUMat(ACCESS_READ);
        isBackground = true;
    } else { isBackground = false; }

    // --- Calibration

    calibrate = false;
    pix2mm = 1;
    calibrationPath = QString("Calibration.txt");

    // Load existing calibration file
    QFileInfo cal(calibrationPath);
    if (cal.exists() && cal.isFile()) {
        QFile file(calibrationPath);
        if (file.open(QIODevice::ReadWrite)) {
            QTextStream F(&file);
            pix2mm = F.readLine().toDouble();
        }
        file.close();
    }

    // --- Bout processing
    minimumBoutThreshold = 0.04;
    bufferDelay = 16;
    prevBoutBufferMaxSize = 45;
    PrevBoutBuffer = new AngleBuffer(prevBoutBufferMaxSize);

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

    // --- Times -----------------------------------------------------------

    frame++;
    time = F.timestamp;

    // --- Compute FPS -----------------------------------------------------

    timestamps.push_back(F.timestamp);
    if (timestamps.size()>100) { timestamps.pop_front(); }
    double dt = 0;
    for (int i=0; i<timestamps.size()-1; i++) { dt += timestamps[i+1] - timestamps[i]; }
    fps = timestamps.size()/dt*1e9;
    emit updateFPS();

    // --- Save background -------------------------------------------------

    if (saveBackground) {

        // Update backround
        Background = F.img.clone();
        isBackground = true;

        // Update file
        imwrite(backgroundPath, F.img);

        qInfo() << "Background saved";
        saveBackground = false;

    }

    // --- Process ---------------------------------------------------------

    bool process_OK = (processCalibration || processFish) && isBackground;

    UMat BW(F.img.size(), CV_8UC1, 0);
    UMat BkgMImg(F.img.size(), CV_8UC1, 0); // Bkg Minus Img
    UMat BWAbsDiff(F.img.size(), CV_8UC1, 0);
    UMat BWHead(F.img.size(), CV_8UC1, 0);
    // UMat BWTail(F.img.size(), CV_8UC1, 0);
    UMat BWMasked(F.img.size(), CV_8UC1, 0);
    UMat CircularMaskEyes(F.img.size(), CV_8UC1, 0);
    UMat CircularMaskFish(F.img.size(), CV_8UC1, 0);
    UMat Res(F.img.size(), CV_8UC3, 0);

    // --- Calibration

    if (processCalibration) {

        long int tref_process = timer.nsecsElapsed();

        if (isBackground) {

            // Threshold
            subtract(Background, F.img, BW);
            cv::threshold(BW, BW, thresholdCalibration*255, 255, cv::THRESH_BINARY);

            // Get contours
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy;
            findContours(BW, contours,hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

            // Keep only the largest contour
            BW = UMat(BW.size(), CV_8UC1, 0);
            int maxid = getMaxAreaContourId(contours);

            if (contours.size() > maxid) {

                outline = contours.at(maxid);
                drawContours(Res, contours, maxid, Scalar(255, 255, 255), FILLED);

                if (calibrate) {

                    RotatedRect R =  minAreaRect(outline);
                    Point2f r[4];
                    R.points(r);
                    pix2mm = crossLength*2/(sqrt(pow(r[3].x-r[1].x,2) + pow(r[3].y-r[1].y,2)) +
                            sqrt(pow(r[0].x-r[2].x,2) + pow(r[0].y-r[2].y,2)));

                    emit updateCalibration();
                    calibrate = false;
                }

            } else { process_OK = false; }

        } else { process_OK = false; }

        processTime = timer.nsecsElapsed() - tref_process;

    }

    // --- Fish
    if (processFish) {

        long int tref_process = timer.nsecsElapsed();

        if (isBackground) {
            // Threshold
            subtract(Background, F.img, BkgMImg);
            // subtract(F.img, Background, BW);
            absdiff(Background, F.img, BWAbsDiff);

            // Apply thresholds
            // cv::threshold(BWAbsDiff, BWTail, thresholdFishTail, 255, cv::THRESH_BINARY);
            cv::threshold(BWAbsDiff, BWAbsDiff, thresholdFishHead, 255, cv::THRESH_BINARY);

            // Identify head points above the threshold
            findNonZero(BWAbsDiff, fishpix);

            if (fishpix.size() > 0) {
                // Get bkg-img values at points above threshold_absdiff
                vector<unsigned int> fishval(fishpix.size());
                // Convert for access to values
                Mat BkgMImgMat = BkgMImg.getMat(ACCESS_READ);
                for (unsigned int i=0; i<fishpix.size(); i++) {
                    fishval[i] = BkgMImgMat.at<uchar>(fishpix[i].y, fishpix[i].x);
                }

                // Sort in decreasing order
                sort(fishval.begin(), fishval.end(), greater<unsigned int>());
                unsigned int threshold_eyes = fishval[nEyes-1];
                unsigned int threshold_torso = fishval[nHead-1];

                // Find eyes
                threshold(BkgMImg, BW, threshold_eyes, 255, THRESH_BINARY);
                Moments momentEyes = moments(BW);
                cmxEyes = momentEyes.m10/momentEyes.m00;
                cmyEyes = momentEyes.m01/momentEyes.m00;

                CircularMaskEyes = UMat(CircularMaskEyes.size(), CV_8UC1, 0);
                circle(CircularMaskEyes, Point(cmxEyes, cmyEyes), eyesROIRadius, 255, FILLED);

                // Find torso
                threshold(BkgMImg, BW, threshold_torso, 255, THRESH_BINARY);
                // Crop to the neighbourhood of eyes
                BW.copyTo(BWHead, CircularMaskEyes);

                Moments momentTorso = moments(BWHead);
                cmxTorso = momentTorso.m10/momentTorso.m00;
                cmyTorso = momentTorso.m01/momentTorso.m00;

                // Define circle ROI of fish and 'crop'
                // Torso --> Eyes orientation vector
                vx = (cmxEyes - cmxTorso);
                vy = (cmyEyes - cmyTorso);
                double absv = sqrt(vx*vx + vy*vy);
                vx /= absv;
                vy /= absv;

                Point circleCenter(cmxEyes - vx*fishROIShift, cmyEyes - vy*fishROIShift);
                CircularMaskFish = UMat(CircularMaskFish.size(), CV_8UC1, 0);
                circle(CircularMaskFish, circleCenter, fishROIRadius, 255, FILLED);

                BW = UMat(CircularMaskFish.size(), CV_8UC1, 0);
                BWAbsDiff.copyTo(BW, CircularMaskFish);

                // Compute body ellipse - in principle not necessary
                fish.body = getEllipse(BW);

                // Restrict to head pixels
                BWHead = UMat(CircularMaskEyes.size(), CV_8UC1, 0);
                BW.copyTo(BWHead, CircularMaskEyes);

                // Get head ellipse
                fish.head = getEllipse(BWHead);

                // Set angles to point to the front (defined by torso=>eyes)
                setEllipseAngles();

                // Get the curvature
                // setCurvature();

                // Update position !!! USING HEAD !!!
                dx = (fish.head.x - width/2);
                dy = (height/2 - fish.head.y);

                // --- Update the RGB Res -----------------------------------

                cvtColor(BWHead, Res, COLOR_GRAY2RGB);
                //circle(Res, circleCenter, ROIRadius, Scalar(0,0,255), 1);
                //circle(Res, Point(cmxEyes, cmyEyes), eyeROIRadius, Scalar(0,0,155), 1);
                circle(Res, circleCenter, fishROIRadius, Scalar(0,255,255), 1);

                // --- Update bouts ----------------------------------------------------

                if (MeanDelayBuffer.size() == (1 + bufferDelay)) {
                    // enough data to work
                    prevBoutBufferMean = MeanDelayBuffer.takeFirst();
                    prevBoutBufferStd = StdDelayBuffer.takeFirst();

                    boutSignal = angleDifference(fish.head.theta, prevBoutBufferMean);

                    double threshold = max(prevBoutBufferStd*thresholdBout, minimumBoutThreshold);
                    // Bout conditions
                    if (abs(boutSignal) >= threshold) {
                        PositiveSignalBuffer.append(1);
                        // adjust size (5)
                        while (PositiveSignalBuffer.size()>5) {
                            PositiveSignalBuffer.pop_front();
                        }
                        if (timer.nsecsElapsed() >= trefBout + minBoutDelay) {
                            // bout allowed
                            double positiveSignalSum = 0;
                            for (int i=0; i<PositiveSignalBuffer.size(); i++) {
                                positiveSignalSum += PositiveSignalBuffer[i];
                            }

                            // Need 3 positives in last 5
                            if (positiveSignalSum>=3) {
                                emit newBout();
                                trefBout = timer.nsecsElapsed();
                            }
                        }
                    } else {
                        PositiveSignalBuffer.append(0);
                    }
                }

                // Append the bout signal to the buffer
                PrevBoutBuffer->appendAngle(fish.head.theta);

                // Calculate and save mean and std into a delay buffer
                MeanDelayBuffer.append(PrevBoutBuffer->getMean());
                StdDelayBuffer.append(PrevBoutBuffer->getStd());

                emit updateFish();

            } else { process_OK = false; }
        } else { process_OK = false; }

        processTime = timer.nsecsElapsed() - tref_process;

    }

   // --- Update display --------------------------------------------------

    if (timer.nsecsElapsed() >= trefDisplay + periodDisplay) {

        QVector<UMat> display;
        display.push_back(F.img);

        if (process_OK) {

            if (processCalibration) {

                display.push_back(Res);

            }

            if (processFish) {

                // Display ellipse
                if (!isnan(fish.body.x)) {

                    //ellipse(Res, Point(fish.body.x,fish.body.y), Size(fish.body.major_length, fish.body.minor_length), -fish.body.theta*180/M_PI, 0, 360, Scalar(255,0,0), 1);
                    //circle(Res, Point(fish.body.x+fish.body.major_length*cos(fish.body.theta), fish.body.y-fish.body.major_length*sin(fish.body.theta)), 2, Scalar(0,255,255));
                    circle(Res, Point(cmxTorso, cmyTorso), 2, Scalar(255,0,0), FILLED);
                    circle(Res, Point(cmxEyes, cmyEyes), 2, Scalar(0,255,0), FILLED);
                    try {
                        ellipse(Res, Point(fish.head.x,fish.head.y), Size(fish.head.major_length, fish.head.minor_length), -fish.head.theta*180/M_PI, 0, 360, Scalar(255,0,255), 1);
                        //if (fish.tail.minor_length > 0) {
                        //    ellipse(Res, Point(fish.tail.x,fish.tail.y), Size(fish.tail.major_length, fish.tail.minor_length),  -fish.tail.theta*180/M_PI, 0, 360, Scalar(0,255,0), 1);
                        //}
                        //circle(Res, Point(fish.xc, fish.yc), 2, Scalar(255,0,0), FILLED);
                        //circle(Res, Point(fish.xc, fish.yc), abs(1/fish.curvature), Scalar(255,0,0));

                    } catch (...) {}
                }

                display.push_back(Res);
                emit updateCurvature();
            }

            emit updateProcessTime();
            emit updateProcessStatus(PROCESS_OK);

        }
        else if (!processCalibration && !processFish) { emit updateProcessStatus(PROCESS_INACTIVE); }
        else if (!isBackground) { emit updateProcessStatus(PROCESS_NOBACK); }
        else { emit updateProcessStatus(PROCESS_FAILED); }

        emit updateDisplay(display);
        trefDisplay += periodDisplay;
    }

}

/* ====================================================================== *
 *      IMAGE PROCESSING                                                  *
 * ====================================================================== */

void Vision::resetBoutProcessing() {
    // --- Empty the buffers
    // head angles
    PrevBoutBuffer->reset();
    // means
    MeanDelayBuffer.clear();
    // stds
    StdDelayBuffer.clear();
    // positives
    PositiveSignalBuffer.clear();

    // Reset other counters...
    // ....... signal counter, time delay
}

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

    // Check if there is signal
    if (moment.m00 < (255*10)) {
        E.minor_length = 0;

        return E;
    } else {
        E.x = moment.m10/moment.m00;
        E.y = moment.m01/moment.m00;
        E.theta = atan((2*moment.mu11)/(moment.mu20-moment.mu02))/2 + (moment.mu20<moment.mu02)*(M_PI/2);
        E.major_length = 2*pow( (((moment.mu20 + moment.mu02) + pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);
        E.minor_length = 2*pow( (((moment.mu20 + moment.mu02) - pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);

        return E;
    }
}

Ellipse Vision::getEllipse(const vector<Point> &contour) {

    Ellipse E;

    Moments moment = getContourMoments(contour);
    E.x = moment.m10/moment.m00;
    E.y = moment.m01/moment.m00;
    E.theta = atan((2*moment.mu11)/(moment.mu20-moment.mu02))/2 + (moment.mu20<moment.mu02)*(M_PI/2);
    E.major_length = 2*pow( (((moment.mu20 + moment.mu02) + pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);
    E.minor_length = 2*pow( (((moment.mu20 + moment.mu02) - pow( (moment.mu20 - moment.mu02)*(moment.mu20 - moment.mu02) + 4*moment.mu11*moment.mu11,0.5))*0.5)/moment.m00, 0.5);

    return E;
}

Moments Vision::getContourMoments(const vector<Point> &contour) {
    double m00 = 0.0;
    double m10 = 0.0;
    double m01 = 0.0;
    double m20 = 0.0;
    double m11 = 0.0;
    double m02 = 0.0;
    double m30 = 0.0;
    double m21 = 0.0;
    double m12 = 0.0;
    double m03 = 0.0;

    for (unsigned int i=0; i<contour.size() ; i++) {
        Point p = contour[i];
        m00 += 1;
        m10 += p.x;
        m01 += p.y;
        m20 += p.x*p.x;
        m11 += p.x*p.y;
        m02 += p.y*p.y;
        m30 += p.x*p.x*p.x;
        m21 += p.y*p.x*p.x;
        m12 += p.y*p.y*p.x;
        m03 += p.y*p.y*p.y;
    }

    return Moments(m00, m10, m01, m20, m11, m02, m30, m21, m12, m03);
}

double Vision::angleDifference(double theta1, double theta2) {
    // difference w.r.t. theta2
    if (abs(theta1-theta2) < M_PI) {
            return theta1-theta2;
    } else {
        if (theta1<theta2) {
            return -(2*M_PI - abs(theta1-theta2)); // <0
        } else {
            return (2*M_PI - abs(theta1-theta2)); // >0
        }
    }
}

void Vision::setEllipseAngles() {

    // Check
    if (!fishpix.size()) { return; }

    // body angle
    // current major axis direction
    double dirx = cos(fish.body.theta);
    double diry = sin(fish.body.theta);

    double scalarProduct = dirx*vx + diry*vy;

    // Angle correction
    fish.body.theta += scalarProduct>0 ? 2*M_PI : 3*M_PI;
    fish.body.theta -= fish.body.theta>2*M_PI ? 2*M_PI : 0;

    // head angle
    // current major axis direction
    dirx = cos(fish.head.theta);
    diry = sin(fish.head.theta);

    scalarProduct = dirx*vx + diry*vy;

    // Angle correction
    fish.head.theta += scalarProduct>0 ? 2*M_PI : 3*M_PI;
    fish.head.theta -= fish.head.theta>2*M_PI ? 2*M_PI : 0;

    // Transform to natural coordinates (y +ve upwards)
    fish.body.theta = 2*M_PI - fish.body.theta;
    fish.head.theta = 2*M_PI - fish.head.theta;
}

void Vision::setHeadTail() {
    // Check
    if (!fishpix.size()) { return; }

    // Define head and tail Point vectors
    vector<Point> chead;
    vector<Point> ctail;

    // Minor axis
    double wx = -cos(fish.body.theta);
    double wy = sin(fish.body.theta);
    Point p;

    // Compute cross product
    for (unsigned int i=0 ; i<fishpix.size() ; i++) {
        p.x = fishpix.at(i).x - fish.body.x;
        p.y = fishpix.at(i).y - fish.body.y;
        if ((fishpix.at(i).x - fish.body.x)*wx - (fishpix.at(i).y - fish.body.y)*wy < 0) {
            chead.push_back(p);
        }
        else {
            ctail.push_back(p);
        }
    }
    // Order the head and tail
    // Compute the direction of the current Head/Tail wrt eyes direction


    // Get head and tail ellipses
    fish.head = getEllipse(chead);
    fish.head.x += fish.body.x;
    fish.head.y += fish.body.y;

    fish.tail = getEllipse(ctail);
    fish.tail.x += fish.body.x;
    fish.tail.y += fish.body.y;
}

void Vision::setCurvature() {
    // check if there is reasonable tail (i.e there was enough pixels in the ellipse computation
    if (fish.tail.minor_length > 0) {
        if (fish.tail.minor_length < 6) {
            // all went well
            double hx = sin(fish.head.theta);
            double hy = -cos(fish.head.theta);
            double tx = sin(fish.tail.theta);
            double ty = -cos(fish.tail.theta);

            fish.curvature = (hx*ty - hy*tx)/((fish.head.y - fish.tail.y)*tx - (fish.head.x - fish.tail.x)*ty);
            fish.xc = fish.head.x + hx/fish.curvature;
            fish.yc = fish.head.y + hy/fish.curvature;
        } else {
            // 'wide' tail caused by noise - do not update
            return;
        }
    } else {
        // tail is probably almost invisible and the fish is not moving
        fish.curvature = 0;
    }
}
