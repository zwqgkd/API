two functions

1. bool mySolvePnPRansac(InputArray _opoints, InputArray _ipoints,	InputArray _cameraMatrix, InputArray _distCoeffs,	OutputArray _rvec, OutputArray _tvec, bool useExtrinsicGuess,int iterationsCount, float reprojectionError, double confidence,OutputArray _inliers, int flags)

2. cv::Mat myFindFundamentalMat(cv::InputArray _points1, cv::InputArray _points2, cv::OutputArray _mask, int method,	double ransacReprojThreshold, double confidence)