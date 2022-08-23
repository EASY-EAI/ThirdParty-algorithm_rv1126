set(OpenCV_module "opencv_core;opencv_imgproc;opencv_imgcodecs")
FOREACH(LIB ${OpenCV_module})
        SET(FOUND_LIB "FOUND_LIB-NOTFOUND")
        FIND_LIBRARY(FOUND_LIB ${LIB} PATHS "${CMAKE_SOURCE_DIR}/thirdparty/image/share" NO_CMAKE_SYSTEM_PATH)
        LIST(APPEND OpenCV_Lib ${FOUND_LIB})
        #MESSAGE("Lib: ${LIB}")
        #MESSAGE("Found Lib: ${FOUND_LIB}")
ENDFOREACH(LIB)
#message(STATUS "OpenCV_Lib: ${OpenCV_Lib}")


file(GLOB sample_common_src "*.cpp")

add_library("FacePass_${DEVICE}" ${sample_common_src})

link_directories(../../lib)
link_directories(../../thirdparty/rknn/share)
link_directories(../../thirdparty/drm/share)
#-Wl,--allow-shlib-undefined "facesdk_${DEVICE}")
target_link_libraries("FacePass_${DEVICE}"
        libstdc++.a
        ${OpenCV_Lib}
        -lm
        -ldrm
        ${RKLib}
        "facesdk_${DEVICE}"
        -Wl,--allow-shlib-undefined -lrknn_utils)

INSTALL(TARGETS "FacePass_${DEVICE}" DESTINATION ${PROJECT_SOURCE_DIR}/lib)
