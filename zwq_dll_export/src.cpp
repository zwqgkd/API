#include "object.h"
#include<vector>
#include<opencv2/opencv.hpp>
#include<exception>
using namespace std;
using namespace cv;

#define __EXPORT extern "C" __declspec(dllexport)

//__EXPORT void convert_color(ParamPtrArray& params) {
//	try {
//		cv::Mat* dst = new cv::Mat;
//		cv::cvtColor(
//			get_data_const_ref<cv::Mat>(params[0]),
//			*dst,
//			get_data<int>(params[1])
//		);
//		params.push_back(make_param("dst", "Mat", dst));
//	}
//	catch (cv::Exception& e) {
//		throw ExecutionFailedException("运行错误", "颜色转换的图片与转换模式冲突");
//	}
//}
//
//__EXPORT void g(ParamPtrArray& params) {
//	int num = get_data<int>(params[0]);
//	int* dst = new int{ num + 1 };
//	params.push_back(make_param("increase", "int", dst));
//}
//
//__EXPORT void f(ParamPtrArray& params) {
//	vector<int>* dst = new vector<int>{ 1,2,3 };
//	params.push_back(make_param("dst", "vector<int>", dst));
//}
//
//
//__EXPORT void threshold(ParamPtrArray& params) {
//	try {
//		cv::Mat* dst = new cv::Mat();
//		cv::threshold(get_data_ref<cv::Mat>(params[0]), *dst, get_data_ref<double>(params[1]),
//			get_data_ref<double>(params[2]), get_data_ref<int>(params[3]));
//		params.push_back(make_param("dst", "Mat", dst));
//	}
//	catch (cv::Exception& e) {
//		throw ExecutionFailedException("运行错误","该二值化模式必须要灰度图");
//	}
//}
//
//__EXPORT void median_filter(ParamPtrArray& params) {
//	Mat src = get_data_ref<Mat>(params[0]);
//	int ksize = get_data_ref<int>(params[1]);
//
//	Mat* dst = new Mat();
//
//	cv::medianBlur(src, *dst, ksize);
//	params.push_back(make_param("dst", "Mat", dst));
//}
//
//__EXPORT void mean_blur(ParamPtrArray& params) {
//	Mat src = get_data_ref<Mat>(params[0]);
//	Size ksize = get_data_ref<Size>(params[1]);
//	int borderType = get_data<int>(params[2]);
//
//	Mat* dst = new Mat;
//	cv::blur(src, *dst, ksize, Point(-1, -1), borderType);
//
//	params.push_back(make_param("dst", "Mat", dst));
//}
//
//__EXPORT void measure_color(ParamPtrArray& params) {
//	//指定区域的范围
//	Rect roi_rect(
//		get_data<int>(params[1]),
//		get_data<int>(params[2]),
//		get_data<int>(params[3]),
//		get_data<int>(params[4])
//	);
//	// 提取指定区域的图像
//	cv::Mat roi = get_data_ref<Mat>(params[0])(roi_rect);
//
//	// 计算每个通道的最大值、最小值、均值和标准差
//	cv::Scalar minValue, maxValue, meanValue, stdDevValue;
//	cv::meanStdDev(roi, meanValue, stdDevValue);
//
//	vector<Mat> channels;
//	cv::split(roi, channels);
//	for (int i = 0; i < channels.size(); ++i) {
//		cv::minMaxLoc(roi, &minValue[i], &maxValue[i]);
//	}
//	//计算直方图
//	int bins = 256;
//	int histSize[] = { bins };
//	float range[] = { 0,256 };
//	const float* histRange = { range };
//	bool uniform = true, accumulate = false;
//	Mat hista, histb, histc;
//	calcHist(&channels[0], 1, 0, Mat(), hista, 1, histSize, &histRange, uniform, accumulate);
//	calcHist(&channels[1], 1, 0, Mat(), histb, 1, histSize, &histRange, uniform, accumulate);
//	calcHist(&channels[2], 1, 0, Mat(), histc, 1, histSize, &histRange, uniform, accumulate);
//
//	int histWidth = 512, histHeight = 400;
//	int binWidth = cvRound((double)histWidth / bins);
//	Mat histImage(histHeight, histWidth, CV_8UC3, Scalar(0, 0, 0));
//
//	//直方图归一化到[0,histImage.rows]
//	normalize(hista, hista, 0, histImage.rows, NORM_MINMAX, -1, Mat());
//	normalize(histb, hista, 0, histImage.rows, NORM_MINMAX, -1, Mat());
//	normalize(histc, hista, 0, histImage.rows, NORM_MINMAX, -1, Mat());
//
//	//绘制
//	for (int i = 1; i < bins; i++) {
//		line(histImage, Point(binWidth * (i - 1), histHeight - cvRound(hista.at<float>(i - 1))), Point(binWidth * (i), histHeight - cvRound(hista.at<float>(i))), Scalar(255, 0, 0), 2, 8, 0);
//		line(histImage, Point(binWidth * (i - 1), histHeight - cvRound(histb.at<float>(i - 1))), Point(binWidth * (i), histHeight - cvRound(histb.at<float>(i))), Scalar(0, 255, 0), 2, 8, 0);
//		line(histImage, Point(binWidth * (i - 1), histHeight - cvRound(histc.at<float>(i - 1))), Point(binWidth * (i), histHeight - cvRound(histc.at<float>(i))), Scalar(0, 0, 255), 2, 8, 0);
//	}
//	params.push_back(make_param("minValue", "Scalar", new Scalar(minValue)));
//	params.push_back(make_param("maxValue", "Scalar", new Scalar(maxValue)));
//	params.push_back(make_param("meanValue", "Scalar", new Scalar(meanValue)));
//	params.push_back(make_param("stdDevValue", "Scalar", new Scalar(stdDevValue)));
//	params.push_back(make_param("histImage", "Mat", new Mat(histImage)));
//}
//
//__EXPORT void test_send_counters(ParamPtrArray& params) {
//	vector < vector<Point>>* dst = new vector<vector<Point>>();
//	vector<Point> data = vector<Point>{ {0,1} };
//	dst->push_back(data);
//
//	params.push_back(make_param("dst", "vector<vector<Point>>", dst));
//}
//
//__EXPORT void test_receive_counters(ParamPtrArray& params) {
//	vector<vector<Point>> src = get_data< vector<vector<Point>> >(params[0]);
//
//	if (src.size() == 1) {
//		params.push_back(make_param("dst", "string", new string{ "OK" }));
//	}
//	else {
//		params.push_back(make_param("dst", "string", new string{ "NG" }));
//	}
//}
//
//__EXPORT void edge_threshold(ParamPtrArray& params) {
//	//灰度图
//	Mat src = get_data_ref<Mat>(params[0]);
//	double threshold1 = get_data<double>(params[1]);
//	double threshold2 = get_data<double>(params[2]);
//
//	Mat dst;
//	Canny(src, dst, threshold1, threshold2);
//	params.push_back(make_param("dst", "Mat", new Mat(dst)));
//}
//
//__EXPORT void region_threshold(ParamPtrArray& params) {
//	Mat src = get_data_ref<Mat>(params[0]);//灰度图
//	int blockSize = get_data<int>(params[1]);//领域大小，应为奇数
//	int c = get_data<int>(params[2]);//偏移调整量
//	Mat dst;
//	cv::adaptiveThreshold(src, dst, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blockSize, c);
//	params.push_back(make_param("dst", "Mat", new Mat(dst)));
//}
//
//__EXPORT void grab_cut_threshold(ParamPtrArray& params) {
//	Mat src = get_data_ref<Mat>(params[0]);//彩色图
//
//	Rect rectangle(
//		get_data<int>(params[1]),
//		get_data<int>(params[2]),
//		get_data<int>(params[3]),
//		get_data<int>(params[4])
//	);
//	// 构建图割模型
//	cv::Mat mask, bgModel, fgModel;
//	cv::grabCut(src, mask, rectangle, bgModel, fgModel, 5, cv::GC_INIT_WITH_RECT);
//
//	// 将图割结果转换为二值图像
//	cv::Mat binary = (mask == cv::GC_PR_FGD) | (mask == cv::GC_FGD);
//	binary = binary * 255;
//
//	params.push_back(make_param("dst", "Mat", new Mat(binary)));
//}
//
//__EXPORT void contour_area(ParamPtrArray& params) {
//	vector<Point> points = get_data_ref<vector<Point>>(params[0]);
//	params.push_back(make_param("area", "double", new double{ cv::contourArea(points) }));
//}
//
//__EXPORT void generate_points(ParamPtrArray& params) {
//	std::vector<cv::Point> points;
//	points.push_back(cv::Point(0, 0));
//	points.push_back(cv::Point(0, 10));
//	points.push_back(cv::Point(10, 10));
//	points.push_back(cv::Point(10, 0));
//	params.push_back(make_param("points", "vector<Point>", new vector<Point>(points)));
//}
//
//__EXPORT void generate_point(ParamPtrArray& params) {
//	params.push_back(make_param("point", "Point", new Point(3, 7)));
//}
//

__EXPORT void gaussian_blur(ParamPtrArray& params) {
    // get in_params
    GeliMat in_param0_mat = get_data_ref<GeliMat>(params[0]);
    int     in_param1_size = get_data<int>(params[1]);
    // 如果算子有roi区域，那么roi相关数据会push到params里
    Roi roi = get_data_ref<Roi>(params[2]);

    // 如果是图形类型，那么需要调用以下函数生成mask_mat
    if (!roi.mat_flag) {
        roi.generate_mask_mat(in_param0_mat);
    }

    // 第一个输出，算子状态
    int status=static_cast<int>(OperatorStatus::OK) ;
    cv::Mat resultImage;
    try {
        cv::GaussianBlur(in_param0_mat.mat, resultImage,
            cv::Size(in_param1_size, in_param1_size), 0, 0,
            cv::BORDER_DEFAULT);
        cv::Mat maskedResult;
        in_param0_mat.mat.copyTo(maskedResult, ~roi.mask_mat.mat);
        resultImage.copyTo(maskedResult, roi.mask_mat.mat);
    }
    catch (std::exception& e) {
        status = static_cast<int>(OperatorStatus::NG);
        throw ExecutionFailedException("执行错误", "高斯滤波执行错误");
    }
    params.push_back(make_param<int>(status));

    // 第二个输出，处理后的图像
    params.push_back(make_param<GeliMat>(cv::Mat(resultImage)));

    // 第三个输出，掩膜图像
    if (roi.mask) {
        // 如果需要输出掩膜图像
        params.push_back(make_param<GeliMat>(roi.mask_mat));
    }
    else {
        params.emplace_back(nullptr);
    }

    // 第四五个输出，关于区域信息：检测区域和roi圆弧
    if (roi.type == "") {
        params.emplace_back(nullptr);
        params.emplace_back(nullptr);
    }
    else if (roi.type == "rect") {
        params.push_back(make_param<GeliRect>(roi.generate_area()));
        params.emplace_back(nullptr);

    }
    else if (roi.type == "circle") {
        params.emplace_back(nullptr);
        params.push_back(make_param<GeliCircle>(roi.generate_circle()));
    }
    // 第六个输出，屏蔽区
    params.push_back(make_param<GeliShield>(roi.shield));
}