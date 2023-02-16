#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)void myMeanFilter(cv::InputArray src, cv::OutputArray dst,
	cv::Size ksize, cv::Point anchor, int borderType);



