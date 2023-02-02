#include"brightnessMeasurement.h"

double BrightnessMeasurement::minimumBrightness(Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	std::vector<cv::Mat>images;
	split(img_hls, images);
	double *minVal = NULL;
	cv::minMaxLoc(images[1], minVal, NULL, NULL, NULL);
	return *minVal;
}

double BrightnessMeasurement::meanBrightness(Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	cv::Scalar scalar = cv::mean(img_hls);
	return scalar.val[1];
}

double BrightnessMeasurement::maximumBrightness(Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	std::vector<cv::Mat>images;
	split(img_hls, images);
	double *maxVal = NULL;
	cv::minMaxLoc(images[1], NULL, maxVal, NULL, NULL);
	return *maxVal;
}

double BrightnessMeasurement::standardDeviation(Mat img) {
	Mat gray, mat_mean, mat_stddev;
	cvtColor(img, gray, CV_RGB2GRAY); // ×ª»»Îª»Ò¶ÈÍ¼
	meanStdDev(gray, mat_mean, mat_stddev);
	double stddev;
	stddev = mat_stddev.at<double>(0, 0);
	return stddev;
}