#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include<vector>
#include"../ImgOperation/ImgOperator.h"
using namespace cv;



class EdgeDetection {
public:
	//laplacian
	virtual void myLaplacian(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int ksize = 1,
		double scale = 1, double delta = 0, int borderType = cv::BORDER_DEFAULT) {
		typedef void(*c)(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int ksize,
			double scale, double delta, int borderType);
		HINSTANCE Hint = LoadLibraryA("../Laplacian/target/laplacian.dll");//load dll path
		c myLap = (c)GetProcAddress(Hint, "myLaplacian");
		myLap(_src, _dst, ddepth, ksize, scale, delta, borderType);
	}
	//sobel
	virtual void mySobel(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int dx, int dy,
		int ksize = 3, double scale = 1, double delta = 0, int borderType = cv::BORDER_DEFAULT) {
		typedef void(*c)(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int dx, int dy,
			int ksize, double scale, double delta, int borderType);
		HINSTANCE Hint = LoadLibraryA("../Sobel/target/sobel.dll");//load dll path
		c mySobel = (c)GetProcAddress(Hint, "mySobel");
		mySobel(_src, _dst, ddepth, dx, dy, ksize, scale, delta, borderType);
	}

};

void testAffineTransform() {
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	HINSTANCE h = LoadLibraryA("AffineTransform.dll");
	typedef void(*a)(
		cv::InputArray,
		cv::OutputArray,
		cv::InputArray,
		cv::Size,
		int,
		int,
		const cv::Scalar&
	);
	typedef cv::Mat(*b)(
		cv::Point2f,
		double,
		double
	);
	typedef cv::Mat(*c)(
		const cv::Point2f[],
		const cv::Point2f[]
	);
	a warpAffineI = (a)GetProcAddress(h, "warpAffineI");
	b getRotationMatrix2DI = (b)GetProcAddress(h, "getRotationMatrix2DI");
	c getAffineTransformI = (c)GetProcAddress(h, "getAffineTransformI");

	cv::Mat src = myread("foo.png", 0);

	cv::Point2f srcTri[3];
	srcTri[0] = cv::Point2f(0.f, 0.f);
	srcTri[1] = cv::Point2f(src.cols - 1.f, 0.f);
	srcTri[2] = cv::Point2f(0.f, src.rows - 1.f);

	cv::Point2f dstTri[3];
	dstTri[0] = cv::Point2f(0.f, src.rows*0.33f);
	dstTri[1] = cv::Point2f(src.cols*0.85f, src.rows*0.25f);
	dstTri[2] = cv::Point2f(src.cols*0.15f, src.rows*0.7f);

	cv::Mat warp_mat = getAffineTransformI(srcTri, dstTri);
	cv::Mat warp_dst = cv::Mat::zeros(src.rows, src.cols, src.type());

	warpAffineI(src, warp_dst, warp_mat, warp_dst.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());
	cv::Point center = cv::Point(warp_dst.cols / 2, warp_dst.rows / 2);
	double angle = -50.0;
	double scale = 0.6;
	cv::Mat rot_mat = getRotationMatrix2DI(center, angle, scale);
	cv::Mat warp_rotate_dst;
	warpAffineI(warp_dst, warp_rotate_dst, rot_mat, warp_dst.size(), 
		cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());

	mywrite("foo_warp.png", warp_dst);
	mywrite("foo_warp_rotate.png", warp_rotate_dst);
}

void testResizeFlipCrop() {
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat src = myread("foo.png", 0);
	HINSTANCE h = LoadLibraryA("Resize.dll");
	typedef void(*a)(
		cv::InputArray,
		cv::OutputArray,
		cv::Size,
		double,
		double,
		int
	);

	a resizeI = (a)GetProcAddress(h, "resizeI");
	cv::Mat dst;
	resizeI(src, dst, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
	mywrite("foo_resize.png", dst);
	cv::flip(src, dst, 0);
	mywrite("foo_resize_flip.png", dst);
	dst = dst(cv::Range(dst.size().height*0.2, dst.size().height*0.8),
		cv::Range(dst.size().width*0.2, dst.size().width*0.8));
	mywrite("foo_resize_flip_crop.png", dst);
}

void testFillPoly() {
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat src = myread("foo.png", 0);

	HINSTANCE h = LoadLibraryA("FillPoly.dll");
	typedef void(*a)(cv::InputOutputArray,
		cv::InputArrayOfArrays,
		const cv::Scalar&,
		int,
		int,
		cv::Point);
	a fillPolyI = (a)GetProcAddress(h, "fillPolyI");

	std::vector<cv::Point> polygon{
		cv::Point(300, 500),
		cv::Point(50, 5),
		cv::Point(500, 500)
	};

	std::vector<std::vector<cv::Point>> pts{
		polygon
	};

	fillPolyI(src, pts, cv::Scalar(), cv::LINE_8, 0, cv::Point());

	mywrite("foo_fillpoly.png", src);
}

void testEdgeDetection() {
	enum ThresholdTypes {
		THRESH_BINARY = 0, //!< \f[\texttt{dst} (x,y) =  \fork{\texttt{maxval}}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{0}{otherwise}\f]
		THRESH_BINARY_INV = 1, //!< \f[\texttt{dst} (x,y) =  \fork{0}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{maxval}}{otherwise}\f]
		THRESH_TRUNC = 2, //!< \f[\texttt{dst} (x,y) =  \fork{\texttt{threshold}}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{src}(x,y)}{otherwise}\f]
		THRESH_TOZERO = 3, //!< \f[\texttt{dst} (x,y) =  \fork{\texttt{src}(x,y)}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{0}{otherwise}\f]
		THRESH_TOZERO_INV = 4, //!< \f[\texttt{dst} (x,y) =  \fork{0}{if \(\texttt{src}(x,y) > \texttt{thresh}\)}{\texttt{src}(x,y)}{otherwise}\f]
		THRESH_MASK = 7,
		THRESH_OTSU = 8, //!< flag, use Otsu algorithm to choose the optimal threshold value
		THRESH_TRIANGLE = 16 //!< flag, use Triangle algorithm to choose the optimal threshold value
	};
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	//test laplacian and sobel
	//cv::Mat src = myread("", 0);
	cv::Mat resultSobel;
	cv::Mat resultLaplacian;
	EdgeDetection ed;

	//ed.mySobel(src, resultSobel, CV_32F, 1, 0);
	//ed.myLaplacian(src, resultLaplacian, CV_32F);

	//mywrite("", resultSobel);
	//mywrite("", resultLaplacian);


	//test for threshold
	cv::Mat src = myread("C:/Users/zwq/Desktop/1.png", 0);
	cv::Mat result;
	//typedef double(*threshold) (cv::InputArray _src, cv::OutputArray _dst, double thresh, double maxval, int type);
	//HINSTANCE hintThreshold = LoadLibraryA("../Threshold/target/threshold.dll");
	//threshold myThreshold = (threshold)GetProcAddress(hintThreshold, "myThreshold");
	//myThreshold(src, result, 200, 255, THRESH_BINARY);
	//mywrite("C:/Users/zwq/Desktop/2.png", result);


	//test for adaptiveThreshold
	typedef double(*adaptivethreshold)(cv::InputArray _src, cv::OutputArray _dst, double maxValue,
		int method, int type, int blockSize, double delta);
	HINSTANCE hintAdaptiveThreshold = LoadLibraryA("../AdaptiveThreshold/target/adaptiveThreshold.dll");
	adaptivethreshold myAdaptiveThreshold = (adaptivethreshold)GetProcAddress(hintAdaptiveThreshold, "myAdaptiveThreshold");
	//myAdaptiveThreshold(src, result, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, -2);
	//mywrite("C:/Users/zwq/Desktop/3.png", result);

	//test for 基于边缘检测分割
	ed.myLaplacian(src, resultLaplacian, CV_8U);
	myAdaptiveThreshold(resultLaplacian, result, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, -2);
	mywrite("C:/Users/zwq/Desktop/4.png", resultLaplacian);
}

//形态学腐蚀操作
void testErode() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/psma.PNG", 0);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // 矩形结构

	typedef void(*erode)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
		cv::Point anchor, int iterations,
		int borderType, const cv::Scalar& borderValue);
	HINSTANCE erodeDll = LoadLibraryA("../Erode/target/erode.dll");
	//HINSTANCE erodeDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/erode/bin/Release/opencv_imgproc343.dll");
	if (erodeDll == NULL)
	{
		printf("加载dll失败\n");
	}
	erode myErode = (erode)GetProcAddress(erodeDll, "myErode");

	if (myErode == NULL)
	{
		printf("加载func失败\n");
	}

	//typedef cv::Mat(*getStructuringElement)(int shape, cv::Size ksize, cv::Point anchor);
	//getStructuringElement myGetStructuringElement=(getStructuringElement)GetProcAddress(erodeDll, "getStructuringElement");
	//cv::Mat element = myGetStructuringElement(cv::MORPH_RECT, cv::Size(25, 25), cv::Point(-1, -1));

	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
	myErode(src, result, kernels, cv::Point(-1, -1),1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	mywrite("C:/Users/14839/Desktop/erodeRes.png", result);

}

//形态学膨胀操作
void testDilate() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/psma.PNG", 0);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // 矩形结构

	typedef void(*erode)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
		cv::Point anchor, int iterations,
		int borderType, const cv::Scalar& borderValue);
	HINSTANCE erodeDll = LoadLibraryA("../Dilate/target/dilate.dll");
	//HINSTANCE erodeDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/dilate/bin/Release/opencv_imgproc343.dll");
	if (erodeDll == NULL)
	{
		printf("加载dll失败\n");
	}
	erode myDilate = (erode)GetProcAddress(erodeDll, "myDilate");

	if (myDilate == NULL)
	{
		printf("加载func失败\n");
	}


	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
	myDilate(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	mywrite("C:/Users/14839/Desktop/dilateRes.png", result);

}

//形态学开运算
void testOpen() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/open.PNG", 0);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // 矩形结构

	typedef void(*open)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
		cv::Point anchor, int iterations,
		int borderType, const cv::Scalar& borderValue);
	HINSTANCE openDll = LoadLibraryA("../Open/target/open.dll");
	//HINSTANCE openDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
	if (openDll == NULL)
	{
		printf("加载dll失败\n");
	}
	open myOpen = (open)GetProcAddress(openDll, "myOpen");

	if (myOpen == NULL)
	{
		printf("加载func失败\n");
	}


	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
	myOpen(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	mywrite("C:/Users/14839/Desktop/openRes.png", result);

}

//形态学闭运算
void testClose() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/close.PNG", 0);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // 矩形结构

	typedef void(*close)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
		cv::Point anchor, int iterations,
		int borderType, const cv::Scalar& borderValue);
	HINSTANCE closeDll = LoadLibraryA("../Close/target/close.dll");
	//HINSTANCE closeDll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
	if (closeDll == NULL)
	{
		printf("加载dll失败\n");
	}
	close myClose = (close)GetProcAddress(closeDll, "myClose");

	if (myClose == NULL)
	{
		printf("加载func失败\n");
	}


	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
	myClose(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	mywrite("C:/Users/14839/Desktop/closeRes.png", result);

}

//形态学梯度运算
void testGradient() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/psma.PNG", 0);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // 矩形结构

	typedef void(*gradient)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
		cv::Point anchor, int iterations,
		int borderType, const cv::Scalar& borderValue);
	HINSTANCE Dll = LoadLibraryA("../Gradient/target/gradient.dll");
	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
	if (Dll == NULL)
	{
		printf("加载dll失败\n");
	}
	gradient myGradient = (gradient)GetProcAddress(Dll, "myGradient");

	if (myGradient == NULL)
	{
		printf("加载func失败\n");
	}


	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
	myGradient(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	mywrite("C:/Users/14839/Desktop/gradientRes.png", result);

}

//形态学黑帽操作
void testBlackhat() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/open.PNG", 0);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // 矩形结构

	typedef void(*blackhat)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
		cv::Point anchor, int iterations,
		int borderType, const cv::Scalar& borderValue);
	HINSTANCE Dll = LoadLibraryA("../Blackhat/target/blackhat.dll");
	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
	if (Dll == NULL)
	{
		printf("加载dll失败\n");
	}
	blackhat myBlackhat = (blackhat)GetProcAddress(Dll, "myBlackhat");

	if (myBlackhat == NULL)
	{
		printf("加载func失败\n");
	}


	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
	myBlackhat(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	mywrite("C:/Users/14839/Desktop/blackhatRes.png", result);

}

//形态学顶帽操作
void testTophat() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	cv::Mat result;
	cv::Mat src = myread("C:/Users/14839/Desktop/close.PNG", 0);
	cv::Mat kernel = cv::Mat::zeros(25, 25, CV_8UC1);   // 矩形结构

	typedef void(*tophat)(cv::InputArray src, cv::OutputArray dst, cv::InputArray kernel,
		cv::Point anchor, int iterations,
		int borderType, const cv::Scalar& borderValue);
	HINSTANCE Dll = LoadLibraryA("../Tophat/target/tophat.dll");
	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
	if (Dll == NULL)
	{
		printf("加载dll失败\n");
	}
	tophat myTophat = (tophat)GetProcAddress(Dll, "myTophat");

	if (myTophat == NULL)
	{
		printf("加载func失败\n");
	}


	cv::Mat kernels = cv::Mat::ones(5, 5, CV_8UC1);
	myTophat(src, result, kernels, cv::Point(-1, -1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue());
	mywrite("C:/Users/14839/Desktop/tophatRes.png", result);

}

//测试基于区域增长的图像分割
void testRegionGrow() {

	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");


	HINSTANCE hintRegionGrow = LoadLibraryA("../RegionGrow/target/regionGrow.dll");
	typedef cv::Mat(*func) (cv::Mat src, cv::Point2i pt, int th);
	func reginGrow = (func)GetProcAddress(hintRegionGrow, "RegionGrow");

	cv::Mat src, result;
	cv::Point point(200, 200);
	int th = 10;
	src = myread("C:\\Users\\zwq\\Desktop\\s.jpg", 0);

	result = reginGrow(src, point, th);
	mywrite("C:\\Users\\zwq\\Desktop\\s_regionGrow.jpg", result);

}


void testGrabcCut() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	HINSTANCE hintGrabCut = LoadLibraryA("../GrabCut/target/grabCut.dll");
	typedef void(*func) (cv::InputArray _img, cv::InputOutputArray _mask, cv::Rect rect,cv::InputOutputArray _bgdModel, cv::InputOutputArray _fgdModel, int iterCount, int mode);
	func myGrabCut = (func)GetProcAddress(hintGrabCut, "myGrabCut");

	Mat src = myread("img/grabcut.png", 1);
	Mat mask, bgModel, fgModel, result;
	mask.create(src.size(),CV_8U);
	Point p1(50,10), p2(410,630);
	Rect rect = Rect(p1, p2);

	mask.setTo(Scalar::all(GC_BGD));//背景
	mask(rect).setTo(Scalar(GC_PR_FGD));//前景	


	bool init = false;
	int count = 4;
	while (count--)
	{
		if (init)//鼠标按下，init变为false
			grabCut(src, mask, rect, bgModel, fgModel, 1, GC_EVAL);//第二次迭代，用mask初始化grabcut
		else
		{
			grabCut(src, mask, rect, bgModel, fgModel, 1, GC_INIT_WITH_RECT);//用矩形窗初始化GrabCut
			init = true;
		}

		Mat binmask;
		binmask = mask & 1;				//进一步掩膜
		if (init)						//进一步抠出无效区域。鼠标按下，init变为false
		{
			src.copyTo(result, binmask);
		}
		else
		{
			result = src.clone();
		}
		rectangle(result, rect, Scalar(0, 0, 255), 2, 8);

	}
	mywrite("img/grabcut_result.png",result);
}

//Harris角点检测
void testHarris() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");


	Mat img = myread("img/lena.jpg", 1);
	if (!img.data)
	{
		//cout << "读取图像错误，请确认图像文件是否正确" << endl;
	}

	Mat gray = myread("1.jpg", 0);

	HINSTANCE Dll = LoadLibraryA("../Harris/target/Harris.dll");
	//HINSTANCE Dll = LoadLibraryA("C:/Users/14839/Desktop/opencv/open/bin/Release/opencv_imgproc343.dll");
	if (Dll == NULL)
	{
		printf("加载dll失败\n");
	}
	typedef void(*myCornerHarrisPointer)(cv::InputArray src, cv::OutputArray dst, int blockSize, int ksize, double k,int borderType);
	myCornerHarrisPointer myCornerHarris = (myCornerHarrisPointer)GetProcAddress(Dll, "myCornerHarris");
	//计算Harris系数
	Mat harris;
	int blockSize = 2;     // 邻域半径
	int apertureSize = 3;  // 邻域大小
	myCornerHarris(gray, harris, blockSize, apertureSize, 0.04,4);
	//归一化便于进行数值比较和结果显示
	Mat harrisn;
	normalize(harris, harrisn, 0, 255, NORM_MINMAX);
	//将图像的数据类型变成CV_8U
	convertScaleAbs(harrisn, harrisn);
	//寻找Harris角点并显示图像
	Mat resultimg = img.clone();
	
	mywrite("img/harris.jpg", harris);
	mywrite("img/harrisn.jpg", harrisn);
}

//图像运算
void testImgOperation() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	ImgOperator imgOperationer;

	Mat img1 = myread("1.jpg", 1);
	Mat img2 = myread("2.jpg", 1);
	Mat result;
	Mat result2;
	//图像加法
	imgOperationer.my_Add(img1, img2, result);
	mywrite("result_add.jpg", result);
	//图像减法
	imgOperationer.my_Subtract(result, img1, result2);
	mywrite("result_substract.jpg", result2);
	//绝对差
	Mat img4 = myread("1.jpg", 0);
	Mat img5 = myread("2.jpg", 0);

	Mat result_absdiff;
	imgOperationer.my_Absdiff(img4, img5, result_absdiff);
	mywrite("result_absdiff.jpg", result_absdiff);
	//图像对应像素的加法
	Mat result3 = img1 + Scalar(50, 50, 50);
	mywrite("result_add_light.jpg", result3);
	//图像位运算
	//按位非
	Mat result_bitwise_not;
	imgOperationer.my_Bitwise_not(img1, result_bitwise_not);
	mywrite("result_bitwise_not.jpg", result_bitwise_not);
	//按位与
	Mat result_bitwise_and;
	imgOperationer.my_Bitwise_and(img1, img2, result_bitwise_and);
	mywrite("result_bitwise_and.jpg", result_bitwise_and);
	//按位异或
	Mat result_bitwise_xor;
	imgOperationer.my_Bitwise_xor(img1, img2, result_bitwise_xor);
	mywrite("result_bitwise_xor.jpg", result_bitwise_xor);
	//最大值最小值
	double minVal = 0.0;
	double maxVal = 0.0;
	imgOperationer.my_MinMaxLoc(img1, &minVal, &maxVal);
	std::cout << minVal << "," << maxVal << std::endl;
	//均值
	Scalar mean = 0.0;
	imgOperationer.my_Mean = cv::mean(img1);
	std::cout << mean[0] << "-" << mean[1] << "-" << mean[2] << std::endl;

	//归一化
	Mat result_normalize;
	imgOperationer.my_Normalize(img1, result_normalize);
	mywrite("result_normalize.jpg", result_normalize);

	//dft
	Mat result_dft;
	Mat img3 = myread("1.jpg", 0);
	Mat tmp;
	img3.convertTo(tmp, CV_64F);

	imgOperationer.my_Dft(tmp, result_dft);
	mywrite("result_dft.jpg", result_dft);

}

int main() {
	testFillPoly();
	testResizeFlipCrop();
	testAffineTransform();
	testErode();
	testDilate();
	testOpen();
	testClose();
	testGradient();
	testBlackhat();
	testTophat();
	testGrabcCut();
	testHarris();
	testImgOperation();
}




