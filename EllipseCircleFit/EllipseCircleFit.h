#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)cv::RotatedRect myFitEllipse(cv::InputArray _points);

extern"C" __declspec(dllexport)void myFitCircle(cv::InputArray _points, cv::Point2f& _center, float& _radius);
