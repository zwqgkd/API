#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)cv::RotatedRect myFitEC(cv::InputArray points, bool isCircle);
