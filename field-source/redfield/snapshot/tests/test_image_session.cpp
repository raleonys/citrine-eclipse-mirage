// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/service_session.hpp"
#include "preloader_l/image_response.hpp"
#include <fstream>
#include <iostream>
using namespace preloader_l;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Fixture {
    std::filesystem::path cwd=std::filesystem::current_path(),root=std::filesystem::temp_directory_path()/("rf09_asset_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())),module=root/std::filesystem::path(u8"DLL Ω & space"),other=root/"other";
    Fixture(){std::filesystem::create_directories(module);std::filesystem::create_directory(other);std::ofstream(other/"surge.cfg")<<R"("Config2" {"User" {"Avatar" "wrong-CWD"}})";std::filesystem::current_path(other);}
    ~Fixture(){std::filesystem::current_path(cwd);std::error_code e;std::filesystem::remove_all(root,e);}
};
std::string utf8(const std::filesystem::path& p){auto s=p.u8string();return {reinterpret_cast<const char*>(s.data()),s.size()};}
int main(){
    Fixture f;ServiceSession s(f.module/"preloader_l.dll");CHECK(s.start()==StartResult::started && s.load_status()==FileConfigResult::missing);
    const auto held=s.configuration();CHECK(held && held->avatar_fallback_path==utf8(f.module/"surge.png") && !std::filesystem::exists(f.module/"surge.png"));
    CHECK(s.stop(1s)==StopResult::stopped && !s.configuration());
    // The session can be gone; the retained snapshot still owns every path byte.
    std::filesystem::current_path(f.root);xml::Request r;CHECK(xml::parse("<LSX><Request id='i'><QueryImage><Width>W</Width><Height>H</Height><ImageId>i</ImageId></QueryImage></Request></LSX>",r)==xml::Result::ok);
    char out[4096];std::size_t n=0;CHECK(dispatch_response_with_config(r,*held,out,sizeof(out),&n)==ResponseResult::response && std::string(out,n).find("DLL Ω &amp; space")!=std::string::npos);
    std::ofstream(f.module/"surge.cfg")<<R"("Config2" {"User" {"Avatar" "custom"}})";
    CHECK(s.start()==StartResult::invalid_state); // Sessions are deliberately single-use.
    ServiceSession loaded(f.module/"preloader_l.dll");CHECK(loaded.start()==StartResult::started && loaded.load_status()==FileConfigResult::loaded);CHECK(loaded.configuration()->avatar_fallback_path==held->avatar_fallback_path && loaded.configuration()!=held);CHECK(loaded.stop(1s)==StopResult::stopped);
    ServiceSession relative("preloader_l.dll");CHECK(relative.start()!=StartResult::started && relative.load_status()==FileConfigResult::invalid_path);
    auto nul=(f.module/"preloader_l.dll").native();nul.push_back(0);nul+=std::filesystem::path("tail").native();ServiceSession embedded{std::filesystem::path(nul)};CHECK(embedded.start()!=StartResult::started && embedded.load_status()==FileConfigResult::invalid_path);
    ServiceSession nonxml(f.root/std::filesystem::path(std::string("bad")+'\1')/"preloader_l.dll");CHECK(nonxml.start()!=StartResult::started && nonxml.load_status()==FileConfigResult::invalid_path);
#if !defined(_WIN32)
    ServiceSession invalid_utf8(f.root/std::filesystem::path(std::string("bad")+char(0xff))/"preloader_l.dll");CHECK(invalid_utf8.start()!=StartResult::started && invalid_utf8.load_status()==FileConfigResult::invalid_path);
#endif
    std::cout<<"IMAGE_DLL_RELATIVE_UTF8_CWD_MISSING_ASSET_RETAINED_SNAPSHOT_PASS\n";
}
