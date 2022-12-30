#include<opencv2/core.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/imgproc/types_c.h>
#include<color.hpp>


#if defined(__cplusplus)
extern "C" {
#endif
	__declspec(dllexport)  void myBrightnessAndContrast(const cv::Mat &src, cv::Mat &dst, float clipHistPercent , int histSize , int lowhist);
#ifdef __cplusplus
}
#endif


// clipHistPercent 剪枝（剪去总像素的多少百分比）
// histSize 最后将所有的灰度值归到多大的范围
// lowhist 最小的灰度值
void myBrightnessAndContrast(const cv::Mat &src, cv::Mat &dst, float clipHistPercent = 0, int histSize = 255, int lowhist = 0)
{

	CV_Assert(clipHistPercent >= 0);
	CV_Assert((src.type() == CV_8UC1) || (src.type() == CV_8UC3) || (src.type() == CV_8UC4));

	float alpha, beta;
	double minGray = 0, maxGray = 0;

	//to calculate grayscale histogram
	cv::Mat gray;
	if (src.type() == CV_8UC1) gray = src;
	else if (src.type() == CV_8UC3) cvtColor(src, gray, CV_BGR2GRAY);
	else if (src.type() == CV_8UC4) cvtColor(src, gray, CV_BGRA2GRAY);
	if (clipHistPercent == 0)
	{
		// keep full available range
		cv::minMaxLoc(gray, &minGray, &maxGray);
	}
	else
	{
		cv::Mat hist; //the grayscale histogram

		float range[] = { 0, 256 };
		const float* histRange = { range };
		bool uniform = true;
		bool accumulate = false;
		calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

		// calculate cumulative distribution from the histogram
		std::vector<float> accumulator(histSize);
		accumulator[0] = hist.at<float>(0);
		for (int i = 1; i < histSize; i++)
		{
			accumulator[i] = accumulator[i - 1] + hist.at<float>(i);
		}

		// locate points that cuts at required value
		float max = accumulator.back();

		int clipHistPercent2;
		clipHistPercent2 = clipHistPercent * (max / 100.0); //make percent as absolute
		clipHistPercent2 /= 2.0; // left and right wings
		// locate left cut
		minGray = 0;
		while (accumulator[minGray] < clipHistPercent2)
			minGray++;

		// locate right cut
		maxGray = histSize - 1;
		while (accumulator[maxGray] >= (max - clipHistPercent2))
			maxGray--;
	}

	// current range
	float inputRange = maxGray - minGray;

	alpha = (histSize - 1) / inputRange;   // alpha expands current range to histsize range
	beta = -minGray * alpha + lowhist;             // beta shifts current range so that minGray will go to 0

	// Apply brightness and contrast normalization
	// convertTo operates with saurate_cast
	src.convertTo(dst, -1, alpha, beta);

	// restore alpha channel from source
	if (dst.type() == CV_8UC4)
	{
		int from_to[] = { 3, 3 };
		cv::mixChannels(&src, 4, &dst, 1, from_to, 1);
	}
}
