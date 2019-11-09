cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -L${CMAKE_SOURCE_DIR}/libs/armeabi-v7a")
include_directories(include)

file(GLOB src_files *.cpp)

add_library( # Sets the name of the library.
        native-lib
        SHARED
        ${src_files})

target_link_libraries( # Specifies the target library.
        native-lib
        #忽略编译的顺序
        -Wl,--start-group
        avcodec avfilter avutil swresample avformat swscale
        -Wl,--end-group
        log
        z
        android)