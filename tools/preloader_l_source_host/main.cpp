#include "preloader_l/runtime/calendar.hpp"
#include "preloader_l/hash/hash.hpp"
#include "preloader_l/config/parser.hpp"
#include "preloader_l/runtime/control.h"
#include "../runtime_probe.hpp"
#include <windows.h>
#include <iostream>

int main() {
    struct Stop {~Stop(){if(DllShutdown(20000)!=DLL_SHUTDOWN_OK)std::terminate();}} stop;
    {
        asio::io_context io;asio::ip::tcp::socket socket(io);std::string buffered;
        (void)preloader_l::tools::wait_for_application(socket,buffered);
        socket.non_blocking(false);
        std::string request="<LSX><Request id='actual-dll' recipient='EbisuSDK'><GetProfile index='0'/></Request></LSX>";request.push_back('\0');
        asio::write(socket,asio::buffer(request));socket.non_blocking(true);
        if(preloader_l::tools::frame(socket,buffered).find("GetProfileResponse")==std::string::npos)return 23;
    }
    if(!GetModuleHandleW(L"preloader.source.dll")) return 10;
    preloader_l::calendar_recovered::BrokenDown epoch{0,0,0,1,0,70,0,0,0};
    if(preloader_l::calendar_recovered::epoch_041ff0(epoch)!=0) return 11;
    preloader_l::hash_recovered::Sha1 hash;
    hash.update("abc");
    if(preloader_l::hash_recovered::classic_hex_digest(hash.finalize_words_043270())
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
    std::cout<<"PASS: actual DLL auto-started; real socket LSX profile, calendar, SHA-1 and PEGTL Config2/VDF executed; shutdown drains application\n";
}
