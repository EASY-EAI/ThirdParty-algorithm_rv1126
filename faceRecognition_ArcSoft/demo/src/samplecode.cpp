#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "arcsoft_face_sdk.h"
#include "amcomdef.h"
#include "asvloffscreen.h"
#include "merror.h"
#include <opencv2/opencv.hpp>
using namespace std;
//测试数据，从开发者中心获取替换
#define APPID "6WukHBZE94yYUJEKErsrsnRf3hJ23jzuSqRb7DuSqDvJ"
#define SDKKEY "8YAoFCM6aZ6ww4qySauiN8aL4MitCGzpYCx1cuE1adhf"
#define ACTIVEKEY "0985-1162-F216-9VDV"  
#define NSCALE 27
#define FACENUM	10  //支持检测的人脸数不超过10个
#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; } 
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; }

int ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen);
void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize);
void RGB2NV21(string infile, char* outfile);

void printSDKInfo()
{
    printf("\n************* ArcFace SDK Info *****************\n");

    MRESULT res = MOK;

    // res = ASFOnlineActivation(APPID, SDKKEY, ACTIVEKEY);
     res = ASFOfflineActivation("./key/09851162F2169VDV.dat"); //激活文件路径
    if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res)
        printf("激活失败错误码: %x\n", res);
    else
        printf("************设备激活成功************（返回值：%x）\n", res);
    //采集当前设备信息，用于离线激活（可注释）
  
  /*  char* deviceInfo = NULL;
    res = ASFGetActiveDeviceInfo(&deviceInfo);
    if (res != MOK) {
        printf("ASFGetActiveDeviceInfo failed: %x\n", res);
    } else {
        printf("ASFGetActiveDeviceInfo sucess: %s\n", deviceInfo);
    }

    */
    //获取激活文件信息
    ASF_ActiveFileInfo activeFileInfo = { 0 };
    res = ASFGetActiveFileInfo(&activeFileInfo);
    if (res != MOK){
        printf("获取激活文件信息【失败】（返回值: %x）\n", res);
    } else {
        //这里仅获取了有效期时间，还需要其他信息直接打印即可
        char startDateTime[32];
        timestampToTime(activeFileInfo.startTime, startDateTime, 32);
        printf("激活文件开始时间: %s\n", startDateTime);
        char endDateTime[32];
        timestampToTime(activeFileInfo.endTime, endDateTime, 32);
        printf("激活文件结束时间: %s\n", endDateTime);
    }

    //SDK版本信息
    const ASF_VERSION version = ASFGetVersion();
    printf("*************SDK版本信息*************\n");
    printf("\nVersion:%s\n", version.Version);
    printf("BuildDate:%s\n", version.BuildDate);
    printf("CopyRight:%s\n", version.CopyRight);
}


int main(int argc, char* argv[])
{
    //激活与打印一些信息
    printSDKInfo();
    RGB2NV21("./images/test.jpeg", "./images/test1.NV21");
    RGB2NV21("./images/test2.jpeg", "./images/test2.NV21");
    RGB2NV21("./images/test2.jpeg", "./images/test3.NV21");
	printf("\n************* Face Recognition *****************\n");
	MRESULT res = MOK;

	//初始化引擎
	MHandle handle = NULL;
    MInt32 initMask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_IMAGEQUALITY |
            ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE | ASF_LIVENESS | ASF_IR_LIVENESS |
            ASF_FACESHELTER | ASF_MASKDETECT | ASF_FACELANDMARK;
     //图片模式；脸部角度：常规预览下正方向；人脸占比：27；最多检测人脸个数：10；检测的功能；返回引擎值：handle
	res = ASFInitEngine(ASF_DETECT_MODE_IMAGE, ASF_OP_0_ONLY,
	        NSCALE, FACENUM, initMask, &handle);
	if (res != MOK)
		printf("初始化引擎【失败】（返回值：%x）\n", res);
	else
		printf("初始化引擎【成功:】（返回值：%x）\n", res);


	/*********以下三张图片均存在，图片保存在 ./images/ 文件夹下*********/
	
	//可见光图像 NV21格式裸数据（YUV420，NV21）
	char* picPath1 = "./images/test1.NV21";
     int Width1 = 720;
    int Height1 = 720;
    // char* picPath1 = "../images/640x480_1.NV21";
    // int Width1 = 640;
    // int Height1 = 480;
	int Format1 = ASVL_PAF_NV21;
	MUInt8* imageData1 = (MUInt8*)malloc(Height1*Width1*3/2);
	FILE* fp1 = fopen(picPath1, "rb");

	//可见光图像 NV21格式裸数据
	char* picPath2 = "./images/test2.NV21";
    int Width2 = 700;
	int Height2 = 700;
    // char* picPath2 = "../images/640x480_2.NV21";
    // int Width2 = 640;
    // int Height2 = 480;
	int Format2 = ASVL_PAF_NV21;
	MUInt8* imageData2 = (MUInt8*)malloc(Height2*Width2*3/2);
	FILE* fp2 = fopen(picPath2, "rb");
	
	//红外图像 NV21格式裸数据
	char* picPath3 = "./images/test3.NV21";
    int Width3 = 720;
	int Height3 = 720;
    // char* picPath3 = "../images/640x480_3.NV21";
    //  int Width3 = 640;
    // int Height3 = 480;
	int Format3 = ASVL_PAF_GRAY;
	MUInt8* imageData3 = (MUInt8*)malloc(Height3*Width3);	//只读NV21前2/3的数据为灰度数据（Y平面）
	FILE* fp3 = fopen(picPath3, "rb");
	if (fp1 && fp2 && fp3)
	{
		fread(imageData1, 1, Height1*Width1*3/2, fp1);	//读NV21裸数据
		fclose(fp1);
		fread(imageData2, 1, Height2*Width2*3/2, fp2);	//读NV21裸数据
		fclose(fp2);
		fread(imageData3, 1, Height3*Width3, fp3);		//读NV21前2/3的数据,用于红外活体检测（Y数据）
		fclose(fp3);
        //第一张人脸
        ASVLOFFSCREEN offscreen1 = {0};
        //图片格式转换
        ColorSpaceConversion(Width1, Height1, ASVL_PAF_NV21, imageData1, offscreen1);

        ASF_MultiFaceInfo detectedFaces1 = {0};//多人脸信息
        ASF_SingleFaceInfo SingleDetectedFaces = {0};//单人脸信息
        ASF_FaceFeature feature1 = {0};//人脸特征信息
        ASF_FaceFeature copyfeature1 = {0};//人脸特征信息


        //传入引擎，图片数据，传出人脸信息结构体
        res = ASFDetectFacesEx(handle, &offscreen1, &detectedFaces1);//图片信息转LPASF_MultiFaceInfo结构体数据
        if (res != MOK || detectedFaces1.faceNum < 1) {
            printf("%s 人脸检测（注册照）【失败】（返回值： %x）\n", picPath1, res);
        } else {
            //传入图片的结构体信息
            SingleDetectedFaces.faceRect.left = detectedFaces1.faceRect[0].left;
            SingleDetectedFaces.faceRect.top = detectedFaces1.faceRect[0].top;
            SingleDetectedFaces.faceRect.right = detectedFaces1.faceRect[0].right;
            SingleDetectedFaces.faceRect.bottom = detectedFaces1.faceRect[0].bottom;
            SingleDetectedFaces.faceOrient = detectedFaces1.faceOrient[0];

            // 第一张认为是注册照，注册照要求不戴口罩
            //传入引擎，图片数据，传入：图片结构体信息，传出：人脸特征，用于注册人脸，mask=0（无口罩）
            res = ASFFaceFeatureExtractEx(handle, &offscreen1, &SingleDetectedFaces, &feature1, ASF_REGISTER, 0);
            if (res != MOK) {
                printf("%s 人脸特征提取（注册照）【失败】（返回值: %x）\n", picPath1, res);
            } else {
                //拷贝feature，否则第二次进行特征提取，会覆盖第一次特征提取的数据，导致比对的结果为1
                copyfeature1.featureSize = feature1.featureSize;
                copyfeature1.feature = (MByte *) malloc(feature1.featureSize);
                memset(copyfeature1.feature, 0, feature1.featureSize);
                memcpy(copyfeature1.feature, feature1.feature, feature1.featureSize);
                printf("%s 人脸特征提取（注册照）【成功】（返回值: %x）\n", picPath1, res);
            }
        }


        //设置遮挡检测阈值
        MFloat ShelterThreshhold = 0.8f;
        //传入引擎，遮挡阈值
        res = ASFSetFaceShelterParam(handle, ShelterThreshhold);
        if (res != MOK) {
            printf("设置遮挡阈值【失败】（返回值: %x）\n", res);
        }

        //第二张人脸
        ASVLOFFSCREEN offscreen2 = {0};
        ColorSpaceConversion(Width2, Height2, ASVL_PAF_NV21, imageData2, offscreen2);//图像格式转换

        ASF_MultiFaceInfo detectedFaces2 = {0};
        ASF_FaceFeature feature2 = {0};
        ASF_MaskInfo maskInfo = {0};              //是否带口罩
        res = ASFDetectFacesEx(handle, &offscreen2, &detectedFaces2);//图片信息转LPASF_MultiFaceInfo结构体数据
        if (res != MOK || detectedFaces2.faceNum < 1) {
            printf("%s 人脸检测（识别照）【失败】（返回值： %x）\n", picPath2, res);
        } else {
            SingleDetectedFaces.faceRect.left = detectedFaces2.faceRect[0].left;
            SingleDetectedFaces.faceRect.top = detectedFaces2.faceRect[0].top;
            SingleDetectedFaces.faceRect.right = detectedFaces2.faceRect[0].right;
            SingleDetectedFaces.faceRect.bottom = detectedFaces2.faceRect[0].bottom;
            SingleDetectedFaces.faceOrient = detectedFaces2.faceOrient[0];

            MFloat imageQualityConfidenceLevel;
            //图像质量检测
            res = ASFImageQualityDetectEx(handle, &offscreen2, &SingleDetectedFaces, &imageQualityConfidenceLevel);
            if (res == MOK) {
                printf("图像质量检测（识别照）【成功】（返回值: %f）\n", imageQualityConfidenceLevel);
            } else {
                printf("图像质量检测（识别照）【失败】（返回值:%x）\n", res);
            }

            // 检测是否戴口罩
            MInt32 proMask = ASF_MASKDETECT;
            res = ASFProcessEx(handle, &offscreen2, &detectedFaces2, proMask);
            if (res != MOK) {
                printf("检测是否带口罩（识别照）【失败】（返回值: %x）\n", res);
            } else {
                printf("检测是否带口罩（识别照）【成功】（返回值: %x）\n", res);
            }

            // 获取是否戴口罩
            res = ASFGetMask(handle, &maskInfo);
            if (res != MOK) {
                printf("获取是否戴口罩（识别照）【失败】（返回值: %x）\n", res);
            } else {
                printf("获取是否戴口罩（识别照）【成功】（0：代表没有带口罩，1：代表带口罩 ,-1：代表不确定）（返回值: %d）\n", maskInfo.maskArray[0]);
            }

            // 作为预览模式下的识别照
            //引擎，图像数据，图像结构体，人脸特征信息，用于识别照，口罩检测相关
            res = ASFFaceFeatureExtractEx(handle, &offscreen2, &SingleDetectedFaces, &feature2,
                                          ASF_RECOGNITION, maskInfo.maskArray[0]);
            if (res != MOK) {
                printf("%s 人脸特征提取（识别照）【失败 】（返回值：%x）\n", picPath2, res);
            } else {
                printf("%s 人脸特征提取（识别照）【成功 】（返回值：%x）\n", picPath2, res);
            }
        }

        //比对之前可加业务逻辑
        //口罩	遮挡	提示语
        // 0	 0	    请佩戴口罩
        // 0	 1	    请佩戴口罩
        // 1	 0	    请正确佩戴
        // 1	 1	    佩戴正确，可识别

        // 单人脸特征比对
        MFloat confidenceLevel;
        //引擎，注册照片，识别照片，传出：比较结果，无口罩，生活照
        res = ASFFaceFeatureCompare(handle, &copyfeature1, &feature2, &confidenceLevel,
                maskInfo.maskArray[0], ASF_LIFE_PHOTO);
        if (res != MOK) {
            printf("识别照与注册照进行人脸特征比对【失败】（返回值:  %x）\n", res);
        } else {
            printf("识别照与注册照进行人脸特征比对【成功】（返回值: %lf）\n", confidenceLevel);
        }

        printf("\n************* Face Process *****************\n");
        //设置活体置信度 SDK内部默认值为 IR：0.7  RGB：0.5（无特殊需要，可以不设置）
        ASF_LivenessThreshold threshold = {0};
        threshold.thresholdmodel_BGR = 0.5;
        threshold.thresholdmodel_IR = 0.7;
        res = ASFSetLivenessParam(handle, &threshold);//设置活体置信度
        if (res != MOK) {
            printf("设置RGB活体检测置信度【失败】（返回值: %x）\n", res);
        }

        // 人脸信息检测
        MInt32 processMask = ASF_FACE3DANGLE | ASF_LIVENESS | ASF_AGE | ASF_GENDER |
                             ASF_FACESHELTER | ASF_MASKDETECT | ASF_FACELANDMARK;
        res = ASFProcessEx(handle, &offscreen2, &detectedFaces2, processMask);
        if (res != MOK)
            printf("人脸特征提取（识别照）【失败】（返回值: %x）\n", res);
        else
            printf("人脸特征提取（识别照）【成功】（返回值: %x）\n", res);

        // 获取年龄
        ASF_AgeInfo ageInfo = {0};
        res = ASFGetAge(handle, &ageInfo);
        if (res != MOK || ageInfo.num < 1)
            printf("%s 获取年龄【失败】（返回值: %x）\n", picPath2, res);
        else
            printf("%s 获取年龄【成功】（返回值: %d）\n", picPath2, ageInfo.ageArray[0]);

        // 获取性别
        ASF_GenderInfo genderInfo = {0};
        res = ASFGetGender(handle, &genderInfo);
        if (res != MOK || genderInfo.num < 1)
            printf("%s 获取性别【失败】（返回值: %x）\n", picPath2, res);
        else
            printf("%s  获取性别【成功】（0：男 ，1：女）（返回值 :%d）\n", picPath2, genderInfo.genderArray[0]);

        // 获取3D角度
        ASF_Face3DAngle angleInfo = {0};
        res = ASFGetFace3DAngle(handle, &angleInfo);
        if (res != MOK || angleInfo.num < 1)
            printf("%s 获取人脸3D角度【失败】（返回值: %x）\n", picPath2, res);
        else
            printf("%s获取人脸3D角度【成功】（返回值：  roll: %lf  yaw: %lf  pitch: %lf）\n", picPath2, angleInfo.roll[0],
                   angleInfo.yaw[0], angleInfo.pitch[0]);

        //获取活体信息
        ASF_LivenessInfo rgbLivenessInfo = {0};
        res = ASFGetLivenessScore(handle, &rgbLivenessInfo);
        if (res != MOK || rgbLivenessInfo.num < 1)
            printf("获取RGB活体检测【失败】（返回值: %x）\n", res);
        else
            printf("获取RGB活体检测【成功】（1：真人，0：非真人）（返回值: %d）\n", rgbLivenessInfo.isLive[0]);

        //获取遮挡结果
        ASF_FaceShelter rgbFaceShelterInfo = {0};
        res = ASFGetFaceShelter(handle, &rgbFaceShelterInfo);
        if (res != MOK || rgbFaceShelterInfo.num < 1)
            printf("获取脸部遮挡【失败】（返回值: %x）\n", res);
        else
            printf("获取脸部遮挡【成功】（1:表示遮挡, 0 :表示未遮挡）（返回值: %d）\n", rgbFaceShelterInfo.FaceShelter[0]);

        //获取口罩检测结果
        ASF_MaskInfo rgbMaskInfo = {0};
        res = ASFGetMask(handle, &rgbMaskInfo);
        if (res != MOK || rgbMaskInfo.num < 1)
            printf("获取口罩检测结果【失败】（返回值: %x）\n", res);
        else
            printf("获取口罩检测结果【成功】（0:代表没有带口罩，1:代表带口罩）（返回值: %d）\n", rgbMaskInfo.maskArray[0]);

        //获取额头区域
        ASF_LandMarkInfo headLandMarkInfo = {0};
        res = ASFGetFaceLandMark(handle, &headLandMarkInfo);
        if (res != MOK)
            printf("获取额头区域数组长度【失败】（返回值: %x）\n", res);
        else
            printf("获取额头区域数组长度【成功】（返回值: %d）\n", headLandMarkInfo.num);


        printf("\n**********IR LIVENESS*************\n");

        //第三张人脸IR图像
        ASVLOFFSCREEN offscreen3 = {0};
        ColorSpaceConversion(Width3, Height3, ASVL_PAF_GRAY, imageData3, offscreen3);

        ASF_MultiFaceInfo detectedFaces3 = {0};
        for(int i = 0; i < 100; i++)
        {
            res = ASFDetectFacesEx(handle, &offscreen3, &detectedFaces3);
            if(res == MOK && detectedFaces3.faceNum){
                break;
            }
        }
        if (res != MOK)
            printf("第三张图片获取人脸数量【失败】（返回值: %x \n", res);
        else
            printf("第三张图片获取人脸数量【成功】（返回值: %d）\n", detectedFaces3.faceNum);

        //IR图像活体检测
        MInt32 processIRMask = ASF_IR_LIVENESS;
        res = ASFProcessEx_IR(handle, &offscreen3, &detectedFaces3, processIRMask);
        if (res != MOK)
            printf("IR图像活体检测【失败】（返回值: %x）\n", res);
        else
            printf("IR图像活体检测【成功】（返回值: %x）\n", res);

        //获取IR活体信息
        ASF_LivenessInfo irLivenessInfo = {0};
        res = ASFGetLivenessScore_IR(handle, &irLivenessInfo);
        if (res != MOK || irLivenessInfo.num < 1)
            printf("获取IR活体信息【失败】（返回值: %x）\n", res);
        else
            printf("获取IR活体信息【成功】（返回值: %d）\n", irLivenessInfo.isLive[0]);

        //释放内存
        SafeFree(copyfeature1.feature);
        SafeArrayDelete(imageData1);
        SafeArrayDelete(imageData2);
        SafeArrayDelete(imageData3);

		//反初始化
		res = ASFUninitEngine(handle);
		if (res != MOK)
			printf("销毁引擎【失败】（返回值： %x）\n", res);
		else
			printf("销毁引擎【成功】（返回值： %x）\n", res);
	}
	else
	{
		printf("没有找到图片.\n");
	}
    return 0;
}


//时间戳转换为日期格式
void timestampToTime(char* timeStamp, char* dateTime, int dateTimeSize)
{
    time_t tTimeStamp = atoll(timeStamp);
    struct tm* pTm = gmtime(&tTimeStamp);
    strftime(dateTime, dateTimeSize, "%Y-%m-%d %H:%M:%S", pTm);
}

//图像颜色格式转换
int ColorSpaceConversion(MInt32 width, MInt32 height, MInt32 format, MUInt8* imgData, ASVLOFFSCREEN& offscreen)
{//(Width1, Height1, ASVL_PAF_NV21, imageData1, offscreen1);
    offscreen.u32PixelArrayFormat = (unsigned int)format;
    offscreen.i32Width = width;
    offscreen.i32Height = height;

    switch (offscreen.u32PixelArrayFormat)
    {
        case ASVL_PAF_RGB24_B8G8R8:
            offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
            offscreen.ppu8Plane[0] = imgData;
            break;
        case ASVL_PAF_I420:
            offscreen.pi32Pitch[0] = width;
            offscreen.pi32Pitch[1] = width >> 1;
            offscreen.pi32Pitch[2] = width >> 1;
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
            offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
            break;
        case ASVL_PAF_NV12:
        case ASVL_PAF_NV21:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
            offscreen.ppu8Plane[0] = imgData;
            offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
            break;
        case ASVL_PAF_YUYV:
        case ASVL_PAF_DEPTH_U16:
            offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
            offscreen.ppu8Plane[0] = imgData;
            break;
        case ASVL_PAF_GRAY:
            offscreen.pi32Pitch[0] = offscreen.i32Width;
            offscreen.ppu8Plane[0] = imgData;
            break;
        default:
            return 0;
    }
    return 1;
}


void RGB2NV21(string infile, char* outfile){
	cv::Mat Img = cv::imread(infile);
	FILE  *fp = fopen(outfile, "wb");

	if (Img.empty()){
		std::cout << "empty!check your image";
		return;
	}
	int cols = Img.cols;
	int rows = Img.rows;
 
	int Yindex = 0;
	int UVindex = rows * cols;
 
	unsigned char* yuvbuff = new unsigned char[int(1.5 * rows * cols)];
 
	cv::Mat OpencvYUV;
	cv::Mat OpencvImg;
	cv::cvtColor(Img, OpencvYUV, CV_BGR2YUV_YV12);
	
	int UVRow{ 0 };
	for (int i=0;i<rows;i++){
		for (int j=0;j<cols;j++){
			int B = Img.at<cv::Vec3b>(i, j)[0];
			int G = Img.at<cv::Vec3b>(i, j)[1];
			int R = Img.at<cv::Vec3b>(i, j)[2];

			int Y = (77 * R + 150 * G + 29 * B) >> 8;
			yuvbuff[Yindex++] = (Y < 0) ? 0 : ((Y > 255) ? 255 : Y);
			if (i%2==0&&(j)%2==0)
			{
				int U = ((-44 * R - 87 * G + 131 * B) >> 8) + 128;
				int V = ((131 * R - 110 * G - 21 * B) >> 8) + 128;
				yuvbuff[UVindex++] = (V < 0) ? 0 : ((V > 255) ? 255 : V);
				yuvbuff[UVindex++] = (U < 0) ? 0 : ((U > 255) ? 255 : U);
			}
		}
	}
    for (int i=0;i< 1.5 * rows * cols;i++){
	    fwrite(&yuvbuff[i], 1, 1, fp);
    }
    fclose(fp);
}








