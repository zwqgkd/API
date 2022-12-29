#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>


using namespace cv;
using namespace std;

extern"C" __declspec(dllexport) void polygoDetect(Mat& srcImg, double epsilon, int minAcreage, vector<vector<float>>& lines, Mat& dst);