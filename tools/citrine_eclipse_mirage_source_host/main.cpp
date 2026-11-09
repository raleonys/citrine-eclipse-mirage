// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/calendar.hpp"
#include "citrine_eclipse_mirage/hash/hash.hpp"
#include "citrine_eclipse_mirage/config/parser.hpp"
#include "citrine_eclipse_mirage/runtime/control.h"
#include "../runtime_probe.hpp"
#include <windows.h>
#include <iostream>

namespace {
#if defined(_DEBUG)
void debug_stage(const char* label) {
    std::cerr << "SOURCE_HOST_DEBUG_STAGE: " << label << '\n';
    std::cerr.flush();
}
struct DebugDestructorFence {
    const char* stage;
    ~DebugDestructorFence() { debug_stage(stage); }
};
#else
void debug_stage(const char*) {}
#endif
}

static int run_source_smoke() {
    struct Stop {~Stop(){
        debug_stage("shutdown-enter");
        const auto status=DllShutdown(20000);
        debug_stage(status==DLL_SHUTDOWN_OK?"shutdown-ok":"shutdown-failed");
        if(status!=DLL_SHUTDOWN_OK)std::terminate();
    }} stop;
#if defined(_DEBUG)
    DebugDestructorFence before_shutdown{"before-stop-destructor"};
#endif
    {
        asio::io_context io;asio::ip::tcp::socket socket(io);std::string buffered;
        debug_stage("greeting-wait-enter");
        (void)citrine_eclipse_mirage::tools::wait_for_application(socket,buffered);
        debug_stage("greeting-received");
        socket.non_blocking(false);
        std::string request="<LSX><Request id='actual-dll' recipient='EbisuSDK'><GetProfile index='0'/></Request></LSX>";request.push_back('\0');
        debug_stage("request-write-enter");
        asio::write(socket,asio::buffer(request));socket.non_blocking(true);
        debug_stage("response-wait-enter");
        if(citrine_eclipse_mirage::tools::frame(socket,buffered).find("GetProfileResponse")==std::string::npos)return 23;
        debug_stage("response-received");
    }
    debug_stage("socket-complete");
    debug_stage("module-handle-enter");
    if(!GetModuleHandleW(L"citrine_eclipse_mirage.source.dll")) return 10;
    debug_stage("module-handle-ok");
    citrine_eclipse_mirage::calendar::BrokenDown epoch{0,0,0,1,0,70,0,0,0};
    debug_stage("calendar-enter");
    if(citrine_eclipse_mirage::calendar::epoch_041ff0(epoch)!=0) return 11;
    debug_stage("calendar-ok");
    debug_stage("sha1-enter");
    citrine_eclipse_mirage::hash::Sha1 hash;
    debug_stage("sha1-constructed");
    debug_stage("sha1-update-enter");
    hash.update("abc");
    debug_stage("sha1-update-ok");
    debug_stage("sha1-finalize-enter");
    const auto sha1_words=hash.finalize_words_043270();
    debug_stage("sha1-finalize-ok");
    debug_stage("sha1-render-enter");
    const auto sha1_text=citrine_eclipse_mirage::hash::classic_hex_digest(sha1_words);
#if defined(_DEBUG)
    DebugDestructorFence before_sha1_locals{"before-sha1-local-destructors"};
#endif
    debug_stage("sha1-render-ok");
    if(sha1_text!="a9993e364706816aba3e25717850c26c9cd0d89d") return 12;
    debug_stage("sha1-ok");
    debug_stage("empty-parser-enter");
    citrine_eclipse_mirage::parser::State state;
#if defined(_DEBUG)
    DebugDestructorFence before_empty_parser_destructor{"before-empty-parser-destructor"};
#endif
    if(!citrine_eclipse_mirage::parser::parse("",state)) return 13;
    debug_stage("empty-parser-ok");
    debug_stage("config-parser-enter");
    citrine_eclipse_mirage::parser::State config;
#if defined(_DEBUG)
    DebugDestructorFence before_config_parser_destructor{"before-config-parser-destructor"};
#endif
    if(!citrine_eclipse_mirage::parser::parse(
        "\xef\xbb\xbf\"Config2\"{/* comment */\"User\"{\"Username\" \"first\"\"Username\" \"last\"}}",config))return 14;
    debug_stage("config-parser-ok");
    const auto* root=config.root.find("Config2");
    const auto* user=root?root->find("User"):nullptr;
    const auto* name=user?user->find("Username"):nullptr;
    if(!config.removed_utf8_bom || !name || name->value!="last")return 15;
    if(user->completed_assignments!=std::vector<std::string>{"Username","Username"})return 16;
    debug_stage("invalid-parser-enter");
    citrine_eclipse_mirage::parser::State invalid;
#if defined(_DEBUG)
    DebugDestructorFence before_invalid_parser_destructor{"before-invalid-parser-destructor"};
#endif
    try {citrine_eclipse_mirage::parser::parse("\"R\"{/}",invalid);return 17;}
    catch(const citrine_eclipse_mirage::parser::ParseError& error) {
        if(error.rule!=citrine_eclipse_mirage::parser::ErrorRule::object_members || error.position.byte!=5)return 18;
    }
    debug_stage("invalid-parser-ok");
    debug_stage("checks-complete");
    debug_stage("checks-return-enter");
    return 0;
}

int main() {
    debug_stage("main-entered");
    // Local C++ objects and the shutdown guard must finish before reporting PASS.
    const auto status=run_source_smoke();
    if(status!=0)return status;
    debug_stage("success-output-enter");
    std::cout<<"PASS: actual DLL auto-started; real socket LSX profile, calendar, SHA-1 and PEGTL Config2/VDF executed; application shutdown completed\n";
    debug_stage("success-output-returned");
    debug_stage("main-return-enter");
}
