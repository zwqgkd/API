#define _CRT_SECURE_NO_WARNINGS
#include "t.h"


int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (NULL == pRgbData)
	{
		return MV_E_PARAMETER;
	}

	for (unsigned int j = 0; j < nHeight; j++)
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}

	return MV_OK;
}

// convert data stream in Mat format
bool Convert2Mat(MV_FRAME_OUT* pFrame, cv::Mat& mat_img)
{
	cv::Mat srcImage;
	unsigned char* pData = pFrame->pBufAddr;
	if (pFrame->stFrameInfo.enPixelType == PixelType_Gvsp_Mono8)
	{
		srcImage = cv::Mat(pFrame->stFrameInfo.nHeight, pFrame->stFrameInfo.nWidth, CV_8UC1, pData);
	}
	else if (pFrame->stFrameInfo.enPixelType == PixelType_Gvsp_RGB8_Packed)
	{
		RGB2BGR(pData, pFrame->stFrameInfo.nWidth, pFrame->stFrameInfo.nHeight);
		srcImage = cv::Mat(pFrame->stFrameInfo.nHeight, pFrame->stFrameInfo.nWidth, CV_8UC3, pData);
	}
	else
	{
		printf("unsupported pixel format\n");
		return false;
	}

	if (NULL == srcImage.data)
	{
		return false;
	}

	mat_img = srcImage.clone();
	srcImage.release();

	return true;
}

MV_CC_DEVICE_INFO_LIST deviceList; //设成全局变量
std::vector<CMvCamera> cameraList; //后端可用相机集合

std::string getAcceCameras() {
	CMvCamera camera;
	std::string acceCameras = camera.getAccessableCamera(&deviceList);
	cameraList = std::vector<CMvCamera>(deviceList.nDeviceNum, CMvCamera());
	return acceCameras;
}

//根据指定id打开相机抓图
bool openCamera(int cameraIndex) {
	int nRet = MV_OK;
	nRet = cameraList[cameraIndex].Open(deviceList.pDeviceInfo[cameraIndex]);
	if (nRet != MV_OK) {
		printf("open camera %d fail nRet: 0x%x\n", cameraIndex, nRet);
		return false;
	}
	return true;
}



void getOneFrame(int cameraIndex, MV_FRAME_OUT* pFrame, cv::Mat* res_img) {
	/*MV_FRAME_OUT pFrame;*/
	int nRet = cameraList[cameraIndex].GetImageBuffer(pFrame, 1000);
	if (nRet != MV_OK) {
		printf("get image buffer fail nRet: 0x%x\n", nRet);
	}

	//数据去转换
	bool bConvertRet = false;
	cv::Mat mat_img, threshold_img, edge, gray;
	bConvertRet = Convert2Mat(pFrame, mat_img);

	// print result
	if (bConvertRet)
	{
		printf("Convert to Mat success. \n");
	}
	else
	{
		printf("OpenCV format convert failed.\n");
	}

	*res_img = mat_img;
	nRet = cameraList[cameraIndex].FreeImageBuffer(pFrame);
	if (nRet != MV_OK) {
		printf("free buffer fail nRet: 0x%x\n", nRet);
	}
}



bool closeCamera(int cameraIndex) {
	int nRet = MV_OK;
	nRet = cameraList[cameraIndex].Close();
	if (nRet != MV_OK) {
		printf("close camera %d fail nRet: 0x%x\n", cameraIndex, nRet);
		return false;
	}
	return true;
}

bool setParams(rapidjson::Value& data) {
	int cameraID = data["cameraID"].GetInt();
	float exposeTime = data["exposureTime"].GetFloat();
	int width = data["width"].GetInt();
	int height = data["height"].GetInt();


	if (cameraID < deviceList.nDeviceNum) {
		int nRet = MV_OK;
		//设置参数
		nRet = cameraList[cameraID].SetIntValue("Width", width);
		if (nRet != MV_OK) {
			printf("set camera %d width %d fail nRet: 0x%x\n", cameraID, width, nRet);
			return false;
		}
		nRet = cameraList[cameraID].SetIntValue("Height", height);
		if (nRet != MV_OK) {
			printf("set camera %d height %d fail nRet: 0x%x\n", cameraID, height, nRet);
			return false;
		}
		nRet = cameraList[cameraID].SetFloatValue("ExposureTime", exposeTime);
		if (nRet != MV_OK) {
			printf("set camera %d exposeTime %f fail nRet: 0x%x\n", cameraID, exposeTime, nRet);
			return false;
		}

	}
	else {
		printf("[Error: Invalid Camera ID %d]\n", cameraID);
		return false;
	}
	return true;
}

/*
触发参数设置步骤如下：
配置触发模式：Trigger mode设置为on
配置触发源：trigger source，选择line0或者line2（取决于硬件接线）
配置触发极性：trigger activation，默认选择上升沿（可配上升沿、下降沿、高、低电平等）
配置触发延时：trigger delay，默认0us
配置触发信号缓存：trigger cache enable，默认关闭
滤波参数设置步骤如下：

选择输入信号源：line selector，选择line0或者line2（取决于硬件接线）
配置滤波参数大小：line debouncer time，默认50us
*/
void setIOParams(ParamPtrArray& params) {
	int cameraID = get_data<int>(params[0]);
	int triggerMode = get_data<int>(params[1]);
	int triggerSource = get_data<int>(params[2]);
	int triggerActivation = get_data<int>(params[3]);
	int triggerDelay = get_data<double>(params[4]);


	if (cameraID < deviceList.nDeviceNum) {
		int nRet = MV_OK;
		//设置参数
		nRet = cameraList[cameraID].SetIntValue("TriggerMode", triggerMode);
		if (nRet != MV_OK) {
			printf("set camera %d triggerMode fail nRet: 0x%x\n", cameraID, nRet);
			return;
		}
		nRet = cameraList[cameraID].SetIntValue("TriggerSource", triggerSource);
		if (nRet != MV_OK) {
			printf("set camera %d triggerSource fail nRet: 0x%x\n", cameraID, nRet);
			return;
		}
		nRet = cameraList[cameraID].SetIntValue("TriggerActivation", triggerActivation);
		if (nRet != MV_OK) {
			printf("set camera %d triggerActivation fail nRet: 0x%x\n", cameraID, nRet);
			return;
		}
		nRet = cameraList[cameraID].SetIntValue("TriggerDelay", triggerDelay);
		if (nRet != MV_OK) {
			printf("set camera %d triggerDelay fail nRet: 0x%x\n", cameraID, nRet);
			return;
		}
	}
	else {
		printf("[Error: Invalid Camera ID %d]\n", cameraID);
		return;
	}
}

void getImg(ParamPtrArray& params) {
	int cameraID = get_data<int>(params[0]);

	MV_FRAME_OUT* pFrame = new MV_FRAME_OUT();

	//获得一张图片
	cv::Mat* img = new cv::Mat();
	if (cameraID < deviceList.nDeviceNum) {

		int nRet = MV_OK;

		nRet = cameraList[cameraID].StartGrabbing();
		if (nRet != MV_OK) {
			printf("start grab fail nRet: 0x%x\n", nRet);
		}

		getOneFrame(cameraID, pFrame, img);

		nRet = cameraList[cameraID].StopGrabbing();
		if (nRet != MV_OK) {
			printf("stop grab fail nRet: 0x%x\n", nRet);
		}

	}
	else {
		printf("[Error: Invalid Camera ID %d]\n", cameraID);
	}

	params.push_back(make_param<cv::Mat>("getImg", "Mat", img));
}

void getImgs(ParamPtrArray& params) {
	int cameraID = get_data<int>(params[0]);
	int n = get_data<int>(params[1]);


	MV_FRAME_OUT* pFrame = new MV_FRAME_OUT();

	std::vector<cv::Mat> res;

	for (int i = 0; i < n; i++) {
		//获得一张图片
		cv::Mat* img = new cv::Mat();
		if (cameraID < deviceList.nDeviceNum) {

			int nRet = MV_OK;

			nRet = cameraList[cameraID].StartGrabbing();
			if (nRet != MV_OK) {
				printf("start grab fail nRet: 0x%x\n", nRet);
			}

			getOneFrame(cameraID, pFrame, img);

			nRet = cameraList[cameraID].StopGrabbing();
			if (nRet != MV_OK) {
				printf("stop grab fail nRet: 0x%x\n", nRet);
			}
			res.push_back(*img);
		}
		else {
			printf("[Error: Invalid Camera ID %d]\n", cameraID);
		}
	}

	params.push_back(make_param<std::vector<cv::Mat>>("getImgs", "vector<Mat>", new std::vector<cv::Mat>(res)));
}


void storeImg(ParamPtrArray& params) {
	cv::Mat input = get_data<cv::Mat>(params[0]);
	std::string path = get_data<std::string>(params[1]);

	if (cv::imwrite(path, input))
		std::cout << "wirte " << path << " done." << std::endl;
	else
		std::cout << "store " << path << " failed." << std::endl;
}
