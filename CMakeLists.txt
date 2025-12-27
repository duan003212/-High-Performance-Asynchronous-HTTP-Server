cmake_minimum_required(VERSION 3.10)
project(AsyncServer)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Boost 库
find_package(Boost REQUIRED COMPONENTS system)

# 添加可执行文件
add_executable(AsyncServer main.cpp AsyncHttpServer.cpp)

# 包含 Boost 头文件路径
target_include_directories(AsyncServer PRIVATE ${Boost_INCLUDE_DIRS})

# 链接 Boost 库
target_link_libraries(AsyncServer PRIVATE ${Boost_LIBRARIES})

# Linux 下通常需要链接 pthread 线程库
if(UNIX)
    target_link_libraries(AsyncServer PRIVATE pthread)
endif()