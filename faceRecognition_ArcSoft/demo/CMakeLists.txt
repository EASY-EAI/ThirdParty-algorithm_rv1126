cmake_minimum_required(VERSION 3.0)		#指定cmake版本 
project(AFCFaceDemo_Linux)				#指定项目的名称，一般和项目的文件夹名称对应

set(CMAKE_CXX_STANDARD 11)				#设置c++标准
set(LD_LIBRARY_PATH /linux_so)

#==========================================================================================

STRING(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} )
MESSAGE("current project: " ${CURRENT_FOLDER})

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")

execute_process(COMMAND which ${CMAKE_C_COMPILER} 
                OUTPUT_VARIABLE gcc_full_path)
get_filename_component(gcc_full_dir ${gcc_full_path} DIRECTORY)
set(HOST_DIR "${gcc_full_dir}/..")
set(CMAKE_TOOLCHAIN_FILE "${HOST_DIR}/share/buildroot/toolchainfile.cmake")

add_definitions(-Wall)

#指定头文件目录
# -I
set(root ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE("==================: " ${root})
set(api_inc
	${root}/include
   )
   
#指定静态和动态文件目录
link_directories(${root}/lib)

LINK_LIBRARIES( arcsoft_face arcsoft_face_engine )
LINK_LIBRARIES( rknn_api )
#该指令的主要作用就是将指定的源文件生成链接文件，然后添加到工程中去。
#arcsoft_face生成的库文件名		[STATIC | SHARED | MODULE] 库类型
add_executable(arcsoft_face_engine_test  ${root}/src/samplecode.cpp)
target_include_directories(arcsoft_face_engine_test PRIVATE ${api_inc} )	#-I
#-fPIC	在给定的作用域内设置一个命名的属性
set_property(TARGET arcsoft_face_engine_test PROPERTY POSITION_INDEPENDENT_CODE ON)

#链接库文件
target_link_libraries(arcsoft_face_engine_test
		-lopencv_calib3d 
		-lopencv_core 
		-lopencv_dnn 
		-lopencv_features2d 
		-lopencv_flann 
		-lopencv_highgui 
		-lopencv_imgcodecs 
		-lopencv_imgproc 
		-lopencv_ml 
		-lopencv_objdetect 
		-lopencv_photo 
		-lopencv_shape 
		-lopencv_stitching 
		-lopencv_superres 
		-lopencv_videoio 
		-lopencv_video 
		-lopencv_videostab
)	#-l
