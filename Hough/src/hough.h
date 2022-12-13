#pragma once

#include <opencv2/core.hpp>

extern"C" __declspec(dllexport) void myHoughLines(cv::InputArray _image, cv::OutputArray lines, double rho, double theta, int threshold, double srn, double stn, double min_theta, double max_theta);
extern "C" _declspec(dllexport) void myHoughCircles(cv::InputArray _image, cv::OutputArray _circles, int method, double dp, double minDist, double param1, double param2, int minRadius, int maxRadius, int maxCircles, double param3);
