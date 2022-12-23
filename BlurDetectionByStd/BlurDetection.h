#pragma once
#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include<vector>

using namespace cv;
class BlurDetectionByStd
{
public:
	//所有图像必须按灰度图像读入
	//可以在imread时第二个参数为0
	//可以按彩图读入用cvtColor转换为灰度图
	double getBlurDetectionScoreByStd(Mat image);
	double getBlurDetectionScoreByTenengrad(Mat image);
	double getBlurDetectionScoreByLaplacian(Mat image);
	double getBlurDetectionScoreBySMD(Mat image);
};

