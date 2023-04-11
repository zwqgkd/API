#include "object.h"
/**
* @file
* @brief Contain functions for image processing.
*/

#define __EXPORT extern "C" __declspec(dllexport)

/**
* @brief Read an image from file.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `string` representing the file path of the image.
*  2. <b><em>input</em></b> An `integer` representing the desired color type of the image.
*  3. <b><em>output</em></b> A `cv::Mat` object representing the read image.
*/

__EXPORT void read(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    *dst = cv::imread(
        get_inner<std::string>(params[0]),
        get_inner<int>(params[1])
    );

    params.push_back(make_param(dst));
}

/**
* 
* @brief Show an image.
* @param params An array of parameters containing:
*   1. <b><em>input</em></b> A `cv::Mat` object representing the image.
*   2. <b><em>output</em></b> A `cv::Mat` object representing the image.
*/

__EXPORT void show(ParamPtrArray& params) {
    params.push_back(params[0]);
}


/**
* @brief Apply morphological opening to an image.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the image.
*  2. <b><em>input</em></b> A `cv::Size` object representing the kernel size for the operation.
*  3. <b><em>output</em></b> A `cv::Mat` object representing the processed image.
*/

__EXPORT void morph_open(ParamPtrArray& params) {
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

/**
* @brief Apply morphological closing to an image.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the image.
*  2. <b><em>input</em></b> A `cv::Size` object representing the kernel size for the operation.
*  3. <b><em>output</em></b> A `cv::Mat` object representing the processed image.
*/

__EXPORT void morph_close(ParamPtrArray& params) {
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

/**
* @brief Apply morphological closing to an image.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the image.
*  2. <b><em>input</em></b> A `double` representing the threshold.
*  3. <b><em>input</em></b> A `double` representing the max value.
*  4. <b><em>input</em></b> A `int` representing the threshold type.
*  5. <b><em>output</em></b> A `cv::Mat` object representing the processed image.
*/

__EXPORT void threshold(ParamPtrArray& params) {
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

/**
* @brief Convert an input image to a different color space.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the image.
*  2. <b><em>input</em></b> A `int` representing the conversion type.
*  3. <b><em>output</em></b> A `cv::Mat` object representing the processed image.
*/

__EXPORT void convert_color(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    cv::cvtColor(
        get_inner_const_ref<cv::Mat>(params[0]),
        *dst, 
        get_inner<int>(params[1])
    );
    params.push_back(make_param(dst));
}

/**
* @brief Apply median blur to an input image.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the image.
*  2. <b><em>input</em></b> A `int` representing the kernel size.
*  3. <b><em>output</em></b> A `cv::Mat` object representing the processed image.
*/

__EXPORT void median_blur(ParamPtrArray& params) {
    cv::Mat* dst = new cv::Mat;
    cv::medianBlur(
        get_inner_const_ref<cv::Mat>(params[0]),
        *dst,
        get_inner<int>(params[1])
    );
    params.push_back(make_param(dst));
}

/**
* @brief Find contours in a binary image.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the input binary image.
*  2. <b><em>output</em></b> A `std::vector<std::vector<cv::Point>>` object representing the found contours.
*/

__EXPORT void find_contours(ParamPtrArray& params) {
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

/**
* @brief Draw contours in a binary image.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the input binary image.
*  2. <b><em>input</em></b> A `std::vector<std::vector<cv::Point>>` object representing the contours.
*  3. <b><em>output</em></b> A `cv::Mat` object representing the image with drawn countours.
*/

__EXPORT void draw_contours(ParamPtrArray& params) {
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

/**
* @brief Select a contour from contours.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `std::vector<std::vector<cv::Point>>` object representing the contours.
*  2. <b><em>input</em></b> A `int` object representing the index of the contour.
*  3. <b><em>output</em></b> A `std::vector<cv::Point>` object representing the selected contour.
*/

__EXPORT void select_contour(ParamPtrArray& params) {
    using ContourType = std::vector<cv::Point>;
    using ContoursType = std::vector<ContourType>;

    const ContoursType& contours = get_inner_const_ref<ContoursType>(params[0]);
    int index = get_inner<int>(params[1]);
    auto contour = new ContourType(contours[index]);
    params.push_back(make_param(contour));
}

/**
* @brief Find the minimum area circumscribing rectangle of a contour.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `std::vector<cv::Point>` object representing the contour.
*  2. <b><em>output</em></b> A `cv::RotatedRect` object representing the minimum area circumscribing rectangle.
*/

__EXPORT void min_area_rect(ParamPtrArray& params) {
    using ContourType = std::vector<cv::Point>;

    const ContourType& contour = get_inner_const_ref<ContourType>(params[0]);
    cv::RotatedRect* result = new cv::RotatedRect(cv::minAreaRect(contour));
    params.push_back(make_param(result));
}

/**
* @brief Find the 4 vertices of a rotated rectangle.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::RotatedRect` object representing the rotated rectangle.
*  2. <b><em>output</em></b> A `std::vector<cv::Point>` object representing the 4 vertices.
*/

__EXPORT void box_points(ParamPtrArray& params) {
    const cv::RotatedRect& rect = get_inner_const_ref<cv::RotatedRect>(params[0]);
    cv::Point2f vertices_array[4];
    rect.points(vertices_array);
    std::vector<cv::Point>* vertices_vector = new std::vector<cv::Point>();
    for (int i = 0; i < 4; ++i) {
        vertices_vector->emplace_back(vertices_array[i].x, vertices_array[i].y);
    }
    params.push_back(make_param(vertices_vector));
}

/**
* @brief Turn a contour into an array of contours.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `std::vector<cv::Point>` object representing the contour.
*  2. <b><em>output</em></b> A `std::vector<std::vector<cv::Point>>` object representing the contours.
*/

__EXPORT void contour_to_contours(ParamPtrArray& params) {
    using ContourType = std::vector<cv::Point>;
    using ContoursType = std::vector<ContourType>;

    ContoursType* contours = new ContoursType;
    contours->push_back(get_inner_const_ref<ContourType>(params[0]));
    params.push_back(make_param(contours));
}