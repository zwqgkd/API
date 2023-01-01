#include<opencv2/core.hpp>
#include<vector>
#include<iostream>
using namespace std;

#if defined(__cplusplus)
extern "C" {
#endif
	__declspec(dllexport)  cv::Mat myShadowCorrection(cv::Mat input, int light);
	_declspec(dllexport)  cv::Mat mytest();
#ifdef __cplusplus
}
#endif

// ͼ����ӰУ��
cv::Mat myShadowCorrection(cv::Mat input, int light)
{
	cout << "����dll��" << endl;
	


	// ���ɻҶ�ͼ
	cv::Mat gray = cv::Mat::zeros(input.size(), CV_32FC1);
	cv::Mat f = input.clone();
	f.convertTo(f, CV_32FC3);
	vector<cv::Mat> pics;
	split(f, pics);
	cout << "1.." <<endl;
	cout << "pics.size:" << pics.size() << endl;
	gray = 0.299f*pics[2] + 0.587*pics[2] + 0.114*pics[0];
	cout << "2.." << endl;
	gray = gray / 255.f;
	
	cout << "�Ҷ�ͼ�������" << endl;
	// ȷ����Ӱ��
	cv::Mat thresh = cv::Mat::zeros(gray.size(), gray.type());
	thresh = (1.0f - gray).mul(1.0f - gray);

	cout << "ȷ����Ӱ�����" << endl;
	// ȡƽ��ֵ��Ϊ��ֵ
	cv::Scalar t = mean(thresh);
	cv::Mat mask = cv::Mat::zeros(gray.size(), CV_8UC1);
	mask.setTo(255, thresh >= t[0]);

	cout << "ȡƽ��ֵ��Ϊ��ֵ���" << endl;

	// ��������
	int max = 4;
	float bright = light / 100.0f / max;
	float mid = 1.0f + max * bright;

	// ��Եƽ������
	cv::Mat midrate = cv::Mat::zeros(input.size(), CV_32FC1);
	cv::Mat brightrate = cv::Mat::zeros(input.size(), CV_32FC1);
	for (int i = 0; i < input.rows; ++i)
	{
		uchar *m = mask.ptr<uchar>(i);
		float *th = thresh.ptr<float>(i);
		float *mi = midrate.ptr<float>(i);
		float *br = brightrate.ptr<float>(i);
		for (int j = 0; j < input.cols; ++j)
		{
			if (m[j] == 255)
			{
				mi[j] = mid;
				br[j] = bright;
			}
			else {
				mi[j] = (mid - 1.0f) / t[0] * th[j] + 1.0f;
				br[j] = (1.0f / t[0] * th[j])*bright;
			}
		}
	}

	cout << "��Եƽ���������" << endl;

	// ��Ӱ��������ȡ���ͼ
	cv::Mat result = cv::Mat::zeros(input.size(), input.type());
	for (int i = 0; i < input.rows; ++i)
	{
		float *mi = midrate.ptr<float>(i);
		float *br = brightrate.ptr<float>(i);
		uchar *in = input.ptr<uchar>(i);
		uchar *r = result.ptr<uchar>(i);
		for (int j = 0; j < input.cols; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				float temp = pow(float(in[3 * j + k]) / 255.f, 1.0f / mi[j])*(1.0 / (1 - br[j]));
				if (temp > 1.0f)
					temp = 1.0f;
				if (temp < 0.0f)
					temp = 0.0f;
				uchar utemp = uchar(255 * temp);
				r[3 * j + k] = utemp;
			}

		}
	}
	cout << "��Ӱ��������ȡ���ͼ���" << endl;

	return result;

}


cv::Mat mytest()
{
	return cv::Mat();
}