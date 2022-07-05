
#pragma once
#include <stdint.h>
#include <string>
#include <string.h>
#include <vector>
#include <memory>

namespace spen {

 const char* GetVersion();
#ifndef SPEN_CAMERA_FRAME
#define SPEN_CAMERA_FRAME
enum FrameFormat{
  NV12,
  NV21,
  GRAY,
  BGR,
  RGB,
};
struct Frame{
    uint8_t* buf;
    int width;
    int height;
    int32_t sequence;
    FrameFormat format;
    // format?
    void release(){
      if(buf!=nullptr){
        delete [] buf;
        width = 0;
        height = 0;
      }
    }
    void create(int w, int h, FrameFormat f, uint8_t* ptr){
      width = w;
      height =h;
      format = f;
      buf = new uint8_t[w*h];
      if(ptr){
        memcpy(buf, ptr, w*h);
      }
    }
};
#endif

#ifndef OCR_CHAR_ITEM
#define OCR_CHAR_ITEM
  struct Pointf {
    float x, y;
  };
 struct CharItem{
        std::string ch;
        float score;
        float cx;
        float cy;
        float lx;
        float rx;
        // points?
        Pointf points[4];
    };
  struct LineItem{
        std::vector<CharItem> chars;
        Pointf vertexes[4]; // 文本行4个顶点坐标，依次从左上x,y 顺时针排序
        int flag=0;
        float score; 
    };
#define OCR_MODEL_FAILED -201
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


struct Image{
    Image(){
      width = 0;
      height = 0;
    }
    Image(const Image&) =delete;
    // Image& operator=( Image& img){
    //   width = img.width;
    //   height = img.height;
    //   data = std::move(img.data);
    //   format = img.format;
    //   img.width = 0;
    //   img.height =0;
    //   return *this;
    // };
    Image& operator=(const Image&)=delete;
    std::unique_ptr<uint8_t[]> data;
    int width;
    int height;
    FrameFormat format; // only GRAY
    bool empty(){
      return width*height==0;
    }
};

class Spen;

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
  int ret = Spen::OnLineAuth("ClientID",”SpenSDK“， "/internal_storage/spen/","");
  if(ret<0){
    // error 
  }


  // 扫描程序每次启动
  SpenEngine spen;
  ret =spen.Init("","/path/of/model");
  // or
  ret = spen.Init("/internal_storage/spen/spen.key","/path/of/model");
  if(ret<0){
    // error 
  }
}*/
int OnLineAuth(const std::string& vendor_id, const std::string& sdk_name, const std::string& data_storage_dir, const std::string& sn_key="");
enum LogLevel{
  NONE = 0,
  ERROR =1,
  INFO = 2,
  DEBUG = 3,
};
// 设置log 打印级别，默认Error
void SetLogLevel(LogLevel level);


#ifdef ANDROID
// 高版本Android系统获取SN方法需要，需要在 Init()前调用
void setJNIEnv(void* env);
#endif 
struct ScanConfig{
  struct {
    int left=0;          // 剪裁区域 左上角顶点 x坐标（图像左上角为原点）
    int top=0;           // 剪裁区域 左上角顶点 y坐标（图像左上角为原点）
    int width=0;         // 剪裁区域宽度
    int height=0;        // 剪裁区域高度
  } crop_area;                 // 帧图像有效区域参数  crop_area的坐标相对于输入的图像的。 推荐在相机驱动这进行剪裁,默认不做剪裁
  int rotate_angle=0;          //帧图像旋转角度，可选 0,90,180,270。 默认0。
  int first_frames_ignore = 3; // 扫描时，要丢弃的开始的帧数（相机刚打开前面几帧曝光不正常，过黑或过白）。 默认丢弃开始的3帧图像.
  int last_frames_ignore = 4;  // 扫描结束时 要丢弃掉最后面（抬笔过程中图像异常）的帧数。默认丢弃最后4帧图像
  bool fast_mode = false;      // 快速识别模式,抬笔后快速出结果，默认为false
  int slow_detect = 0;         // 0默认， 1: 检测变慢但更准确（适合非常小行距的情况）
};



enum LangCode{
  Default, // *,ALL
  Symbol, // sy 半角符号
  Number,
  English, // en
  Chinese, // zh
  ChineseSimplified, // zh_cn
  ChineseTraditional,// zh_tw or zh_hk
  Japanese, // ja  日语
  Korean, //ko 韩语
  Latin,  // rm 拉丁语
  France,  // fr 法语
  Spanish, //es 西班牙语
  Swedish, //sv 瑞典语
  //... 
  Portuguese,
  German,  //de 德语(标准)
  Italian, // it 意大利语
  Russian,  // ru
  Vietnamese, // vi 越南语
  Indonesia, //in 印尼
  Thai, //th 泰语
  Denish, // de 丹麦语
  Dutch, //nl 荷兰语
  Finnish,// fi 芬兰语
  Undefined, // 未支持
};
struct LangType{
  LangCode lang;
  float score;
};


class SpenEngine{
public:  
  SpenEngine();
  ~SpenEngine();

  // 从文件加载授权文件，和ocr模型，
  // key_file_path : 授权文件路径
  // 模型文件路径
  // 返回 >=0 初始化成功，AUTH_FAILED 授权失败
  int Init(const std::string& key_file_path, const std::string& model);

  // 从内存加载授权文件，和ocr模型，
  // key_data ，key_data_size: 授权文件数据，及数据大小
  // modelsize: 模型文件内容，及大小
  // 返回 >=0 初始化成功
  int Init(const char* key_data, size_t key_data_size, const char* model, size_t size);

  // 调用此函数，将会将视频帧数据保存到文件 data_file_path 中
  // 在Begin()之前调用该函数。
  // 仅用于Debug代码，切勿用于线上产品。
  // data_file_path：目标文件地址（不是目录地址）
  int SaveFrameData(const char* data_file_path);

  // 设置影响性能相关参数
  // struct ScanConfig{
  //  struct {
  //    int left;
  //    int top;
  //    int width;
  //    int height;
  //  } crop_area;                 // 剪裁参数： 帧图像中有效区域, 推荐在相机驱动这进行剪裁,默认不做剪裁
  //  int rotate_angle=0;          // 帧图像旋转角度，可选 0,90,180,270。 默认0。
  //  int crop_area[4]={0,0,0,0};  // 帧图像有效区域{left, top ,width,height}. 推荐在相机驱动这进行剪裁,默认不做剪裁
  //  int first_frames_ignore = 3; // 扫描时，要丢弃的开始的帧数（相机刚打开前面几帧曝光不正常，过黑或过白）。 默认丢弃开始的3帧图像.
  //  int last_frames_ignore = 4;  // 扫描结束时 要丢弃掉最后面（抬笔过程中图像异常）的帧数。默认丢弃最后4帧图像
  //  bool fast_mode = false;      // 实验功能： 快速识别模式,抬笔后快速出结果，默认为false
  //};
  void Config(ScanConfig config);
  // 开启 新一行图像扫描识别
  // precised ： true:精准扫描  只识别滑过的文字， false：识别所有扫描的文字
  // left_hand： true，为左手模式， false，为右手模式， 
  int Begin(bool precised, bool left_hand);

  // 追加一帧图像，相机获得的帧图像数据，只支持GRAY, NV12, NV21格式，last: false 为中间帧，true为最后一帧
  // return  返回已经拼接的图像的宽度, 
  //        如果 返回 -1，表示Push前没有调用Begin()
  //        如果 返回 -2，表示扫描长度过长（超过5000像素宽度）,不再接收新图像帧进行拼接(正常用户使用中不会超过这个限制)
  int Push(const Frame& frame, bool last);

  // 获得当前拼接剪裁的文本行图像, 在快扫模式（ScanConfig.fast=true) 返回已拼接的原始图像。用于Debug 调试
  int GetImage(Image& output);

  // 获得当前拼接的原始图像 , 用于Debug 调试
  int GetRawImage(Image& output);

  // 边扫边识别模式，获取已扫图像的的ocr识别结果，last: false 为中间帧，true为最后一帧
  // return 1 :表示文字结果相比上次有更新，0文字结果跟上次调用相同
  int GetSliceOcrResult(std::string& output, bool last);

  // 扫完再识别模式，
  // fast_mode 为false时，获取当前拼好的整张图像做OCR， 返回OCR结果
  //           为true时，使用中间结果作为最终结果。
  int GetOcrResult(std::string& output);
  
  // 识别文本行图像，支持NV12，NV21,BGR,RGB,GRAY图像格式
  int RecogLine(const Frame& line_image, std::vector<CharItem>& char_items);
  
  // 检测文本行，支持NV12，NV21,BGR,RGB,GRAY图像格式
  int DetectLines(const Frame& line_image, std::vector<LineItem>& line_items);

  // 全文识别
  int RecogImage(const Frame& line_image, std::vector<LineItem>& line_items);

  // 判断文本行图像文字方向,用于判断横向文字，和竖向文字（如日语）
  int CheckTextDirection(const Frame& line_image, int & dir, float& score);
  

  // 判断字符串语言类型，仅支持 英语(LangCode::English)，中语LangCode::Chinese)，日语(LangCode::Japanese)，韩语(LangCode::Korean),和无法判断(LangCode::Default)
  // 返回结果保存在LangType.lang(LangCode中)
  int CheckTextLanguage(const std::string text,LangType& outtype);
  // 一行扫描结束，释放相关内存
  void End();
private:
  Spen* impl;

};

 int64_t now() ;
 int64_t now_us() ;
}  // namespace spen
