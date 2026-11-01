// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
#include <optional>
#include <string>
namespace preloader_l {
struct PortBackend {
    void* context=nullptr;
    // nullopt is absent; an empty string is present. false is a read error.
    bool (*read)(void*,std::optional<std::string>&) noexcept=nullptr;
    bool (*write)(void*,const char*) noexcept=nullptr; // nullptr removes the value
};
// Serialized owner. External writers must coordinate: the OS offers no env CAS.
class PortPublication final {
public:
    explicit PortPublication(PortBackend backend={}):backend_(backend){}
    ~PortPublication();
    PortPublication(const PortPublication&)=delete;
    PortPublication& operator=(const PortPublication&)=delete;
    bool publish(std::uint16_t) noexcept;
    bool restore() noexcept;
    bool pending() const noexcept{return held_;}
private:
    PortBackend backend_;
    std::optional<std::string> previous_;
    std::string published_;
    bool held_=false;
};
}
