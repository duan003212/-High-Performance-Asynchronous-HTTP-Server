/**
 * @file AsyncHttpServer.hpp
 * @brief 声明 HTTP 连接处理类和服务器核心类
 */

#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <thread>

// 使用简短别名
using tcp = boost::asio::ip::tcp;

/**
 * @class HttpSession
 * @brief 负责处理单个客户端连接的生命周期
 * 继承 enable_shared_from_this 以便在异步回调中保持对象存活
 */
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    // 工厂方法创建共享指针
    static std::shared_ptr<HttpSession> create(tcp::socket socket);

    void start(); // 启动会话

private:
    // 私有构造，强制使用 create
    explicit HttpSession(tcp::socket socket);

    void do_read();  // 异步读取
    void do_write(std::size_t length); // 异步写入
    void handle_request(std::size_t length); // 简单的协议处理

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length]; // 接收缓冲区
};

/**
 * @class HttpServer
 * @brief 服务器主类，负责接收连接并分配 I/O 任务
 */
class HttpServer {
public:
    HttpServer(const std::string& address, unsigned short port, int threads);
    void run();

private:
    void do_accept(); // 异步接收连接

    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::thread> thread_pool_; // 线程池
    int thread_count_;
};