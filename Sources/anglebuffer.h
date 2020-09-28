#ifndef ANGLEBUFFER_H
#define ANGLEBUFFER_H

#include <cmath>
#include "opencv2/opencv.hpp"
#include <QVector>

using namespace cv;
using namespace std;

class AngleBuffer
{
    int maxSize;
    QVector<double> xBuffer;
    QVector<double> yBuffer;
    double mean;
    bool mean_updated;

public:
    AngleBuffer(int maxBufferSize);
    void appendAngle(double);
    double getMean();
    double getStd();
    void reset();
};

#endif // ANGLEBUFFER_H
