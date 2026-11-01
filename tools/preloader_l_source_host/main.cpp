// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/calendar.hpp"
#include "preloader_l/hash/hash.hpp"
#include "preloader_l/config/parser.hpp"
#include "preloader_l/runtime/control.h"
#include "../runtime_probe.hpp"
#include <windows.h>
#include <iostream>

namespace {
#if defined(_DEBUG)
void debug_stage(const char* label) {
    std::cerr << "SOURCE_HOST_DEBUG_STAGE: " << label << '\n';
    std::cerr.flush();
}
#else
void debug_stage(const char*) {}
#endif
}

int main() {
    debug_stage("main-entered");
    struct Stop {~Stop(){
        debug_stage("shutdown-enter");
        const auto status=DllShutdown(20000);
        debug_stage(status==DLL_SHUTDOWN_OK?"shutdown-ok":"shutdown-failed");
        if(status!=DLL_SHUTDOWN_OK)std::terminate();
    }} stop;
    {
        asio::io_context io;asio::ip::tcp::socket socket(io);std::string buffered;
        debug_stage("greeting-wait-enter");
        (void)preloader_l::tools::wait_for_application(socket,buffered);
        debug_stage("greeting-received");
        socket.non_blocking(false);
        std::string request="<LSX><Request id='actual-dll' recipient='EbisuSDK'><GetProfile index='0'/></Request></LSX>";request.push_back('\0');
        debug_stage("request-write-enter");
        asio::write(socket,asio::buffer(request));socket.non_blocking(true);
        debug_stage("response-wait-enter");
        if(preloader_l::tools::frame(socket,buffered).find("GetProfileResponse")==std::string::npos)return 23;
        debug_stage("response-received");
    }
    debug_stage("socket-complete");
    if(!GetModuleHandleW(L"preloader.source.dll")) return 10;
    preloader_l::calendar::BrokenDown epoch{0,0,0,1,0,70,0,0,0};
    if(preloader_l::calendar::epoch_041ff0(epoch)!=0) return 11;
    preloader_l::hash::Sha1 hash;
    hash.update("abc");
    if(preloader_l::hash::classic_hex_digest(hash.finalize_words_043270())
       !="a9993e364706816aba3e25717850c26c9cd0d89d") return 12;
    preloader_l::parser::State state;
    if(!preloader_l::parser::parse("",state)) return 13;
    preloader_l::parser::State config;
    if(!preloader_l::parser::parse(
        "\xef\xbb\xbf\"Config2\"{/* comment */\"User\"{\"Username\" \"first\"\"Username\" \"last\"}}",config))return 14;
    const auto* root=config.root.find("Config2");
    const auto* user=root?root->find("User"):nullptr;
    const auto* name=user?user->find("Username"):nullptr;
    if(!config.removed_utf8_bom || !name || name->value!="last")return 15;
    if(user->completed_assignments!=std::vector<std::string>{"Username","Username"})return 16;
    preloader_l::parser::State invalid;
    try {preloader_l::parser::parse("\"R\"{/}",invalid);return 17;}
    catch(const preloader_l::parser::ParseError& error) {
        if(error.rule!=preloader_l::parser::ErrorRule::object_members || error.position.byte!=5)return 18;
    }
    debug_stage("checks-complete");
    std::cout<<"PASS: actual DLL auto-started; real socket LSX profile, calendar, SHA-1 and PEGTL Config2/VDF executed; shutdown drains application\n";
}
