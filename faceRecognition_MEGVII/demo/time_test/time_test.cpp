//
// Created by megvii on 19-6-24.
//
#include "argparse.h"
#include "tool.h"

#define Crimson cv::Scalar(245, 240, 255)
#define MediumBlue cv::Scalar(205, 0, 0)
#define CornflowerBlue cv::Scalar(237, 149, 100)
#define Cyan cv::Scalar(255, 255, 0)
#define Maroon cv::Scalar(113, 179, 60)
#define BrulyWood cv::Scalar(135, 184, 222)

#define _BLUE cv::Scalar(255, 0, 0)

#define _RED cv::Scalar(114, 38, 249)
#define MID_GREEN cv::Scalar(94, 252, 103)
#define _GREEN cv::Scalar(0, 252, 0)
#define _YELLOW cv::Scalar(0, 255, 255)
#define Magenta cv::Scalar(255, 0, 255)

FaceRetCode wakeupNpu(int width, int height, PixelFormat format);

void sdkProcess(std::vector<FaceHandle> &faceHandleArr, const Image &image,
                const std::string &writePath) {
    FaceRetCode retCode;
    /// create image to draw result
    cv::Mat img;
    if (Format_BGR888 == image.pixel_format) {
        img =
            cv::Mat(image.height, image.width, CV_8UC3, image.vir_addr).clone();
    } else if (Format_YUV420SP_NV21 == image.pixel_format) {
        cv::Mat nv21(image.height * 3 / 2, image.width, CV_8UC1,
                     image.vir_addr);
        cv::cvtColor(nv21, img, cv::COLOR_YUV2BGR_NV21);
    }

    for (size_t i = 0; i < faceHandleArr.size(); ++i) {
        FaceRect rect;
        {
            auto sp = cw::ProfilerFactory::get()->make("getFaceRect");
            retCode = getFaceRect(faceHandleArr[i], &rect);
        }
        if (RET_OK == retCode) {
            printf(
                "[FACEPASS_TEST] getFaceRect: "
                "left=%d--top==%d--right=%d--bottom=%d\n",
                rect.left, rect.top, rect.right, rect.bottom);
        } else {
            printf("[FACEPASS_TEST] can not getFaceRect!\n");
        }

        Landmark landmark;
        retCode = getFaceLandmark(faceHandleArr[i], &landmark);
        printf("[FACEPASS_TEST] getFaceLandmark Score:%f\n", landmark.score);
        // if need fine landmark
        {
            auto sp = cw::ProfilerFactory::get()->make("refineLandmark");
            retCode = refineLandmark(&image, faceHandleArr[i], &landmark);
        }

        FacePoseBlur poseBlur;
        {
            auto sp = cw::ProfilerFactory::get()->make("getPoseBlurAttribute");
            retCode = getPoseBlurAttribute(&image, faceHandleArr[i], &poseBlur);
        }
        if (RET_OK == retCode) {
            printf(
                "[FACEPASS_TEST] "
                "getPoseBlurAttribute:roll=%f--yaw=%f--pitch=%f--blur=%f\n",
                poseBlur.roll, poseBlur.yaw, poseBlur.pitch, poseBlur.blur);
        } else {
            printf("[FACEPASS_TEST] can not getPoseBlurAttribute!\n");
        }

        float livenessScore = 0;
        {
            auto sp = cw::ProfilerFactory::get()->make("getLiveness_bgr");
            retCode = getLiveness_bgr(&image, faceHandleArr[i], &livenessScore);
        }
        if (RET_OK == retCode) {
            printf("[FACEPASS_TEST] getLiveness_bgr:%f\n", livenessScore);
        } else {
            printf("[FACEPASS_TEST] can not detect liveness!\n");
        }

        FaceAttr faceAttr;
        {
            auto sp = cw::ProfilerFactory::get()->make("getFaceAttrResult");
            retCode = getFaceAttrResult(&image, faceHandleArr[i], &faceAttr);
        }
        if (RET_OK == retCode) {
            printf("[FACEPASS_TEST] getFaceAttrResult:age=%f\n", faceAttr.age);
            printf(
                "[FACEPASS_TEST] "
                "getFaceAttrResult:gender[0]=%f--gender[1]=%f\n",
                faceAttr.gender[0], faceAttr.gender[1]);
            std::cout << "hat=[" << faceAttr.hat[0] << ", " << faceAttr.hat[1]
                      << ", " << faceAttr.hat[2] << ", " << faceAttr.hat[3]
                      << "]" << std::endl;
            std::cout << "respirator=[" << faceAttr.respirator[0] << ", "
                      << faceAttr.respirator[1] << ", "
                      << faceAttr.respirator[2] << ", "
                      << faceAttr.respirator[3] << "]" << std::endl;

            std::cout << "glasses=[" << faceAttr.glasses[0] << ", "
                      << faceAttr.glasses[1] << ", " << faceAttr.glasses[2]
                      << ", " << faceAttr.glasses[3] << "]" << std::endl;
        } else {
            printf("[FACEPASS_TEST] can not getFaceAttrResult!\n");
        }

        char *feature;
        int featureLength;
        {
            auto sp = cw::ProfilerFactory::get()->make("extract");
            retCode =
                extract(&image, faceHandleArr[i], &feature, &featureLength);
        }
        if (RET_OK != retCode) {
            printf("[FACEPASS_TEST] can not extract feature!\n");
        }

        float compare_result;
        {
            auto sp = cw::ProfilerFactory::get()->make("compare");
            retCode = compare(feature, feature, featureLength, &compare_result);
        }
        if (RET_OK == retCode) {
            printf("[FACEPASS_TEST] compare feature with itself:%f\n",
                   compare_result);
        } else {
            printf("[FACEPASS_TEST] error occurred during compare feature!\n");
        }
        releaseFeature(feature);
        feature = nullptr;

        /*FaceOccl faceOccl;
          {
          auto sp = cw::ProfilerFactory::get()->make("getFaceOcclResult");
          retCode = getFaceOcclResult(faceHandleArr[i], &faceOccl);
          }

          if(RET_OK == retCode){
          std::cout<<"leftEyeOccl=["<<faceOccl.leftEye[0]<<",
          "<<faceOccl.leftEye[1]<<", "
          <<faceOccl.leftEye[2]<<", "<<faceOccl.leftEye[3]<<"]"<<std::endl;
          std::cout<<"rightEyeOccl=["<<faceOccl.rightEye[0]<<",
          "<<faceOccl.rightEye[1]<<", "
          <<faceOccl.rightEye[2]<<", "<<faceOccl.rightEye[3]<<"]"<<std::endl;
          std::cout<<"noseOccl=["<<faceOccl.nose[0]<<", "<<faceOccl.nose[1]<<",
          "
          <<faceOccl.nose[2]<<"]"<<std::endl;
          std::cout<<"headOccl=["<<faceOccl.head[0]<<", "<<faceOccl.head[1]<<",
          "
          <<faceOccl.head[2]<<"]"<<std::endl;
          std::cout<<"leftCheekOccl=["<<faceOccl.leftCheek[0]<<",
          "<<faceOccl.leftCheek[1]<<", "
          <<faceOccl.leftCheek[2]<<"]"<<std::endl;
          std::cout<<"rightCheekOccl=["<<faceOccl.rightCheek[0]<<",
          "<<faceOccl.rightCheek[1]<<", "
          <<faceOccl.rightCheek[2]<<"]"<<std::endl;
          std::cout<<"mouthAndChinOccl=["<<faceOccl.mouthAndChin[0]<<",
          "<<faceOccl.mouthAndChin[1]<<", "
          <<faceOccl.mouthAndChin[2]<<"]"<<std::endl;

          }*/

        /// draw result in image

        cv::rectangle(img, cv::Point(rect.left, rect.top),
                      cv::Point(rect.right, rect.bottom), _GREEN, 2);

        for (auto item : landmark.points) {
            cv::circle(img, cv::Point((int)item.x, (int)item.y), 2, _BLUE);
        }

        char text[2048];
        int font_face = cv::FONT_HERSHEY_SIMPLEX;
        double font_scale = 0.5;
        int thickness = 1;
        cv::Point origin;

        int text_origin_x = rect.right + 10;
        int text_origin_y = rect.top;
        if (img.cols < rect.right + 80) {
            text_origin_x = rect.left;
            text_origin_y = rect.bottom + 10;
        }
        int text_stride = 20;
        sprintf(text, "lmk:%.2f", landmark.score);
        origin.x = text_origin_x;
        origin.y = text_origin_y;
        cv::putText(img, text, origin, font_face, font_scale, _RED, thickness,
                    8, 0);

        sprintf(text, "blur:%.2f", poseBlur.blur);
        origin.x = text_origin_x;
        origin.y += text_stride;
        cv::putText(img, text, origin, font_face, font_scale, _RED, thickness,
                    8, 0);

        sprintf(text, "yaw:%.2f", poseBlur.yaw);
        origin.x = text_origin_x;
        origin.y += text_stride;
        cv::putText(img, text, origin, font_face, font_scale, _RED, thickness,
                    8, 0);

        sprintf(text, "pitch:%.2f", poseBlur.pitch);
        origin.x = text_origin_x;
        origin.y += text_stride;
        cv::putText(img, text, origin, font_face, font_scale, _RED, thickness,
                    8, 0);

        sprintf(text, "age:%.0f", faceAttr.age);
        origin.x = text_origin_x;
        origin.y += text_stride;
        cv::putText(img, text, origin, font_face, font_scale, MID_GREEN,
                    thickness, 8, 0);

        if (faceAttr.gender[0] > 0.5)
            sprintf(text, "gender:male:%.2f", faceAttr.gender[0]);
        else
            sprintf(text, "gender:female:%.2f", faceAttr.gender[0]);
        origin.x = text_origin_x;
        origin.y += text_stride;
        cv::putText(img, text, origin, font_face, font_scale, MID_GREEN,
                    thickness, 8, 0);

        sprintf(text, "live:%.3f", livenessScore);
        origin.x = text_origin_x;
        origin.y += text_stride;
        cv::putText(img, text, origin, font_face, font_scale, Magenta,
                    thickness, 8, 0);

        auto resp_predict = [](float preds[], size_t num) {
            std::vector<std::string> classes = {"mn", "m", "no", "un"};
            size_t idx = 0;
            float probability = 0;
            for (size_t i = 0; i < num; ++i) {
                if (probability < preds[i]) {
                    probability = preds[i];
                    idx = i;
                }
            }
            return std::make_pair(classes[idx], probability);
        };
        const auto resp = resp_predict(faceAttr.respirator, 4);
        sprintf(text, "resp %s:%.2f", resp.first.c_str(), resp.second);
        origin.x = text_origin_x;
        origin.y += text_stride;
        const cv::Scalar deeppink(147, 20, 255);
        cv::putText(img, text, origin, font_face, font_scale, deeppink,
                    thickness, 8, 0);

        auto glass_predict = [](float preds[], size_t num) {
            std::vector<std::string> classes = {"no", "blk", "oth", "un"};
            size_t idx = 0;
            float probability = 0;
            for (size_t i = 0; i < num; ++i) {
                if (probability < preds[i]) {
                    probability = preds[i];
                    idx = i;
                }
            }
            return std::make_pair(classes[idx], probability);
        };
        const auto glasses = glass_predict(faceAttr.glasses, 4);
        sprintf(text, "%s glass:%.2f", glasses.first.c_str(), glasses.second);
        origin.x = text_origin_x;
        origin.y += text_stride;
        cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
                    thickness, 8, 0);
        /*auto occl_predict = [](float preds[], size_t num) {
          std::vector<std::string> classes = {"no", "oc", "un"};
          size_t idx = 0;
          float probability = 0;
          for(size_t i=0;i<num;++i){
          if(probability < preds[i]){
          probability = preds[i];
          idx = i;
          }
          }
          return std::make_pair(classes[idx], probability);
          };
          const auto head = predict(faceOccl.head, 3);
          sprintf(text, "head %s:%.2f", head.first.c_str(), head.second);
          origin.x = text_origin_x;
          origin.y += text_stride;
          cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
          thickness, 8, 0);

          const auto leftEye = occl_predict(faceOccl.leftEye, 3);
          sprintf(text, "leftEye %s:%.2f", leftEye.first.c_str(),
          leftEye.second); origin.x = text_origin_x; origin.y += text_stride;
          cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
          thickness, 8, 0);

          const auto rightEye = occl_predict(faceOccl.rightEye, 3);
          sprintf(text, "rightEye %s:%.2f", rightEye.first.c_str(),
          rightEye.second); origin.x = text_origin_x; origin.y += text_stride;
          cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
          thickness, 8, 0);

          const auto nose = occl_predict(faceOccl.nose, 3);
          sprintf(text, "nose %s:%.2f", nose.first.c_str(), nose.second);
          origin.x = text_origin_x;
          origin.y += text_stride;
          cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
          thickness, 8, 0);

          const auto leftCheek = occl_predict(faceOccl.leftCheek, 3);
          sprintf(text, "leftCheek %s:%.2f", leftCheek.first.c_str(),
          leftCheek.second); origin.x = text_origin_x; origin.y += text_stride;
          cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
          thickness, 8, 0);

          const auto rightCheek = occl_predict(faceOccl.rightCheek, 3);
          sprintf(text, "rightCheek %s:%.2f", rightCheek.first.c_str(),
          rightCheek.second); origin.x = text_origin_x; origin.y += text_stride;
          cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
          thickness, 8, 0);

          const auto mouth = occl_predict(faceOccl.mouthAndChin, 3);
          sprintf(text, "mouth %s:%.2f", mouth.first.c_str(), mouth.second);
          origin.x = text_origin_x;
          origin.y += text_stride;
          cv::putText(img, text, origin, font_face, font_scale, _YELLOW,
          thickness, 8, 0);*/
    }
    printf("[FACEPASS_TEST] write path : %s\n", writePath.c_str());
    cv::imwrite(writePath, img);
}
FaceRetCode wakeupNpu(int width, int height, PixelFormat format) {
    float elem_size = 3;
    if (Format_BGR888 == format) {
        elem_size = 3;
    } else if (Format_YUV420SP_NV21 == format) {
        elem_size = 1.5;
    }
    std::vector<uint8_t> dummy =
        std::vector<uint8_t>(static_cast<int>(height * width * elem_size));
    const Image image = {dummy.data(), nullptr, -1,     width,
                         height,       width,   height, format};
    int nCount = 128; /*max number of detected faces*/
    std::vector<FaceHandle> faceHandleArr(nCount, nullptr);

    FaceRetCode retCode =
        detect(&image, 0.75, MIN_FACE, faceHandleArr.data(), &nCount);

    for (int i = 0; i < nCount; ++i) {
        releaseFace(faceHandleArr[i]);
    }
    return retCode;
}
void imageProcess(const std::string &imagePath,
                  const std::string &outputFolder) {
    printf("[FACEPASS_TEST] process image path : %s\n", imagePath.c_str());

    auto img = cv::imread(imagePath, cv::IMREAD_COLOR);

    if (img.empty()) {
        printf("[FACEPASS_TEST] load image error!!\n");
        assert(0);
    }

    int nCount = 128; /*max number of detected faces*/
    std::vector<FaceHandle> faceHandleArr(nCount, nullptr);
    const Image image = {img.data, nullptr,  -1,       img.cols,
                         img.rows, img.cols, img.rows, Format_BGR888};

    // wakeupNpu(image.width, image.height, image.pixel_format);

    FaceRetCode retCode;
    {
        auto sp = cw::ProfilerFactory::get()->make("detect");
        retCode = detect(&image, 0.75, MIN_FACE, faceHandleArr.data(), &nCount);
    }
    if (RET_NO_FACE == retCode) {
        printf("[FACEPASS_TEST]  detect no faces in %s!\n", imagePath.c_str());
    } else if (RET_OK == retCode) {
        printf("[FACEPASS_TEST] image width=%d--height=%d\n", image.width,
               image.height);
        printf("[FACEPASS_TEST] detect %d faces \n", nCount);
        faceHandleArr.resize(nCount);
        const std::string writePath =
            cw::Path::join(outputFolder, cw::Path::get_name(imagePath));
        sdkProcess(faceHandleArr, image, writePath);
        for (int i = 0; i < nCount; ++i) {
            releaseFace(faceHandleArr[i]);
        }

    } else {
        printf("[FACEPASS_TEST] error occurred during detect!\n");
    }
}

void nv21Process(const std::string &imagePath, const std::string &outputFolder,
                 int width, int height) {
    printf("[FACEPASS_TEST] process image path : %s\n", imagePath.c_str());
    std::vector<char> data = ImageUtil::readFile(imagePath);
    if (data.empty()) {
        printf("[FACEPASS_TEST] load image error!!\n");
        assert(0);
    }
    printf("[FACEPASS_TEST] image width=%d--height=%d\n", width, height);

    int nCount = 128; /*max number of detected faces*/
    std::vector<FaceHandle> faceHandleArr(nCount, nullptr);
    const Image image = {static_cast<void *>(data.data()),
                         nullptr,
                         -1,
                         width,
                         height,
                         width,
                         height,
                         Format_YUV420SP_NV21};
    // wakeupNpu(image.width, image.height, image.pixel_format);
    FaceRetCode retCode;
    {
        auto sp = cw::ProfilerFactory::get()->make("detect");
        retCode = detect(&image, 0.75, MIN_FACE, faceHandleArr.data(), &nCount);
    }
    if (RET_NO_FACE == retCode) {
        printf("[FACEPASS_TEST]  detect no faces in %s!\n", imagePath.c_str());
    } else if (RET_OK == retCode) {
        printf("[FACEPASS_TEST] detect %d faces \n", nCount);
        faceHandleArr.resize(nCount);
        auto writePath = cw::Path::join(
            outputFolder, cw::Path::get_basename(imagePath) + ".png");
        sdkProcess(faceHandleArr, image, writePath);
        for (int i = 0; i < nCount; ++i) {
            releaseFace(faceHandleArr[i]);
        }
    } else {
        printf("[FACEPASS_TEST] error occurred during detect!\n");
    }
}

void binaryProcess(const std::string &imagePath,
                   const std::string &outputFolder, int width, int height) {
    printf("[FACEPASS_TEST] process image path : %s\n", imagePath.c_str());
    std::vector<char> data = ImageUtil::readFile(imagePath);
    if (data.empty()) {
        printf("[FACEPASS_TEST] load image error!!\n");
        assert(0);
    }
    printf("[FACEPASS_TEST] image width=%d--height=%d\n", width, height);

    int nCount = 128; /*max number of detected faces*/
    std::vector<FaceHandle> faceHandleArr(nCount, nullptr);
    const Image image = {static_cast<void *>(data.data()),
                         nullptr,
                         -1,
                         width,
                         height,
                         width,
                         height,
                         Format_BGR888};
    // wakeupNpu(image.width, image.height, image.pixel_format);
    FaceRetCode retCode;
    {
        auto sp = cw::ProfilerFactory::get()->make("detect");
        retCode = detect(&image, 0.75, MIN_FACE, faceHandleArr.data(), &nCount);
    }
    if (RET_NO_FACE == retCode) {
        printf("[FACEPASS_TEST]  detect no faces in %s!\n", imagePath.c_str());
    } else if (RET_OK == retCode) {
        printf("[FACEPASS_TEST] detect %d faces \n", nCount);
        auto writePath = cw::Path::join(
            outputFolder, cw::Path::get_basename(imagePath) + ".png");
        sdkProcess(faceHandleArr, image, writePath);
        for (int i = 0; i < nCount; ++i) {
            releaseFace(faceHandleArr[i]);
        }
    } else {
        printf("[FACEPASS_TEST] error occurred during detect!\n");
    }
}

static void help() {
    std::cout
        << "\nThis program demostrates basic interface facepass sdk\n"
           "and give some important interface running time.\n"
           "It prints some basic information, and draw face rect and landmark "
           "for every face handle.\n"
           "Usage: \n"
           "nv21 and binary image format, need setting width and height, "
           "default 1080*1920\n"
           "./time_test model.yaml -r bgr -i ./image.jpg -o ./output/\n"
           "./time_test model.yaml -r nv21 -i ./input/ -o ./output/ -w width "
           "-h height\n"
           "./time_test model.yaml -r binary -i ./input/ -o ./output/ -w "
           "-width -h height\n\n";
}

int main(int argc, char *argv[]) {
    help();

    ArgumentParser parser("FACEPASS_DV300_TEST TIME TEST.");
    parser.add_positional_arg("model_config", "1", "The path of model.yaml");
    parser.add_keyword_arg("-r", "--run_option", "", true, "",
                           "bgr is run bgr image test, nv21 is run nv21 image "
                           "test, binary is run binary format image test");
    parser.add_keyword_arg("-i", "--input_path", "", true, "",
                           "the path or folder for input image");
    parser.add_keyword_arg("-o", "--output_path", "", true, "",
                           "the folder for write image");
    parser.add_keyword_arg("-w", "--width", "", false, "1080",
                           "the width of input image");
    parser.add_keyword_arg("-h", "--height", "", false, "1920",
                           "the height of input image");

    ArgumentResult args = parser.parse_args(argc, argv);

    const auto modelConfig = args.args["model_config"];
    const auto runOption = args.args["run_option"];
    const auto imagePath = args.args["input_path"];
    const auto outputFolder = args.args["output_path"];
    const auto width = std::stoi(args.args["width"]);
    const auto height = std::stoi(args.args["height"]);

    // setLogLevel(LOG_LEVEL_INFO);
    setLogLevel(LOG_LEVEL_ERROR);

    if (initSDK(modelConfig) != 0) {
        printf("[FACEPASS_TEST] sdk init error! \n");
        release();
        return 1;
    };

    cw::File path(imagePath);
    if (path.exists()) {
        printf("[FACEPASS_TEST] input imagePath %s \n", imagePath.c_str());
    } else {
        printf("[FACEPASS_TEST] imagePath %s is not exists! \n",
               imagePath.c_str());
        return 1;
    }

    if (runOption == "bgr") {
        if (path.is_dir()) {
            const std::vector<std::string> images =
                path.list_file_by_ext_recursive(
                    {".jpg", ".bmp", ".png", ".jpeg"});
            printf("[FACEPASS_TEST] %d images in imagePath %s\n", images.size(),
                   imagePath.c_str());
            for (auto &imageFile : images) {
                imageProcess(imageFile, outputFolder);
            }
        } else {
            imageProcess(imagePath, outputFolder);
        }
    } else if (runOption == "nv21") {
        if (path.is_dir()) {
            const std::vector<std::string> images =
                path.list_file_by_ext_recursive({".yuv"});
            printf("[FACEPASS_TEST] %d images in imagePath %s\n", images.size(),
                   imagePath.c_str());
            for (auto &imageFile : images) {
                nv21Process(imageFile, outputFolder, width, height);
            }
        } else {
            nv21Process(imagePath, outputFolder, width, height);
        }
    } else if (runOption == "binary") {
        if (path.is_dir()) {
            const std::vector<std::string> images =
                path.list_file_by_ext_recursive({".bmp"});
            printf("[FACEPASS_TEST] %d images in imagePath %s\n", images.size(),
                   imagePath.c_str());
            for (auto &imageFile : images) {
                binaryProcess(imageFile, outputFolder, width, height);
            }
        } else {
            binaryProcess(imagePath, outputFolder, width, height);
        }
    }

    ::release();
    return 0;
}
