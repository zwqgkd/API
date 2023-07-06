#include "object.h"
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#define __EXPORT extern "C" __declspec(dllexport)

__EXPORT void intIO(ParamPtrArray& params) {
	int* dst = new int(get_data<int>(params[0]));
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

	params.push_back(make_param("dst", "Mat", dst));
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
	params.push_back(make_param("dst", "Mat", dst));
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
	params.push_back(make_param("dst", "Mat", dst));
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
	params.push_back(make_param("contours", "vector<vector<Point>>", contours));
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
	params.push_back(make_param("contour", "vector<vector<Point>>", contour));
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
	params.push_back(make_param("rect", "RotatedRect", result));
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
	params.push_back(make_param("points", "vector<Point>", vertices_vector));
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
	params.push_back(make_param("contours", "vector<vector<Point>>", contours));
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

	params.push_back(make_param("dst", "Mat", dst));
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

	params.push_back(make_param("dst", "Mat", dst));
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

__EXPORT void hist_statistics(ParamPtrArray& params) {
	using namespace cv;
	// 确定目标区域，例如这里是左上角的100x100像素区域
	Rect roi_rect(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4])
	);
	Mat roi = get_data_const_ref<Mat>(params[0])(roi_rect);

	// 统计目标区域中的像素个数、灰度值均值、最小值、最大值、峰值、标准差、像素数量和对比度
	int* num_pixels = new int(roi.total());
	params.push_back(make_param("num_pixels", "int", num_pixels));

	Scalar* mean_gray = new Scalar(mean(roi));
	params.push_back(make_param("mean_gray", "Scalar", mean_gray));

	double* min_gray = new double;
	double* max_gray = new double;
	minMaxLoc(roi, min_gray, max_gray);
	params.push_back(make_param("min_gray", "double", min_gray));
	params.push_back(make_param("max_gray", "double", max_gray));

	Mat hist;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	int histSize = 256;
	bool uniform = true, accumulate = false;
	calcHist(&roi, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

	double* hist_max_val = new double;
	Point* hist_max_loc = new Point;

	minMaxLoc(hist, 0, hist_max_val, 0, hist_max_loc);
	params.push_back(make_param("hist_max_val", "double", hist_max_val));
	params.push_back(make_param("hist_max_loc", "Point", hist_max_loc));

	Scalar* std_dev = new Scalar;
	meanStdDev(roi, *mean_gray, *std_dev);
	double* contrast = new double((*max_gray - *min_gray) / (*max_gray + *min_gray));

	params.push_back(make_param("std_dev", "Scalar", std_dev));
	params.push_back(make_param("contrast", "double", contrast));

	int hist_w = 512, hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat* hist_image = new Mat(hist_h, hist_w, CV_8UC1, Scalar(0));
	normalize(hist, hist, 0, hist_image->rows, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < histSize; i++) {
		line(*hist_image, Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
			Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
			Scalar(255), 2, LINE_AA);
	}

	params.push_back(make_param("hist_image", "Mat", hist_image));
}

__EXPORT void match_template_grayscale(ParamPtrArray& params) {
	cv::Mat dst;
	auto result = new cv::Mat;
	cv::cvtColor(
		get_data_const_ref<cv::Mat>(params[0]),
		dst,
		cv::COLOR_BGR2GRAY
	);
	cv::matchTemplate(dst,
		get_data_const_ref<cv::Mat>(params[1]),
		*result,
		get_data<int>(params[2]),
		get_data_const_ref<cv::Mat>(params[3]));
	params.push_back(make_param("result", "Mat", result));
}

__EXPORT void match_template_gradient(ParamPtrArray& params) {
	cv::Mat dst;
	auto result = new cv::Mat;
	cv::cvtColor(get_data_const_ref<cv::Mat>(params[0]),
		dst,
		cv::COLOR_BGR2GRAY);
	cv::Sobel(dst, dst, 3, 1, 0);
	cv::matchTemplate(dst,
		get_data_const_ref<cv::Mat>(params[1]),
		*result,
		get_data<int>(params[2]),
		get_data_const_ref < cv::Mat>(params[3]));
	params.push_back(make_param("result", "Mat", result));
}

__EXPORT void match_template_edge(ParamPtrArray& params) {
	cv::Mat dst;
	auto result = new cv::Mat;
	cv::cvtColor(get_data_const_ref<cv::Mat>(params[0]), dst, cv::COLOR_BGR2GRAY);
	cv::Canny(dst, dst, 100, 200);
	cv::matchTemplate(dst, 
		get_data_const_ref<cv::Mat>(params[1]), 
		*result, 
		get_data<int>(params[2]), 
		get_data_const_ref<cv::Mat>(params[3]));
	params.push_back(make_param("result", "Mat", result));
}

__EXPORT void fast(ParamPtrArray& params) {
	cv::Mat *dst = new cv::Mat;
	int threshold = get_data<int>(params[1]);
	bool nonmaxSuppression = get_data<bool>(params[2]);
	// 创建Fast算法对象，设置阈值为50
	cv::Ptr<cv::FastFeatureDetector> detector = cv::FastFeatureDetector::create(threshold,nonmaxSuppression);

	// 检测关键点
	std::vector<cv::KeyPoint> keypoints;
	detector->detect(get_data_const_ref<cv::Mat>(params[0]), keypoints);
	// 绘制关键点
	cv::drawKeypoints(get_data_const_ref<cv::Mat>(params[0]), keypoints, *dst);
	params.push_back(make_param("result", "Mat", dst));
}

__EXPORT void canny(ParamPtrArray& params) {
	int threhold1, threhold2;

	threhold1 = get_data<int>(params[1]);
	threhold2 = get_data<int>(params[2]);

	//canny
	cv::Mat *dst = new cv::Mat;
	Canny(get_data_const_ref<cv::Mat>(params[0]), *dst, threhold1, threhold2);
	params.push_back(make_param("result", "Mat", dst));
}

__EXPORT void surf(ParamPtrArray& params) {
	using namespace cv::xfeatures2d;

	int minHessian = get_data<int>(params[1]);
	auto& src = get_data_const_ref<cv::Mat>(params[0]);
	cv::Mat *dst = new cv::Mat;

	auto detector = SURF::create(minHessian);
	std::vector<cv::KeyPoint> keypoints;
	detector->detect(src, keypoints, cv::Mat());//找出关键点

	drawKeypoints(src, keypoints, *dst, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT);

	params.push_back(make_param("result", "Mat", dst));
}

__EXPORT void sift(ParamPtrArray& params) {
	auto& src = get_data_const_ref<cv::Mat>(params[0]);
	auto sift = cv::SIFT::create();
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
	sift->detectAndCompute(src, cv::noArray(), keypoints, descriptors);

	cv::Mat* dst = new cv::Mat;
	cv::drawKeypoints(src, keypoints, *dst);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void brisk(ParamPtrArray& params) {
	auto& src = get_data_const_ref<cv::Mat>(params[0]);
	auto sift = cv::BRISK::create();
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
	sift->detectAndCompute(src, cv::noArray(), keypoints, descriptors);

	cv::Mat* dst = new cv::Mat;
	cv::drawKeypoints(src, keypoints, *dst);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void orb(ParamPtrArray& params) {
	auto& src = get_data_const_ref<cv::Mat>(params[0]);
	auto sift = cv::ORB::create();
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
	sift->detectAndCompute(src, cv::noArray(), keypoints, descriptors);

	cv::Mat* dst = new cv::Mat;
	cv::drawKeypoints(src, keypoints, *dst);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void freak(ParamPtrArray& params) {
	using namespace cv::xfeatures2d;

	int minHessian = get_data<int>(params[1]);
	auto& src = get_data_const_ref<cv::Mat>(params[0]);
	cv::Mat* dst = new cv::Mat;

	auto detector = FREAK::create(minHessian);
	std::vector<cv::KeyPoint> keypoints;
	detector->detect(src, keypoints, cv::Mat());//找出关键点

	drawKeypoints(src, keypoints, *dst, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT);

	params.push_back(make_param("result", "Mat", dst));
}