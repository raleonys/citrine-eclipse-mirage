// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/application/environment.hpp"
#include <memory>
#include <string>
namespace preloader_l {
class Connection;
class Application final {
public:
    explicit Application(Environment&);
    ~Application();
    Application(const Application&)=delete;
    Application& operator=(const Application&)=delete;
    void initialize();
    void initialize_online();
    void shutdown();
    configuration::State& configuration();
    std::unique_ptr<Connection> connect();
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    friend class Connection;
};
class Connection final {
public:
    explicit Connection(Application&);
    ~Connection();
    std::string greeting();
    std::string process(std::string_view);
private:
    struct Impl;std::unique_ptr<Impl> impl_;
};
}
