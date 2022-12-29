#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)void myMedianBlur(cv::InputArray _src0, cv::OutputArray _dst, int ksize);