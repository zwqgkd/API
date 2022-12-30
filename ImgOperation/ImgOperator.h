#pragma once
#include<opencv2/core.hpp>
#include<windows.h>
#include<iostream>
#include<opencv2/imgproc.hpp>
#include<vector>
using namespace cv;
class ImgOperator
{
public:
	//加法
	virtual void my_Add(InputArray src1, InputArray src2, OutputArray dst,
		InputArray mask = noArray(), int dtype = -1) {
		//add(src1, src2, dst, mask, dtype);
	}

	//减法
	virtual void my_Subtract(InputArray src1, InputArray src2, OutputArray dst,
		InputArray mask = noArray(), int dtype = -1) {
		//subtract(src1, src2, dst, mask, dtype);
	}

	//绝对差
	virtual void my_Absdiff(InputArray src1, InputArray src2, OutputArray dst) {
		//absdiff(src1, src2, dst);
	}

	//按位非
	virtual void my_Bitwise_not(InputArray src, OutputArray dst,
		InputArray mask = noArray()) {
		//bitwise_not(src, dst, mask);
	}

	//按位与
	virtual void my_Bitwise_and(InputArray src1, InputArray src2,
		OutputArray dst, InputArray mask = noArray()) {
		//bitwise_and(src1, src2, dst, mask);
	}
	//按位与
	virtual void my_Bitwise_xor(InputArray src1, InputArray src2,
		OutputArray dst, InputArray mask = noArray()) {
		//bitwise_xor(src1, src2, dst, mask);
	}

	//最大值最小值
	virtual void my_MinMaxLoc(InputArray src, CV_OUT double* minVal,
		CV_OUT double* maxVal = 0, CV_OUT Point* minLoc = 0,
		CV_OUT Point* maxLoc = 0, InputArray mask = noArray()) {
		//minMaxLoc(src, minVal, maxVal, minLoc, maxLoc, mask);
	}

	//均值
	virtual Scalar my_Mean(InputArray src, InputArray mask = noArray()) {
		//return mean(src, mask);
	}

	//归一化
	virtual void my_Normalize(InputArray src, InputOutputArray dst, double alpha = 1, double beta = 0,
		int norm_type = NORM_L2, int dtype = -1, InputArray mask = noArray()) {
		//normalize(src, dst, alpha, beta, norm_type, dtype, mask);
	}

	//dft
	virtual void my_Dft(InputArray src, OutputArray dst, int flags = 0, int nonzeroRows = 0) {
		//dft(src, dst, flags, nonzeroRows);
	}
};

