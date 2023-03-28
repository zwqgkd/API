#include "object.h"

#define EXPORT extern "C" __declspec(dllexport)

EXPORT void read(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    *dst = cv::imread(
        get_inner<std::string>(params[0]),
        get_inner<int>(params[1])
    );

    params.push_back(make_param(dst));
}

EXPORT void show(ParamPtrArray& params) {
    params.push_back(params[0]);
}

EXPORT void morph_open(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT,
        get_inner<cv::Size>(params[1])
    );
    cv::morphologyEx(
        get_inner_const_ref<cv::Mat>(params[0]),
        *dst,
        cv::MORPH_OPEN,
        kernel
    );
    params.push_back(make_param(dst));
}

EXPORT void morph_close(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT,
        get_inner<cv::Size>(params[1])
    );
    cv::morphologyEx(
        get_inner_const_ref<cv::Mat>(params[0]),
        *dst,
        cv::MORPH_CLOSE,
        kernel
    );
    params.push_back(make_param(dst));
}

EXPORT void threshold(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    cv::threshold(
        get_inner_const_ref<cv::Mat>(params[0]),
        *dst,
        get_inner<double>(params[1]),
        get_inner<double>(params[2]),
        get_inner<int>(params[3])
    );
    params.push_back(make_param(dst));
}

EXPORT void convert_color(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    cv::cvtColor(
        get_inner_const_ref<cv::Mat>(params[0]),
        *dst, 
        get_inner<int>(params[1])
    );
    params.push_back(make_param(dst));
}

EXPORT void median_blur(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    cv::medianBlur(
        get_inner_const_ref<cv::Mat>(params[0]),
        *dst,
        get_inner<int>(params[1])
    );
    params.push_back(make_param(dst));
}

EXPORT void find_contours(ParamPtrArray& params) {
    using ContoursType = std::vector<std::vector<cv::Point>>;
    ContoursType* contours = new ContoursType;
    cv::findContours(
        get_inner_const_ref<cv::Mat>(params[0]),
        *contours,
        cv::RETR_TREE,
        cv::CHAIN_APPROX_SIMPLE
    );
    params.push_back(make_param(contours));
}

EXPORT void draw_contours(ParamPtrArray& params) {
    using ContoursType = std::vector<std::vector<cv::Point>>;
    cv::Mat* dst = new cv::Mat(get_inner_const_ref<cv::Mat>(params[0]));
    cv::drawContours(
        *dst,
        get_inner_const_ref<ContoursType>(params[1]),
        -1,
        cv::Scalar(0, 255, 0),
        2
    );
    params.push_back(make_param(dst));
}