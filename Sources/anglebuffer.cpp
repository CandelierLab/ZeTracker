#include "anglebuffer.h"

AngleBuffer::AngleBuffer(int maxBufferSize) {
    maxSize = maxBufferSize;
    xBuffer = QVector<double>();
    yBuffer = QVector<double>();
    mean = 0.0;
    mean_updated = false;
}

void AngleBuffer::appendAngle(double theta) {
    xBuffer.append(cos(theta));
    yBuffer.append(sin(theta));

    // Adjust the length of the buffer
    while (xBuffer.size() > maxSize) {
        xBuffer.pop_front();
        yBuffer.pop_front();
    }
    mean_updated = false;
}

double AngleBuffer::getMean() {
    if (xBuffer.size()>0 & yBuffer.size()>0) {
        if (mean_updated) {
            return mean;
        } else {
            // compute the mean
            if (xBuffer.size()==yBuffer.size()) {
                double xSum = 0.0;
                double ySum = 0.0;

                for (int i=0; i<xBuffer.size(); i++) {
                    xSum += xBuffer[i];
                    ySum += yBuffer[i];
                }

                mean = atan2(ySum, xSum);
                if (mean<0) {
                    // convert  to [0, 2\pi) domain
                    mean += 2*M_PI;
                }
                mean_updated = true;
                return mean;
            } else {
                this->reset();
                return 0.0;
            }
        }

    } else {
        return 0.0;
    }
}

double AngleBuffer::getStd() {
    double m = getMean();
    double xm = cos(m);
    double ym = sin(m);

    double std = 0.0;
    for (int i=0; i<xBuffer.size(); i++) {
        std += pow(acos(yBuffer[i]*ym+xBuffer[i]*xm), 2);
    }
    std /= xBuffer.size();
    std = pow(std, 0.5);
    return std;
}

void AngleBuffer::reset() {
    xBuffer.clear();
    yBuffer.clear();
    mean = 0.0;
    mean_updated = false;
}
