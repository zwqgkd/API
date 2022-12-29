#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)cv::RotatedRect myFitEllipse(cv::InputArray _points);