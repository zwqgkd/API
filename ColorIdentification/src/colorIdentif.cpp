#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <iostream>
#include <vector>
using namespace std;
using namespace cv;


#if defined(__cplusplus)
extern "C" {
#endif
	__declspec(dllexport)  void myColorIdentif(const cv::Mat matSrc);
#ifdef __cplusplus
}
#endif

void myColorIdentif(const cv::Mat matSrc)
{
	cout << "prepare" << endl;
	Mat matHsv;
	cvtColor(matSrc, matHsv, COLOR_BGR2HSV);

	vector<int> colorVec;
	colorVec.push_back(matHsv.at<uchar>(0, 0));
	colorVec.push_back(matHsv.at<uchar>(0, 1));
	colorVec.push_back(matHsv.at<uchar>(0, 2));

	if ((colorVec[0] >= 0 && colorVec[0] <= 180)
		&& (colorVec[1] >= 0 && colorVec[1] <= 255)
		&& (colorVec[2] >= 0 && colorVec[2] <= 46)) {

		cout << "ºÚ" << endl;
	}
	else if ((colorVec[0] >= 0 && colorVec[0] <= 180)
		&& (colorVec[1] >= 0 && colorVec[1] <= 43)
		&& (colorVec[2] >= 46 && colorVec[2] <= 220)) {

		cout << "»Ò" << endl;
	}
	else if ((colorVec[0] >= 0 && colorVec[0] <= 180)
		&& (colorVec[1] >= 0 && colorVec[1] <= 30)
		&& (colorVec[2] >= 221 && colorVec[2] <= 255)) {

		cout << "°×" << endl;
	}
	else if (((colorVec[0] >= 0 && colorVec[0] <= 10) || (colorVec[0] >= 156 && colorVec[0] <= 180))
		&& (colorVec[1] >= 43 && colorVec[1] <= 255)
		&& (colorVec[2] >= 46 && colorVec[2] <= 255)) {

		cout << "ºì" << endl;

	}
	else if ((colorVec[0] >= 11 && colorVec[0] <= 25)
		&& (colorVec[1] >= 43 && colorVec[1] <= 255)
		&& (colorVec[2] >= 46 && colorVec[2] <= 255)) {

		cout << "³È" << endl;
	}
	else if ((colorVec[0] >= 26 && colorVec[0] <= 34)
		&& (colorVec[1] >= 43 && colorVec[1] <= 255)
		&& (colorVec[2] >= 46 && colorVec[2] <= 255)) {

		cout << "»Æ" << endl;
	}
	else if ((colorVec[0] >= 35 && colorVec[0] <= 77)
		&& (colorVec[1] >= 43 && colorVec[1] <= 255)
		&& (colorVec[2] >= 46 && colorVec[2] <= 255)) {

		cout << "ÂÌ" << endl;
	}
	else if ((colorVec[0] >= 78 && colorVec[0] <= 99)
		&& (colorVec[1] >= 43 && colorVec[1] <= 255)
		&& (colorVec[2] >= 46 && colorVec[2] <= 255)) {

		cout << "Çà" << endl;
	}
	else if ((colorVec[0] >= 100 && colorVec[0] <= 124)
		&& (colorVec[1] >= 43 && colorVec[1] <= 255)
		&& (colorVec[2] >= 46 && colorVec[2] <= 255)) {

		cout << "À¶" << endl;
	}
	else if ((colorVec[0] >= 125 && colorVec[0] <= 155)
		&& (colorVec[1] >= 43 && colorVec[1] <= 255)
		&& (colorVec[2] >= 46 && colorVec[2] <= 255)) {

		cout << "×Ï" << endl;
	}
	else {

		cout << "Î´Öª" << endl;
	}

}

