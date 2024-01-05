#include "t.h"
#include <Windows.h>
int main() {

	HINSTANCE h = LoadLibraryA("D:\\Documents\\opencv_splitFunction_test\\camera_demo\\x64\\Release\\HKCamera.dll");
	if (h == nullptr) {
		std::cout << "Error: " << GetLastError() << std::endl;
		return 1;
	}

	getAcceCameras();

	openCamera(1);

	ParamPtrArray params;
	params.push_back(make_param<int>("cameraid", "int", new int(1)));
	params.push_back(make_param<int>("nums", "int", new int(2)));
	//typedef cv::Mat(*f)(ParamPtrArray&);
	//auto getImg = (f)GetProcAddress(h, "getImg");
	getImgs(params);
	std::vector<cv::Mat> res = get_data<std::vector<cv::Mat>>(params.back());

	closeCamera(1);

	return 1;

}