#pragma once


extern"C" __declspec(dllexport)double myKmeans(cv::InputArray _data, int K, cv::InputOutputArray _bestLabels, cv::TermCriteria criteria, int attempts,
	int flags, cv::OutputArray _centers);

