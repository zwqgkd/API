#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)void myGaussianBlur(cv::InputArray _src, cv::OutputArray _dst, cv::Size ksize,
	double sigma1, double sigma2, int borderType);