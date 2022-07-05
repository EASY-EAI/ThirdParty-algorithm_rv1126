
#pragma once
// 请参考C++头文件scan_pen.h
// 请参考C++头文件scan_pen.h
// 请参考C++头文件scan_pen.h
#ifndef SCAN_PEN_C_API
#define  SCAN_PEN_C_API
#include <stdint.h>
#include <stdbool.h>


#define MAX_BYTE_IN_LINE 1024*3
#ifdef __cplusplus
extern "C" {
#endif

#define RET_OK 0
#define AUTH_OK RET_OK                // 授权成功，模型加载成功
// Init 返回 错误码 授权状态(错误码)
#define INIT_OK AUTH_OK                // 授权成功，模型加载成功
#define INIT_MODEL_FAILED -201        // 模型加载失败
#define AUTH_FAILED -100              // 授权失败，授权文件与设备不符
#define AUTH_TIME_TRY 1               // 有时间限制的试用授权，在一定日期前，可以使用
#define AUTH_ERROR_KEY_NOT_FOUND -104   // 授权文件未找到, 功能限制的试用授权
#define AUTH_READ_DEVICE_ID_ERROR -106   // 无法读取设备ID

// OnLineAuth 返回错误码
#define AUTH_EXCEED_MAX_COUNT -101    // 在线授权失败，达到设备数量限制
#define AUTH_NETWORK_ERROR -102       //在线授权失败，网络错误
#define AUTH_SERVER_ERROR -103        // 在线授权失败，服务器异常
#define AUTH_FILE_SAVE_ERROR -105        // 授权文件本地保存错误

// spen_CheckTextLanguage 函数返回的语言码定义
#define LangCode_Default 0
#define LangCode_English 3
#define LangCode_Chinese 4
#define LangCode_Japanese 7
#define LangCode_Korean 8

struct ScanCfg{
  struct {
    int left;// =0;          // 剪裁区域 左上角顶点 x坐标（图像左上角为原点）
    int top;// =0;           // 剪裁区域 左上角顶点 y坐标（图像左上角为原点）
    int width;// =0;         // 剪裁区域宽度，<=0默认不剪裁
    int height;// =0;        // 剪裁区域高度，<=0默认不剪裁
  } crop_area;                 // 帧图像有效区域参数  crop_area的坐标相对于输入的图像的。 推荐在相机驱动这进行剪裁,默认不做剪裁
  int rotate_angle;// =0;          //帧图像旋转角度，可选 0,90,180,270。 默认0。
  int first_frames_ignore;//  = 3; // 扫描时，要丢弃的开始的帧数（相机刚打开前面几帧曝光不正常，过黑或过白）。 默认丢弃开始的3帧图像.
  int last_frames_ignore;//  = 4;  // 扫描结束时 要丢弃掉最后面（抬笔过程中图像异常）的帧数。默认丢弃最后4帧图像
  int fast_mode;//  = false;      // 快速识别模式,抬笔后快速出结果，默认为0,开启为1
  int slow_detect;                // 0默认， 1: 检测变慢但更准确（适合非常小行距的情况）
};

#define LEVEL_NONE    0
#define LEVEL_ERROR   1
#define LEVEL_INFO    2
#define LEVEL_DEBUG   3

void spen_SetLogLevel(int level);

/*
// 联网授权, 发送设备ID(MAC地址)到授权服务器，获取授权文件。
// 如果检测到已获得授权文件（判断第三个参数目录中是否有合法授权文件），则立即返回，不会重复联网授权。
// 因为由联网请求，需要再后台线程中调用
// vendor_id:  客户ID，咨询技术支持获取
// sdk_name:  参数值可选如下：如果为完整扫描笔SDK，"SpenSDK"。 如果只购买文本行识别，为"SpenOcrSDK"。
// data_storeage_dir， 授权文件存储目录，
// sn_key ,当使用mac地址时，传空字符串，如果时使用Android 的SN时，则为sn的KEY名称, 默认为"ro.serialno"
// return 0 成功 <0 失败。
// 授权成功，会把授权文件保存再  {data_storeage_dir}/spen.key 文件中。
// 联网授权成功后。 仍需要调用函数spen.Init(key_file_path,model) 初始化引擎，但第一个参数可以为空，也可以为{data_storeage_dir}/spen.key
// 示例代码：
{
// 设备第一次启动
  int ret = spen_OnLineAuth("ClientID",”SpenSDK“， "/internal_storage/spen/",NULL);
  // or android 平台使用SN
  int ret = spen_OnLineAuth("ClientID",”SpenSDK“， "/internal_storage/spen/","android SN key name");

  if(ret<0){
    // error 
  }


  // 扫描程序每次启动
  void* spen;
  ret =spen_Init(&spen, "","/path/of/model");
  // or
  ret = spen_Init(&spen, "/internal_storage/spen/spen.key","/path/of/model");
  if(ret<0){
    // error 
  }
}*/
  int spen_OnLineAuth(const char* vendor_id, const char* sdk_name, const char* data_storage_dir, const char* sn_key);


  // 加载ocr模型，
  // key_file_path : 授权文件路径
  // 模型文件路径
  // 返回 >=0 初始化成功
  int spen_Init(void** engine, const char*  key_file_path, const char* model);

  // 从内存加载授权文件，和ocr模型，
  // key_data ，key_data_size: 授权文件数据，及数据大小
  // modelsize: 模型文件内容，及大小
  // 返回 >=0 初始化成功
  int spen_InitMem(void** engine, const char* key_data, long key_data_size,const char* model_ptr, int model_bytes);

  // 由于部分C编译器不支持结构体默认成员赋值，需调用此函数 设置ScanCfg 配置项默认值，
  void spen_config_init(struct ScanCfg* config);

  // config 需要首先调用spen_config_init(&config) 初始化默认值，然后修改要改变的值，再调用spen_Config()函数。
  void spen_Config(void* engine, struct ScanCfg config);
 // 开启 新一行图像扫描识别
 // precised : 1:精准扫描（只识别滑过的字（透明罩左侧的））， 0： 非精准扫描
 // left_hand 0： 右手模式，1：左手模式
  int spen_Begin(void* engine, int precised, int left_hand);

 // 追加一帧图像，相机获得的帧图像数据，只支持GRAY, NV12, NV21格式，last: false 为中间帧，true为最后一帧
  // return  返回已经拼接的图像的宽度, 如果 返回 -1，表示Push前没有调用Begin()
  int spen_Push(void* engine, uint8_t* yuv_buf, int width, int height, int last);

 // 扫完再识别模式，获取当前拼好的整张图像做OCR， 返回OCR结果
  // utf8_buf : char[MAX_BYTE_IN_LINE]
  // return 1 :表示文字结果相比上次有更新，0文字结果跟上次调用相同
  int spen_GetOcrResult(void* engine, char* utf8_buf);

 // 边扫边识别模式，获取已扫图像的的ocr识别结果
  // utf8_buf : char[MAX_BYTE_IN_LINE]
  int spen_GetSliceOcrResult(void* engine,  char* utf8_buf, int last);

  // 获得当前拼接的原始图像 , 用于Debug 调试
  // ptr, store the memory address of image pixels, release memory by delete [] *ptr; 
  // width, height :save the size of image.
  int spen_GetRawImage(void* engine, uint8_t**ptr, int* width, int* height);

  // 调用此函数，将会将Begin()及End()之间Push()的视频帧数据保存到文件 data_file_path 中
  // 在Begin()之前调用该函数。
  // 仅用于Debug代码，切勿用于线上产品。
  int spen_SaveFrameData(void* engine, const char* data_file_path);

  // 一行扫描结束，释放相关内存
  void spen_End(void* engine);

  // 判断字符串 语言类型，只支持仅支持 英语(LangCode_English)，中语 LangCode_Chinese)，日语(LangCode_Japanese)，韩语(LangCode_Korean),和无法判断(LangCode_Default)
  // tyoe  存放语言码
  // score 存放置信度（仅作参考）
  int spen_CheckTextLanguage(void* engine,const char* str, int* type,float*score);

  // 释放整个引擎
  void spen_Release(void* engine);




// camera API 仅用于Demo
#define CAMERA_OK 0;
#define CAMERA_OPEN_FAILED  -1;
#define CAMERA_IOCTL_FAILED  -2;
#define CAMERA_NOT_GOOD -3;
#define CAMERA_MMAP_FAILED -4;
#define CAMERA_PREVIEW_FAILED -5;

  struct YuvFrame {
    uint8_t* buf;
    int width;
    int height;
    uint32_t sequence;
    // format?
  };

  // return: 0 ok
  int spen_camera_Open(void** camera, const char* device, int width,
                       int height);

  int spen_camera_Start(void* camera);

  int spen_camera_ReadFream(void* camera, struct YuvFrame* frame);

  int spen_camera_Stop(void* camera);

  int spen_camera_Close(void* camera);

#ifdef __cplusplus
}
#endif
#endif
