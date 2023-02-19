#include<opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

extern "C" __declspec(dllexport) cv::VideoCapture getCamera(int index) {
	return cv::VideoCapture(index);
}

extern "C" __declspec(dllexport) cv::Mat cameraCapture(cv::VideoCapture & camera) {
	cv::Mat img;
	camera >> img;
	return img;
}

extern "C" __declspec(dllexport) void saveImage(const cv::Mat & img, const std::string & filename) {
	cv::imwrite(filename, img);
}