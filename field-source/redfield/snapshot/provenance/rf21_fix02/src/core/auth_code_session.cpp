// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/auth_code_session.hpp"
#include "redfield/config.hpp"
#include "auth_code_algorithm.hpp"
#include "auth_code_formatter.hpp"
#include <random>
namespace redfield {
namespace {
bool default_seed(void*,std::uint64_t& seed) noexcept {
    try {
        std::random_device device;
        std::uint64_t raw=0;
        for(unsigned i=0;i<8;++i)raw=(raw<<8) | (device()&0xffu);
        seed=mix64(raw);
        return true;
    } catch(...) {return false;}
}
ResponseResult result(HandlerStatus status,std::size_t capacity,std::size_t* written) noexcept {
    if(status==HandlerStatus::unsupported || status==HandlerStatus::wrong_command){*written=0;return ResponseResult::no_response;}
    if(status!=HandlerStatus::ok || *written>=capacity){*written=0;return ResponseResult::rejected;}
    return ResponseResult::response;
}
}
ResponseResult AuthCodeSession::respond(const xml::Request& request,const ConfigSnapshot& config,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;
    if(!output || !capacity)return ResponseResult::rejected;
    const auto checked=check_auth_code_request(request,written);
    if(checked!=HandlerStatus::ok)return result(checked,capacity,written);
    if(!safe_attr(request.id) || !safe_attr(preloader_l::xml::response_sender_policy(request)))return ResponseResult::rejected;
    try {
        std::lock_guard lock(mutex_);
        if(closed_)return ResponseResult::rejected;
        if(!config.online)return result(render_auth_offline(request,output,capacity,written),capacity,written);
        std::array<char,9> candidate=value_;
        if(!ready_){
            std::uint64_t seed=0;
            if(!(source_?source_:default_seed)(context_,seed))return ResponseResult::rejected;
            generate_auth_code(candidate.data(),seed);
        }
        const auto response=result(render_auth_code(request,{candidate.data(),8},output,capacity,written),capacity,written);
        if(response==ResponseResult::response){value_=candidate;ready_=true;}
        return response;
    } catch(...) {*written=0;return ResponseResult::rejected;}
}
bool AuthCodeSession::ready() const {std::lock_guard lock(mutex_);return ready_;}
void AuthCodeSession::close(){std::lock_guard lock(mutex_);value_.fill('\0');ready_=false;closed_=true;}
}
