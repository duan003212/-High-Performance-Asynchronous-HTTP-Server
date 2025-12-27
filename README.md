# High-Performance Async HTTP Server

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Boost.Asio](https://img.shields.io/badge/Boost-Asio-orange.svg)
![Build](https://img.shields.io/badge/Build-CMake-green.svg)
![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)

A high-concurrency, asynchronous HTTP server implemented in **Modern C++ (C++17)**. Powered by **Boost.Asio**, this project demonstrates the **Reactor pattern**, **Multi-threading**, and strict **Memory Safety** practices suitable for high-load network environments.

## 📖 Introduction

This project is a lightweight implementation of a multi-threaded HTTP server designed to handle thousands of concurrent connections efficiently. Unlike traditional synchronous (blocking) servers that require a thread per connection, this server uses **Non-blocking I/O** and an event-driven architecture to maximize CPU utilization with a fixed thread pool.

It serves as a demonstration of advanced C++ network programming concepts, including:
- **RAII** for resource management.
- **`std::shared_ptr` & `std::enable_shared_from_this`** for safe asynchronous lifecycles.
- **One Loop Per Thread** (Shared `io_context`) architecture.

## ✨ Key Features

*   **Asynchronous I/O**: Fully non-blocking operations for Accept, Read, and Write using `boost::asio`.
*   **Thread Pool**: Automatically detects hardware concurrency and distributes I/O events across worker threads.
*   **Memory Safety**: Solves the "dangling pointer" problem in async callbacks using extended object lifecycles (`shared_from_this`).
*   **HTTP Keep-Alive**: Supports persistent connections to reduce TCP handshake overhead.
*   **Scalability**: Designed to handle high concurrency with low memory footprint.

## 🛠️ Tech Stack

*   **Language**: C++17
*   **Library**: Boost.Asio (Standalone or via Boost), Boost.System
*   **Build System**: CMake (>= 3.10)
*   **Platform**: Linux / macOS / Windows (WSL recommended)

## 🚀 Getting Started

### Prerequisites

Ensure you have a C++ compiler and Boost libraries installed.

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install cmake g++ libboost-all-dev
```

**macOS:**
```bash
brew install cmake boost
```

### Build & Run

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/AsyncHttpServer.git
    cd AsyncHttpServer
    ```

2.  **Build using CMake:**
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```

3.  **Run the server:**
    ```bash
    ./AsyncServer
    ```
    *Output:*
    `Listening on 0.0.0.0:8080`
    `Server starting with 8 threads...` (Depends on your CPU)

### Testing

You can test the server using `curl` or a browser.

**Basic Request:**
```bash
curl -v http://localhost:8080
```

**Benchmark (using `wrk` or `ab`):**
```bash
# Example using Apache Bench (1000 requests, 100 concurrent)
ab -n 1000 -c 100 http://127.0.0.1:8080/
```

## 🧩 Architecture

### The Reactor Model with Thread Pool
The server initializes a single `boost::asio::io_context`. Multiple threads are spawned, and each thread calls `io_context.run()`. This creates a thread pool where:
1.  **OS Kernel** (via epoll/kqueue) monitors socket events.
2.  Any available thread in the pool picks up a ready event (callback).
3.  This ensures automatic load balancing without complex locking mechanisms for task queues.

### Lifecycle Management
A critical challenge in async C++ is ensuring the Session object survives until the asynchronous operation completes.
*   We use `std::shared_ptr<HttpSession>`.
*   Inside `do_read()` or `do_write()`, we capture `self = shared_from_this()` in the lambda.
*   This increments the reference count, preventing the object from being destroyed even if the main handle is lost, until the callback returns.

```cpp
// Example from source
void HttpSession::do_read() {
    auto self(shared_from_this()); // Keep alive!
    socket_.async_read_some(..., [this, self](...) {
        // Safe to use 'this' here
    });
}
```

## 🔮 Future Improvements

*   [ ] Implement a complete HTTP Request Parser (e.g., using `llhttp`).
*   [ ] Add a timer to handle timeouts (protect against Slowloris attacks).
*   [ ] Implement a dynamic router for handling different URL paths.
*   [ ] Add structured logging (e.g., spdlog).
