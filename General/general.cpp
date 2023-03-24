#include<opencv2/opencv.hpp>

#define EXPORT extern "C" __declspec(dllexport)

EXPORT void morph_open(std::vector<void*>& values) {
    cv::Mat* dst = new cv::Mat;
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT,
        *static_cast<cv::Size*>(values[1])
    );
    cv::morphologyEx(
        *static_cast<cv::Mat*>(values[0]),
        *dst,
        cv::MORPH_OPEN,
        kernel
    );
    values[2] = dst;
}

EXPORT void morph_close(std::vector<void*>& values) {
    cv::Mat* dst = new cv::Mat;
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT,
        *static_cast<cv::Size*>(values[1]));
    cv::morphologyEx(
        *static_cast<cv::Mat*>(values[0]),
        *dst,
        cv::MORPH_CLOSE,
        kernel
    );
    values[2] = dst;
}

EXPORT void threshold(std::vector<void*>& values) {
    cv::Mat* dst = new cv::Mat;
    cv::threshold(
        *static_cast<cv::Mat*>(values[0]),
        *dst,
        *static_cast<double*>(values[1]),
        *static_cast<double*>(values[2]),
        *static_cast<int*>(values[3])
    );
    values[4] = dst;
}

EXPORT void convert_color(std::vector<void*>& values) {
    cv::Mat* dst = new cv::Mat;
    cv::cvtColor(
        *static_cast<cv::Mat*>(values[0]),
        *dst, 
        *static_cast<int*>(values[1])
    );
    values[2] = dst;
}

EXPORT void median_blur(std::vector<void*>& values) {
    cv::Mat* dst = new cv::Mat;
    cv::medianBlur(
        *static_cast<cv::Mat*>(values[0]),
        *dst,
        *static_cast<int*>(values[1])
    );
    values[2] = dst;
}

EXPORT void find_contours(std::vector<void*>& values) {
    using ContoursType = std::vector<std::vector<cv::Point>>;
    ContoursType* contours = new ContoursType;
    cv::findContours(
        *static_cast<cv::Mat*>(values[0]),
        *contours,
        cv::RETR_TREE,
        cv::CHAIN_APPROX_SIMPLE
    );
    values[1] = contours;
}

EXPORT void draw_contours(std::vector<void*>& values) {
    using ContoursType = std::vector<std::vector<cv::Point>>;
    cv::Mat* dst = new cv::Mat(*static_cast<cv::Mat*>(values[0]));
    cv::drawContours(
        *dst,
        *static_cast<ContoursType*>(values[1]),
        -1,
        cv::Scalar(0, 255, 0),
        2
    );
    values[2] = dst;
}
//EXPORT void morph_open(std::vector<void*>& values) {
//	cv::Mat* dst = new cv::Mat;
//	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, (*(cv::Size*)values[1]));
//	cv::morphologyEx(
//		(*(cv::Mat*)values[0]),
//		*dst,
//		cv::MORPH_OPEN,
//		kernel
//	);
//	values[2] = dst;
//}
//
//EXPORT void morph_close(std::vector<void*>& values) {
//	cv::Mat* dst = new cv::Mat;
//	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, (*(cv::Size*)values[1]));
//	cv::morphologyEx(
//		(*(cv::Mat*)values[0]),
//		*dst,
//		cv::MORPH_CLOSE,
//		kernel
//	);
//	values[2] = dst;
//}
//
//EXPORT void threshold(std::vector<void*>& values) {
//	cv::Mat* dst = new cv::Mat;
//	cv::threshold(*(cv::Mat*)values[0], *dst, *(double*)values[1], *(double*)values[2], *(int*)values[3]);
//	values[4] = dst;
//}
//
//EXPORT void convert_color(std::vector<void*>& values) {
//	cv::Mat* dst = new cv::Mat;
//	cv::cvtColor(*(cv::Mat*)values[0], *dst, *(int*)values[1]);
//	values[2] = dst;
//}

//EXPORT void median_blur(std::vector<void*>& values) {
//	cv::Mat* dst = new cv::Mat;
//	cv::medianBlur(*(cv::Mat*)values[0], *dst, *(int*)values[1]);
//	values[2] = dst;
//}
//
//EXPORT void find_contours(std::vector<void*>& values) {
//	using ContoursType = std::vector<std::vector<cv::Point>>;
//	ContoursType* contours = new ContoursType;
//	cv::findContours(*(cv::Mat*)values[0], *contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
//	values[1] = contours;
//}
//
//EXPORT void draw_contours(std::vector<void*>& values) {
//	using ContoursType = std::vector<std::vector<cv::Point>>;
//	cv::Mat* dst = new cv::Mat(*(cv::Mat*)values[0]);
//	cv::drawContours(*dst, *(ContoursType*)values[1], -1, cv::Scalar(0, 255, 0), 2);
//	values[2] = dst;
//}