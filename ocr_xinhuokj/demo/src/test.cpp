#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include <iostream>
#include "scan_pen.h"
#include<vector>
#include<fstream>

int main(int argc,char *argv[]){
	cv::Mat img = cv::imread(argv[1]);//传入的图像
	std::string model_path = argv[3];//传入的模型
    std::string key_path = argv[2];//传入的Key
	
	spen::SpenEngine spen_engine;
    int ret = spen_engine.Init(key_path, model_path);//加载授权文件key（正常：返回值>0）
	printf("SpenSDK Init %d\n", ret);

	//判断返回值ret当前状态
    std::string msg ;
    if (ret == AUTH_TIME_TRY) {
        msg = "有效期试用授权";
    } else if (ret == AUTH_ERROR_KEY_NOT_FOUND) {
         msg = "授权文件打开失败， 功能限制的试用授权";
    } else if (ret < 0) {
        printf("[ERROR]授权错误: ");
        switch (ret) {
            case AUTH_FAILED:
                printf("授权文件与设备ID不符\n");
                break;
            case INIT_MODEL_FAILED:
                printf("模型加载失败\n");
                break;
            case AUTH_READ_DEVICE_ID_ERROR:
                printf("设备ID读取出错\n");
                break;
        }
        return 0;
    }else{
        msg = "设备授权成功，初始化成功";
    }
    printf("%s\n",msg.c_str());
	

	spen::Frame frame;
	//根据传入图形创建一个frame对象
	frame.buf =  img.data;
	frame.width = img.cols;
	frame.height = img.rows;
	frame.format = spen::BGR;
	std::vector<spen::LineItem> line_items; //创建一个LineItem类型的容器


	ret = spen_engine.DetectLines(frame, line_items);//检测文本行，放入line_items容器
	std::cout<<"DetectLines:"<<ret<<std::endl;
	std::cout<<"图片行数："<<line_items.size()<<std::endl;//图片多少行

	spen_engine.SaveFrameData("test.txt");

	cv::Mat img_clone = img.clone();//拷贝一张原图img
	std::ofstream out_file("out.txt");//创建一个txt文件
	//遍历容器对文本进行画线
	for(int i = 0; i <line_items.size();i++){
			std::vector<cv::Point2f> src_corners(4);//创建4个2维float型矩阵
			for (int j = 0; j < 4; j++) {
				src_corners[j].x = line_items[i].vertexes[j].x;//vertexes文本行的4个顶点坐标，左上角顺时针排序
				src_corners[j].y = line_items[i].vertexes[j].y;
				// std::cout<<line_items[i].vertexes[j].x<<","<<line_items[i].vertexes[j].y<<std::endl;
				cv::line(img_clone, cv::Point(line_items[i].vertexes[j].x, line_items[i].vertexes[j].y), cv::Point(line_items[i].vertexes[(j + 1) % 4].x, line_items[i].vertexes[(j + 1) % 4].y), cv::Scalar(0, 0, 255), 1);
			}
			
			int w =line_items[i].vertexes[1].x-line_items[i].vertexes[0].x;
			int h = line_items[i].vertexes[3].y-line_items[i].vertexes[0].y;
			std::vector<cv::Point2f> dst_corners(4);
			dst_corners[0].x = 0;
			dst_corners[0].y = 0;
			dst_corners[1].x = w;
			dst_corners[1].y = 0;
			dst_corners[2].x = w;
			dst_corners[2].y = h;
			dst_corners[3].x = 0;
			dst_corners[3].y = h;

			//对图像做一个透视变换（斜矩形------>正矩形）
				cv::Mat warpmatrix = cv::getPerspectiveTransform(src_corners, dst_corners);
				cv::Mat sub_img = cv::Mat::zeros(h, w, CV_8UC3);
				cv::warpPerspective(img, sub_img, warpmatrix, sub_img.size());
				cv::imwrite(std::to_string(i)+".bmp",sub_img);
				
				spen::Frame line_image;// 拼接剪裁好的单行文本图像。
				line_image.buf = sub_img.data ;//文本行图像的像素
				line_image.width = sub_img.cols; // 文本行图像的宽度
				line_image.height = sub_img.rows;// 文本行图像的高度
				line_image.format = spen::BGR;// 支持 GRAY， RGB，BGR格式。
		

				//进行文本识别，存入line_items
				spen_engine.RecogLine( line_image, line_items[i].chars);
				std::cout<<"line_items长度:"<<line_items[i].chars.size()<<std::endl;//多少个字
				for(spen::CharItem item: line_items[i].chars){//遍历打印文本信息
					out_file.write(item.ch.c_str(), item.ch.length());
					printf("%s",item.ch.c_str());
				}
				out_file<<std::endl;
				out_file.close();
				printf("\n");
				printf("\n");
				printf("\n");
				printf("\n");

	}
	cv::imwrite("result.bmp",img_clone);
}