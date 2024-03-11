#include"brightnessMeasurement.h"


void measureBrightness(ParamPtrArray& params) {
    // get in_params
    GeliMat in_param0_mat = get_data_ref<GeliMat>(params[0]);
    // 如果算子有roi区域，那么roi相关数据会push到params里
    Roi roi = get_data_ref<Roi>(params[1]);

    // 如果是图形类型，那么需要调用以下函数生成mask_mat
    if (!roi.mat_flag) {
        roi.generate_mask_mat(in_param0_mat);
    }

    // 第一个输出，算子状态
    int status = static_cast<int>(OperatorStatus::OK);
    cv::Mat img = in_param0_mat.mat;
    std::vector<double> ans;
    try {
        ans.push_back(minimumBrightness(img));
        ans.push_back(meanBrightness(img));
        ans.push_back(maximumBrightness(img));
        ans.push_back(standardDeviation(img));
    }
    catch (std::exception& e) {
        status = static_cast<int>(OperatorStatus::NG);
        throw ExecutionFailedException("执行错误", "高斯滤波执行错误");
    }
    params.push_back(make_param<int>(status));

    // 第二个输出，结果集vector<double>
    params.push_back(make_param<std::vector<double>>(ans));

    // 第三个输出，掩膜图像
    if (roi.mask) {
        // 如果需要输出掩膜图像
        params.push_back(make_param<GeliMat>(roi.mask_mat));
    }
    else {
        params.emplace_back(nullptr);
    }

    // 第四五个输出，关于区域信息：检测区域和roi圆弧
    if (roi.type == "") {
        params.emplace_back(nullptr);
        params.emplace_back(nullptr);
    }
    else if (roi.type == "rect") {
        params.push_back(make_param<GeliRect>(roi.generate_area()));
        params.emplace_back(nullptr);

    }
    else if (roi.type == "circle") {
        params.emplace_back(nullptr);
        params.push_back(make_param<GeliCircle>(roi.generate_circle()));
    }
    // 第六个输出，屏蔽区
    params.push_back(make_param<GeliShield>(roi.shield));
}

double minimumBrightness(cv::Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	std::vector<cv::Mat>images;
	split(img_hls, images);
	double minVal = 0;
	cv::minMaxLoc(images[1], &minVal, NULL, NULL, NULL);
	return minVal;
}

double meanBrightness(cv::Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	cv::Scalar scalar = cv::mean(img_hls);
	return scalar.val[1];
}

double maximumBrightness(cv::Mat img) {
	cv::Mat img_hls;
	cv::cvtColor(img, img_hls, CV_BGR2HLS);
	std::vector<cv::Mat>images;
	split(img_hls, images);
	double maxVal = 0;
	cv::minMaxLoc(images[1], NULL, &maxVal, NULL, NULL);
	return maxVal;
}

double standardDeviation(cv::Mat img) {
    cv::Mat gray, mat_mean, mat_stddev;
	cvtColor(img, gray, CV_RGB2GRAY); // 转换为灰度图
	meanStdDev(gray, mat_mean, mat_stddev);
	double stddev;
	stddev = mat_stddev.at<double>(0, 0);
	return stddev;
}