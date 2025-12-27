/**
 * @file main.cpp
 * @brief 程序入口
 */

#include "AsyncHttpServer.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        std::string ip = "0.0.0.0";
        unsigned short port = 8080;
        
        // 线程数通常设置为 CPU 核心数
        int threads = std::thread::hardware_concurrency(); 
        if (threads == 0) threads = 1;

        std::cout << "Listening on " << ip << ":" << port << std::endl;

        HttpServer server(ip, port, threads);
        server.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}