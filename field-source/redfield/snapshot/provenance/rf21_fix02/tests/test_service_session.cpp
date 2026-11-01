// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/service_session.hpp"
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
using namespace redfield;
using namespace std::chrono_literals;
#define CHECK(x) do { if (!(x)) { std::cerr<<"line "<<__LINE__<<": " #x "\n"; return 1; } } while(false)
struct Workspace {
    std::filesystem::path old=std::filesystem::current_path(),root;
    Workspace(){
        for(unsigned i=0;i<100;++i){
            root=std::filesystem::temp_directory_path()/("redfield_rf01_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"_"+std::to_string(i));
            if(std::filesystem::create_directory(root))break;
        }
        std::filesystem::create_directory(root/"module with spaces");
        std::filesystem::create_directory(root/"other cwd");
    }
    ~Workspace(){std::error_code e;std::filesystem::current_path(old,e);std::filesystem::remove_all(root,e);}
    void write(const std::filesystem::path& p,std::string_view content){
        std::ofstream f(p,std::ios::binary);f.write(content.data(),static_cast<std::streamsize>(content.size()));f.close();
        if(!f)throw std::runtime_error("fixture file write failed");
    }
};
int main(){
    Workspace work;const auto directory=work.root/"module with spaces";
    const auto module=directory/"preloader_l.dll";
    work.write(work.root/"other cwd/redfield.cfg",R"("Config2" { "Emulator" { "Online" "true" } })");
    std::filesystem::current_path(work.root/"other cwd");
    {
        ServiceSession session(module);
        CHECK(!session.configuration());CHECK(session.start()==StartResult::started);
        CHECK(session.load_status()==FileConfigResult::missing && !session.configuration()->online);
        CHECK(session.stop(0ms)==StopResult::stopped);
    }
    work.write(directory/"redfield.cfg",R"("Config2" { "Emulator" { "Online" "true" } })");
    {
        ServiceSession session(module);CHECK(session.start()==StartResult::started);
        CHECK(session.load_status()==FileConfigResult::loaded);
        auto retained=session.configuration();CHECK(retained && retained->online);
        xml::Request request{};
        CHECK(session.parse_request("<LSX><Request id='x&amp;lt;'><GetSetting/></Request></LSX>",request)==SessionParseResult::parsed);
        CHECK(preloader_l::xml::span_equals(request.id,"x&lt;"));
        std::atomic<bool> failed{false},done{false};
        std::thread reader([&]{
            while(!done.load()){
                xml::Request out{};auto result=session.parse_request("<LSX><Request><A/></Request></LSX>",out);
                if(result!=SessionParseResult::parsed && result!=SessionParseResult::closed)failed=true;
                auto snapshot=session.configuration();if(snapshot && !snapshot->online)failed=true;
            }
        });
        const auto stopped=session.stop(1000ms);done=true;reader.join();
        CHECK(stopped==StopResult::stopped && !failed);
        CHECK(!session.configuration() && retained->online);
        CHECK(session.parse_request("<LSX><A/></LSX>",request)==SessionParseResult::closed && !request.command.data);
        CHECK(session.stop(0ms)==StopResult::stopped);
    }
    ConfigStore store;CHECK(load_module_config(module,store)==FileConfigResult::loaded);
    const auto old=store.snapshot();
    work.write(directory/"redfield.cfg","\"Config2\" {");
    CHECK(load_module_config(module,store)==FileConfigResult::invalid_config && store.snapshot()==old);
    {
        ServiceSession session(module);CHECK(session.start()==StartResult::start_failed);
        CHECK(session.load_status()==FileConfigResult::invalid_config);
        CHECK(session.snapshot().owned==0 && !session.configuration());
        CHECK(session.stop(0ms)==StopResult::stopped);
    }
    work.write(directory/"redfield.cfg",std::string(ConfigStore::max_bytes+1,'x'));
    CHECK(load_module_config(module,store)==FileConfigResult::too_large && store.snapshot()==old);
    work.write(directory/"redfield.cfg",std::string("\"Config2\" {}\0tail",18));
    CHECK(load_module_config(module,store)==FileConfigResult::invalid_config && store.snapshot()==old);
    CHECK(load_module_config("relative/preloader_l.dll",store)==FileConfigResult::invalid_path);
    CHECK(load_module_config(directory/"",store)==FileConfigResult::invalid_path);
    std::filesystem::remove(directory/"redfield.cfg");std::filesystem::create_directory(directory/"redfield.cfg");
    CHECK(load_module_config(module,store)==FileConfigResult::io_error && store.snapshot()==old);
    std::filesystem::remove(directory/"redfield.cfg");
    CHECK(load_module_config(module,store)==FileConfigResult::missing && store.snapshot()==old);
    std::cout<<"SERVICE_SESSION_PASS\n";
}
