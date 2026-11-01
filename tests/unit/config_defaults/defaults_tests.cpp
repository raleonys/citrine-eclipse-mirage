// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/application.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

namespace allocation {
struct alignas(std::max_align_t) Header {bool tracked;};
bool armed=false;
std::size_t remaining=0,live=0;
void* acquire(std::size_t size) {
    if(armed&&--remaining==0)throw std::bad_alloc();
    if(size>std::numeric_limits<std::size_t>::max()-sizeof(Header))throw std::bad_alloc();
    auto* p=static_cast<Header*>(std::malloc(sizeof(Header)+(size?size:1)));
    if(!p)throw std::bad_alloc();
    p->tracked=armed;if(p->tracked)++live;return p+1;
}
void release(void* memory) noexcept {
    if(memory){auto* p=static_cast<Header*>(memory)-1;if(p->tracked)--live;std::free(p);}
}
}
void* operator new(std::size_t size){return allocation::acquire(size);}
void* operator new[](std::size_t size){return allocation::acquire(size);}
void operator delete(void* p) noexcept {allocation::release(p);}
void operator delete[](void* p) noexcept {allocation::release(p);}
void operator delete(void* p,std::size_t) noexcept {allocation::release(p);}
void operator delete[](void* p,std::size_t) noexcept {allocation::release(p);}

namespace {
// Test-only breadcrumb: preserve the last stage if MSVC Debug terminates
// before main can report an exception. No production code is modified.
void debug_stage(const char* stage, std::size_t fail_at=0) noexcept {
#if defined(_MSC_VER) && defined(_DEBUG)
    std::fprintf(stderr, "CONFIG_DEFAULTS_DEBUG stage=%s fail_at=%zu live=%zu\n",
                 stage, fail_at, allocation::live);
    std::fflush(stderr);
#else
    (void)stage;
    (void)fail_at;
#endif
}
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void constructor_contract() {
    const auto s=preloader_l::application::initial_state();
    require(s.executable_path.empty()&&s.user_directory.empty()&&s.cookie_path.empty()&&
        s.last_profile_path.empty()&&s.last_avatar_path.empty()&&s.avatar.empty()&&s.default_avatar.empty(),
        "constructor owns empty path slots before configuration");
    require(s.language_from_arguments==0&&s.registry_spoof==0&&s.language_registry_key.empty()&&s.documents.empty(),
        "constructor does not enable language/registry/document overrides");
    require(s.content_id=="0"&&s.license_directory.empty()&&s.denuvo_token.empty()&&s.exe_hash.empty()&&s.dll_hash.empty()&&s.denuvo_dll.empty(),
        "content identity and empty license/hash state match the constructor");
    require(s.username=="SURGE"&&s.persona_id=="1144668899"&&s.user_id=="1000200030000",
        "owner-selected product name preserves the separate constructor profile identities");
    require(s.server_port==0&&s.pretend_connected==1&&s.fake_auth==1&&s.block_reruns==0&&s.use_last_online_profile==0,
        "listener is not yet bound and initial feature flags are preserved");
    require(s.extra_dlls.empty()&&s.extra_dlls_from_main.empty()&&s.content.empty()&&s.entitlements.empty()&&
        s.achievements.empty()&&s.achievements_set.empty()&&s.steam_app_id.empty(),
        "fresh configuration contains no prior libraries, content or achievements");
    require(s.country=="US"&&s.currency=="USD"&&s.is_subscriber=="true"&&s.is_steam_subscriber=="false"&&
        s.subscriber_level=="2"&&s.is_under_age=="false","account defaults retain their string representation");
    const decltype(s.emulator_attributes) expected_emulator{{"ENVIRONMENT","production"},{"IS_IGO_AVAILABLE","false"},
        {"IS_TELEMETRY_ENABLED","false"},{"IS_MANUAL_OFFLINE","false"},{"LANGUAGE","en_US"},{"IS_IGO_ENABLED","false"}};
    require(s.emulator_attributes==expected_emulator,"all six constructor environment attributes are present exactly once");
    require(s.game_attributes.size()==14&&s.game_attributes.at("DISPLAY_NAME")=="Game"&&
        s.game_attributes.at("INSTALLED_VERSION")=="1.0.0.0"&&s.game_attributes.at("AVAILABLE_VERSION")=="1.0.0.0"&&
        s.game_attributes.at("UPTODATE")=="true"&&s.game_attributes.at("FULLGAME_IS_RELEASED")=="true"&&
        s.game_attributes.at("FULLGAME_PURCHASED")=="true"&&s.game_attributes.at("FREETRIAL")=="false"&&
        s.game_attributes.at("FULLGAME_RELEASE_DATE")=="2010-01-01T00:00:00"&&s.game_attributes.at("EXPIRATION")=="0000-00-00T00:00:00"&&
        s.game_attributes.at("ENTITLEMENT_SOURCE")=="ORIGIN"&&s.game_attributes.at("MAX_GROUP_SIZE")=="16"&&
        s.game_attributes.at("EXPIRATION_DURATION")=="36000000"&&s.game_attributes.at("INSTALLED_LANGUAGE")=="en_US",
        "game metadata preserves exact key names, values and textual sentinels");
    require(s.game_attributes.at("LANGUAGES")==
        "ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW",
        "counted language list preserves original order and spelling");
}
void independent_values() {
    auto first=preloader_l::application::initial_state();
    const auto second=preloader_l::application::initial_state();
    first.game_attributes.at("LANGUAGES")[0]='X';first.emulator_attributes.clear();first.username="changed";
    first.extra_dlls.push_back(u"local.dll");first.server_port=3226;
    require(second.username=="SURGE"&&second.game_attributes.at("LANGUAGES").starts_with("ar_SA")&&
        second.emulator_attributes.size()==6&&second.extra_dlls.empty()&&second.server_port==0,
        "independent results share no mutable configuration state");
}
void allocation_unwind() {
    unsigned failures=0;bool completed=false;
    for(std::size_t fail_at=1;fail_at<=256&&!completed;++fail_at) {
        debug_stage("allocation-attempt", fail_at);
        allocation::remaining=fail_at;allocation::armed=true;
        try {auto state=preloader_l::application::initial_state();allocation::armed=false;completed=state.game_attributes.size()==14;}
        catch(const std::bad_alloc&){allocation::armed=false;++failures;}
        debug_stage("allocation-result", fail_at);
        require(allocation::live==0,"every injected construction failure releases all partially constructed allocations");
    }
    require(completed&&failures>20,"all actual constructor allocation points were exhausted before a successful call");
    constructor_contract();
}
}
int main(int argc,char** argv) try {
#if defined(_MSC_VER) && defined(_DEBUG)
    // Test-only: avoid interactive MSVC CRT dialogs that stall hosted CI.
    // Preserve errors as stderr output and an actual nonzero test exit.
    for (int kind : {_CRT_ERROR, _CRT_ASSERT}) {
        _CrtSetReportMode(kind, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(kind, _CRTDBG_FILE_STDERR);
    }
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    std::set_terminate([]() noexcept {
        std::fputs("CONFIG_DEFAULTS_DEBUG stage=terminate", stderr);
        if (const auto eptr = std::current_exception()) {
            try { std::rethrow_exception(eptr); }
            catch (const std::bad_alloc&) { std::fputs(" exception=bad_alloc", stderr); }
            catch (const std::exception&) { std::fputs(" exception=std_exception", stderr); }
            catch (...) { std::fputs(" exception=unknown", stderr); }
        }
        std::fputc('\n', stderr);
        std::fflush(stderr);
        std::_Exit(86);
    });
#endif
#if defined(_MSC_VER) && defined(_DEBUG)
    if (argc == 2 && std::strncmp(argv[1], "--probe=", 8) == 0) {
        const char* mode = argv[1] + 8;
        const bool raw = std::strcmp(mode, "raw-new") == 0;
        const bool map = std::strcmp(mode, "map-default") == 0;
        const bool state = std::strcmp(mode, "state-default") == 0;
        const bool stateDirect = std::strcmp(mode, "state-direct-init") == 0;
        if (!raw && !map && !state && !stateDirect) return 2;
        std::fprintf(stderr, "PROBE_NOEXCEPT state=%d map=%d\n",
                     int(noexcept(preloader_l::configuration::State{})),
                     int(noexcept(std::map<std::string, std::string, std::less<>>{})));
        std::fflush(stderr);
        debug_stage(mode);
        allocation::remaining = 1;
        allocation::armed = true;
        try {
            if (raw) {
                void* p = ::operator new(sizeof(int));
                ::operator delete(p);
            } else if (map) {
                std::map<std::string, std::string, std::less<>> value;
                (void)value;
            } else if (state) {
                preloader_l::configuration::State value{};
                (void)value;
            } else {
                preloader_l::configuration::State value;
                (void)value;
            }
            allocation::armed = false;
            std::fprintf(stderr, "PROBE_UNEXPECTED_SUCCESS mode=%s\n", mode);
            return 3;
        } catch (const std::bad_alloc&) {
            allocation::armed = false;
            std::fprintf(stderr, "PROBE_CAUGHT mode=%s exception=bad_alloc live=%zu\n",
                         mode, allocation::live);
            std::fflush(stderr);
            return allocation::live == 0 ? 0 : 4;
        }
    }
#else
    (void)argc;
    (void)argv;
#endif
    debug_stage("constructor-contract-start");
    constructor_contract();
    debug_stage("independent-values-start");
    independent_values();
    debug_stage("allocation-unwind-start");
    allocation_unwind();
    debug_stage("complete");
    std::cout<<"PASS: constructor defaults, independent state and allocation-failure recovery\n";return 0;
}catch(const std::exception& e){allocation::armed=false;std::cerr<<e.what()<<'\n';return 1;}
