#pragma once

#include "precomp.hpp"

extern"C" __declspec(dllexport) void myHoughLines(cv::InputArray _image, cv::OutputArray lines, double rho, double theta, int threshold, double srn, double stn, double min_theta, double max_theta);
