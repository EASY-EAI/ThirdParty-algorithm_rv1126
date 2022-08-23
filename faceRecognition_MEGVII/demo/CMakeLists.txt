cmake_minimum_required(VERSION 3.10.2)
project(FacePass_RV1109)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_CXX_STANDARD 14)

set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib )

if(NOT DEVICE)
    message(STATUS "No device type specified, defaulting to RV1109")
    string(TOLOWER "RV1109" DEVICE)
else()
    string(TOLOWER DEVICE DEVICE)
endif()

include_directories(include)

include_directories(thirdparty/image/include)

link_directories(thirdparty/image/share)

link_directories(./lib)

link_directories(thirdparty/rknn/share)
link_directories(thirdparty/drm/share)

set( opencv_dep
        -lopencv_calib3d
        -lopencv_core
        #-lopencv_ml
        -lopencv_features2d
        -lopencv_imgproc
        -lopencv_imgcodecs
        -lopencv_flann
        #-lopencv_highgui

        #-lopencv_video
        #-lopencv_videoio
        -ldl -lpthread)


set(THIRD_PARTY_LIBS
        "-lfacesdk_${DEVICE}"
        libstdc++.a
        ${opencv_dep}
        -lm
        )
set(RKLib
    -lrknn_api
    -ldrm
    )
add_subdirectory(src)
add_subdirectory(time_test)
#add_subdirectory(search)
#add_subdirectory(irprocess)
#add_subdirectory(track)
#add_subdirectory(videoProcess)
#add_subdirectory(calibrate)
