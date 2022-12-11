#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include<vector>

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
enum AdaptiveThresholdTypes {
	/** the threshold value \f$T(x,y)\f$ is a mean of the \f$\texttt{blockSize} \times
	\texttt{blockSize}\f$ neighborhood of \f$(x, y)\f$ minus C */
	ADAPTIVE_THRESH_MEAN_C = 0,
	/** the threshold value \f$T(x, y)\f$ is a weighted sum (cross-correlation with a Gaussian
	window) of the \f$\texttt{blockSize} \times \texttt{blockSize}\f$ neighborhood of \f$(x, y)\f$
	minus C . The default sigma (standard deviation) is used for the specified blockSize . See
	#getGaussianKernel*/
	ADAPTIVE_THRESH_GAUSSIAN_C = 1
};

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

	//test for »ùÓÚ±ßÔµ¼ì²â·Ö¸î
	ed.myLaplacian(src, resultLaplacian, CV_8U);
	myAdaptiveThreshold(resultLaplacian, result, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, -2);
	mywrite("C:/Users/zwq/Desktop/4.png", resultLaplacian);
}

int main() {
	testFillPoly();
	testResizeFlipCrop();
	testAffineTransform();
}


