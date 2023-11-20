#include "object.h"
#include<vector>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

#define __EXPORT extern "C" __declspec(dllexport)


__EXPORT void g(ParamPtrArray& params) {
	int num = get_data<int>(params[0]);
	int* dst = new int{ num + 1 };
	params.push_back(make_param("increase", "int", dst));
}

__EXPORT void f(ParamPtrArray& params) {
	vector<int>* dst = new vector<int>{ 1,2,3 };
	params.push_back(make_param("dst", "vector<int>", dst));
}


__EXPORT void threshold(ParamPtrArray& params) {
	cv::Mat* dst = new cv::Mat();
	cv::threshold(get_data_ref<cv::Mat>(params[0]), *dst, get_data_ref<double>(params[1]),
		get_data_ref<double>(params[2]), get_data_ref<int>(params[3]));
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void median_filter(ParamPtrArray& params) {
	Mat src = get_data_ref<Mat>(params[0]);
	int ksize = get_data_ref<int>(params[1]);

	Mat* dst = new Mat();

	cv::medianBlur(src, *dst, ksize);
	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void mean_blur(ParamPtrArray& params) {
	Mat src = get_data_ref<Mat>(params[0]);
	Size ksize = get_data_ref<Size>(params[1]);
	int borderType = get_data<int>(params[2]);

	Mat* dst = new Mat;
	cv::blur(src, *dst, ksize, Point(-1, -1), borderType);

	params.push_back(make_param("dst", "Mat", dst));
}

__EXPORT void measure_color(ParamPtrArray& params) {
	//指定区域的范围
	Rect roi_rect(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4])
	);
	// 提取指定区域的图像
	cv::Mat roi = get_data_ref<Mat>(params[0])(roi_rect);

	// 计算每个通道的最大值、最小值、均值和标准差
	cv::Scalar minValue, maxValue, meanValue, stdDevValue;
	cv::meanStdDev(roi, meanValue, stdDevValue);

	vector<Mat> channels;
	cv::split(roi, channels);
	for (int i = 0; i < channels.size(); ++i) {
		cv::minMaxLoc(roi, &minValue[i], &maxValue[i]);
	}
	//计算直方图
	int bins = 256;
	int histSize[] = { bins };
	float range[] = { 0,256 };
	const float* histRange = { range };
	bool uniform = true, accumulate = false;
	Mat hista, histb, histc;
	calcHist(&channels[0], 1, 0, Mat(), hista, 1, histSize, &histRange, uniform, accumulate);
	calcHist(&channels[1], 1, 0, Mat(), histb, 1, histSize, &histRange, uniform, accumulate);
	calcHist(&channels[2], 1, 0, Mat(), histc, 1, histSize, &histRange, uniform, accumulate);

	int histWidth = 512, histHeight = 400;
	int binWidth = cvRound((double)histWidth / bins);
	Mat histImage(histHeight, histWidth, CV_8UC3, Scalar(0, 0, 0));

	//直方图归一化到[0,histImage.rows]
	normalize(hista, hista, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(histb, hista, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	normalize(histc, hista, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	//绘制
	for (int i = 1; i < bins; i++) {
		line(histImage, Point(binWidth * (i - 1), histHeight - cvRound(hista.at<float>(i - 1))), Point(binWidth * (i), histHeight - cvRound(hista.at<float>(i))), Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, Point(binWidth * (i - 1), histHeight - cvRound(histb.at<float>(i - 1))), Point(binWidth * (i), histHeight - cvRound(histb.at<float>(i))), Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, Point(binWidth * (i - 1), histHeight - cvRound(histc.at<float>(i - 1))), Point(binWidth * (i), histHeight - cvRound(histc.at<float>(i))), Scalar(0, 0, 255), 2, 8, 0);
	}
	params.push_back(make_param("minValue", "Scalar", new Scalar(minValue)));
	params.push_back(make_param("maxValue", "Scalar", new Scalar(maxValue)));
	params.push_back(make_param("meanValue", "Scalar", new Scalar(meanValue)));
	params.push_back(make_param("stdDevValue", "Scalar", new Scalar(stdDevValue)));
	params.push_back(make_param("histImage", "Mat", new Mat(histImage)));
}

__EXPORT void test_send_counters(ParamPtrArray& params) {
	vector < vector<Point>>* dst = new vector<vector<Point>>();
	vector<Point> data = vector<Point>{ {0,1} };
	dst->push_back(data);

	params.push_back(make_param("dst", "vector<vector<Point>>", dst));
}

__EXPORT void test_receive_counters(ParamPtrArray& params) {
	vector<vector<Point>> src = get_data< vector<vector<Point>> >(params[0]);

	if (src.size() == 1) {
		params.push_back(make_param("dst", "string", new string{ "OK" }));
	}
	else {
		params.push_back(make_param("dst", "string", new string{ "NG" }));
	}
}

__EXPORT void edge_threshold(ParamPtrArray& params) {
	//灰度图
	Mat src = get_data_ref<Mat>(params[0]);
	double threshold1 = get_data<double>(params[1]);
	double threshold2 = get_data<double>(params[2]);

	Mat dst;
	Canny(src, dst, threshold1, threshold2);
	params.push_back(make_param("dst", "Mat", new Mat(dst)));
}

__EXPORT void region_threshold(ParamPtrArray& params) {
	Mat src = get_data_ref<Mat>(params[0]);//灰度图
	int blockSize = get_data<int>(params[1]);//领域大小，应为奇数
	int c = get_data<int>(params[2]);//偏移调整量
	Mat dst;
	cv::adaptiveThreshold(src, dst, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blockSize, c);
	params.push_back(make_param("dst", "Mat", new Mat(dst)));
}

__EXPORT void grab_cut_threshold(ParamPtrArray& params) {
	Mat src = get_data_ref<Mat>(params[0]);//彩色图

	Rect rectangle(
		get_data<int>(params[1]),
		get_data<int>(params[2]),
		get_data<int>(params[3]),
		get_data<int>(params[4])
	);
	// 构建图割模型
	cv::Mat mask, bgModel, fgModel;
	cv::grabCut(src, mask, rectangle, bgModel, fgModel, 5, cv::GC_INIT_WITH_RECT);

	// 将图割结果转换为二值图像
	cv::Mat binary = (mask == cv::GC_PR_FGD) | (mask == cv::GC_FGD);
	binary = binary * 255;

	params.push_back(make_param("dst", "Mat", new Mat(binary)));
}

__EXPORT void contour_area(ParamPtrArray& params) {
	vector<Point> points = get_data_ref<vector<Point>>(params[0]);
	params.push_back(make_param("area", "double", new double{ cv::contourArea(points) }));
}

__EXPORT void generate_points(ParamPtrArray& params) {
	std::vector<cv::Point> points;
	points.push_back(cv::Point(0, 0));
	points.push_back(cv::Point(0, 10));
	points.push_back(cv::Point(10, 10));
	points.push_back(cv::Point(10, 0));
	params.push_back(make_param("points", "vector<Point>", new vector<Point>(points)));
}

__EXPORT void generate_point(ParamPtrArray& params) {
	params.push_back(make_param("point", "Point", new Point(3, 7)));
}

