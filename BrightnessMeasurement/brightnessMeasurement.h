#pragma once
#include<opencv2/imgproc.hpp>
#include<opencv2/imgproc/types_c.h>

using namespace cv;

#if defined(__cplusplus)//
extern "C" {
#endif
	__declspec(dllexport) double minimumBrightness(Mat img);
	__declspec(dllexport) double meanBrightness(Mat img);
	__declspec(dllexport) double maximumBrightness(Mat img);
	__declspec(dllexport) double standardDeviation(Mat img);
#ifdef __cplusplus
}
#endif


double minimumBrightness(Mat img);

double meanBrightness(Mat img);

double maximumBrightness(Mat img);

double standardDeviation(Mat img);
