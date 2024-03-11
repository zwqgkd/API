#pragma once
#include<opencv2/imgproc.hpp>
#include<opencv2/imgproc/types_c.h>
#include"./src/object.h"

#if defined(__cplusplus)//
extern "C" {
#endif
/*
  	__declspec(dllexport) double minimumBrightness(ParamPtrArray& params);
	__declspec(dllexport) double meanBrightness(ParamPtrArray& params);
	__declspec(dllexport) double maximumBrightness(ParamPtrArray& params);
	__declspec(dllexport) double standardDeviation(ParamPtrArray& params);
*/
	__declspec(dllexport) void measureBrightness(ParamPtrArray& params);
#ifdef __cplusplus
}
#endif
 

void measureBrightness(ParamPtrArray& params);

double minimumBrightness(cv::Mat mat);

double meanBrightness(cv::Mat mat);

double maximumBrightness(cv::Mat mat);

double standardDeviation(cv::Mat mat);
