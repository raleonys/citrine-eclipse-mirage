// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config_file.hpp"
#include "redfield/runtime.hpp"
#include "redfield/xml.hpp"
#include <memory>
namespace redfield {
enum class SessionParseResult { parsed, closed, invalid_frame };
class ServiceSession final {
public:
    explicit ServiceSession(std::filesystem::path module_filename);
    StartResult start() { return runtime_.start(); }
    StopResult stop(std::chrono::milliseconds budget) { return runtime_.stop(budget); }
    FileConfigResult load_status() const noexcept { return load_status_.load(std::memory_order_acquire); }
    std::shared_ptr<const ConfigSnapshot> configuration();
    SessionParseResult parse_request(std::string_view frame, xml::Request& request);
    RuntimeSnapshot snapshot() const { return runtime_.snapshot(); }
private:
    static bool start_configuration(void*) noexcept;
    static bool stop_configuration(void*) noexcept;
    std::filesystem::path module_filename_;
    std::unique_ptr<ConfigStore> configuration_;
    std::atomic<FileConfigResult> load_status_{FileConfigResult::missing};
    Resource configuration_resource_;
    Runtime runtime_;
};
}
