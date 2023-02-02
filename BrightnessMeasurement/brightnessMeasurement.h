#pragma once
#include<opencv2/imgproc.hpp>
#include<opencv2/imgproc/types_c.h>

using namespace cv;

class BrightnessMeasurement {
public:
	double minimumBrightness(Mat img);
	double meanBrightness(Mat img);
	double maximumBrightness(Mat img);
	double standardDeviation(Mat img);
};
