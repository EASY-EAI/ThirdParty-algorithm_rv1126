//
// Created by megvii on 19-6-24.
//

#ifndef TOOL_H
#define TOOL_H

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>

#include "config.h"
#include "face_sdk.h"

#define MIN_FACE 10

namespace cw {

class Path {
   public:
    static std::string get_panrent_dir(std::string path);
    static std::string get_name(std::string path);
    static std::string get_basename(std::string path);
    static std::string get_basename_path(std::string path);
    static std::string get_extname(std::string path);
    static std::string join(std::string path, std::string name);
    static std::string normalize_path(const std::string &path);
};

class File {
   public:
    File(std::string path) { m_path = path; }

    std::string path() { return m_path; }

    bool is_dir() {
        struct stat path_stat;
        stat(m_path.c_str(), &path_stat);
        return S_ISDIR(path_stat.st_mode);
    }

    bool exists() { return access(m_path.c_str(), F_OK) != -1; }

    bool make_dir() { return mkdir(m_path.c_str(), 0777) == 0; }

    std::vector<std::string> list_file();
    std::vector<std::string> list_file_by_ext(std::vector<std::string> exts);
    std::vector<std::string> list_file_by_ext_recursive(
        std::vector<std::string> exts);

   private:
    std::string m_path;

    std::string to_uppercase(std::string str) {
        std::for_each(str.begin(), str.end(),
                      [](char &c) { c = ::toupper(c); });
        return str;
    }
};

class FileIO {
   public:
    template <typename T>
    static bool readFile(std::string path, std::vector<T> &data);
    template <typename T>
    static bool writeFile(std::string path, const std::vector<T> &data);
    static bool readString(std::string path, std::string &output);
    static bool readLines(std::string path, std::vector<std::string> &lines);
};

struct ProfilerInfo {
    ProfilerInfo() : total(0.0), count(0) {}
    double total;
    uint32_t count;
};

class ScopedProfiler {
   public:
    typedef std::chrono::high_resolution_clock Clock;
    // typedef chrono::milliseconds Unit;
    typedef std::chrono::nanoseconds Unit;

    ScopedProfiler(std::string tag, uint32_t count = 1)
        : m_count(count), m_tag(tag), m_profiler_info(nullptr) {
        m_start = Clock::now();
    }

    ScopedProfiler(std::string tag, ProfilerInfo *pi, uint32_t count = 0)
        : m_count(count), m_tag(tag), m_profiler_info(pi) {
        if (m_profiler_info) m_profiler_info->count++;
        m_start = Clock::now();
    }

    ~ScopedProfiler() {
        std::chrono::duration<double> elapsed = Clock::now() - m_start;
        Unit u = std::chrono::duration_cast<Unit>(elapsed);
        if (m_profiler_info)
            m_profiler_info->total += (double)u.count() / 1000000.0;
        if (m_count != 0)
            printf("=== [%s] count: %u avgtime: %lf ms\n", m_tag.c_str(),
                   m_count, (double)u.count() / m_count / 1000000.0);
    }

   private:
    uint32_t m_count;
    std::string m_tag;
    ProfilerInfo *m_profiler_info;
    std::chrono::time_point<std::chrono::system_clock> m_start;
};

class ProfilerFactory {
   public:
    static ProfilerFactory *get() { return &s_instance; }

    ~ProfilerFactory() {
        for (auto &iter : m_info_maps) {
            printf("=== [ProfilerFactory] [%s] count: %u avgtime: %lf ms\n",
                   iter.first.c_str(), iter.second.count,
                   iter.second.total / iter.second.count);
        }
    }

    ScopedProfiler make(std::string tag) {
        return ScopedProfiler(tag, &m_info_maps[tag], 1);
    }

   private:
    static ProfilerFactory s_instance;
    std::map<std::string, ProfilerInfo> m_info_maps;
};

}  // namespace cw

#define ALIGN_UP(x, a) ((((x) + ((a)-1)) / a) * a)
#define ALIGN_DOWN(x, a) (((x) / (a)) * (a))

class ImageUtil {
   public:
    static std::vector<char> readFile(std::string path);
    static cv::Mat readNV21Byte(std::string path, int w, int h);
    static cv::Mat bgr2nv21(const cv::Mat &image);
    static cv::Mat normalizeImage(const cv::Mat &image, int w, int h);
    static void drawBox(cv::Mat &img, const FaceRect &rect, const int id,
                        const cv::Scalar &color);
    static void drawScore(cv::Mat &img, const cv::Point &leftTop,
                          const float score, const cv::Scalar &color);
    static void drawTrackResult(const FaceHandle *const faceHandles,
                                const int length, cv::Mat &image,
                                const int frameId);
};

class FacePassProWrapper {
   public:
    FacePassProWrapper(const std::string &path, const std::string &image_type,
                       int width = 0, int height = 0);
    ~FacePassProWrapper();
    std::vector<char> extract_max_face();

   private:
    int face_num = 128;
    char *feature = nullptr;
    int feature_len = 0;
    cv::Mat img;
    Image image = {0};
    std::vector<FaceHandle> faceHandleArr =
        std::vector<FaceHandle>(128, nullptr);
};

int initSDK(const std::string &configPath);

std::vector<std::pair<std::string, std::vector<char>>> getFeatureWithName(
    const char *path, const std::string &image_type, const int width,
    const int height);

#endif  // TOOL_H
