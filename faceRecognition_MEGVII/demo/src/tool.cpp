//
// Created by megvii on 19-6-24.
//

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//#include "config.h"
//#include "face_sdk.h"
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "tool.h"

namespace cw {
ProfilerFactory ProfilerFactory::s_instance;

std::string Path::get_panrent_dir(std::string path) {
    path = normalize_path(path);
    size_t pos = path.rfind("/");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(0, pos);
}

std::string Path::get_name(std::string path) {
    path = normalize_path(path);
    size_t pos = path.rfind("/");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1, path.size());
}

std::string Path::get_basename(std::string path) {
    std::string name = get_name(path);
    size_t pos = name.rfind(".");
    if (pos == std::string::npos) {
        return name;
    }
    return name.substr(0, pos);
}

std::string Path::get_basename_path(std::string path) {
    size_t pos = path.rfind(".");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(0, pos);
}

std::string Path::get_extname(std::string path) {
    std::string name = get_name(path);
    size_t pos = name.rfind(".");
    if (pos == std::string::npos) {
        return "";
    }
    return name.substr(pos, name.size());
}

std::string Path::join(std::string path, std::string name) {
    return normalize_path(path) + "/" + name;
}

std::string Path::normalize_path(const std::string &path) {
    std::string ret = path;
    if (ret[ret.size() - 1] == '/') {
        ret = ret.substr(0, ret.size() - 1);
    }
    return ret;
}

std::vector<std::string> File::list_file() {
    std::vector<std::string> ret;
    auto d = opendir(m_path.c_str());
    if (d) {
        for (auto dir = readdir(d); dir != NULL; dir = readdir(d)) {
            ret.push_back(Path::join(m_path, dir->d_name));
        }
        closedir(d);
    }
    return ret;
}

std::vector<std::string> File::list_file_by_ext(std::vector<std::string> exts) {
    std::vector<std::string> ret;
    for (auto &ext : exts) {
        ext = to_uppercase(ext);
    }
    for (auto &p : list_file()) {
        std::string ext_name = to_uppercase(Path::get_extname(p));
        if (find(exts.begin(), exts.end(), ext_name) != exts.end()) {
            ret.push_back(p);
        }
    }
    return ret;
}

std::vector<std::string> File::list_file_by_ext_recursive(
    std::vector<std::string> exts) {
    std::vector<std::string> ret;
    for (auto &ext : exts) {
        ext = to_uppercase(ext);
    }
    for (auto &path : list_file()) {
        File file(path);
        if (file.is_dir()) {
            std::string basename = Path::get_name(path);
            if (basename == "." || basename == "..") {
                continue;
            }
            auto list = file.list_file_by_ext_recursive(exts);
            ret.insert(ret.end(), list.begin(), list.end());
            continue;
        }
        std::string ext_name = to_uppercase(Path::get_extname(path));
        if (find(exts.begin(), exts.end(), ext_name) != exts.end()) {
            ret.push_back(path);
        }
    }

    return ret;
}

template <typename T>
bool FileIO::readFile(std::string path, std::vector<T> &data) {
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (ifs.fail()) {
        return false;
    }

    size_t size = ifs.tellg();

    data.resize((size + sizeof(T) - 1) / sizeof(T));
    ifs.seekg(std::ios::beg);
    ifs.read((char *)data.data(), size);

    if (ifs.fail()) {
        return false;
    }

    return true;
}

template <typename T>
bool FileIO::writeFile(std::string path, const std::vector<T> &data) {
    std::ofstream ofs(path, std::ios::binary);
    if (ofs.fail()) {
        return false;
    }

    ofs.write((char *)data.data(), data.size() * sizeof(T));

    if (ofs.fail()) {
        return false;
    }

    return true;
}

bool FileIO::readString(std::string path, std::string &output) {
    std::ifstream ifs(path);
    if (ifs.fail()) {
        return false;
    }

    output.clear();
    for (char c; ifs.get(c);) output += c;

    return true;
}

bool FileIO::readLines(std::string path, std::vector<std::string> &lines) {
    std::ifstream ifs(path);

    if (ifs.fail()) {
        return false;
    }

    lines.clear();
    for (std::string line; getline(ifs, line);) {
        lines.push_back(line);
    }

    if (ifs.fail()) {
        return false;
    }

    return true;
}

}  // namespace cw

std::vector<char> ImageUtil::readFile(std::string path) {
    std::vector<char> ret;
    std::ifstream ifs(path, std::ios::ate | std::ios::binary);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    ret.resize(size);
    if (!ifs.read(ret.data(), size)) {
        printf("read %s failed\n", path.c_str());
        return std::vector<char>();
    }
    // printf("read %s %u byte\n", path.c_str(), size);
    return ret;
}

cv::Mat ImageUtil::readNV21Byte(std::string path, int w, int h) {
    auto data = readFile(path);
    cv::Mat ret(h + h / 2, w, CV_8UC1, data.data());
    cv::cvtColor(ret, ret, cv::COLOR_YUV2BGR_NV21);
    return ret;
}

cv::Mat ImageUtil::bgr2nv21(const cv::Mat &image) {
    if (image.empty()) {
        printf("image is empty\n");
        return cv::Mat();
    }

    const int width = ALIGN_DOWN(image.cols, 2);
    const int height = ALIGN_DOWN(image.rows, 2);
    cv::Rect rect(0, 0, width, height);
    cv::Mat image_roi = image(rect);
    if (image_roi.empty()) {
        printf("buf empty\n");
        return cv::Mat();
    }

    cv::Mat nv21;
    cvtColor(image_roi, nv21, cv::COLOR_BGR2YUV_I420);

    const size_t size = image_roi.rows * image_roi.cols;
    unsigned char *p = nv21.data + size;
    std::vector<uint8_t> uv(p, p + size / 2);
    for (size_t i = 0; i < uv.size() / 2; ++i) {
        p[2 * i] = uv[i + uv.size() / 2];
        p[2 * i + 1] = uv[i];
    }
    return nv21;
}

cv::Mat ImageUtil::normalizeImage(const cv::Mat &image, int w, int h) {
    int iw = image.cols;
    int ih = image.rows;
    float aspet_ratio = (float)iw / ih;
    if (aspet_ratio > (float)w / h) {
        iw = w;
        ih = w / aspet_ratio;
    } else {
        ih = h;
        iw = h * aspet_ratio;
    }
    printf("iw:%d,ih:%d\n", iw, ih);
    cv::Mat image_resized = cv::Mat::zeros(ih, iw, image.type());
    cv::resize(image, image_resized, image_resized.size());
    cv::Mat ret = cv::Mat::zeros(h, w, image.type());
    int pw = (w - iw) / 2;
    int ph = (h - ih) / 2;
    image_resized.copyTo(ret(cv::Rect(pw, ph, iw, ih)));
    return ret;
}

void ImageUtil::drawBox(cv::Mat &img, const FaceRect &rect, const int id,
                        const cv::Scalar &color) {
    cv::rectangle(img, cv::Point(rect.left, rect.top),
                  cv::Point(rect.right, rect.bottom), color, 2);

    const std::string text = std::to_string(id);
    const int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    const double fontScale = 1;
    const int thickness = 2;

    int baseline = 0;
    cv::Size textSize =
        cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
    baseline += thickness;

    // center the text
    cv::Point textOrg(rect.left, rect.top + textSize.height);

    // draw the box
    cv::rectangle(img, textOrg - cv::Point(0, textSize.height),
                  textOrg + cv::Point(textSize.width, baseline), color,
                  cv::FILLED);

    // then put the text itself
    cv::putText(img, text, textOrg, fontFace, fontScale, cv::Scalar::all(255),
                thickness, 8);
    // cv::Scalar::all(255), thickness, 8, true);
}

void ImageUtil::drawScore(cv::Mat &img, const cv::Point &leftTop,
                          const float score, const cv::Scalar &color) {
    const std::string text = "live:" + std::to_string(score);
    const int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    const double fontScale = 1;
    const int thickness = 2;

    int baseline = 0;
    cv::Size textSize =
        cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
    baseline += thickness;

    // center the text
    cv::Point textOrg = leftTop - cv::Point(0, textSize.height);

    cv::putText(img, text, textOrg, fontFace, fontScale, color, thickness, 8);
}
void ImageUtil::drawTrackResult(const FaceHandle *const faceHandles,
                                const int length, cv::Mat &image,
                                const int frameId) {
    cv::putText(image, std::to_string(frameId).c_str(), cv::Point(50, 50),
                cv::FONT_HERSHEY_PLAIN, 3.0, cv::Scalar(0, 255, 255), 2);
    for (int i = 0; i < length; ++i) {
        FaceRect rect;
        getFaceRect(faceHandles[i], &rect);
        int trackId;
        getTrackId(faceHandles[i], &trackId);
        cv::rectangle(image, cv::Point(rect.left, rect.top),
                      cv::Point(rect.right, rect.bottom),
                      cv::Scalar(0, 255, 255), 2);
        cv::putText(image, std::to_string(trackId).c_str(),
                    cv::Point(rect.left, rect.top), cv::FONT_HERSHEY_PLAIN, 2,
                    cv::Scalar(0, 255, 0), 2);
    }
}

FacePassProWrapper::FacePassProWrapper(const std::string &path,
                                       const std::string &image_type, int width,
                                       int height) {
    FaceRetCode state = RET_OK;
    if (image_type == "bgr") {
        img = cv::imread(path.c_str());
        image = {img.data, nullptr,  -1,       img.cols,
                 img.rows, img.cols, img.rows, Format_BGR888};
        state = detect(&image, 0.75, MIN_FACE, faceHandleArr.data(), &face_num);
        faceHandleArr.resize(face_num);
    } else if (image_type == "nv21" || image_type == "nv12") {
        std::vector<char> data = ImageUtil::readFile(path);
        img = cv::Mat(height * 3 / 2, width, CV_8UC1,
                      static_cast<void *>(data.data()))
                  .clone();
        PixelFormat format = Format_YUV420SP_NV12;
        if (image_type == "nv21") {
            format = Format_YUV420SP_NV21;
        }
        image = {img.data, nullptr, -1, width, height, width, height, format};
        state = detect(&image, 0.75, MIN_FACE, faceHandleArr.data(), &face_num);
        faceHandleArr.resize(face_num);
    } else if (image_type == "binary") {
        std::vector<char> data = ImageUtil::readFile(path);
        // printf("[FACEPASS_TEST] data.size = %d\n", data.size());
        img = cv::Mat(height, width, CV_8UC3, data.data()).clone();
        image = {img.data, nullptr,  -1,       img.cols,
                 img.rows, img.cols, img.rows, Format_BGR888};
        state = detect(&image, 0.75, MIN_FACE, faceHandleArr.data(), &face_num);
        faceHandleArr.resize(face_num);
    }
    if (RET_NO_FACE == state) {
        printf("[FACEPASS_TEST]  detect no faces in %s!\n", path.c_str());
    } else if (RET_OK != state) {
        printf("[FACEPASS_TEST] error occurred during detect!\n");
    }

    printf("[FACEPASS_TEST] detect faces number is %d\n", face_num);
}

FacePassProWrapper::~FacePassProWrapper() {
    for (int i = 0; i < face_num; ++i) {
        releaseFace(faceHandleArr[i]);
    }
}

std::vector<char> FacePassProWrapper::extract_max_face() {
    int index = -1;
    int max_area = 0;

    if (face_num <= 0) {
        printf("[FACEPASS_TEST] no faces\n");
    }

    for (int i = 0; i < face_num; i++) {
        FaceRect rect;
        getFaceRect(faceHandleArr[i], &rect);
        int area = (rect.right - rect.left) * (rect.bottom - rect.top);
        if (area > max_area) {
            max_area = area;
            index = i;
        }
    }
    std::vector<char> ret;
    if (index != -1) {
        char *feature;
        int feature_len;
        extract(&image, faceHandleArr[index], &feature, &feature_len);
        ret.resize(feature_len);
        memcpy(ret.data(), feature, feature_len * sizeof(char));
        releaseFeature(feature);
        feature = nullptr;
    }

    return ret;
}

int initSDK(const std::string &configPath) {
    FaceModels models;
    memset(&models, 0, sizeof(FaceModels));

    Config::setParameterFile(configPath);

    std::string anchor_path = Config::get<std::string>("anchor_path");
    std::string detect_model = Config::get<std::string>("detect_model");
    std::string postfilter_model = Config::get<std::string>("postfilter_model");
    std::string poseblur_model = Config::get<std::string>("poseblur_model");
    std::string refine_model = Config::get<std::string>("refine_model");

    std::string liveness_bgr_model =
        Config::get<std::string>("liveness_bgr_model");
    std::string liveness_ir_model =
        Config::get<std::string>("liveness_ir_model");
    std::string liveness_bgrir_model =
        Config::get<std::string>("liveness_bgrir_model");

    std::string age_gender_model = Config::get<std::string>("age_gender_model");
    std::string rc_model = Config::get<std::string>("rc_model");
    std::string occl_model = Config::get<std::string>("occl_model");
    std::string stn_model_path = Config::get<std::string>("stn_model");
    std::string feature_model = Config::get<std::string>("feature_model");
    std::string group_model_path = Config::get<std::string>("group_model_path");

    models.anchor_path = anchor_path.c_str();
    models.detect_model = detect_model.c_str();
    models.postfilter_model = postfilter_model.c_str();
    models.pose_blur_model = poseblur_model.c_str();
    models.refine_model = refine_model.c_str();

    models.liveness_bgr_model = liveness_bgr_model.c_str();
    models.liveness_ir_model = liveness_ir_model.c_str();
    models.liveness_bgrir_model = liveness_bgrir_model.c_str();

    models.age_gender_model = age_gender_model.c_str();
    models.rc_model = rc_model.c_str();
    models.occl_model = occl_model.c_str();
    models.stn_model = stn_model_path.c_str();
    models.feature_model = feature_model.c_str();
    models.group_model_path = group_model_path.c_str();

    auto detect_threshold = Config::get<float>("detect_threshold");
    auto postfilter_threshold = Config::get<float>("postfilter_threshold");

    printf("[FACEPASS_TEST] using anchor_path: %s\n", models.anchor_path);
    printf("[FACEPASS_TEST] using detect_model: %s\n", models.detect_model);
    printf("[FACEPASS_TEST] using postfilter_model: %s\n",
           models.postfilter_model);
    printf("[FACEPASS_TEST] using pose_blur_model: %s\n",
           models.pose_blur_model);
    printf("[FACEPASS_TEST] using refine_model: %s\n", models.refine_model);

    printf("[FACEPASS_TEST] using liveness_bgr_model: %s\n",
           models.liveness_bgr_model);
    printf("[FACEPASS_TEST] using liveness_ir_model: %s\n",
           models.liveness_ir_model);
    printf("[FACEPASS_TEST] using liveness_bgrir_model: %s\n",
           models.liveness_bgrir_model);

    printf("[FACEPASS_TEST] using age_gender_model: %s\n",
           models.age_gender_model);
    printf("[FACEPASS_TEST] using rc_model: %s\n", models.rc_model);
    printf("[FACEPASS_TEST] using occl_model: %s\n", models.occl_model);
    printf("[FACEPASS_TEST] using stn_model: %s\n", models.stn_model);
    printf("[FACEPASS_TEST] using feature_model: %s\n", models.feature_model);
    printf("[FACEPASS_TEST] using group_model_path: %s\n",
           models.group_model_path);

    printf(
        "[FACEPASS_TEST] using detect param: detect_threshold:%f, "
        "postfilter_threshold:%f\n",
        detect_threshold, postfilter_threshold);
    const char *sdk_info = getVersion();
    printf("[FACEPASS_TEST] sdk version is %s\n", sdk_info);

    auto ret = ::init(models);
    if (ret != RET_OK) {
        printf("[FACEPASS_TEST] error: init return %d\n", ret);
        return 1;
    }
    set_detect_config(detect_threshold, postfilter_threshold);

    //    set_match_config(0.991393, 16.6103,  1.07092, -45.5217);
    set_match_config(1.00758, -52.66727, 1.00936, 12.62161, 0.4);

    return 0;
};

std::vector<std::pair<std::string, std::vector<char>>> getFeatureWithName(
    const char *path, const std::string &image_type, const int width,
    const int height) {
    std::vector<std::pair<std::string, std::vector<char>>> vFeatures;
    auto file = cw::File(path);

    if (file.is_dir()) {
        std::vector<std::string> imagePaths;

        if (image_type == "bgr") {
            imagePaths = file.list_file_by_ext({".jpg", ".bmp", ".png"});
        } else if (image_type == "nv21") {
            imagePaths = file.list_file_by_ext({".yuv", ".nv21"});
        } else if (image_type == "nv12") {
            imagePaths = file.list_file_by_ext({".yuv", ".nv12"});
        } else if (image_type == "binary") {
            imagePaths = file.list_file_by_ext({".bmp"});  // define by usr
        }
        printf("[FACEPASS_TEST] %d images in %s\n", imagePaths.size(), path);

        for (auto &item : imagePaths) {
            printf("[FACEPASS_TEST] image path : %s\n", item.c_str());

            auto feature = FacePassProWrapper(item, image_type, width, height)
                               .extract_max_face();
            if (!feature.empty()) {
                vFeatures.push_back(
                    std::make_pair<std::string, std::vector<char>>(
                        cw::Path::get_basename(item), move(feature)));
            }
        }

    } else {
        auto feature = FacePassProWrapper(path, image_type, width, height)
                           .extract_max_face();
        if (!feature.empty()) {
            vFeatures.push_back(std::make_pair<std::string, std::vector<char>>(
                cw::Path::get_basename(path), move(feature)));
        }
    }

    return vFeatures;
}
