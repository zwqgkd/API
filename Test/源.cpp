#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include<opencv2/features2d.hpp>
#include<vector>
#include"../ImgOperation/ImgOperator.h"
#include"../BlurDetectionByStd/BlurDetection.h"
#include<opencv2/stitching.hpp>

//�������������
#include "zbar.h"

using namespace cv;
using namespace std;




//class EdgeDetection {
//public:
//	//laplacian
//	virtual void myLaplacian(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int ksize = 1,
//		double scale = 1, double delta = 0, int borderType = cv::BORDER_DEFAULT) {
//		typedef void(*c)(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int ksize,
//			double scale, double delta, int borderType);
//		HINSTANCE Hint = LoadLibraryA("../Laplacian/target/laplacian.dll");//load dll path
//		c myLap = (c)GetProcAddress(Hint, "myLaplacian");
//		myLap(_src, _dst, ddepth, ksize, scale, delta, borderType);
//	}
//	//sobel
//	virtual void mySobel(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int dx, int dy,
//		int ksize = 3, double scale = 1, double delta = 0, int borderType = cv::BORDER_DEFAULT) {
//		typedef void(*c)(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int dx, int dy,
//			int ksize, double scale, double delta, int borderType);
//		HINSTANCE Hint = LoadLibraryA("../Sobel/target/sobel.dll");//load dll path
//		c mySobel = (c)GetProcAddress(Hint, "mySobel");
//		mySobel(_src, _dst, ddepth, dx, dy, ksize, scale, delta, borderType);
//	}
//
//};
//
//
//class Methods {
//public:
//	//kmeans
//	virtual void myKmeans(cv::InputArray _data, int K, cv::InputOutputArray _bestLabels, cv::TermCriteria criteria, int attempts, int flags, cv::OutputArray _centers) {
//		typedef double(*c) (cv::InputArray _data, int K, cv::InputOutputArray _bestLabels, cv::TermCriteria criteria, int attempts, int flags, cv::OutputArray _centers);
//
//		HINSTANCE Hint = LoadLibraryA("../Kmeans/target/Kmeans.dll");
//		c myKmeans = (c)GetProcAddress(Hint, "myKmeans");
//
//		myKmeans(_data, K, _bestLabels, criteria, attempts, flags, _centers);
//	}
//
//	//HoughLines
//	virtual void myHoughLines(cv::InputArray _image, cv::OutputArray lines, double rho, double theta, int threshold, double srn, double stn, double min_theta, double max_theta) {
//		typedef double(*c) (cv::InputArray _image, cv::OutputArray lines, double rho, double theta, int threshold, double srn, double stn, double min_theta, double max_theta);
//
//		HINSTANCE Hint = LoadLibraryA("..\\Hough\\target\\Hough.dll");
//
//		c HoughLines = (c)GetProcAddress(Hint, "myHoughLines");
//
//		HoughLines(_image, lines, rho, theta, threshold, srn, stn, min_theta, max_theta);
//	}
//
//	//HoughLinesP
//	virtual void myHoughLinesP(cv::InputArray _image, cv::OutputArray _lines, double rho, double theta, int threshold, double minLineLength, double maxGap) {
//		typedef double(*c) (cv::InputArray _image, cv::OutputArray _lines, double rho, double theta, int threshold, double minLineLength, double maxGap);
//
//		HINSTANCE Hint = LoadLibraryA("..\\Hough\\target\\Hough.dll");
//
//		c HoughLines = (c)GetProcAddress(Hint, "myHoughLinesP");
//
//		HoughLines(_image, _lines, rho, theta, threshold, minLineLength, maxGap);
//	}
//
//	//HoughCircles
//	virtual void myHoughCircles(cv::InputArray _image, cv::OutputArray _circles, int method, double dp, double minDist, double param1, double param2, int minRadius, int maxRadius, int maxCircles, double param3) {
//		typedef double(*c) (cv::InputArray _image, cv::OutputArray _circles, int method, double dp, double minDist, double param1, double param2, int minRadius, int maxRadius, int maxCircles, double param3);
//
//		HINSTANCE Hint = LoadLibraryA("..\\Hough\\target\\Hough.dll");
//
//		c HoughCircles = (c)GetProcAddress(Hint, "myHoughCircles");
//
//		HoughCircles(_image, _circles, method, dp, minDist, param1, param2, minRadius, maxRadius, maxCircles, param3);
//	}
//
//	//least square
//	virtual void mySolve(cv::InputArray _src, cv::InputArray _src2arg, cv::OutputArray _dst, int method) {
//		typedef double(*c) (cv::InputArray _src, cv::InputArray _src2arg, cv::OutputArray _dst, int method);
//
//		HINSTANCE Hint = LoadLibraryA("../LeastSquares/target/LeastSquares.dll");
//
//		c mySlove = (c)GetProcAddress(Hint, "mySolve");
//
//		mySlove(_src, _src2arg, _dst, method);
//	}
//
//};
//
//void testAffineTransform() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	HINSTANCE h = LoadLibraryA("AffineTransform.dll");
//	typedef void(*a)(
//		cv::InputArray,
//		cv::OutputArray,
//		cv::InputArray,
//		cv::Size,
//		int,
//		int,
//		const cv::Scalar&
//		);
//	typedef cv::Mat(*b)(
//		cv::Point2f,
//		double,
//		double
//		);
//	typedef cv::Mat(*c)(
//		const cv::Point2f[],
//		const cv::Point2f[]
//		);
//	a warpAffineI = (a)GetProcAddress(h, "warpAffineI");
//	b getRotationMatrix2DI = (b)GetProcAddress(h, "getRotationMatrix2DI");
//	c getAffineTransformI = (c)GetProcAddress(h, "getAffineTransformI");
//
//	cv::Mat src = myread("foo.png", 0);
//
//	cv::Point2f srcTri[3];
//	srcTri[0] = cv::Point2f(0.f, 0.f);
//	srcTri[1] = cv::Point2f(src.cols - 1.f, 0.f);
//	srcTri[2] = cv::Point2f(0.f, src.rows - 1.f);
//
//	cv::Point2f dstTri[3];
//	dstTri[0] = cv::Point2f(0.f, src.rows*0.33f);
//	dstTri[1] = cv::Point2f(src.cols*0.85f, src.rows*0.25f);
//	dstTri[2] = cv::Point2f(src.cols*0.15f, src.rows*0.7f);
//
//	cv::Mat warp_mat = getAffineTransformI(srcTri, dstTri);
//	cv::Mat warp_dst = cv::Mat::zeros(src.rows, src.cols, src.type());
//
//	warpAffineI(src, warp_dst, warp_mat, warp_dst.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());
//	cv::Point center = cv::Point(warp_dst.cols / 2, warp_dst.rows / 2);
//	double angle = -50.0;
//	double scale = 0.6;
//	cv::Mat rot_mat = getRotationMatrix2DI(center, angle, scale);
//	cv::Mat warp_rotate_dst;
//	warpAffineI(warp_dst, warp_rotate_dst, rot_mat, warp_dst.size(),
//		cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());
//
//	mywrite("foo_warp.png", warp_dst);
//	mywrite("foo_warp_rotate.png", warp_rotate_dst);
//}
//
//void testResizeFlipCrop() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat src = myread("foo.png", 0);
//	HINSTANCE h = LoadLibraryA("Resize.dll");
//	typedef void(*a)(
//		cv::InputArray,
//		cv::OutputArray,
//		cv::Size,
//		double,
//		double,
//		int
//		);
//
//	a resizeI = (a)GetProcAddress(h, "resizeI");
//	cv::Mat dst;
//	resizeI(src, dst, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
//	mywrite("foo_resize.png", dst);
//	cv::flip(src, dst, 0);
//	mywrite("foo_resize_flip.png", dst);
//	dst = dst(cv::Range(dst.size().height*0.2, dst.size().height*0.8),
//		cv::Range(dst.size().width*0.2, dst.size().width*0.8));
//	mywrite("foo_resize_flip_crop.png", dst);
//}
//
//void testFillPoly() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat src = myread("foo.png", 0);
//
//	HINSTANCE h = LoadLibraryA("FillPoly.dll");
//	typedef void(*a)(cv::InputOutputArray,
//		cv::InputArrayOfArrays,
//		const cv::Scalar&,
//		int,
//		int,
//		cv::Point);
//	a fillPolyI = (a)GetProcAddress(h, "fillPolyI");
//
//	std::vector<cv::Point> polygon{
//		cv::Point(300, 500),
//		cv::Point(50, 5),
//		cv::Point(500, 500)
//	};
//
//	std::vector<std::vector<cv::Point>> pts{
//		polygon
//	};
//
//	fillPolyI(src, pts, cv::Scalar(), cv::LINE_8, 0, cv::Point());
//
//	mywrite("foo_fillpoly.png", src);
//}
//
//void testStitching() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	std::vector<cv::Mat> imgs{
//		myread("1.png", 0),
//		myread("2.png", 0),
//		myread("3.png", 0),
//	};
//
//	HINSTANCE h = LoadLibraryA("Stitching.dll");
//	typedef cv::Ptr<cv::Stitcher>(*a)(
//		cv::Stitcher::Mode,
//		bool
//		);
//	a createStitcher = (a)GetProcAddress(h, "createStitcher");
//	cv::Ptr<cv::Stitcher> stitcher = createStitcher(cv::Stitcher::PANORAMA, false);
//
//	cv::Mat pano;
//	cv::Stitcher::Status status = stitcher->stitch(imgs, pano);
//	if (status != cv::Stitcher::OK)
//	{
//		cout << "Can't stitch images, error code = " << int(status) << endl;
//		return;
//	}
//	mywrite("pano.png", pano);
//	cout << "stitching completed successfully\n" << "pano.png" << " saved!";
//	getchar();
//}
//
//void testEdgeDetection() {
//	enum ThresholdTypes {
//		THRESH_BINARY = 0, //!< \f[\texttt{dst} (x,y) =  \fork{\texttt{maxval}}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{0}{otherwise}\f]
//		THRESH_BINARY_INV = 1, //!< \f[\texttt{dst} (x,y) =  \fork{0}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{maxval}}{otherwise}\f]
//		THRESH_TRUNC = 2, //!< \f[\texttt{dst} (x,y) =  \fork{\texttt{threshold}}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{src}(x,y)}{otherwise}\f]
//		THRESH_TOZERO = 3, //!< \f[\texttt{dst} (x,y) =  \fork{\texttt{src}(x,y)}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{0}{otherwise}\f]
//		THRESH_TOZERO_INV = 4, //!< \f[\texttt{dst} (x,y) =  \fork{0}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{src}(x,y)}{otherwise}\f]
//		THRESH_MASK = 7,
//		THRESH_OTSU = 8, //!< flag, use Otsu algorithm to choose the optimal threshold value
//		THRESH_TRIANGLE = 16 //!< flag, use Triangle algorithm to choose the optimal threshold value
//	};
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	//test laplacian and sobel
//	//cv::Mat src = myread("", 0);
//	cv::Mat resultSobel;
//	cv::Mat resultLaplacian;
//	EdgeDetection ed;
//
//	//ed.mySobel(src, resultSobel, CV_32F, 1, 0);
//	//ed.myLaplacian(src, resultLaplacian, CV_32F);
//
//	//mywrite("", resultSobel);
//	//mywrite("", resultLaplacian);
//
//
//	//test for threshold
//	cv::Mat src = myread("C:/Users/zwq/Desktop/1.png", 0);
//	cv::Mat result;
//	//typedef double(*threshold) (cv::InputArray _src, cv::OutputArray _dst, double thresh, double maxval, int type);
//	//HINSTANCE hintThreshold = LoadLibraryA("../Threshold/target/threshold.dll");
//	//threshold myThreshold = (threshold)GetProcAddress(hintThreshold, "myThreshold");
//	//myThreshold(src, result, 200, 255, THRESH_BINARY);
//	//mywrite("C:/Users/zwq/Desktop/2.png", result);
//
//
//	//test for adaptiveThreshold
//	typedef double(*adaptivethreshold)(cv::InputArray _src, cv::OutputArray _dst, double maxValue,
//		int method, int type, int blockSize, double delta);
//	HINSTANCE hintAdaptiveThreshold = LoadLibraryA("../AdaptiveThreshold/target/adaptiveThreshold.dll");
//	adaptivethreshold myAdaptiveThreshold = (adaptivethreshold)GetProcAddress(hintAdaptiveThreshold, "myAdaptiveThreshold");
//	//myAdaptiveThreshold(src, result, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, -2);
//	//mywrite("C:/Users/zwq/Desktop/3.png", result);
//
//	//test for ���ڱ�Ե���ָ�
//	ed.myLaplacian(src, resultLaplacian, CV_8U);
//	myAdaptiveThreshold(resultLaplacian, result, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, -2);
//	mywrite("C:/Users/zwq/Desktop/4.png", resultLaplacian);
//}
//
////��̬ѧ��ʴ����
//void testErode() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/psma.PNG", 0);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef void(*erode)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
//		cv::Point anchor, int iterations,
//		int borderType, const cv::Scalar& borderValue);
//	HINSTANCE erodeDll = LoadLibraryA("../Erode/target/erode.dll");
//	//HINSTANCE erodeDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/erode/bin/Release/opencv_imgproc343.dll");
//	if (erodeDll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	erode myErode = (erode)GetProcAddress(erodeDll, "myErode");
//
//	if (myErode == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//	//typedef cv::Mat(*getStructuringElement)(int shape, cv::Size ksize, cv::Point anchor);
//	//getStructuringElement myGetStructuringElement=(getStructuringElement)GetProcAddress(erodeDll, "getStructuringElement");
//	//cv::Mat element = myGetStructuringElement(cv::MORPH_RECT, cv::Size(25, 25), cv::Point(-1, -1));
//
//	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
//	myErode(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
//	mywrite("C:/Users/14839/Desktop/erodeRes.png", result);
//
//}
//
////��̬ѧ���Ͳ���
//void testDilate() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/psma.PNG", 0);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef void(*erode)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
//		cv::Point anchor, int iterations,
//		int borderType, const cv::Scalar& borderValue);
//	HINSTANCE erodeDll = LoadLibraryA("../Dilate/target/dilate.dll");
//	//HINSTANCE erodeDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/dilate/bin/Release/opencv_imgproc343.dll");
//	if (erodeDll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	erode myDilate = (erode)GetProcAddress(erodeDll, "myDilate");
//
//	if (myDilate == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
//	myDilate(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
//	mywrite("C:/Users/14839/Desktop/dilateRes.png", result);
//
//}
//
////��̬ѧ������
//void testOpen() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/open.PNG", 0);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef void(*open)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
//		cv::Point anchor, int iterations,
//		int borderType, const cv::Scalar& borderValue);
//	HINSTANCE openDll = LoadLibraryA("../Open/target/open.dll");
//	//HINSTANCE openDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
//	if (openDll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	open myOpen = (open)GetProcAddress(openDll, "myOpen");
//
//	if (myOpen == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
//	myOpen(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
//	mywrite("C:/Users/14839/Desktop/openRes.png", result);
//
//}
//
////��̬ѧ������
//void testClose() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/close.PNG", 0);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef void(*close)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
//		cv::Point anchor, int iterations,
//		int borderType, const cv::Scalar& borderValue);
//	HINSTANCE closeDll = LoadLibraryA("../Close/target/close.dll");
//	//HINSTANCE closeDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
//	if (closeDll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	close myClose = (close)GetProcAddress(closeDll, "myClose");
//
//	if (myClose == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
//	myClose(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
//	mywrite("C:/Users/14839/Desktop/closeRes.png", result);
//
//}
//
////��̬ѧ�ݶ�����
//void testGradient() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/psma.PNG", 0);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef void(*gradient)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
//		cv::Point anchor, int iterations,
//		int borderType, const cv::Scalar& borderValue);
//	HINSTANCE Dll = LoadLibraryA("../Gradient/target/gradient.dll");
//	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
//	if (Dll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	gradient myGradient = (gradient)GetProcAddress(Dll, "myGradient");
//
//	if (myGradient == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
//	myGradient(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
//	mywrite("C:/Users/14839/Desktop/gradientRes.png", result);
//
//}
//
////��̬ѧ��ñ����
//void testBlackhat() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/open.PNG", 0);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef void(*blackhat)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
//		cv::Point anchor, int iterations,
//		int borderType, const cv::Scalar& borderValue);
//	HINSTANCE Dll = LoadLibraryA("../Blackhat/target/blackhat.dll");
//	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
//	if (Dll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	blackhat myBlackhat = (blackhat)GetProcAddress(Dll, "myBlackhat");
//
//	if (myBlackhat == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
//	myBlackhat(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
//	mywrite("C:/Users/14839/Desktop/blackhatRes.png", result);
//
//}
//
////��̬ѧ��ñ����
//void testTophat() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/close.PNG", 0);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef void(*tophat)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
//		cv::Point anchor, int iterations,
//		int borderType, const cv::Scalar& borderValue);
//	HINSTANCE Dll = LoadLibraryA("../Tophat/target/tophat.dll");
//	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
//	if (Dll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	tophat myTophat = (tophat)GetProcAddress(Dll, "myTophat");
//
//	if (myTophat == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
//	myTophat(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
//	mywrite("C:/Users/14839/Desktop/tophatRes.png", result);
//
//}
//
//
//
////���Ի�������������ͼ��ָ�
//void testRegionGrow() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	HINSTANCE hintRegionGrow = LoadLibraryA("../RegionGrow/target/regionGrow.dll");
//	typedef cv::Mat(*func) (cv::Mat src, cv::Point2i pt, int th);
//	func reginGrow = (func)GetProcAddress(hintRegionGrow, "RegionGrow");
//
//	cv::Mat src, result;
//	cv::Point point(200, 200);
//	int th = 10;
//	src = myread("C:\\Users\\zwq\\Desktop\\s.jpg", 0);
//
//	result = reginGrow(src, point, th);
//	mywrite("C:\\Users\\zwq\\Desktop\\s_regionGrow.jpg", result);
//
//	
//}
//
//
//void testGrabcCut() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	HINSTANCE hintGrabCut = LoadLibraryA("../GrabCut/target/grabCut.dll");
//	typedef void(*func) (cv::InputArray _img, cv::InputOutputArray _mask, cv::Rect rect, cv::InputOutputArray _bgdModel, cv::InputOutputArray _fgdModel, int iterCount, int mode);
//	func myGrabCut = (func)GetProcAddress(hintGrabCut, "myGrabCut");
//
//	Mat src = myread("img/grabcut.png", 1);
//	Mat mask, bgModel, fgModel, result;
//	mask.create(src.size(), CV_8U);
//	Point p1(50, 10), p2(410, 630);
//	Rect rect = Rect(p1, p2);
//
//	mask.setTo(Scalar::all(GC_BGD));//����
//	mask(rect).setTo(Scalar(GC_PR_FGD));//ǰ��	
//
//
//	bool init = false;
//	int count = 4;
//	while (count--)
//	{
//		if (init)//��갴�£�init��Ϊfalse
//			grabCut(src, mask, rect, bgModel, fgModel, 1, GC_EVAL);//�ڶ��ε�������mask��ʼ��grabcut
//		else
//		{
//			grabCut(src, mask, rect, bgModel, fgModel, 1, GC_INIT_WITH_RECT);//�þ��δ���ʼ��GrabCut
//			init = true;
//		}
//
//		Mat binmask;
//		binmask = mask & 1;				//��һ����Ĥ
//		if (init)						//��һ���ٳ���Ч������갴�£�init��Ϊfalse
//		{
//			src.copyTo(result, binmask);
//		}
//		else
//		{
//			result = src.clone();
//		}
//		rectangle(result, rect, Scalar(0, 0, 255), 2, 8);
//
//	}
//	mywrite("img/grabcut_result.png", result);
//}
//
//
////Harris�ǵ���
//void testHarris() {
//	//prepare for read and write
//}
//
//void testFAST() {
//
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//
//	Mat img = myread("img/lena.jpg", 1);
//	if (!img.data)
//	{
//		//cout << "��ȡͼ�������ȷ��ͼ���ļ��Ƿ���ȷ" << endl;
//	}
//
//	Mat gray = myread("1.jpg", 0);
//
//	HINSTANCE Dll = LoadLibraryA("../Harris/target/Harris.dll");
//	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
//	if (Dll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	typedef void(*myCornerHarrisPointer)(cv::InputArray src, cv::OutputArray dst, int blockSize, int ksize, double k, int borderType);
//	myCornerHarrisPointer myCornerHarris = (myCornerHarrisPointer)GetProcAddress(Dll, "myCornerHarris");
//	//����Harrisϵ��
//	Mat harris;
//	int blockSize = 2;     // ����뾶
//	int apertureSize = 3;  // �����С
//	myCornerHarris(gray, harris, blockSize, apertureSize, 0.04, 4);
//	//��һ�����ڽ�����ֵ�ȽϺͽ����ʾ
//	Mat harrisn;
//	normalize(harris, harrisn, 0, 255, NORM_MINMAX);
//	//��ͼ����������ͱ��CV_8U
//	convertScaleAbs(harrisn, harrisn);
//	//Ѱ��Harris�ǵ㲢��ʾͼ��
//	Mat resultimg = img.clone();
//
//	mywrite("img/harris.jpg", harris);
//	mywrite("img/harrisn.jpg", harrisn);
//}
//
////ͼ������
//void testImgOperation() {
//
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	//prepare for read and write
//	cv::Mat src = myread("foo.png", 0);
//	HINSTANCE h = LoadLibraryA("Fast.dll");
//	typedef void(*a)(
//		cv::InputArray,
//		std::vector<cv::KeyPoint>&,
//		int,
//		bool,
//		int
//		);
//	a FASTI = (a)GetProcAddress(h, "FASTI");
//
//	std::vector<cv::KeyPoint> keypoints;
//	FASTI(src, keypoints, 1, true, 1);
//}
//
//void testBRISK() {
//
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//
//	ImgOperator imgOperationer;
//
//	Mat img1 = myread("1.jpg", 1);
//	Mat img2 = myread("2.jpg", 1);
//	Mat result;
//	Mat result2;
//	//ͼ��ӷ�
//	imgOperationer.my_Add(img1, img2, result);
//	mywrite("result_add.jpg", result);
//	//ͼ�����
//	imgOperationer.my_Subtract(result, img1, result2);
//	mywrite("result_substract.jpg", result2);
//	//���Բ�
//	Mat img4 = myread("1.jpg", 0);
//	Mat img5 = myread("2.jpg", 0);
//
//	Mat result_absdiff;
//	imgOperationer.my_Absdiff(img4, img5, result_absdiff);
//	mywrite("result_absdiff.jpg", result_absdiff);
//	//ͼ���Ӧ���صļӷ�
//	Mat result3 = img1 + Scalar(50, 50, 50);
//	mywrite("result_add_light.jpg", result3);
//	//ͼ��λ����
//	//��λ��
//	Mat result_bitwise_not;
//	imgOperationer.my_Bitwise_not(img1, result_bitwise_not);
//	mywrite("result_bitwise_not.jpg", result_bitwise_not);
//	//��λ��
//	Mat result_bitwise_and;
//	imgOperationer.my_Bitwise_and(img1, img2, result_bitwise_and);
//	mywrite("result_bitwise_and.jpg", result_bitwise_and);
//	//��λ���
//	Mat result_bitwise_xor;
//	imgOperationer.my_Bitwise_xor(img1, img2, result_bitwise_xor);
//	mywrite("result_bitwise_xor.jpg", result_bitwise_xor);
//	//���ֵ��Сֵ
//	double minVal = 0.0;
//	double maxVal = 0.0;
//	imgOperationer.my_MinMaxLoc(img1, &minVal, &maxVal);
//	std::cout << minVal << "," << maxVal << std::endl;
//	//��ֵ
//	Scalar mean;
//	mean = imgOperationer.my_Mean(cv::mean(img1));
//	std::cout << mean[0] << "-" << mean[1] << "-" << mean[2] << std::endl;
//
//	//��һ��
//	Mat result_normalize;
//	imgOperationer.my_Normalize(img1, result_normalize);
//	mywrite("result_normalize.jpg", result_normalize);
//
//	//dft
//	Mat result_dft;
//	Mat img3 = myread("1.jpg", 0);
//	Mat tmp;
//	img3.convertTo(tmp, CV_64F);
//
//	imgOperationer.my_Dft(tmp, result_dft);
//	mywrite("result_dft.jpg", result_dft);
//
//
//	cv::Mat src = myread("foo.png", 0);
//	HINSTANCE h = LoadLibrary("Brisk.dll");
//	typedef cv::Ptr<cv::BRISK>(*a)(
//		int,
//		int,
//		float
//		);
//
//	a createBRISK = (a)GetProcAddress(h, "createBRISK");
//	cv::Ptr<cv::BRISK> b = createBRISK(30, 3, 1.0f);
//	std::vector<cv::KeyPoint> keypoints;
//
//	b->detect(src, keypoints);
//
//}
//
////�����ȶ�
////������״����ģ��
//vector<Point> ImageTemplateContours(Mat img_template)
//{
//	//�ҶȻ�
//	Mat gray_img_template = img_template;//����ʱ������ǻҶ�ͼ
//
//	//��ֵ�ָ�
//	Mat thresh_img_template;
//	threshold(gray_img_template, thresh_img_template, 0, 255, THRESH_OTSU);
//	//���ʹ���
//	Mat ellipse = getStructuringElement(MORPH_ELLIPSE, Size(15, 15));
//	Mat erode_img_template;
//	erode(thresh_img_template, erode_img_template, ellipse);
//	morphologyEx(thresh_img_template, thresh_img_template, MORPH_OPEN, ellipse, Point(-1, -1), 1);
//
//	//Ѱ�ұ߽�
//	vector<vector<Point>> contours_template;
//	vector<Vec4i> hierarchy;
//	findContours(thresh_img_template, contours_template, hierarchy, RETR_LIST, CHAIN_APPROX_NONE, Point());
//
//	//���Ʊ߽�
//	drawContours(img_template, contours_template, 0, Scalar(0, 0, 255), 1, 8, hierarchy);
//
//
//	return contours_template[0];
//}
////������״ģ��ƥ��
//vector<Point2d> ShapeTemplateMatch(Mat image, vector<Point> imgTemplatecontours, double minMatchValue)
//{
//	vector<Point2d> image_coordinates;
//	//�ҶȻ�
//	Mat gray_img = image;//����ʱ������ǻҶ�ͼ
//
//	//��ֵ�ָ�
//	Mat thresh_img;
//	threshold(gray_img, thresh_img, 0, 255, THRESH_OTSU);
//
//	//Ѱ�ұ߽�
//	vector<vector<Point>> contours_img;
//	vector<Vec4i> hierarchy;
//	findContours(thresh_img, contours_img, hierarchy, RETR_LIST, CHAIN_APPROX_NONE, Point());
//	//������״ģ�����ƥ��
//	int min_pos = -1;
//	double	min_value = minMatchValue;//ƥ���ֵ��С�ڸ�ֵ��ƥ��ɹ�
//	for (int i = 0; i < contours_img.size(); i++)
//	{
//		//�������������ɸѡ��һЩû��Ҫ��С����
//		if (contourArea(contours_img[i]) > 12000)
//		{
//			//�õ�ƥ���ֵ 
//			double value = matchShapes(contours_img[i], imgTemplatecontours, CONTOURS_MATCH_I3, 0.0);
//			//��ƥ���ֵ���趨��ֵ���бȽ� 
//			if (value < min_value)
//			{
//				min_pos = i;
//				//����Ŀ��߽�
//				drawContours(image, contours_img, min_pos, Scalar(0, 0, 255), 1, 8, hierarchy, 0);
//
//				//��ȡ���ĵ�
//				Moments M;
//				M = moments(contours_img[min_pos]);
//				double cX = double(M.m10 / M.m00);
//				double cY = double(M.m01 / M.m00);
//				//��ʾĿ�����Ĳ���ȡ�����
//				circle(image, Point2d(cX, cY), 1, Scalar(0, 255, 0), 2, 8);
//				//putText(image, "center", Point2d(cX - 20, cY - 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1, 8);
//							//��Ŀ����������궼���������� 
//				image_coordinates.push_back(Point2d(cX, cY));//�������д�ŵ������
//			}
//		}
//	}
//	return image_coordinates;
//}
////����
//void testMatchContourShape() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat img_template = myread(".\\img\\lookForCircles.png", 0);
//	vector<Point> imgTemplatecontours = ImageTemplateContours(img_template);
//	vector<Point2d> image_coordinates = ShapeTemplateMatch(img_template, imgTemplatecontours, 0.3);
//	std::cout << image_coordinates[0].x << std::endl;
//}
//
//
//void testKmeans() {
//
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//
//	cv::Mat src = myread("img/test.jpg", 1);
//
//	cv::Scalar colorTab[] = {
//		cv::Scalar(0,0,255),
//		cv::Scalar(0,255,0),
//		cv::Scalar(255,0,0),
//		cv::Scalar(0,255,255),
//		cv::Scalar(255,0,255),
//		cv::Scalar(255,255,0)
//	};
//
//	int width = src.cols;
//	int height = src.rows;
//	int dims = src.channels();
//
//	// ��ʼ������
//	int sampleCount = width * height;
//	int clusterCount = 3;
//	cv::Mat points(sampleCount, dims, CV_32F, cv::Scalar(10));
//	cv::Mat labels;
//	cv::Mat centers(clusterCount, 1, points.type());
//
//	// RGB ����ת������������
//	int index = 0;
//	for (int row = 0; row < height; row++) {
//		cv::Vec3b* bgr_ptr = src.ptr<cv::Vec3b>(row);
//		for (int col = 0; col < width; col++) {
//			index = row * width + col;
//			points.at<float>(index, 0) = static_cast<int>(bgr_ptr[col][0]);
//			points.at<float>(index, 1) = static_cast<int>(bgr_ptr[col][1]);
//			points.at<float>(index, 2) = static_cast<int>(bgr_ptr[col][2]);
//		}
//	}
//
//	// ִ��K-means����
//	cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 0.1);
//
//	Methods methods;
//	methods.myKmeans(points, clusterCount, labels, criteria, 3, cv::KMEANS_PP_CENTERS, centers);
//
//	// ��ʾͼ��ָ���
//	cv::Mat result = cv::Mat::zeros(src.size(), src.type());
//	for (int row = 0; row < height; row++) {
//		cv::Vec3b* result_ptr = result.ptr<cv::Vec3b>(row);
//		for (int col = 0; col < width; col++) {
//			index = row * width + col;
//			int label = labels.at<int>(index, 0);
//			result_ptr[col][0] = colorTab[label][0];
//			result_ptr[col][1] = colorTab[label][1];
//			result_ptr[col][2] = colorTab[label][2];
//		}
//	}
//
//	//�����������
//	for (int i = 0; i < centers.rows; i++) {
//		int x = centers.at<float>(i, 0);
//		int y = centers.at<float>(i, 1);
//		std::cout << "[x, y] = " << x << ", " << y << std::endl;
//		// ��������Ϊ����ɫ������
//		//[x, y] = 194, 211
//		//[x, y] = 49, 43
//		//[x, y] = 146, 149
//	}
//
//
//	mywrite("img/test_kmeans.jpg", result);
//
//	if (Hint_wr != NULL)
//	{
//		FreeLibrary(Hint_wr);
//		Hint_wr = NULL;
//	}
//}
//
//void testHough() {
//
//
//	//���ֱ��
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat srcImage = imread("img/rectangle.jpg", 0);
//
//	Mat mid, dst, dst1;
//	//����Դ������ģ������˸�������
//	Canny(srcImage, mid, 100, 200, 3);
//	cv::cvtColor(mid, dst, COLOR_GRAY2BGR);
//	cv::cvtColor(mid, dst1, COLOR_GRAY2BGR);
//
//	//��3�����л����߱任
//	vector<Vec2f> lines;//����һ��ʸ���ṹlines���ڴ�ŵõ����߶�ʸ������
//	vector<Vec4f> lines1;
//	Methods methods;
//	methods.myHoughLines(mid, lines, 1, CV_PI / 180, 150, 0, 0, 0, CV_PI);
//
//	//��4��������ͼ�л��Ƴ�ÿ���߶�
//	for (size_t i = 0; i < lines.size(); i++)
//	{
//		float rho = lines[i][0], theta = lines[i][1];
//		Point pt1, pt2;
//		double a = cos(theta), b = sin(theta);
//		double x0 = a * rho, y0 = b * rho;
//		pt1.x = cvRound(x0 + 1000 * (-b));
//		pt1.y = cvRound(y0 + 1000 * (a));
//		pt2.x = cvRound(x0 - 1000 * (-b));
//		pt2.y = cvRound(y0 - 1000 * (a));
//
//		//����Դ�������line�����������˸�������
//		cv::line(dst, pt1, pt2, Scalar(0, 245, 0), 1, LINE_AA);
//	}
//	imwrite("img/rectangle_houghlines.jpg", dst);
//
//	methods.myHoughLinesP(mid, lines1, 1, CV_PI / 180, 10, 0, 10);
//	//5. ��ʾ��⵽��ֱ��
//	Scalar color = Scalar(0, 0, 255);//������ɫ
//	for (size_t i = 0; i < lines1.size(); i++)
//	{
//		Vec4f hline = lines1[i];
//		cv::line(dst1, Point(hline[0], hline[1]), Point(hline[2], hline[3]), color, 3, LINE_AA);//����ֱ��
//	}
//
//	imwrite("img/rectangle_houghlinesP.jpg", dst1);
//
//
//	//���Բ��
//	cv::Mat src = imread("img/circle.jpg", 1);
//	cv::Mat src_gray;
//	/// Convert it to gray
//	cv::cvtColor(src, src_gray, CV_BGR2GRAY);
//
//	vector<Vec3f> circles;
//
//	/// Apply the Hough Transform to find the circles
//	//HoughCircle(_image, _circles, method, dp, minDist, param1, param2, minRadius, maxRadius, maxCircles, param3);
//	methods.myHoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows / 8, 200, 100, 0, 0, -1, 3);
//
//	/// Draw the circles detected
//	for (size_t i = 0; i < circles.size(); i++)
//	{
//		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
//		int radius = cvRound(circles[i][2]);
//		// circle center
//		circle(src, center, 3, Scalar(0, 255, 0), -1, 8, 0);
//		// circle outline
//		circle(src, center, radius, Scalar(0, 0, 255), 3, 8, 0);
//	}
//	imwrite("img/circle_houghcircle.png", src);
//
//
//	if (Hint_wr != NULL)
//	{
//		FreeLibrary(Hint_wr);
//		Hint_wr = NULL;
//	}
//}
//
//void testLeastSquares() {
//
//
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	vector<Point>points;
//	//(27 39) (8 5) (8 9) (16 22) (44 71) (35 44) (43 57) (19 24) (27 39) (37 52)
//
//	points.push_back(Point(27, 39));
//	points.push_back(Point(8, 5));
//	points.push_back(Point(8, 9));
//	points.push_back(Point(16, 22));
//	points.push_back(Point(44, 71));
//	points.push_back(Point(35, 44));
//	points.push_back(Point(43, 57));
//	points.push_back(Point(19, 24));
//	points.push_back(Point(27, 39));
//	points.push_back(Point(37, 52));
//	Mat src = Mat::zeros(400, 400, CV_8UC3);
//
//	for (int i = 0; i < points.size(); i++)
//	{
//		//��ԭͼ�ϻ�����
//		circle(src, points[i], 3, Scalar(0, 0, 255), 1, 8);
//	}
//	//����A���� 
//	int N = 2;
//	Mat A = Mat::zeros(N, N, CV_64FC1);
//
//	for (int row = 0; row < A.rows; row++)
//	{
//		for (int col = 0; col < A.cols; col++)
//		{
//			for (int k = 0; k < points.size(); k++)
//			{
//				A.at<double>(row, col) = A.at<double>(row, col) + pow(points[k].x, row + col);
//			}
//		}
//	}
//	//����B����
//	Mat B = Mat::zeros(N, 1, CV_64FC1);
//	for (int row = 0; row < B.rows; row++)
//	{
//
//		for (int k = 0; k < points.size(); k++)
//		{
//			B.at<double>(row, 0) = B.at<double>(row, 0) + pow(points[k].x, row)*points[k].y;
//		}
//	}
//	//A*X=B
//	Mat X;
//	//cout << A << endl << B << endl;
//
//	Methods methods;
//
//	methods.mySolve(A, B, X, DECOMP_LU);
//	cout << X << endl;
//	vector<Point>lines;
//	for (int x = 0; x < src.size().width; x++)
//	{				// y = b + ax;
//		double y = X.at<double>(0, 0) + X.at<double>(1, 0)*x;
//		printf("(%d,%lf)\n", x, y);
//		lines.push_back(Point(x, y));
//	}
//	polylines(src, lines, false, Scalar(255, 0, 0), 1, 8);
//	imwrite("img/test_leastSquare.jpg", src);
//}
//
//void detectLineWithHough() {
//
//	//���ֱ��
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat srcImage = imread("img/polygon.png", 1);
//
//	Mat mid, dst;
//	//����Դ������ģ������˸�������
//	Canny(srcImage, mid, 100, 200, 3);
//	cvtColor(mid, dst, COLOR_GRAY2BGR);
//
//	//��3�����л����߱任
//	vector<Vec2f> lines;//����һ��ʸ���ṹlines���ڴ�ŵõ����߶�ʸ������
//	Methods methods;
//	methods.myHoughLines(mid, lines, 1, CV_PI / 180, 150, 0, 0, 0, CV_PI);
//
//	//��4��������ͼ�л��Ƴ�ÿ���߶�
//	for (size_t i = 0; i < lines.size(); i++)
//	{
//		float rho = lines[i][0], theta = lines[i][1];
//		Point pt1, pt2;
//		double a = cos(theta), b = sin(theta);
//		double x0 = a * rho, y0 = b * rho;
//		pt1.x = cvRound(x0 + 1000 * (-b));
//		pt1.y = cvRound(y0 + 1000 * (a));
//		pt2.x = cvRound(x0 - 1000 * (-b));
//		pt2.y = cvRound(y0 - 1000 * (a));
//
//		//����Դ�������line�����������˸�������
//		cv::line(dst, pt1, pt2, Scalar(0, 245, 0), 1, LINE_AA);
//	}
//	imwrite("img/polygon_houghlines.jpg", dst);
//
//	if (Hint_wr != NULL)
//	{
//		FreeLibrary(Hint_wr);
//		Hint_wr = NULL;
//	}
//}
//
//void testCvColor() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	//׼��cvtColor����
//	HINSTANCE hintCvtColor = LoadLibraryA("../CvtColor/target/cvtColor.dll");
//	typedef void(*func) (cv::InputArray _src, cv::OutputArray _dst, int code, int dcn);
//	func myCvtColor = (func)GetProcAddress(hintCvtColor, "myCvtColor");
//	//����RGBת�Ҷ�ͼ
//	//��һ����ɫͼ��
//	enum ColorConversionCodes {
//		COLOR_RGB2GRAY = 7,
//		COLOR_RGB2HSV = 41,
//		COLOR_RGB2YUV = 83
//	};
//	Mat src = myread("img/1.png", 1);
//	Mat result;
//	myCvtColor(src, result, COLOR_RGB2HSV, 0);
//	mywrite("img/RGB2HSV.png", result);
//
//}
//void testBlurDetection() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat srcImage = imread("img/polygon.png", 0);
//
//	//�����ȼ��
//	BlurDetectionByStd BlurDectOperator;
//	//double result = BlurDectOperator.getBulrDetectionScore(srcImage);
//	stringstream meanValueStream;
//	//meanValueStream << result;
//	string meanValueString;
//	meanValueStream >> meanValueString;
//	cout << "Articulation(Variance Method) of image" + meanValueString << endl;
//}
//
//
////��ɫת��RGBתHSI
//void testRgb2hsi() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("C:/MyProgram/opencv/mywr/bin/Release/opencv_imgcodecs343.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	HINSTANCE hint = LoadLibraryA("C:\\Users\\zwq\\Desktop\\API\\Rgb2hsi\\target\\rgb2hsi.dll");
//	typedef void(*func)(Mat &image, Mat &hsi);
//	func rgb2hsi = (func)GetProcAddress(hint, "rgb2hsi");
//
//	Mat src = myread("C:\\Users\\zwq\\Desktop\\API\\Test\\img\\1.png", 1);
//	Mat result;
//	//result = Mat(Size(src.rows, src.cols), CV_8UC3);
//	result = Mat(Size(src.rows, src.cols), src.type());
//	rgb2hsi(src, result);
//	mywrite("C:\\Users\\zwq\\Desktop\\API\\Test\\img\\1_hsi.png", result);
//}
////��ɫ��ȡ
//void testInRange() {
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("C:/MyProgram/opencv/mywr/bin/Release/opencv_imgcodecs343.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	HINSTANCE hint = LoadLibraryA("C:\\Users\\zwq\\Desktop\\API\\InRange\\target\\inRange.dll");
//	typedef void(*func)(InputArray src, InputArray lowerb, InputArray upperb, OutputArray dst);
//	func myInRange = (func)GetProcAddress(hint, "myInRange");
//
//	Mat src = myread("C:\\Users\\zwq\\Desktop\\API\\Test\\img\\1.png", 1);
//	Mat dstImage;
//	cout << src.size();
//	inRange(src, Scalar(75, 75, 75), Scalar(85, 85, 85), dstImage);
//	mywrite("C:\\Users\\zwq\\Desktop\\API\\Test\\img\\1_range.png", dstImage);
//}
//
//void testDetectPolygon() {
//
//	//���ֱ��
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat srcImage = imread("img/poly_closed.png", 1);
//
//	Mat dst = Mat::zeros(srcImage.size(), CV_8UC3);
//
//	vector<vector<float>> lines;
//
//	HINSTANCE Hint_poly = LoadLibraryA("../PolygonDetect/target/PolygonDetect.dll");
//	typedef void(*a) (Mat& srcImg, double epsilon, int minAcreage, vector<vector<float>>& lines, Mat& dst);
//
//	a polygonDetect = (a)GetProcAddress(Hint_poly, "polygoDetect");
//
//	polygonDetect(srcImage, 8, 2000, lines, dst);
//
//	imwrite("img/polygon_houghlines.jpg", dst);
//
//	FreeLibrary(Hint_wr);
//	Hint_wr = NULL;
//	FreeLibrary(Hint_poly);
//	Hint_poly = NULL;
//}
//
//void testDetectParaLines() {
//
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//
//	Mat srcImage = imread("img/poly_unclosed.png", 1);
//
//	Mat mid, dst;
//	//����Դ������ģ������˸�������
//	Canny(srcImage, mid, 100, 200, 3);
//	dst = Mat::zeros(srcImage.size(), CV_8UC3);
//
//	//��3�����л����߱任
//	vector<Vec2f> lines;//����һ��ʸ���ṹlines���ڴ�ŵõ����߶�ʸ������
//
//	Methods methods;
//	/*
//	��һ��������InputArray���͵�image������ͼ�񣬼�Դͼ����Ϊ8λ�ĵ�ͨ��������ͼ�񣬿��Խ������Դͼ����������ɺ����޸ĳɴ˸�ʽ�����������
//	�ڶ���������InputArray���͵�lines����������HoughLines�����󴢴��˻����߱任��⵽���������ʸ����ÿһ�����ɾ�������Ԫ�ص�ʸ����ʾ�����У���������ԭ��((0,0)��Ҳ����ͼ������Ͻǣ��ľ��롣 �ǻ���������ת�Ƕȣ�0~��ֱ�ߣ���/2~ˮƽ�ߣ���
//	������������double���͵�rho��������Ϊ��λ�ľ��뾫�ȡ���һ�����ݷ�ʽ��ֱ������ʱ�Ľ����ߴ�ĵ�λ�뾶��PS:Latex��/rho�ͱ�ʾ ��
//	���ĸ�������double���͵�theta���Ի���Ϊ��λ�ĽǶȾ��ȡ���һ�����ݷ�ʽ��ֱ������ʱ�Ľ����ߴ�ĵ�λ�Ƕȡ�
//	�����������int���͵�threshold���ۼ�ƽ�����ֵ��������ʶ��ĳ����Ϊͼ�е�һ��ֱ��ʱ�����ۼ�ƽ���б���ﵽ��ֵ��������ֵthreshold���߶βſ��Ա����ͨ�������ص�����С�
//	������������double���͵�srn����Ĭ��ֵ0�����ڶ�߶ȵĻ���任�����ǵ��������������ߴ�rho�ĳ������롣���Ե��ۼ��������ߴ�ֱ���ǵ���������rho������ȷ���ۼ��������ߴ�Ϊrho/srn��
//	���߸�������double���͵�stn����Ĭ��ֵ0�����ڶ�߶Ȼ���任��srn��ʾ���ĸ����������ߴ�ĵ�λ�Ƕ�theta�ĳ������롣�����srn��stnͬʱΪ0���ͱ�ʾʹ�þ���Ļ���任����������������Ӧ�ö�Ϊ������
//	�ڰ˸�������double���͵� min_theta�����ڱ�׼�Ͷ�߶�Hough�任�������������С�Ƕȡ��������0��max_theta֮�䡣
//	�ھŸ�������double���͵� max_theta, ���ڱ�׼�Ͷ�߶�Hough�任��������������Ƕȡ��������min_theta��CV_PI֮��.
//ԭ�����ӣ�https://blog.csdn.net/weixin_41906949/article/details/85049238
//	*/
//	methods.myHoughLines(mid, lines, 1, CV_PI / 180, 100, 0, 0, 0, CV_PI);
//
//	//��4��������ͼ�л��Ƴ�ÿ���߶�
//
//	float min_theta = 0.01; //������
//	for (size_t i = 0; i < lines.size(); i++)
//	{
//		for (size_t j = i + 1; j < lines.size(); j++) {
//			Vec2f hlinei = lines[i], hlinej = lines[j];
//			if (abs(hlinei[1] - hlinej[1]) < min_theta) {
//				float rho = hlinei[0], theta = hlinei[1];
//				Point pt1, pt2;
//				double a = cos(theta), b = sin(theta);
//				double x0 = a * rho, y0 = b * rho;
//				pt1.x = cvRound(x0 + 1000 * (-b));
//				pt1.y = cvRound(y0 + 1000 * (a));
//				pt2.x = cvRound(x0 - 1000 * (-b));
//				pt2.y = cvRound(y0 - 1000 * (a));
//				cv::line(dst, pt1, pt2, Scalar(0, 245, 0), 1, LINE_AA);
//
//				rho = hlinej[0];
//				theta = hlinej[1];
//				a = cos(theta);
//				b = sin(theta);
//				x0 = a * rho;
//				y0 = b * rho;
//				pt1.x = cvRound(x0 + 1000 * (-b));
//				pt1.y = cvRound(y0 + 1000 * (a));
//				pt2.x = cvRound(x0 - 1000 * (-b));
//				pt2.y = cvRound(y0 - 1000 * (a));
//				cv::line(dst, pt1, pt2, Scalar(0, 245, 0), 1, LINE_AA);
//
//				cout << hlinei[0] << "," << hlinei[1] << " : " << hlinej[0] << "," << hlinej[1] << endl;
//			}
//		}
//	}
//	imwrite("img/paralle_lines.jpg", dst);
//
//	FreeLibrary(Hint_wr);
//	Hint_wr = NULL;
//}
//
//void pixesStatistic() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat srcImage = imread("img/lena.jpg", 0);
//
//	//ָ������ֵ�͸���ֵ
//	int lowthresHold = 100, highthresHold = 200, lowCount = 0, midCount = 0, highCount = 0;
//	for (int i = 0; i < srcImage.rows; i++) {
//		for (int j = 0; j < srcImage.cols; j++) {
//			if (srcImage.at<int>(i, j) < lowthresHold)
//				lowCount += 1;
//			else if (srcImage.at<int>(i, j) < highthresHold)
//				midCount += 1;
//			else
//				highCount += 1;
//		}
//	}
//
//	cout << lowCount << " " << midCount << " " << highCount << " " << endl;
//}
//
//void testQRdetect() {
//
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r imread = (r)GetProcAddress(Hint_wr, "myread");
//	w imwrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	//cv::Mat image = imread("img/qr_angle.jpg", 1);
//	cv::Mat image = imread("img/barcode.png", 1);
//
//	zbar::ImageScanner scanner;
//	scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
//
//	cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
//	int width = image.cols;
//	int height = image.rows;
//	uchar *raw = (uchar *)image.data;
//	zbar::Image imageZbar(width, height, "Y800", raw, width*height);
//	scanner.scan(imageZbar); //ɨ������  
//	zbar::Image::SymbolIterator symbol = imageZbar.symbol_begin();
//	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
//	{
//		std::cout << "��ѯ����ʧ�ܣ�����ͼƬ��" << std::endl;
//	}
//	for (; symbol != imageZbar.symbol_end(); ++symbol)
//	{
//		std::cout << "���ͣ�" << symbol->get_type_name() << std::endl;
//		std::cout << "���룺" << symbol->get_data() << std::endl;
//	}
//}
//
//// ��˹�˲�����
//void testGaussianBlur() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat src = myread("blur_test.jpg", 0);
//	HINSTANCE h = LoadLibraryA("GaussianBlur.dll");
//	typedef void(*a)(
//		cv::InputArray,
//		cv::OutputArray,
//		cv::Size,
//		double,
//		double,
//		int
//		);
//
//	a GaussianBlurI = (a)GetProcAddress(h, "GaussianBlurI");
//	cv::Mat dst;
//	GaussianBlurI(src, dst, cv::Size(3, 3), 0.5, 0.5, cv::INTER_LINEAR);
//}
//
//// ��ֵ�˲�����
//void testMedianBlur() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat src = myread("blur_test.jpg", 0);
//	HINSTANCE h = LoadLibraryA("MedianBlur.dll");
//	typedef void(*a)(
//		cv::InputArray _src,
//		cv::OutputArray _dst,
//		cv::Size ksize
//		);
//
//	a MedianBlurI = (a)GetProcAddress(h, "MedianBlurI");
//	cv::Mat dst;
//	MedianBlurI(src, dst, cv::Size(3, 3));
//}
//
//// �����˲�����
//void testBoxFilter() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat src = myread("blur_test.jpg", 0);
//	HINSTANCE h = LoadLibraryA("BoxFilter.dll");
//	typedef void(*a)(
//		cv::InputArray _src,
//		cv::OutputArray _dst,
//		int ddepth,
//		cv::Size ksize,
//		cv::Point anchor,
//		bool normalize,
//		int borderType
//		);
//
//	a BoxFilterI = (a)GetProcAddress(h, "BoxFilterI");
//	cv::Mat dst;
//	BoxFilterI(src, dst, -1, cv::Size(3, 3), cv::Point(-1, -1), true, cv::BORDER_DEFAULT);
//}
//
//// ˫���˲�����
//void testBilateralFilter() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat src = myread("blur_test.jpg", 0);
//	HINSTANCE h = LoadLibraryA("BilateralFilter.dll");
//	typedef void(*a)(
//		cv::InputArray _src,
//		cv::OutputArray _dst,
//		int d,
//		double sigmaColor,
//		double sigmaSpace,
//		int borderType
//		);
//
//	a BilateralFilterI = (a)GetProcAddress(h, "BilateralFilterI");
//	cv::Mat dst;
//	BilateralFilterI(src, dst, 9, 50, 25 / 2, cv::BORDER_DEFAULT);
//}
//// ֱ����ϲ���
//void testLineFit() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	HINSTANCE h = LoadLibraryA("LineFit.dll");
//	typedef void(*a)(
//		cv::InputArray _points,  // ��ά��������vector 
//		cv::OutputArray _line,   // ���ֱ��,Vec4f (2d)��Vec6f (3d)��vector 
//		int distType,			 // �������� 
//		double param,			 // �������
//		double reps,			 // ����ľ��Ȳ���  ��ʾֱ�ߵ�ԭ�����ľ��ȣ�����ȡ 0.01����Ϊ0�����Զ�ѡ������ֵ
//		double aeps);			 // �ǶȾ��Ȳ���  ��ʾֱ�߽Ƕȵľ��ȣ�����ȡ 0.01
//
//
//	a LineFitI = (a)GetProcAddress(h, "LineFitI");
//
//	cv::Mat src = myread("blur_test.jpg", 0);
//	if (src.empty()) {
//		std::cout << "Couldn't open image!" << std::endl;
//		return;
//	}
//
//	int width = src.cols; // ��
//	int height = src.rows; // ��
//
//	cv::Mat gray_image, bool_image;
//	cv::cvtColor(src, gray_image, cv::COLOR_RGB2GRAY);
//	cv::threshold(gray_image, bool_image, 0, 255, cv::THRESH_OTSU);
//
//	// ��ȡ��ά�㼯
//	vector<Point> PointSet;
//	Point point_temp;
//	for (int i = 0; i < height; ++i) {
//		for (int j = 0; j < width; ++j) {
//			if (bool_image.at<unsigned char>(i, j) < 255) {
//				point_temp.x = j;
//				point_temp.y = i;
//				PointSet.push_back(point_temp);
//			}
//		}
//	}
//
//	void cv::fitLine(
//		cv::InputArray points, // ��ά��������vector  
//		cv::OutputArray line, // ���ֱ��,Vec4f (2d)��Vec6f (3d)��vector  
//		int distType, // ��������  
//		double param, // �������  
//		double reps, // ����ľ��Ȳ���  ��ʾֱ�ߵ�ԭ�����ľ��ȣ�����ȡ 0.01����Ϊ0�����Զ�ѡ������ֵ
//		double aeps // �ǶȾ��Ȳ���  ��ʾֱ�߽Ƕȵľ��ȣ�����ȡ 0.01
//	);
//
//	// ֱ�����
//	Vec4f Line;
//	LineFitI(PointSet, Line, cv::DIST_L2, 0, 0.01, 0.01);
//}
//
//// ��Բ��ϲ���
//void testEllipseFit() {
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	HINSTANCE h = LoadLibraryA("EllipseFit.dll");
//	typedef cv::RotatedRect(*a)(
//		cv::InputArray _points  // ��ά��������vector 
//		);
//
//	a EllipseFitI = (a)GetProcAddress(h, "EllipseFitI");
//
//
//	cv::Mat src_image = myread("blur_test.jpg", 0);
//	if (src_image.empty())
//	{
//		std::cout << "Couldn't open image!" << std::endl;
//		return;
//	}
//
//	// ����
//	vector<vector<Point>> contours;
//
//	//ʹ��canny������Ե
//	Mat edge_image;
//	cv::Canny(src_image, edge_image, 30, 70);
//
//	//��Ե׷�٣�û�д洢��Ե����֯�ṹ
//	cv::findContours(edge_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
//	Mat cimage = Mat::zeros(edge_image.size(), CV_8UC3);
//
//	for (size_t i = 0; i < contours.size(); i++)
//	{
//		//��ϵĵ�����Ϊ6
//		size_t count = contours[i].size();
//		if (count < 6)
//			continue;
//
//		//��Բ���
//		RotatedRect box = EllipseFitI(contours[i]);
//
//		//�������ȴ���30�����ų����������
//		if (MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height) * 30)
//			continue;
//
//		//����׷�ٳ�������
//		cv::drawContours(cimage, contours, (int)i, Scalar::all(255), 1, 8);
//
//		//������ϵ���Բ
//		cv::ellipse(cimage, box, Scalar(0, 0, 255), 1, CV_AA);
//	}
//}
//
//// ���ȺͶԱȶ�
//void testBrightnessAndContrast()
//{
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/shadow.PNG", 1);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	// clipHistPercent ��֦����ȥ�����صĶ��ٰٷֱȣ�
//	// histSize ������еĻҶ�ֵ�鵽���ķ�Χ
//	// lowhist ��С�ĻҶ�ֵ
//	typedef void(*brightnessandcontrast)(const cv::Mat &src, cv::Mat &dst, float clipHistPercent, int histSize, int lowhist);
//	HINSTANCE Dll = LoadLibraryA("../BrightnessAndContrast/target/brightnessandcontrast.dll");
//	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/BrightnessAndContrast/bin/Release/opencv_imgproc343.dll");
//	if (Dll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	brightnessandcontrast myBrightnessAndContrast = (brightnessandcontrast)GetProcAddress(Dll, "myBrightnessAndContrast");
//
//	if (myBrightnessAndContrast == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	myBrightnessAndContrast(src, result, 0, 255, 55);
//	mywrite("C:/Users/14839/Desktop/brightnessandcontrastRes.png", result);
//}
//
//
////��ӰУ��
//void testShadow()
//{
//
//	//prepare for read and write
//	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
//	typedef cv::Mat(*r) (const char*filename, int flag);
//	typedef void(*w) (const char*filename, cv::Mat result);
//	r myread = (r)GetProcAddress(Hint_wr, "myread");
//	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");
//
//
//	cv::Mat result;
//	cv::Mat src = myread("C:/Users/14839/Desktop/shadow.PNG", 1);
//	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ
//
//	typedef cv::Mat(*shadow)(cv::Mat input, int light);
//	HINSTANCE Dll = LoadLibraryA("../ShadowCorrection/target/shadowcorrection.dll");
//	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
//	if (Dll == NULL)
//	{
//		printf("����dllʧ��\n");
//	}
//	shadow myShadowCorrection = (shadow)GetProcAddress(Dll, "myShadowCorrection");
//
//	if (myShadowCorrection == NULL)
//	{
//		printf("����funcʧ��\n");
//	}
//
//
//	result = myShadowCorrection(src, 10);
//	mywrite("C:/Users/14839/Desktop/shadowRes.png", result);
//}

//��ɫʶ��
void testColorIdentif()
{

	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");


	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/RED.PNG", 1);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // ���νṹ

	typedef void(*colorIdentif)(cv::Mat matSrc);
	//HINSTANCE Dll = LoadLibraryA("../ShadowCorrection/target/shadowcorrection.dll");
	HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/BrightnessAndContrast/bin/Release/opencv_imgproc343.dll");
	if (Dll == NULL)
	{
		printf("����dllʧ��\n");
	}
	colorIdentif myColorIdentif = (colorIdentif)GetProcAddress(Dll, "myColorIdentif");

	if (myColorIdentif == NULL)
	{
		printf("����funcʧ��\n");
	}


	 myColorIdentif(src);
}


int main() {
	//testFillPoly();
	//testResizeFlipCrop();
	//testAffineTransform();
	//testErode();
	//testDilate();
	//testOpen();
	//testClose();
	//testGradient();
	//testBlackhat();
	//testTophat();
	//testGrabcCut();

	//testHarris();
	//testImgOperation();
	//testBlurDetection();

	//testFAST();
	//testBRISK();

	//testMatchContourShape();

	//testKmeans();
	//testHough();
	//detectLineWithHough();
	//testStitching();

	//testDetectPolygon();
	//pixesStatistic();
	//testQRdetect();
	//testDetectParaLines();


	//testGaussianBlur();
	//testBilateralFilter();
	//testBoxFilter();
	//testLineFit();
	//testEllipseFit();

	//testBrightnessAndContrast();
	//testShadow();

	testColorIdentif();
}