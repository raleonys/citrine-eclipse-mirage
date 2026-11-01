// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/session_retirement.hpp"
#include <asio/ip/tcp.hpp>
#include <filesystem>
#include <iostream>
using namespace preloader_l;
using namespace std::chrono_literals;
#define CHECK(x) do {if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Hooks {
    bool blocked=true;
    bool stop(std::uint32_t) noexcept {return !blocked;}
};
int main() {
    for(unsigned mode=0;mode<2;++mode) {
        Hooks hooks;SessionRetirement retirement{hooks};
        auto service=std::make_unique<LocalService>(std::filesystem::temp_directory_path()/
            "PRELOADER_L_RETIREMENT_TEST"/"preloader_l.dll");
        CHECK(service->start());
        asio::io_context io;asio::ip::tcp::socket client(io);
        client.connect({asio::ip::address_v4::loopback(),service->snapshot().port});
        const auto deadline=std::chrono::steady_clock::now()+3s;
        while(service->snapshot().active!=1 && std::chrono::steady_clock::now()<deadline)service->poll_for(1ms);
        CHECK(service->snapshot().active==1);
        if(mode)retirement.retain(service);
        else CHECK(!retirement.finish(service));
        CHECK(!service && retirement.pending());
        CHECK(!retirement.retry(0) && retirement.pending());
        hooks.blocked=false;
        CHECK(retirement.retry(1000) && !retirement.pending());
        CHECK(retirement.retry(0) && retirement.finish(service));
    }
    Hooks hooks;hooks.blocked=false;SessionRetirement retirement{hooks};
    auto idle=std::make_unique<LocalService>(std::filesystem::temp_directory_path()/"preloader_l.dll");
    CHECK(retirement.finish(idle) && !idle && !retirement.pending());
    std::cout<<"SESSION_RETIREMENT_WORKER_HANDOFF_BLOCKED_FINISH_RETRY_AND_IDLE_PASS\n";
}
