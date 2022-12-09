#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>


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

int main() {
	//prepare for read and write
	HINSTANCE Hint_wr = LoadLibraryA("wr.dll");
	typedef cv::Mat(*r) (const char*filename, int flag);
	typedef void(*w) (const char*filename, cv::Mat result);
	r myread = (r)GetProcAddress(Hint_wr, "myread");
	w mywrite = (w)GetProcAddress(Hint_wr, "mywrite");

	//test laplacian and sobel
	cv::Mat src = myread("", 0);
	cv::Mat resultSobel;
	cv::Mat resultLaplacian;
	EdgeDetection ed;

	ed.mySobel(src, resultSobel, CV_32F, 1, 0);
	ed.myLaplacian(src, resultLaplacian, CV_32F);

	mywrite("", resultSobel);
	mywrite("", resultLaplacian);

}


