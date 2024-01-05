#include "MvCamera.h"
#include <fmt/core.h>

CMvCamera::CMvCamera()
{
    m_hDevHandle = MV_NULL;
}

CMvCamera::~CMvCamera()
{
    Close();
}

// ch:获取SDK版本号 | en:Get SDK Version
int CMvCamera::GetSDKVersion()
{
    return MV_CC_GetSDKVersion();
}

// ch:枚举设备 | en:Enumerate Device
int CMvCamera::EnumDevices(unsigned int nTLayerType, MV_CC_DEVICE_INFO_LIST* pstDevList)
{
    return MV_CC_EnumDevices(nTLayerType, pstDevList);
}

// ch:判断设备是否可达 | en:Is the device accessible
bool CMvCamera::IsDeviceAccessible(MV_CC_DEVICE_INFO* pstDevInfo, unsigned int nAccessMode)
{
    return MV_CC_IsDeviceAccessible(pstDevInfo, nAccessMode);
}

// ch:打开设备 | en:Open Device
int CMvCamera::Open(MV_CC_DEVICE_INFO* pstDeviceInfo)
{
    if (MV_NULL == pstDeviceInfo)
    {
        return MV_E_PARAMETER;
    }

    if (m_hDevHandle)
    {
        return MV_E_CALLORDER;
    }

    int nRet = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    nRet = MV_CC_OpenDevice(m_hDevHandle);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle = MV_NULL;
    }

    return nRet;
}

// ch:关闭设备 | en:Close Device
int CMvCamera::Close()
{
    if (MV_NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }

    MV_CC_CloseDevice(m_hDevHandle);

    int nRet = MV_CC_DestroyHandle(m_hDevHandle);
    m_hDevHandle = MV_NULL;

    return nRet;
}

// ch:判断相机是否处于连接状态 | en:Is The Device Connected
bool CMvCamera::IsDeviceConnected()
{
    return MV_CC_IsDeviceConnected(m_hDevHandle);
}

// ch:注册图像数据回调 | en:Register Image Data CallBack
int CMvCamera::RegisterImageCallBack(void(__stdcall* cbOutput)(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser), void* pUser)
{
    return MV_CC_RegisterImageCallBackEx(m_hDevHandle, cbOutput, pUser);
}

// ch:开启抓图 | en:Start Grabbing
int CMvCamera::StartGrabbing()
{
    return MV_CC_StartGrabbing(m_hDevHandle);
}

// ch:停止抓图 | en:Stop Grabbing
int CMvCamera::StopGrabbing()
{
    return MV_CC_StopGrabbing(m_hDevHandle);
}

// ch:主动获取一帧图像数据 | en:Get one frame initiatively
int CMvCamera::GetImageBuffer(MV_FRAME_OUT* pFrame, int nMsec)
{
    return MV_CC_GetImageBuffer(m_hDevHandle, pFrame, nMsec);
}

// ch:释放图像缓存 | en:Free image buffer
int CMvCamera::FreeImageBuffer(MV_FRAME_OUT* pFrame)
{
    return MV_CC_FreeImageBuffer(m_hDevHandle, pFrame);
}

// ch:设置显示窗口句柄 | en:Set Display Window Handle
int CMvCamera::DisplayOneFrame(MV_DISPLAY_FRAME_INFO* pDisplayInfo)
{
    return MV_CC_DisplayOneFrame(m_hDevHandle, pDisplayInfo);
}

// ch:设置SDK内部图像缓存节点个数 | en:Set the number of the internal image cache nodes in SDK
int CMvCamera::SetImageNodeNum(unsigned int nNum)
{
    return MV_CC_SetImageNodeNum(m_hDevHandle, nNum);
}

// ch:获取设备信息 | en:Get device information
int CMvCamera::GetDeviceInfo(MV_CC_DEVICE_INFO* pstDevInfo)
{
    return MV_CC_GetDeviceInfo(m_hDevHandle, pstDevInfo);
}

// ch:获取GEV相机的统计信息 | en:Get detect info of GEV camera
int CMvCamera::GetGevAllMatchInfo(MV_MATCH_INFO_NET_DETECT* pMatchInfoNetDetect)
{
    if (MV_NULL == pMatchInfoNetDetect)
    {
        return MV_E_PARAMETER;
    }

    MV_CC_DEVICE_INFO stDevInfo = { 0 };
    GetDeviceInfo(&stDevInfo);
    if (stDevInfo.nTLayerType != MV_GIGE_DEVICE)
    {
        return MV_E_SUPPORT;
    }

    MV_ALL_MATCH_INFO struMatchInfo = { 0 };

    struMatchInfo.nType = MV_MATCH_TYPE_NET_DETECT;
    struMatchInfo.pInfo = pMatchInfoNetDetect;
    struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_NET_DETECT);
    memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_NET_DETECT));

    return MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
}

// ch:获取U3V相机的统计信息 | en:Get detect info of U3V camera
int CMvCamera::GetU3VAllMatchInfo(MV_MATCH_INFO_USB_DETECT* pMatchInfoUSBDetect)
{
    if (MV_NULL == pMatchInfoUSBDetect)
    {
        return MV_E_PARAMETER;
    }

    MV_CC_DEVICE_INFO stDevInfo = { 0 };
    GetDeviceInfo(&stDevInfo);
    if (stDevInfo.nTLayerType != MV_USB_DEVICE)
    {
        return MV_E_SUPPORT;
    }

    MV_ALL_MATCH_INFO struMatchInfo = { 0 };

    struMatchInfo.nType = MV_MATCH_TYPE_USB_DETECT;
    struMatchInfo.pInfo = pMatchInfoUSBDetect;
    struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_USB_DETECT);
    memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_USB_DETECT));

    return MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
}

// ch:获取和设置Int型参数，如 Width和Height，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
// en:Get Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int CMvCamera::GetIntValue(IN const char* strKey, OUT MVCC_INTVALUE_EX* pIntValue)
{
    return MV_CC_GetIntValueEx(m_hDevHandle, strKey, pIntValue);
}

int CMvCamera::SetIntValue(IN const char* strKey, IN int64_t nValue)
{
    return MV_CC_SetIntValueEx(m_hDevHandle, strKey, nValue);
}

// ch:获取和设置Enum型参数，如 PixelFormat，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
// en:Get Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int CMvCamera::GetEnumValue(IN const char* strKey, OUT MVCC_ENUMVALUE* pEnumValue)
{
    return MV_CC_GetEnumValue(m_hDevHandle, strKey, pEnumValue);
}

int CMvCamera::SetEnumValue(IN const char* strKey, IN unsigned int nValue)
{
    return MV_CC_SetEnumValue(m_hDevHandle, strKey, nValue);
}

int CMvCamera::SetEnumValueByString(IN const char* strKey, IN const char* sValue)
{
    return MV_CC_SetEnumValueByString(m_hDevHandle, strKey, sValue);
}

int CMvCamera::GetEnumEntrySymbolic(IN const char* strKey, IN MVCC_ENUMENTRY* pstEnumEntry)
{
    return MV_CC_GetEnumEntrySymbolic(m_hDevHandle, strKey, pstEnumEntry);
}

// ch:获取和设置Float型参数，如 ExposureTime和Gain，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
// en:Get Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int CMvCamera::GetFloatValue(IN const char* strKey, OUT MVCC_FLOATVALUE* pFloatValue)
{
    return MV_CC_GetFloatValue(m_hDevHandle, strKey, pFloatValue);
}

int CMvCamera::SetFloatValue(IN const char* strKey, IN float fValue)
{
    return MV_CC_SetFloatValue(m_hDevHandle, strKey, fValue);
}

// ch:获取和设置Bool型参数，如 ReverseX，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
// en:Get Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int CMvCamera::GetBoolValue(IN const char* strKey, OUT bool* pbValue)
{
    return MV_CC_GetBoolValue(m_hDevHandle, strKey, pbValue);
}

int CMvCamera::SetBoolValue(IN const char* strKey, IN bool bValue)
{
    return MV_CC_SetBoolValue(m_hDevHandle, strKey, bValue);
}

// ch:获取和设置String型参数，如 DeviceUserID，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件UserSetSave
// en:Get String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int CMvCamera::GetStringValue(IN const char* strKey, MVCC_STRINGVALUE* pStringValue)
{
    return MV_CC_GetStringValue(m_hDevHandle, strKey, pStringValue);
}

int CMvCamera::SetStringValue(IN const char* strKey, IN const char* strValue)
{
    return MV_CC_SetStringValue(m_hDevHandle, strKey, strValue);
}

// ch:执行一次Command型命令，如 UserSetSave，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
// en:Execute Command once, such as UserSetSave, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int CMvCamera::CommandExecute(IN const char* strKey)
{
    return MV_CC_SetCommandValue(m_hDevHandle, strKey);
}

// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
int CMvCamera::GetOptimalPacketSize(unsigned int* pOptimalPacketSize)
{
    if (MV_NULL == pOptimalPacketSize)
    {
        return MV_E_PARAMETER;
    }

    int nRet = MV_CC_GetOptimalPacketSize(m_hDevHandle);
    if (nRet < MV_OK)
    {
        return nRet;
    }

    *pOptimalPacketSize = (unsigned int)nRet;

    return MV_OK;
}

// ch:注册消息异常回调 | en:Register Message Exception CallBack
int CMvCamera::RegisterExceptionCallBack(void(__stdcall* cbException)(unsigned int nMsgType, void* pUser), void* pUser)
{
    return MV_CC_RegisterExceptionCallBack(m_hDevHandle, cbException, pUser);
}

// ch:注册单个事件回调 | en:Register Event CallBack
int CMvCamera::RegisterEventCallBack(const char* pEventName, void(__stdcall* cbEvent)(MV_EVENT_OUT_INFO* pEventInfo, void* pUser), void* pUser)
{
    return MV_CC_RegisterEventCallBackEx(m_hDevHandle, pEventName, cbEvent, pUser);
}

// ch:强制IP | en:Force IP
int CMvCamera::ForceIp(unsigned int nIP, unsigned int nSubNetMask, unsigned int nDefaultGateWay)
{
    return MV_GIGE_ForceIpEx(m_hDevHandle, nIP, nSubNetMask, nDefaultGateWay);
}

// ch:配置IP方式 | en:IP configuration method
int CMvCamera::SetIpConfig(unsigned int nType)
{
    return MV_GIGE_SetIpConfig(m_hDevHandle, nType);
}

// ch:设置网络传输模式 | en:Set Net Transfer Mode
int CMvCamera::SetNetTransMode(unsigned int nType)
{
    return MV_GIGE_SetNetTransMode(m_hDevHandle, nType);
}

// ch:像素格式转换 | en:Pixel format conversion
int CMvCamera::ConvertPixelType(MV_CC_PIXEL_CONVERT_PARAM_EX* pstCvtParam)
{
    return MV_CC_ConvertPixelTypeEx(m_hDevHandle, pstCvtParam);
}

// ch:保存图片 | en:save image
int CMvCamera::SaveImage(MV_SAVE_IMAGE_PARAM_EX3* pstParam)
{
    return MV_CC_SaveImageEx3(m_hDevHandle, pstParam);
}

// ch:保存图片为文件 | en:Save the image as a file
int CMvCamera::SaveImageToFile(MV_SAVE_IMAGE_TO_FILE_PARAM_EX* pstSaveFileParam)
{
    return MV_CC_SaveImageToFileEx(m_hDevHandle, pstSaveFileParam);
}

// ch:绘制圆形辅助线 | en:Draw circle auxiliary line
int CMvCamera::DrawCircle(MVCC_CIRCLE_INFO* pCircleInfo)
{
    return MV_CC_DrawCircle(m_hDevHandle, pCircleInfo);
}

// ch:绘制线形辅助线 | en:Draw lines auxiliary line
int CMvCamera::DrawLines(MVCC_LINES_INFO* pLinesInfo)
{
    return MV_CC_DrawLines(m_hDevHandle, pLinesInfo);
}


// ch:输出设备信息
static bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
        printf("DeviceVersion: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chDeviceVersion);
        printf("ManufacturerName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chManufacturerName);


    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}


static std::string getUserDefinedName(MV_CC_DEVICE_INFO* pstMVDevInfo) {
    if (NULL == pstMVDevInfo) {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return "";
    }
    unsigned char* devicename = pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber;

    return std::string((const char*)&(devicename[0]), sizeof devicename);

}

static std::string getDeviceVendorName(MV_CC_DEVICE_INFO* pstMVDevInfo) {
    if (NULL == pstMVDevInfo) {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return "";
    }
    unsigned char* devicename = pstMVDevInfo->SpecialInfo.stUsb3VInfo.chVendorName;

    return std::string((const char*)&(devicename[0]), sizeof devicename);

}

static std::string getCameraIP(MV_CC_DEVICE_INFO* pstMVDevInfo) {
    if (NULL == pstMVDevInfo) {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return "";
    }
    std::string IP = "";
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp &
            0xff000000) >>
            24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp &
            0x00ff0000) >>
            16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp &
            0x0000ff00) >>
            8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp &
            0x000000ff);
        IP.append(std::to_string(nIp1) + "." + std::to_string(nIp2) +
            "." + std::to_string(nIp2) + "." +
            std::to_string(nIp2));

    }

    return IP;
}

// ch:返回可用相机 | en:Get Accessable Camera
std::string CMvCamera::getAccessableCamera(MV_CC_DEVICE_INFO_LIST* deviceList) {


    //枚举设备
    int nRet = EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, deviceList);
    std::string res = "[";
    if (nRet != MV_OK) {
        printf("Enum Devices Fail! nRet [0x%x]\n", nRet);
    }

    if (deviceList->nDeviceNum > 0) {
        for (int i = 0; i < deviceList->nDeviceNum; i++) {
            //printf("[Device %d]:", i);
            bool access = IsDeviceAccessible(deviceList->pDeviceInfo[i], 1);
            //printf("Accessable %d\n", access);
            std::string IP = getCameraIP(deviceList->pDeviceInfo[i]);
            //获得相机参数
            Open(deviceList->pDeviceInfo[i]);
            MVCC_INTVALUE_EX width = { 0 };
            nRet = GetIntValue("Width", &width);
            if (nRet != MV_OK)
                printf("Get Width Fail! nRet [0x%x]\n", nRet);
            MVCC_INTVALUE_EX height = { 0 };
            nRet = GetIntValue("Height", &height);
            if (nRet != MV_OK)
                printf("Get height Fail! nRet [0x%x]\n", nRet);
            MVCC_INTVALUE_EX maxWidth = { 0 };
            nRet = GetIntValue("WidthMax", &maxWidth);
            if (nRet != MV_OK)
                printf("Get maxWidth Fail! nRet [0x%x]\n", nRet);
            MVCC_INTVALUE_EX maxHeight = { 0 };
            nRet = GetIntValue("HeightMax", &maxHeight);
            if (nRet != MV_OK)
                printf("Get maxHeight Fail! nRet [0x%x]\n", nRet);
            MVCC_FLOATVALUE exposureTime = { 0 };
            nRet = GetFloatValue("ExposureTime", &exposureTime);
            if (nRet != MV_OK)
                printf("Get exposureTime  Fail! nRet [0x%x]\n", nRet);
            Close();

            if (IP == "")
                res.append(fmt::format(
                    R"({{ "deviceId" : "{}" , 
"accessable" : "{}", 
"userDefinedName" : "{}", 
"deviceVendorName" : "{}",
"width" : "{}",
"height" : "{}",
"widthMax" : "{}",
"heightMax" : "{}",
"exposureTime" : "{}"}},)",
i, access, getUserDefinedName(deviceList->pDeviceInfo[i]), getDeviceVendorName(deviceList->pDeviceInfo[i]),
width.nCurValue, height.nCurValue, maxWidth.nCurValue, maxHeight.nCurValue, exposureTime.fCurValue));
            else
                res.append(fmt::format(
                    R"({{ "deviceId" : "{}" , 
"accessable" : "{}", 
"userDefinedName" : "{}" ,
"deviceVendorName" : "{}", 
"IP" : "{}",
"width" : "{}",
"height" : "{}",
"widthMax" : "{}",
"heightMax" : "{}",
"exposureTime" : "{}"}},)",
i, access, getUserDefinedName(deviceList->pDeviceInfo[i]), getDeviceVendorName(deviceList->pDeviceInfo[i]), IP, width.nCurValue, height.nCurValue, maxWidth.nCurValue,
maxHeight.nCurValue, exposureTime.fCurValue));
            //PrintDeviceInfo(deviceList->pDeviceInfo[i]);

            //if (access) res.push_back(i);
            Close();
        }
        if (res.back() == ',') res.pop_back();

    }
    res.append("]");
    return res;
}


int CMvCamera::GetOneFrameTimeout(
    unsigned char* pData,
    unsigned int  nDataSize,
    MV_FRAME_OUT_INFO_EX* pstFrameInfo,
    unsigned int  nMsec
) {
    return MV_CC_GetOneFrameTimeout(m_hDevHandle, pData, nDataSize, pstFrameInfo, nMsec);
}