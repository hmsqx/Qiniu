cmake_minimum_required(VERSION 3.1)
project(tencent_cloud_cpp_sample)

set(CMAKE_CXX_STANDARD 11)

# 查找 jsoncpp 包
find_package(jsoncpp REQUIRED)

# 收集源码并添加包含目录
file(GLOB SERVER_SOURCES
    src/*.cc
)
include_directories(include)
include_directories(.)

add_executable(tencent_cloud_cpp_sample main.cc ${SERVER_SOURCES})

# 线程库
find_package(Threads REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

# MySQL 客户端库
find_library(MYSQLCLIENT_LIB mysqlclient)
if(NOT MYSQLCLIENT_LIB)
    message(WARNING "未找到 mysqlclient 库，尝试直接链接 -lmysqlclient")
    set(MYSQLCLIENT_LIB mysqlclient)
endif()

# 链接依赖
target_link_libraries(tencent_cloud_cpp_sample 
    tencentcloud-sdk-cpp-ai3d 
    tencentcloud-sdk-cpp-core 
    jsoncpp_lib
    ${MYSQLCLIENT_LIB}
    OpenSSL::Crypto
    Threads::Threads
)