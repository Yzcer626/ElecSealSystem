# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(ElecSealSystem VERSION 1.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# 包含目录
include_directories(${CMAKE_SOURCE_DIR})

# 源文件
set(SOURCES
    seal.cpp
    gainchar_bytxt.cpp
    bmpbuild.cpp
    seal_generator.cpp
)

# 创建可执行文件
add_executable(seal ${SOURCES})

# Windows 平台链接必要的库
if(WIN32)
    target_link_libraries(seal kernel32 user32 gdi32 shell32)
endif()

# 设置编译选项
if(MSVC)
    target_compile_options(seal PRIVATE /W4 /utf-8)
else()
    target_compile_options(seal PRIVATE -Wall -Wextra)
endif()
