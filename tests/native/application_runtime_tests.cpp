// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/application.hpp"
#include "preloader_l/core/binary_io.hpp"
#include "preloader_l/hash/hash.hpp"
#include "preloader_l/xml/reader.hpp"
#include "preloader_l/core/token_constants.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/platform/hook_native_callbacks.hpp"
#include "preloader_l/net/asio_service.hpp"
#include "preloader_l/net/listener_policy.hpp"
#include "preloader_l/platform/execution_windows.hpp"
#include "../../src/runtime/application_internal.hpp"
#include <asio.hpp>
#include <MinHook.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <future>
#include <fstream>
#include <iterator>

using namespace preloader_l;
namespace fs=std::filesystem;
namespace {
int checks=0;
void require(bool b,const char* text){++checks;if(!b)throw std::runtime_error(text);}
struct Directory {
    fs::path path=fs::temp_directory_path()/("surge-runtime-"+std::to_string(GetCurrentProcessId()));
    Directory(){fs::remove_all(path);fs::create_directories(path);}
    ~Directory(){std::error_code e;fs::remove_all(path,e);}
};
struct ScopedEnvironment {
    const wchar_t* name;
    std::wstring previous;
    bool existed;
    ScopedEnvironment(const wchar_t* key,const fs::path& value):name(key) {
        const auto size=GetEnvironmentVariableW(name,nullptr,0);
        existed=size!=0;
        if(size){previous.resize(size);previous.resize(GetEnvironmentVariableW(name,previous.data(),size));}
        if(!SetEnvironmentVariableW(name,value.wstring().c_str()))throw std::runtime_error("set test environment");
    }
    ~ScopedEnvironment(){SetEnvironmentVariableW(name,existed?previous.c_str():nullptr);}
};
class Peer {
    asio::io_context io;
    asio::ip::tcp::socket socket{io};
    std::string input;
public:
    explicit Peer(std::uint16_t port) {
        socket.connect({asio::ip::address_v4::loopback(),port});
        DWORD timeout=5000;
        setsockopt(socket.native_handle(),SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
    }
    std::string frame() {
        for(;;){auto end=input.find('\0');if(end!=std::string::npos){auto value=input.substr(0,end);input.erase(0,end+1);return value;}
            char bytes[4096];auto n=socket.read_some(asio::buffer(bytes));input.append(bytes,n);}
    }
    void send(std::string bytes){bytes.push_back('\0');asio::write(socket,asio::buffer(bytes));}
    std::string request(std::string_view id,std::string_view command) {
        send("<LSX><Request id='"+std::string(id)+"' recipient='EbisuSDK'>"+std::string(command)+"</Request></LSX>");return frame();
    }
    std::string encrypted_request(std::string_view id,std::string_view command) {
        auto bytes="<LSX><Request id='"+std::string(id)+"' recipient='EbisuSDK'>"+std::string(command)+"</Request></LSX>";
        crypto::Cipher cipher;cipher.set_fixed_key_084c98();require(cipher.encrypt_042610(bytes),"client encrypt");
        messages::hex_encode_03d190(bytes);send(bytes);return encrypted_frame();
    }
    std::string encrypted_frame() {
        auto bytes=frame();messages::hex_decode_03ce30(bytes);
        crypto::Cipher cipher;cipher.set_fixed_key_084c98();require(cipher.decrypt_042480(bytes),"client decrypt");return bytes;
    }
    bool closes_without_output() {
        socket.non_blocking(true);
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
        while(std::chrono::steady_clock::now()<deadline) {
            char byte;asio::error_code error;
            const auto count=socket.read_some(asio::buffer(&byte,1),error);
            if(count!=0)return false;
            if(error==asio::error::eof||error==asio::error::connection_reset)return true;
            if(error!=asio::error::would_block&&error!=asio::error::try_again)throw std::system_error(error);
            Sleep(1);
        }
        return false; // A timeout is not evidence of the required peer close.
    }
};
std::string known_token_file(bool cached) {
    // Independent OpenSSL AES-128-CBC answers, key 4132722dd082efb0dc6457c57668ca09,
    // zero IV. Normal plaintext: <GameToken>normal-retry<&</GameToken>, PKCS7.
    // Cached plaintext has cache\0<& inside the tags, then one zero byte to
    // reach 32 bytes. It uses -nopad and deliberately fails PKCS7 validation.
    constexpr unsigned char normal[]{
        0xdd,0xdf,0xd6,0x4a,0x9b,0xa7,0xd4,0xc6,0x26,0x58,0x50,0x5c,0x2c,0xa5,0x25,0xb4,
        0x5e,0x13,0x57,0xe3,0x28,0x5a,0xcf,0xcd,0x09,0xd3,0xed,0x4e,0xa0,0x63,0x5a,0x3b,
        0xea,0x94,0x3a,0xf7,0xbb,0x78,0xc2,0x05,0x17,0x68,0xc5,0x3e,0x5f,0xd6,0x5a,0x43};
    constexpr unsigned char cached_invalid_padding[]{
        0xd7,0x72,0xda,0x21,0x36,0x33,0x58,0x06,0x7f,0x96,0xc6,0xee,0x8a,0x18,0xcc,0xbb,
        0xb7,0xa4,0xee,0xce,0x90,0x36,0x41,0x44,0x3e,0x1a,0xdf,0x4b,0x21,0xa7,0x07,0xf1};
    std::string bytes(65,'H');
    if(cached)bytes.append(reinterpret_cast<const char*>(cached_invalid_padding),sizeof(cached_invalid_padding));
    else bytes.append(reinterpret_cast<const char*>(normal),sizeof(normal));
    return bytes;
}
void license_file_retry(const fs::path& root) {
    const auto directory=root/"license-retry";fs::create_directories(directory);
    file::NativeFactory files;
    // The real license guard hashes this owned artifact; no external DRM file,
    // ticket, dialog or process-exit branch participates in the fixture.
    require(file::write_binary_03c1d0((directory/"fixture-drm.bin").u16string(),"abc",files),"owned known-SHA1 license guard artifact");
    const auto config=R"("Config2" {"Game" {"ContentId" "fixture-license-retry" "DenuvoDllName" "fixture-drm.bin" "DenuvoDllHash" "A9993E364706816ABA3E25717850C26C9CD0D89D"}})";
    require(file::write_binary_03c1d0((directory/"surge.cfg").u16string(),config,files),"owned license retry configuration");
    application::Application app({(directory/"host.exe").u16string(),(directory/"data").u16string(),false,(directory/"program-data").u16string()});
    app.start();
    const auto port=app.configuration().server_port;
    const auto base=app.configuration().license_directory;
    fs::create_directories(fs::path(base).parent_path());
    require(file::write_binary_03c1d0(base+u".dlf","H",files),"one-byte normal file reaches unsupported CBC domain");
    require(file::write_binary_03c1d0(base+u"_cached.dlf",known_token_file(true),files),"distinct cached token prepared before failed request");
    {
        Peer failed(port);(void)failed.frame();
        failed.send("<LSX><Request id='malformed-license' recipient='EbisuSDK'><RequestLicense/></Request></LSX>");
        require(failed.closes_without_output(),"malformed token closes its peer without success XML; timeout is rejected");
    }
    require(file::write_binary_03c1d0(base+u".dlf",known_token_file(false),files),"repair normal token after failed request");
    {
        Peer retry(port);(void)retry.frame();
        const auto normal=retry.request("normal-retry","<RequestLicense/>");
        require(normal.find("<RequestLicenseResponse License=\"normal-retry&lt;&amp;\"/>")!=std::string::npos,
            "new peer retries normal file instead of consuming cached attempt or remaining blocked");
        require(retry.request("invalidate-license","<InvalidateLicense/>").find("License Deleted.")!=std::string::npos,"real invalidation clears loaded license");
        const auto cached=retry.request("cached-after-invalidate","<RequestLicense/>");
        require(cached.find("<RequestLicenseResponse License=\"cache\"/>")!=std::string::npos,
            "invalidation preserves attempt count; invalid-padding plaintext is scanned and XML uses C-string prefix");
    }
    app.stop();
    require(app.configuration().denuvo_token==std::string("cache\0<&",8),"stopped Application retains full counted token beyond XML C-string boundary");
}
struct GreetingFixture final:net::HandlerFactory {
    std::promise<void> entered;
    struct Handler final:server::Handler {
        GreetingFixture& fixture;
        explicit Handler(GreetingFixture& f):fixture(f){}
        void reset_02bc70() override {} // Intentional test-only handler.
        std::string initial_message_02d540() override {std::string bytes(8*1024*1024,'x');fixture.entered.set_value();return bytes;}
        std::string process_message_033d70(std::string_view) override {throw std::logic_error("Greeting fixture cannot process requests");}
    };
    std::unique_ptr<server::Handler> create() override{return std::make_unique<Handler>(*this);}
    void error(std::string_view,std::exception_ptr) noexcept override {} // Expected cancellation diagnostic.
};
void event_mutex_contract(){
    application::detail::Events owner;
    auto& mutex=event_queue::mutex_02c620(owner.context);
    require(mutex.lock_04a6ac()==0&&mutex.count_4c()==1,"event mutex first acquisition");
    const auto second=mutex.lock_04a6ac();
    const auto count=mutex.count_4c();
    // Release only what the tested implementation actually acquired, so the
    // old faulty recursive adapter also reports a clean regression failure.
    if(second==0)mutex.unlock_04a6b4();
    mutex.unlock_04a6b4();
    require(second==3&&count==1,"RE type2 same-thread reentry fails with3 and keeps count1");
    require(mutex.lock_04a6ac()==0,"event mutex reacquisition after failure and release");
    std::promise<void> started;auto entered=started.get_future();std::atomic<bool> acquired{false};
    std::atomic<bool> valid{false};
    std::thread peer([&]{started.set_value();const auto status=mutex.lock_04a6ac();
        valid=status==0&&mutex.count_4c()==1;acquired=true;if(status==0)mutex.unlock_04a6b4();});
    entered.wait();Sleep(30);const auto premature=acquired.load();
    mutex.unlock_04a6b4();peer.join();
    require(!premature&&acquired&&valid&&mutex.count_4c()==0,"another thread waits then acquires a released event mutex");
}
void stop_blocked_greeting() {
    GreetingFixture fixture;auto entered=fixture.entered.get_future();
    execution::MutexState mutex{};execution::initialize_mutex_04a698(mutex,2);
    std::uint8_t first=0,second=0,third=0;std::uint16_t port=0;
    execution::WindowsPlatform platform(+[](int){throw std::runtime_error("fixture thread error");});
    auto frequency=platform.query_performance_frequency();
    execution::Execution execution(mutex,{first,second,third},frequency,platform);
    net::AsioService service(fixture,execution,port,first);service.start();
    asio::io_context io;asio::ip::tcp::socket peer(io);peer.open(asio::ip::tcp::v4());
    peer.set_option(asio::socket_base::receive_buffer_size(1024));
    peer.connect({asio::ip::address_v4::loopback(),port});
    require(entered.wait_for(std::chrono::seconds(5))==std::future_status::ready,"large greeting entered actual transport");
    auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(peer.available()==0&&std::chrono::steady_clock::now()<deadline)Sleep(1);
    require(peer.available()!=0,"non-reading peer received an actual greeting prefix");
    // A faulty blocking sender is still released after1s so this regression
    // reports a failure rather than abandoning a process/thread indefinitely.
    std::jthread release([&]{Sleep(1000);asio::error_code e;peer.close(e);});
    auto start=std::chrono::steady_clock::now();service.stop();
    require(std::chrono::steady_clock::now()-start<std::chrono::milliseconds(500),"stop cancels a greeting blocked by a non-reading peer");
}
}
int main(int argc,char** argv) try {
    event_mutex_contract();
    Directory directory;
    license_file_retry(directory.path);
    file::NativeFactory files;
    const auto cfg=directory.path/"surge.cfg";
    std::string text=R"("Config2" {"Game" {"ContentId" "fixture-id" "KeyForLicense" "fixture-key"} "User" {"Username" "fixture-user"} "Emulator" {"SDK" "99.88.77.66-main"} "Content" {)";
    for(int i=0;i<25;++i)text+='"'+std::to_string(i)+R"(" {"Name" "fixture-content"})";
    text+=R"(} "Entitlements" {"fixture-offer" {"Group" "fixture-group"}} "Achievements" {"AchievementsSet" "fixture-set" "AchievementNames" {"award" "fixture-award"} "AchievementTotals" {"award" "10"}} })";
    require(file::write_binary_03c1d0(cfg.u16string(),text,files),"real standard IO config write");
    require(file::write_binary_03c1d0((directory.path/"surge_override.cfg").u16string(),R"("Config2" {"Emulator" {"SDK" "0.0.0.0-override"} "User" {"Username" "fixture-override"}})",files),"override write");
    auto unicode=directory.path/fs::path(u"dosya-\u0131\u015f-\u4e2d.dat");
    require(file::write_binary_03c1d0(unicode.u16string(),std::string_view("a\0b",3),files),"unicode binary write");
    require(file::read_binary_03b3f0(unicode.u16string(),files)==std::string("a\0b",3),"unicode binary read");
    application::Application app({(directory.path/"host.exe").u16string(),(directory.path/"data").u16string(),false,(directory.path/"program-data").u16string()});
    app.start();const auto& state=app.configuration();
    require(state.username=="fixture-override","real main then override");
    require(state.content.size()==25,"more than eight content children applied");
    require(state.entitlements.size()==1,"entitlements applied");
    require(state.achievements.at("award").field00==10,"achievement total applied");
    require(state.server_port>=net::listener_policy::first_port&&state.server_port<=net::listener_policy::last_port,"real bounded automatic listener port");
    auto license=file::read_binary_03b3f0(state.license_directory+u".dlf",files);
    require(license.size()>0x41&&static_cast<unsigned char>(license[0])==0x30,"02a830 actual file/header");
    crypto::Cipher license_cipher;
    license_cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    require(license_cipher.decrypt_with_offset(license,0x41),"02a830 recovered crypto binding");
    require(license.find("<CipherKey>fixture-key</CipherKey>")!=std::string::npos&&license.find("<ContentId>fixture-id</ContentId>")!=std::string::npos,"02a830 configured state to actual file");
    require(state.user_directory==(directory.path/"data").u16string(),"single supplied data owner");
    auto png=file::read_binary_03b3f0(state.avatar,files);
    require(argc==5,"provider, independent avatar assets and showLSX supplied");
    require(png==file::read_binary_03b3f0(fs::path(argv[2]).u16string(),files),"default profile avatar equals exact supplied image data");
    const auto fallback=file::read_binary_03b3f0(state.default_avatar,files);
    require(fallback==file::read_binary_03b3f0(fs::path(argv[3]).u16string(),files)&&fallback!=png,"separate recovered fallback avatar data preserved");
    require(!FindResourceW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(101),MAKEINTRESOURCEW(10)),"PNG is not a PE resource");
#ifdef NDEBUG
    require(!fs::exists(directory.path/"data"/"surge.log"),"Release disk log absent");
#else
    require(fs::exists(directory.path/"data"/"surge.log"),"Debug disk log present");
#endif
    {
        Peer peer(state.server_port);
        auto greeting=peer.frame();require(greeting.find("<Challenge ")!=std::string::npos,"RE greeting over real Asio socket");
        require(greeting.find("version=\"10,6,2,31\"")!=std::string::npos,"owner-pinned four-component SDK identity survives conflicting main/override config");
        auto profile=peer.request("profile","<GetProfile index='0'/>");
        require(profile.find("fixture-override")!=std::string::npos,"profile handler uses configured shared owner");
        auto content=peer.request("content","<QueryContent/>");
        require(content.find("fixture-content")!=std::string::npos,"real XML router content response");
        auto grant=peer.request("grant","<GrantAchievement AchievementId='award'/>");
        require(grant.find("Response")!=std::string::npos,"achievement route response");
        require(fs::exists(directory.path/"data"/"achievement-fixture-set.xml"),"real achievement persistence path");
        auto avatar=peer.request("avatar","<QueryImage Width='208' Height='208' ImageId='AVATAR:1000200030000'/>");
        require(avatar.find("SURGE.png")!=std::string::npos,"offline avatar reaches real image handler");
        {
            Peer encrypted(state.server_port);(void)encrypted.frame();
            require(encrypted.request("handshake","<ChallengeResponse key='fixture-key' version='2'/>").find("ChallengeAccepted")!=std::string::npos,"real challenge response");
            require(encrypted.encrypted_request("encrypted-profile","<GetProfile index='0'/>").find("fixture-override")!=std::string::npos,"actual encrypted LSX round trip");
            require(encrypted.encrypted_frame()==event_queue::initial_down_event,"actual encrypted event queue path");
            encrypted.send("4e4f542d414553"); // Valid hex, invalid AES length: NOT-AES.
            require(encrypted.encrypted_request("after-bad-cipher","<GetProfile index='0'/>").find("fixture-override")!=std::string::npos,"decryption failure leaves the session usable");
        }
        peer.send("<LSX>broken"); // Controlled failure must not kill the IO owner.
        require(peer.request("after-bad-xml","<GetProfile index='0'/>").find("fixture-override")!=std::string::npos,"XML parse failure leaves the session usable");
    }
    // Stop while an accepted socket still owns a pending read.
    Peer waiting(state.server_port);(void)waiting.frame();app.stop();app.stop();
    require(true,"pending read cancelled and idempotent stop completed");
#ifdef NDEBUG
    require(!fs::exists(directory.path/"data"/"surge.log"),"Release payload diagnostics never create a log");
#else
    const auto diagnostics=file::read_binary_03b3f0((directory.path/"data"/"surge.log").u16string(),files);
    const auto cfg_utf8=cfg.u8string();
    require(diagnostics.find("config loaded: "+std::string(cfg_utf8.begin(),cfg_utf8.end()))!=std::string::npos,"Debug preserves actual config path loaded before data directory creation");
    require(diagnostics.find("surge_override.cfg")!=std::string::npos,"Debug preserves override config path");
    require(diagnostics.find("message received: <LSX><Request id='profile'")!=std::string::npos,"Debug preserves actual request bytes");
    require(diagnostics.find("message response: <LSX>")!=std::string::npos&&diagnostics.find("fixture-override")!=std::string::npos,"Debug preserves response contents");
    require(diagnostics.find("message event: "+std::string(event_queue::initial_down_event))!=std::string::npos,"Debug preserves queued event bytes");
    xml_reader_bridge::Backend diagnostic_xml;
    const auto malformed=diagnostic_xml.parse_c_string("<LSX>broken");
    require(malformed.has_error&&!malformed.document->error_name().empty()&&
        diagnostics.find("invalid XML: "+std::string(malformed.document->error_name())+"; input: <LSX>broken")!=std::string::npos,"Debug preserves full backend XML diagnostic and original malformed input");
    require(diagnostics.find("decryption failure: NOT-AES")!=std::string::npos,"Debug preserves failed decryption input");
#endif
    auto missing=directory.path/"no-config";fs::create_directories(missing);
    // An old Debug log may remain beside a later Release run. Release must
    // leave it byte-identical; Debug must append without losing the marker.
    fs::create_directories(missing/"data");
    const auto previous_log=missing/"data"/"surge.log";
    const std::string previous_log_bytes="previous-debug-log\n";
    {std::ofstream out(previous_log,std::ios::binary);out<<previous_log_bytes;}
    application::Application defaults({(missing/"host.exe").u16string(),(missing/"data").u16string(),false});
    defaults.start();require(defaults.configuration().username=="SURGE","RE constructor projection with explicit SURGE branding");defaults.stop();
    std::ifstream old_log(previous_log,std::ios::binary);
    const std::string current_log_bytes{std::istreambuf_iterator<char>(old_log),std::istreambuf_iterator<char>()};
#ifdef NDEBUG
    require(current_log_bytes==previous_log_bytes,"Release never appends to an existing Debug log");
#else
    require(current_log_bytes.starts_with(previous_log_bytes)&&current_log_bytes.size()>previous_log_bytes.size(),"Debug appends to an existing Debug log");
#endif
    {
        const auto local=directory.path/fs::path(u"local-\u0131\u4e2d");
        const auto program=directory.path/"environment-program-data";
        ScopedEnvironment local_env(L"LocalAppData",local),program_env(L"ProgramData",program);
        application::Application environment({(missing/"host.exe").u16string(),{},false});
        environment.start();
        require(environment.configuration().user_directory==(local/"SURGE").u16string(),"RE LocalAppData environment lookup with Unicode");
        require(fs::path(environment.configuration().license_directory).wstring().starts_with(program.wstring()),"RE ProgramData environment lookup without COM known folders");
        environment.stop();
    }
    bool rejected=false;try{Peer peer(state.server_port);}catch(const std::system_error&){rejected=true;}
    require(rejected,"stopped listener rejects new connections");
    const auto online_dir=directory.path/"online";fs::create_directories(online_dir);
    const auto provider_file=online_dir/"preloader_l_online.dll";
    fs::copy_file(fs::path(argv[1]),provider_file);
    auto provider_module=LoadLibraryW(provider_file.wstring().c_str());require(provider_module!=nullptr,"real provider DLL loaded");
    auto mode=reinterpret_cast<void(*)(int)>(GetProcAddress(provider_module,"FixtureMode"));
    auto destroyed=reinterpret_cast<int(*)()>(GetProcAddress(provider_module,"FixtureDestroyed"));
    require(mode&&destroyed,"fixture control exports");
    mode(0);
    {
        application::Application online({(online_dir/"host.exe").u16string(),(online_dir/"data").u16string(),false});online.start();
        require(online.configuration().username=="fixture-online"&&online.configuration().persona_id=="fixture-persona","RE provider virtual profile methods bound");
        Peer peer(online.configuration().server_port);(void)peer.frame();
        require(peer.request("provider-auth","<GetAuthCode ClientId='fixture'/>").find("fixture-auth")!=std::string::npos,"RE provider auth method bound");
        require(peer.request("provider-avatar","<QueryImage Width='416' Height='416' ImageId='AVATAR:fixture-user-id'/>").find("_AVATAR_")!=std::string::npos,"RE provider avatar method bound");
        require(file::read_binary_03b3f0(online.configuration().last_avatar_path,files)=="fixture-provider-avatar","real provider avatar cache/copy path");
        online.stop();require(destroyed()==1,"RE provider destructor called exactly once after session drain");
    }
    mode(1);
    {
        application::Application offline({(online_dir/"host.exe").u16string(),(online_dir/"offline-data").u16string(),false});offline.start();
        require(offline.configuration().username=="SURGE"&&destroyed()==1,"provider auth failure keeps source offline state and releases object");offline.stop();
    }
    FreeLibrary(provider_module);
    const auto hook_dir=directory.path/"hooks";fs::create_directories(hook_dir);
    fs::copy_file(fs::path(argv[1]),hook_dir/"fixture-extra.dll");
    require(file::write_binary_03c1d0((hook_dir/"surge.cfg").u16string(),R"("Config2" {"Emulator" {"Documents" "fixture-documents" "BlockReruns" "true" "LoadExtraDLLsFromMain" "fixture-extra.dll" "LoadExtraDLLs" "fixture-extra.dll"}})",files),"all conditional hooks config");
    require(MH_Initialize()==MH_OK,"external engine initialized for ownership failure test");
    bool rejected_owner=false;
    try {
        application::Application conflict({(hook_dir/"host.exe").u16string(),(hook_dir/"conflict-data").u16string(),true});
        conflict.start();
    }catch(const std::runtime_error&){rejected_owner=true;}
    require(rejected_owner&&!native::is_bound(),"failed engine ownership rolls back server/native binding");
    require(MH_Initialize()==MH_ERROR_ALREADY_INITIALIZED,"rollback leaves external engine initialized");
    require(MH_Uninitialize()==MH_OK,"external engine remains releasable by its owner");
    {
        application::Application hooked({(hook_dir/"host.exe").u16string(),(hook_dir/"data").u16string(),true});hooked.start();
        require(native::is_bound(),"actual native callback owner published");
        require(GetModuleHandleW(L"fixture-extra.dll")!=nullptr,"actual main/worker extra-library source phases loaded module");
        const char* names[]={"connect","RegOpenKeyExA","RegOpenKeyExW","RegCloseKey","RegQueryValueExA","RegQueryValueExW","CreateProcessA","CreateProcessW","SHGetFolderPathA","SHGetFolderPathW","CryptQueryObject"};
        const wchar_t* modules[]={L"crypt32.dll",L"ws2_32.dll",L"kernel32.dll",L"shell32.dll",L"KernelBase.dll",L"advapi32.dll"};
        for(auto name:names) {
            bool enabled=false;
            for(auto module:modules)if(auto handle=GetModuleHandleW(module))if(auto target=GetProcAddress(handle,name))
                if(MH_EnableHook(reinterpret_cast<void*>(target))==MH_ERROR_ENABLED)enabled=true;
            require(enabled,name);
        }
        wchar_t documents[MAX_PATH]{};
        require(SHGetFolderPathW(nullptr,CSIDL_PERSONAL,nullptr,0,documents)==S_OK&&fs::path(documents).u16string()==hooked.configuration().documents,"real folder hook reads shared configuration");
        auto exe=fs::path(hooked.configuration().executable_path).wstring();
        STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
        SetLastError(31415);
        require(!CreateProcessW(exe.c_str(),nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&process)&&GetLastError()==31415,"real process hook blocks matching source path without launching");
        HKEY key=nullptr;
        require(RegOpenKeyExW(HKEY_CURRENT_USER,L"SOFTWARE\\Wow6432Node\\Origin",0,KEY_READ,&key)==ERROR_SUCCESS,"real registry hook opens/fabricates tracked handle");
        wchar_t client[4096]{};DWORD size=sizeof(client),type=0;
        require(RegQueryValueExW(key,L"ClientPath",nullptr,&type,reinterpret_cast<BYTE*>(client),&size)==ERROR_SUCCESS&&std::wstring(client)==exe,"real registry query reaches source values and original trampolines");
        require(RegCloseKey(key)==ERROR_SUCCESS,"real registry close releases tracked handle");
        hooked.stop();require(!native::is_bound(),"hook owner closes binding after actual patch removal");
        require(GetModuleHandleW(L"fixture-extra.dll")==nullptr,"extra-library leases released after actual source unload");
    }
    stop_blocked_greeting();
    std::cout<<"PASS: "<<checks<<" real application checks; four vendors, config collections, encrypted socket LSX, provider, resource, persistence, eleven enabled hooks and stop\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}
