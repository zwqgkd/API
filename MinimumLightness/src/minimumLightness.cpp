#include"precomp.hpp"

double meanLightness(cv::Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	std::vector<cv::Mat>images;
	split(img_hls, images);
	double *minVal = NULL;
	cv::minMaxLoc(images[1], minVal, NULL, NULL, NULL);
	return *minVal;

}