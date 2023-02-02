#pragma once
#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include<vector>
#include <opencv2/imgproc/types_c.h>


//#if defined(__cplusplus)//
//extern "C" {
//#endif
//	__declspec(dllexport) double maxmumLightness(cv::Mat img);
//#ifdef __cplusplus
//}
//#endif
//
//double maxmumLightness(cv::Mat img);

class Lightness {
public:
	double maxmumLightness(cv::Mat img);
};

