#include "maxmumLightness.h"

double maxmumLightness(cv::Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	std::vector<cv::Mat>images;
	split(img_hls, images);
	double *maxVal=NULL;
	cv::minMaxLoc(images[1], NULL, maxVal, NULL, NULL);
	return *maxVal;
	
}