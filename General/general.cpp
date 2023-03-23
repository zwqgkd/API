#include<opencv2/opencv.hpp>

#define EXPORT extern "C" __declspec(dllexport)

union value {
	std::vector<int>* int_values;
	std::vector<double>* double_values;
	std::vector<std::string>* string_values;
	std::vector<cv::Mat>* mat_values;
	std::vector<cv::Size>* size_values;
};

EXPORT void morph_open(std::vector<value> values) {
	cv::Mat dst;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, (*values[1].size_values)[0]);
	cv::morphologyEx(
		(*values[0].mat_values)[0],
		dst,
		cv::MORPH_OPEN,
		kernel
	);
	values[2].mat_values->push_back(dst);
}

EXPORT void morph_close(std::vector<value> values) {
	cv::Mat dst;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, (*values[1].size_values)[0]);
	cv::morphologyEx(
		(*values[0].mat_values)[0],
		dst,
		cv::MORPH_CLOSE,
		kernel
	);
	values[2].mat_values->push_back(dst);
}

EXPORT void threshold(std::vector<value> values) {
	cv::Mat dst;
	cv::threshold((*values[0].mat_values)[0], dst, (*values[1].double_values)[0], (*values[2].double_values)[0], (*values[3].int_values)[0]);
	values[4].mat_values->push_back(dst);

}

EXPORT void convert_color(std::vector<value> values) {
	cv::Mat dst;
	cv::cvtColor((*values[0].mat_values)[0], dst, (*values[1].int_values)[0], (*values[2].int_values)[0]);
	values[3].mat_values->push_back(dst);
}

EXPORT void median_blur(std::vector<value> values) {
	cv::Mat dst;
	cv::medianBlur((*values[0].mat_values)[0], dst, (*values[1].int_values)[0]);
	values[2].mat_values->push_back(dst);
}

EXPORT void find_contours(std::vector<value> values) {
	std::vector<cv::Mat> contours;
	cv::findContours((*values[0].mat_values)[0], contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
	*values[1].mat_values = contours;
}

EXPORT void draw_contours(std::vector<value> values) {
	cv::Mat dst((*values[0].mat_values)[0]);
	cv::drawContours(dst, *values[1].mat_values, -1, cv::Scalar(0, 255, 0), 2);
	values[2].mat_values->push_back(dst);
}