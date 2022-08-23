#pragma once

typedef enum Log_Level {
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_INFO = 2,
} Log_Level;

typedef struct FaceModels {
    const char *detect_model;
    const char *postfilter_model;
    const char *refine_model;
    const char *pose_blur_model;
    const char *stn_model;
    const char *feature_model;
    const char *liveness_ir_model;
    const char *liveness_bgr_model;
    const char *liveness_bgrir_model;
    const char *anchor_path;
    const char *group_model_path;
    const char *age_gender_model;
    const char *rc_model;
    const char *occl_model;
} FaceModels;

/**
 * @brief image pixel format
 */
typedef enum PixelFormat_t {
    Format_GRAY8 = 0,      ///< Gray8
    Format_RGB888,         ///< RGB888
    Format_BGR888,         ///< BGR888
    Format_RGBA8888,       ///< RGBA8888
    Format_BGRA8888,       ///< BGRA8888
    Format_YUV420P_YU12,   ///< YUV420P YU12: YYYYYYYYUUVV
    Format_YUV420P_YV12,   ///< YUV420P YV12: YYYYYYYYVVUU
    Format_YUV420SP_NV12,  ///< YUV420SP NV12: YYYYYYYYUVUV
    Format_YUV420SP_NV21,  ///< YUV420SP NV21: YYYYYYYYVUVU
    Format_YUV422P_YU16,   ///< YUV422P YU16: YYYYYYYYUUUUVVVV
    Format_YUV422P_YV16,   ///< YUV422P YV16: YYYYYYYYVVVVUUUU
    Format_YUV422SP_NV16,  ///< YUV422SP NV16: YYYYYYYYUVUVUVUV
    Format_YUV422SP_NV61,  ///< YUV422SP NV61: YYYYYYYYVUVUVUVU
    Format_MAX,
} PixelFormat;

/**
 * @brief image
 */
typedef struct Image_t {
    void *vir_addr; /* virtual address */
    void *phy_addr; /* physical address */
    int fd;         /* shared fd */
    int width;      /* width */
    int height;     /* height */
    int wstride;    /* wstride */
    int hstride;    /* hstride */
    PixelFormat pixel_format;
} Image;

typedef void *FaceHandle;

typedef enum FaceRetCode {
    RET_OK = 0,
    RET_INTERNAL_ERROR,
    RET_NO_FACE,
    RET_NULL_POINTER,
    RET_UNEXPECTED_MODEL,
    RET_BROKEN_FILE,
    RET_OUT_OF_RANGE,
    RET_FILE_NOT_FOUND,
    RET_INVALID_ARGUMENT,
    RET_UNAUTHORIZED,
    RET_UNSUPPORTED,
    RET_UNINITIALIZED,
    RET_NOT_FOUND,
    RET_DUP_INIT,
} FaceRetCode;

typedef struct FaceRect {
    int left;
    int top;
    int right;
    int bottom;
} FaceRect;

typedef struct Point {
    float x;
    float y;
} Point;

typedef struct Landmark {
    float score;
    Point points[81];
} Landmark;

typedef struct FacePose {
    float roll;
    float pitch;
    float yaw;
    float blur;
} FacePoseBlur;

typedef struct FaceAttr {
    float age;            // age
    float gender[2];      // ["male", "female"]
    float hair[5];        // [bald, little_hair, short_hair, long_hair, unknown]
    float beard[4];       // [no_beard, moustache, whisker, unknown]
    float hat[4];         // no_hat, safety_helmet, other_hats, unknown]
    float respirator[4];  // [mouthandnose, mouth, no_respirator,unknown]
    float glasses[4];     //[no_glasses, sunglasses, other_glasses, unknown]
    float skin_color[4];  // [yellow, white, black, brown]
} FaceAttr;

typedef struct FaceOccl {
    float leftEye[4];          // [no_occlusion, occlusion, unknown, eye_closed]
    float leftEye_confidence;  // 0~1，confidence of the predict
    float rightEye[4];         // [no_occlusion, occlusion, unknown, eye_closed]
    float rightEye_confidence;      // 0~1，confidence of the predict
    float nose[3];                  // [no_occlusion, occlusion, unknown]
    float nose_confidence;          // 0~1，confidence of the predict
    float head[3];                  // [no_occlusion, occlusion, unknown]
    float head_confidence;          // 0~1，confidence of the predict
    float leftCheek[3];             // [no_occlusion, occlusion, unknown]
    float leftCheek_confidence;     // 0~1，confidence of the predict
    float rightCheek[3];            // [no_occlusion, occlusion, unknown]
    float rightCheek_confidence;    // 0~1，confidence of the predict
    float mouthAndChin[3];          // [no_occlusion, occlusion, unknown]
    float mouthAndChin_confidence;  // 0~1，confidence of the predict
} FaceOccl;

typedef void *FaceGroupHandle;
