#pragma once

#include "../nuby_engine.hpp"
#include <string>
#include <memory>
#include <thread>
#include <atomic>

namespace nuby::server {

class WebServer {
private:
    int port_{8080};
    std::string host_{"0.0.0.0"};
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> server_thread_;
    NubyBrowserEngine engine_;

    void handle_client(int client_sock);

public:
    explicit WebServer(int port = 8080, const std::string& host = "0.0.0.0")
        : port_(port), host_(host), engine_(1000, 800) {}

    ~WebServer() {
        stop();
    }

    void start();
    void stop();
    void run_synchronous();
};

} // namespace nuby::server
