#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)void myBoxFilter(cv::InputArray _src, cv::OutputArray _dst, int ddepth,
	cv::Size ksize, cv::Point anchor, bool normalize, int borderType);



