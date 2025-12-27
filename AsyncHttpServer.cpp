/**
 * @file AsyncHttpServer.cpp
 * @brief 实现异步服务器逻辑
 */

#include "AsyncHttpServer.hpp"
#include <iostream>
#include <sstream>

// ================= HttpSession 实现 =================

std::shared_ptr<HttpSession> HttpSession::create(tcp::socket socket) {
    return std::shared_ptr<HttpSession>(new HttpSession(std::move(socket)));
}

HttpSession::HttpSession(tcp::socket socket)
    : socket_(std::move(socket)) {
}

void HttpSession::start() {
    do_read();
}

void HttpSession::do_read() {
    // 面试点：这里捕获 self (shared_ptr) 增加了引用计数
    // 只要异步操作未完成，HttpSession 对象就不会被析构
    auto self(shared_from_this());

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                handle_request(length);
            } else {
                // 发生错误或对端关闭连接，shared_ptr 计数减一，自动析构
            }
        });
}

void HttpSession::handle_request(std::size_t length) {
    // 简单的 HTTP 响应构建 (实际项目中应使用 HTTP 解析器)
    // 这里为了演示，仅仅解析请求并返回固定内容
    
    // 构造 HTTP 响应
    std::string response_body = "<html><body><h1>Hello from Async C++ Server!</h1></body></html>";
    std::string response_header = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(response_body.size()) + "\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";
    
    std::string response = response_header + response_body;

    // 将响应复制到缓冲区并发送
    // 注意：实际高并发场景可能需要分散/聚合 I/O (scatter/gather)
    std::copy(response.begin(), response.end(), data_);
    do_write(response.size());
}

void HttpSession::do_write(std::size_t length) {
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                // 如果是 keep-alive，可以继续读，否则关闭
                // 这里简单起见，写完后继续等待下一个请求（长连接）
                do_read(); 
            }
            // 发生错误自动退出，self 销毁
        });
}

// ================= HttpServer 实现 =================

HttpServer::HttpServer(const std::string& address, unsigned short port, int threads)
    : io_context_(threads),
      acceptor_(io_context_),
      thread_count_(threads) {

    // 解析地址并绑定
    auto endpoint = tcp::endpoint(boost::asio::ip::make_address(address), port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    do_accept();
}

void HttpServer::do_accept() {
    // 异步等待新连接
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                // 修改点：使用 create 工厂方法，而不是 make_shared
                HttpSession::create(std::move(socket))->start();
            }

            // 继续监听下一个连接
            do_accept();
        });
}
void HttpServer::run() {
    std::cout << "Server starting with " << thread_count_ << " threads..." << std::endl;

    // 启动线程池运行 io_context
    // 面试点：One Loop Per Thread 模型。
    // 所有线程竞争执行 io_context 队列中的事件（回调），实现负载均衡。
    for (int i = 0; i < thread_count_; ++i) {
        thread_pool_.emplace_back([this]() {
            io_context_.run();
        });
    }

    // 等待所有线程结束（通常服务端是无限循环，不会走到这里）
    for (auto& t : thread_pool_) {
        if (t.joinable()) t.join();
    }
}