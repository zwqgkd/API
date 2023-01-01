#include"precomp.hpp"

int meanLightness(cv::Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	cv::Scalar scalar = cv::mean(img_hls);
	return scalar.val[1];
}