#pragma once
#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include<vector>

using namespace cv;
class BlurDetectionByStd
{
public:
	//����ͼ����밴�Ҷ�ͼ�����
	//������imreadʱ�ڶ�������Ϊ0
	//���԰���ͼ������cvtColorת��Ϊ�Ҷ�ͼ
	double getBlurDetectionScoreByStd(Mat image);
	double getBlurDetectionScoreByTenengrad(Mat image);
	double getBlurDetectionScoreByLaplacian(Mat image);
	double getBlurDetectionScoreBySMD(Mat image);
};

