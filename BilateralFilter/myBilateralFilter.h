#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)void myBilateralFilter(cv::InputArray _src, cv::OutputArray _dst,
	int d, double sigmaColor, double sigmaSpace, int borderType);