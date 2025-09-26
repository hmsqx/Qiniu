cmake_minimum_required(VERSION 3.1)
project(tencent_cloud_cpp_sample)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 jsoncpp 包
find_package(jsoncpp REQUIRED)

# 收集源码并添加包含目录
file(GLOB SERVER_SOURCES
    src/*.cc
)
include_directories(include)
include_directories(.)

# 确保包含新的数据库连接管理文件
set(SERVER_SOURCES ${SERVER_SOURCES} )

# 完整版本（包含所有性能优化功能）
add_executable(tencent_cloud_cpp_sample main.cc ${SERVER_SOURCES})

# 简化版本（基础功能）
# add_executable(tencent_cloud_cpp_sample_simple simple_main.cc src/handlers.cc src/auth.cc src/tx_ai3d.cc src/db_utils.cc src/model_downloader.cc src/db_connection.cc)

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

# CURL库
find_package(CURL REQUIRED)

# 链接依赖
target_link_libraries(tencent_cloud_cpp_sample 
    tencentcloud-sdk-cpp-ai3d 
    tencentcloud-sdk-cpp-core 
    jsoncpp_lib
    ${MYSQLCLIENT_LIB}
    OpenSSL::Crypto
    Threads::Threads
    CURL::libcurl
)

#target_link_libraries(tencent_cloud_cpp_sample_simple 
#    tencentcloud-sdk-cpp-ai3d 
 #   tencentcloud-sdk-cpp-core 
 #   jsoncpp_lib
#    ${MYSQLCLIENT_LIB}
#    OpenSSL::Crypto
#    Threads::Threads
#    CURL::libcurl
#)