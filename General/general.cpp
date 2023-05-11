#include "object.h"

#define __EXPORT extern "C" __declspec(dllexport)

__EXPORT void intIO(ParamPtrArray& params) {
	int *dst = new int(get_data<int>(params[0]));
	params.push_back(make_param("dst", "int", dst));
}

__EXPORT void gaussian_blur(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::GaussianBlur(get_data<cv::Mat>(params[0]), *dst, get_data<cv::Size>(params[1]), get_data<double>(params[2]), get_data<double>(params[3]), get_data<int>(params[4]));
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void read(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	*dst = cv::imread(
		get_data<std::string>(params[0]),
		get_data<int>(params[1])
	);

	params.push_back(make_param("dst","Mat",dst));
}



__EXPORT void morph_open(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::Mat kernel = cv::getStructuringElement(
		cv::MORPH_RECT,
		get_data<cv::Size>(params[1])
	);
	cv::morphologyEx(
		get_data_const_ref<cv::Mat>(params[0]),
		*dst,
		cv::MORPH_OPEN,
		kernel
	);
	params.push_back(make_param("dst","Mat",dst));
}

__EXPORT void morph_close(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::Mat kernel = cv::getStructuringElement(
		cv::MORPH_RECT,
		get_data<cv::Size>(params[1])
	);
	cv::morphologyEx(
		get_data_const_ref<cv::Mat>(params[0]),
		*dst,
		cv::MORPH_CLOSE,
		kernel
	);
	params.push_back(make_param("dst","Mat",dst));
}

__EXPORT void threshold(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::threshold(
		get_data_const_ref<cv::Mat>(params[0]),
		*dst,
		get_data<double>(params[1]),
		get_data<double>(params[2]),
		get_data<int>(params[3])
	);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void convert_color(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	
	cv::cvtColor(
		get_data_const_ref<cv::Mat>(params[0]),
		*dst,
		get_data<int>(params[1])
	);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void median_blur(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::medianBlur(
		get_data_const_ref<cv::Mat>(params[0]),
		*dst,
		get_data<int>(params[1])
	);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void find_contours(ParamPtrArray& params) {
	using ContoursType = std::vector<std::vector<cv::Point>>;
	ContoursType* contours = new ContoursType;
	cv::findContours(
		get_data_const_ref<cv::Mat>(params[0]),
		*contours,
		cv::RETR_TREE,
		cv::CHAIN_APPROX_SIMPLE
	);
	params.push_back(make_param("contours","vector<vector<Point>>",contours));
}

__EXPORT void draw_contours(ParamPtrArray& params) {
	using ContoursType = std::vector<std::vector<cv::Point>>;
	cv::Mat* dst = new cv::Mat(get_data_const_ref<cv::Mat>(params[0]));
	cv::drawContours(
		*dst,
		get_data_const_ref<ContoursType>(params[1]),
		-1,
		cv::Scalar(0, 255, 0),
		2
	);
	params.push_back(make_param("dst", "Mat", dst));
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

	const ContoursType& contours = get_data_const_ref<ContoursType>(params[0]);
	int index = get_data<int>(params[1]);
	auto contour = new ContourType(contours[index]);
	params.push_back(make_param("contour","vector<vector<Point>>",contour));
}

/**
* @brief Find the minimum area circumscribing rectangle of a contour.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `std::vector<cv::Point>` object representing the contour.
*  2. <b><em>output</em></b> A `cv::RotatedRect` object representing the minimum area circumscribing rectangle.
*/

__EXPORT void min_area_rect(ParamPtrArray& params) {
	using ContourType = std::vector<cv::Point>;

	const ContourType& contour = get_data_const_ref<ContourType>(params[0]);
	cv::RotatedRect* result = new cv::RotatedRect(cv::minAreaRect(contour));
	params.push_back(make_param("rect","RotatedRect",result));
}

/**
* @brief Find the 4 vertices of a rotated rectangle.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::RotatedRect` object representing the rotated rectangle.
*  2. <b><em>output</em></b> A `std::vector<cv::Point>` object representing the 4 vertices.
*/

__EXPORT void box_points(ParamPtrArray& params) {
	const cv::RotatedRect& rect = get_data_const_ref<cv::RotatedRect>(params[0]);
	cv::Point2f vertices_array[4];
	rect.points(vertices_array);
	std::vector<cv::Point>* vertices_vector = new std::vector<cv::Point>();
	for (int i = 0; i < 4; ++i) {
		vertices_vector->emplace_back(vertices_array[i].x, vertices_array[i].y);
	}
	params.push_back(make_param("points","vector<Point>",vertices_vector));
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
	contours->push_back(get_data_const_ref<ContourType>(params[0]));
	params.push_back(make_param("contours","vector<vector<Point>>",contours));
}

/**
* @brief Apply warp affine to an input image.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Mat` object representing the input image.
*  2. <b><em>input</em></b> A `cv::Mat` object representing the affine matrix.
*  3. <b><em>input</em></b> A `cv::Size` object representing the size of the output image.
*  4. <b><em>output</em></b> A `cv::Mat` object representing the output image.
*/

__EXPORT void warp_affine(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::warpAffine(get_data_const_ref<cv::Mat>(params[0]),
		*dst,
		get_data_const_ref<cv::Mat>(params[1]),
		get_data<cv::Size>(params[2]));

	params.push_back(make_param("dst","Mat",dst));
}

/**
* @brief Get a 2D rotation matrix.
* @param params An array of parameters containing:
*  1. <b><em>input</em></b> A `cv::Point2f` object representing the center of rotation in the source image.
*  2. <b><em>input</em></b> A `double` representing the rotation angle in degrees.
*  3. <b><em>input</em></b> A `double` representing the isotropic scale factor.
*  4. <b><em>output</em></b> A `cv::Mat` object representing the rotation matrix.
*/

__EXPORT void get_rotation_matrix_2D(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat(cv::getRotationMatrix2D(get_data<cv::Point2f>(params[0]),
		get_data<double>(params[1]),
		get_data<double>(params[2])));

	params.push_back(make_param("dst","Mat",dst));
}

__EXPORT void total_pixels(ParamPtrArray& params) {
	cv::Mat src = get_data<cv::Mat>(params[0]);
	cv::Rect rect(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4]));
	int* total = new int(src(rect).total());
	params.push_back(make_param("total", "int", total));
}

__EXPORT void mean(ParamPtrArray& params) {
	cv::Mat src = get_data<cv::Mat>(params[0]);
	cv::Rect rect(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4]));
	cv::Scalar* mean_value = new cv::Scalar(cv::mean(src(rect)));
	params.push_back(make_param("mean_value", "int", mean_value));
}

__EXPORT void min_value(ParamPtrArray& params) {
	cv::Mat src = get_data<cv::Mat>(params[0]);
	cv::Rect rect(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4]));
	double max_value;
	double* min_value = new double;
	cv::minMaxLoc(src(rect), min_value, &max_value);
	params.push_back(make_param("min_value", "double", min_value));
}

__EXPORT void max_value(ParamPtrArray& params) {
	cv::Mat src = get_data<cv::Mat>(params[0]);
	cv::Rect rect(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4]));
	double min_value;
	double* max_value = new double;
	cv::minMaxLoc(src(rect), &min_value, max_value);
	params.push_back(make_param("max_value", "double", max_value));
}

__EXPORT void flip(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::flip(
		get_data<cv::Mat>(params[0]), 
		*dst, 
		get_data<int>(params[1])
	);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void resize(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::resize(
		get_data<cv::Mat>(params[0]),
		*dst,
		get_data<cv::Size>(params[1])
	);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void crop(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat;
	cv::Rect rect(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4])
	);
	get_data<cv::Mat>(params[0])(rect).copyTo(*dst);
	params.push_back(make_param("dst", "Mat", dst));
}