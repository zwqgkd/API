#pragma once

#include "opencv2/core.hpp"
extern"C" __declspec(dllexport)void myFitLine(cv::InputArray _points, cv::OutputArray _line, int distType,
	double param, double reps, double aeps);