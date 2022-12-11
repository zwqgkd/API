#include "opencv2/core.hpp"

extern "C" __declspec(dllexport) bool solvePnPRansac(cv::InputArray _opoints, cv::InputArray _ipoints, cv::InputArray _cameraMatrix, cv::InputArray _distCoeffs, cv::OutputArray _rvec, cv::OutputArray _tvec, bool useExtrinsicGuess, int iterationsCount, float reprojectionError, double confidence, cv::OutputArray _inliers, int flags);

extern "C" __declspec(dllexport) cv::Mat myFindFundamentalMat(cv::InputArray _points1, cv::InputArray _points2, cv::OutputArray _mask, int method, double ransacReprojThreshold, double confidence);