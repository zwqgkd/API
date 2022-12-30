#include "BlurDetection.h"
double getBulrDetectionScoreByStd(Mat image) {
	//要求图像按灰度图读入
	//检查是否为单通道，如果不是则转换为单通道灰度图
	Mat meanValueImage;
	Mat meanStdValueImage;
	meanStdDev(image, meanValueImage, meanStdValueImage);
	double meanValue = 0.0;
	meanValue = meanStdValueImage.at<double>(0, 0);
	return meanValue * meanValue;
}


double getBulrDetectionScoreByGenengrad(Mat image) {

	//定义一些sobel用到的参数
	int ksize = 3;
	double scale = 1;
	double delta = 0;
	int borderType = cv::BORDER_DEFAULT;
	typedef void(*c)(cv::InputArray _src, cv::OutputArray _dst, int ddepth, int dx, int dy,
		int ksize, double scale, double delta, int borderType);
	HINSTANCE Hint = LoadLibraryA("../Sobel/target/sobel.dll");
	c mySobel = (c)GetProcAddress(Hint, "mySobel");


	Mat imageSobel;
	//mySobel(img1, imageSobel, CV_16U, 1, 1,ksize,scale,delta,borderType);
	Sobel(image, imageSobel, CV_16U, 1, 1, ksize, scale, delta, borderType);

	//imwrite("re"+filename, imageSobel);

	//图像的平均灰度
	return mean(imageSobel)[0];

}


double getBlurDetectionScoreByLaplacian(Mat image) {
	//检查是否为单通道
	//定义一些lap用到的参数
	int ksize = 3;
	double scale = 1;
	double delta = 0;
	int borderType = cv::BORDER_DEFAULT;
	typedef void(*c)(cv::InputArray _src, cv::OutputArray _dst, int ddepth,
		int ksize, double scale, double delta, int borderType);
	HINSTANCE Hint = LoadLibraryA("../Laplacian/target/laplacian.dll");
	c mylap = (c)GetProcAddress(Hint, "myLaplacian");


	Mat imageLap;
	mylap(image, imageLap, CV_32FC1, ksize, scale, delta, borderType);
	//Laplacian(img1, imageLap, CV_16U, ksize, scale, delta, borderType);

	//图像的平均灰度
	double meanValue = 0.0;
	abs(imageLap);
	return mean(imageLap)[0];


};

double getBlurDetectionScoreBySMD(Mat image) {

	cv::Mat kernel_x(3, 3, CV_32F, cv::Scalar(0));
	kernel_x.at<float>(1, 2) = -1.0;
	kernel_x.at<float>(1, 1) = 1.0;
	cv::Mat kernel_y(3, 3, CV_32F, cv::Scalar(0));
	kernel_y.at<float>(0, 1) = -1.0;
	kernel_y.at<float>(1, 1) = 1.0;
	Mat smd_image_x;
	Mat smd_image_y;
	cv::filter2D(image, smd_image_x, image.depth(), kernel_x);
	cv::filter2D(image, smd_image_y, image.depth(), kernel_y);
	smd_image_x = abs(smd_image_x);
	smd_image_y = abs(smd_image_y);
	Mat G;
	G = smd_image_x + smd_image_y;
	return mean(G)[0];

}