// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <asio.hpp>
#include <windows.h>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string>

// Tool-side observation of the existing RE EALsxPort + LSX protocol.
// No diagnostic export or added application protocol is needed.
namespace preloader_l::tools {
using ProbeClock=std::chrono::steady_clock;
inline std::string frame(asio::ip::tcp::socket& socket,std::string& buffered,ProbeClock::time_point until) {
    while(ProbeClock::now()<until) {
        const auto end=buffered.find('\0');
        if(end!=std::string::npos){auto out=buffered.substr(0,end);buffered.erase(0,end+1);return out;}
        char bytes[4096];asio::error_code error;
        const auto count=socket.read_some(asio::buffer(bytes),error);
        if(!error)buffered.append(bytes,count);
        else if(error==asio::error::would_block||error==asio::error::try_again)Sleep(1);
        else throw std::runtime_error("socket closed before complete LSX frame");
    }
    throw std::runtime_error("LSX frame deadline exceeded");
}
inline unsigned short wait_for_application(asio::ip::tcp::socket& socket,std::string& buffered) {
    const auto until=ProbeClock::now()+std::chrono::seconds(20);
    while(ProbeClock::now()<until) {
        char value[16]{};const auto size=GetEnvironmentVariableA("EALsxPort",value,sizeof value);
        char* end=nullptr;const auto port=size&&size<sizeof value?std::strtoul(value,&end,10):0;
        if(port&&port<=65535&&end&&!*end) {
            asio::error_code error;socket.close(error);buffered.clear();
            socket.connect({asio::ip::address_v4::loopback(),static_cast<unsigned short>(port)},error);
            if(!error) {
                socket.non_blocking(true);
                try {
                    if(frame(socket,buffered,until).find("<Challenge ")!=std::string::npos)
                        return static_cast<unsigned short>(port);
                }catch(const std::runtime_error&){}
            }
        }
        Sleep(10);
    }
    throw std::runtime_error("application did not publish a usable EALsxPort/LSX greeting before deadline");
}
inline std::string frame(asio::ip::tcp::socket& socket,std::string& buffered) {
    return frame(socket,buffered,ProbeClock::now()+std::chrono::seconds(10));
}
}
