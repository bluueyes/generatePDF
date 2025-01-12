cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 添加include文件夹到包含路径中
include_directories(${CMAKE_SOURCE_DIR}/include)


set(ZLIB_LIBRARY ${CMAKE_SOURCE_DIR}/include/libz.a)  

set(SOURCES
    test.cpp

)

add_executable(MyProject ${SOURCES})

target_link_libraries(MyProject hpdf freetype png ${ZLIB_LIBRARY})