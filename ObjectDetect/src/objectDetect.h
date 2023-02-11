// https://blog.csdn.net/qq_27158179/article/details/81915740
#include <opencv2/dnn/dnn.hpp>
#include <fstream>
#include <string>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <Windows.h>

using namespace std;
using namespace cv;



extern "C" __declspec(dllexport) void myNMSBoxes(const std::vector<Rect>& bboxes, const std::vector<float>& scores, const float score_threshold, const float nms_threshold, std::vector<int>& indices, const float eta = 1.f, const int top_k = 0);


extern "C" __declspec(dllexport) cv::dnn::Net myReadNet(const String &cfgFile);


extern "C" __declspec(dllexport) void myBlobFromImage(InputArray image, OutputArray blob, double scalefactor = 1.0, const Size& size = Size(), const Scalar& mean = Scalar(), bool swapRB = true, bool crop = true, int ddepth = CV_32F);