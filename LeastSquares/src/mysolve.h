#include <opencv2/core.hpp>


extern"C" _declspec(dllexport)bool mySolve(cv::InputArray _src, cv::InputArray _src2arg, cv::OutputArray _dst, int method);