// SPDX-License-Identifier: GPL-3.0-only
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>
#include <optional>

using Status=std::uint64_t(*)() noexcept;
using Faults=void(*)(unsigned,unsigned) noexcept;
using Shutdown=std::uint32_t(__stdcall*)(std::uint32_t) noexcept;
using Transport=std::uint64_t(*)() noexcept;
struct Workspace {
    std::filesystem::path old=std::filesystem::current_path(),root;
    Workspace(){
        root=std::filesystem::temp_directory_path()/
            (L"PRELOADER_L RF20 \u00c7orlu "+std::to_wstring(GetCurrentProcessId())+L" "+std::to_wstring(GetTickCount64()));
        if(!std::filesystem::create_directory(root))throw std::runtime_error("native workspace already exists");
        std::filesystem::create_directories(root/L"unrelated cwd");
        std::ofstream probe(root/L"unrelated cwd"/L"preloader_l.cfg",std::ios::binary);
        probe<<"\"Config2\" { \"Emulator\" { \"Online\" \"true\" } }";probe.close();
        if(!probe)throw std::runtime_error("CWD probe config write failed");
        std::filesystem::current_path(root/L"unrelated cwd");
    }
    ~Workspace(){std::error_code e;std::filesystem::current_path(old,e);std::filesystem::remove_all(root,e);}
};
bool await_terminal(Status status,unsigned expected,std::uint64_t& result){
    const auto begin=GetTickCount64();
    do {
        result=status();const auto phase=static_cast<unsigned>(result&255u);
        if(phase==expected)return true;
        if(phase==3 || phase==4 || phase==5 || phase==6)return false;
        Sleep(1);
    } while(GetTickCount64()-begin<10000);
    return false;
}
bool peer_closed(asio::ip::tcp::socket& socket){
    asio::error_code error;socket.non_blocking(true,error);if(error)return false;
    const auto begin=GetTickCount64();char byte;
    do{
        const auto count=socket.read_some(asio::buffer(&byte,1),error);
        if(count)return false; // all expected RF03 replies were already consumed
        if(error==asio::error::eof || error==asio::error::connection_reset)return true;
        if(error!=asio::error::would_block && error!=asio::error::try_again)return false;
        Sleep(1);
    }while(GetTickCount64()-begin<5000);
    return false;
}
std::optional<std::string> read_port_value(){
    SetLastError(ERROR_SUCCESS);const auto size=GetEnvironmentVariableA("EALsxPort",nullptr,0);
    if(!size){if(GetLastError()==ERROR_ENVVAR_NOT_FOUND)return std::nullopt;if(GetLastError()==ERROR_SUCCESS)return std::string{};throw std::runtime_error("port query");}
    std::string out(size,'\0');SetLastError(ERROR_SUCCESS);const auto count=GetEnvironmentVariableA("EALsxPort",out.data(),size);
    if(count>=size || (!count && GetLastError()!=ERROR_SUCCESS))throw std::runtime_error("port read");
    out.resize(count);return out;
}
struct EnvironmentGuard {
    std::optional<std::string> prior=read_port_value();
    EnvironmentGuard(){if(!SetEnvironmentVariableA("EALsxPort","RF03_HOST_ORIGINAL"))throw std::runtime_error("port fixture");}
    ~EnvironmentGuard(){SetEnvironmentVariableA("EALsxPort",prior?prior->c_str():nullptr);}
};
bool receive_exact(asio::ip::tcp::socket& socket,const std::string& expected){
    asio::error_code error;socket.non_blocking(true,error);if(error)return false;
    std::string received;const auto begin=GetTickCount64();
    while(received.size()<expected.size() && GetTickCount64()-begin<10000){
        char buffer[4096];const auto count=socket.read_some(asio::buffer(buffer),error);received.append(buffer,count);
        if(error && error!=asio::error::would_block && error!=asio::error::try_again)return false;
        if(!count)Sleep(1);
    }
    return received==expected;
}
int wmain(int argc,wchar_t** argv){
    if(argc!=2)return 2;
    try{
        const auto source=std::filesystem::absolute(argv[1]);Workspace workspace;EnvironmentGuard environment;
        struct Case{const wchar_t* name;const char* config;unsigned phase,file;bool online,faults,drop_host_ref;};
        const Case cases[]{
            {L"missing",nullptr,2,1,false,false,false},
            {L"online_true","\"Config2\" { \"Emulator\" { \"Online\" \"true\" } }",2,0,true,false,false},
            {L"numeric_ignored","\"Config2\" { \"Emulator\" { \"Online\" \"1\" } }",2,0,false,false,false},
            {L"invalid_config","\"Config2\" {",3,4,false,false,false},
            {L"cleanup_retry","\"Config2\" {}",2,0,false,true,false},
            {L"self_pin","\"Config2\" {}",2,0,false,false,true},
            {L"hooks","\"Config2\" { \"Game\" { \"ClientKey\" \"PRELOADER_L_RF21_NATIVE_MISSING\" \"ClientPath\" \"RF21_CLIENT\" } }",2,0,false,false,false}
        };
        for(const auto& test:cases){
            const auto dir=workspace.root/test.name;std::filesystem::create_directory(dir);
            const auto path=dir/L"preloader_l_native_test.dll";std::filesystem::copy_file(source,path);
            if(test.config){
                std::ofstream config(dir/L"preloader_l.cfg",std::ios::binary);config<<test.config;config.close();
                if(!config)return 9;
            }
            auto module=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
            if(!module)return 10;
            auto status=reinterpret_cast<Status>(GetProcAddress(module,"RFTestStatus"));
            auto faults=reinterpret_cast<Faults>(GetProcAddress(module,"RFTestSetFaults"));
            auto shutdown=reinterpret_cast<Shutdown>(GetProcAddress(module,"DllShutdown"));
            auto transport=reinterpret_cast<Transport>(GetProcAddress(module,"RFTestTransport"));
            if(!status || !faults || !shutdown || !transport)return 11;
            std::uint64_t observed=0;
            if(!await_terminal(status,test.phase,observed) || ((observed>>8)&255u)!=test.file ||
               ((observed>>16)&1u)!=static_cast<unsigned>(test.online))return 12;
            asio::io_context client_io;std::optional<asio::ip::tcp::socket> client;
            if(test.phase==2){
                if(((observed>>33)&7u)!=2u)return 50;
                const auto port=static_cast<std::uint16_t>((observed>>17)&0xffffu);if(!port)return 31;
                if(read_port_value()!=std::to_string(port))return 39;
                client.emplace(client_io);asio::error_code ec;
                // Real ws2_32!connect interception: every ready case connects
                // to the legacy SDK port, not to the published listener port.
                client->connect({asio::ip::address_v4::loopback(),3216},ec);if(ec)return 32;
                if(((status()>>40)&0xffffu)==0)return 51;
                if(std::wstring(test.name)==L"hooks") {
                    HKEY key=nullptr;
                    if(RegOpenKeyExA(HKEY_CURRENT_USER,"PRELOADER_L_RF21_NATIVE_MISSING",0,KEY_READ,&key)!=ERROR_SUCCESS || !key)return 52;
                    char value[32]{};DWORD type=0,bytes=sizeof(value);
                    if(RegQueryValueExA(key,"ClientPath",nullptr,&type,reinterpret_cast<BYTE*>(value),&bytes)!=ERROR_SUCCESS ||
                       type!=REG_SZ || std::string(value)!="RF21_CLIENT")return 53;
                    // Pseudo-handle preflight must reject shutdown without
                    // removing its still-needed RegCloseKey detour.
                    if(shutdown(10000)!=10)return 54;
                    if(RegCloseKey(key)!=ERROR_SUCCESS)return 55;
                    const auto retried=shutdown(10000);
                    std::printf("NATIVE_HOOKS_SHUTDOWN_RETRY:result=%lu status=%llu transport=%llu\n",
                        static_cast<unsigned long>(retried),static_cast<unsigned long long>(status()),
                        static_cast<unsigned long long>(transport()));
                    if(retried!=0)return 56;
                    if(!FreeLibrary(module) || GetModuleHandleW(path.c_str()))return 57;
                    std::puts("NATIVE_CASE_PASS:hooks_redirect_registry_pending_owner_retry");
                    continue;
                }
                const std::string frame="<LSX><Request id='native&amp;lt;'><IsProgressiveInstallationAvailable/></Request></LSX>";
                asio::write(*client,asio::buffer(frame.data(),5),ec);if(ec)return 33;
                asio::write(*client,asio::buffer(frame.data()+5,frame.size()-5),ec);if(ec)return 34;
                const auto pair=frame+frame;asio::write(*client,asio::buffer(pair),ec);if(ec)return 35;
                const auto begin=GetTickCount64();
                while((transport()&0xffffffffu)<3 && GetTickCount64()-begin<10000)Sleep(1);
                const auto counts=transport();if((counts&0xffffffffu)!=3 || (counts>>48)!=1 || ((counts>>32)&0xffffu)!=1)return 36;
                if((status()&255u)!=2)return 37;
                const std::string reply=" <LSX>\n  <Response id=\"native&amp;lt;\" sender=\"EbisuSDK\">\n    <IsProgressiveInstallationAvailableResponse ItemId=\"\" Available=\"false\"/>\n  </Response>\n </LSX>\n";
                if(!receive_exact(*client,reply+reply+reply))return 40;
                const std::string settings_request="<LSX><Request id='settings'><GetSettings/></Request></LSX><LSX><Request id='language'><GetSetting SettingId='LANG&#85;AGE'/></Request></LSX><LSX><Request id='connected'><GetInternetConnectedState/></Request></LSX>";
                asio::write(*client,asio::buffer(settings_request),ec);if(ec)return 43;
                const std::string yes=test.online?"true":"false",offline=test.online?"false":"true";
                const std::string settings_reply=" <LSX>\n  <Response id=\"settings\" sender=\"EbisuSDK\">\n    <GetSettingsResponse IsManualOffline=\""+offline+"\" Environment=\"production\" IsTelemetryEnabled=\"false\" IsIGOAvailable=\""+yes+"\" IsIGOEnabled=\""+yes+"\" Language=\"en_US\" IsAutomaticGameUpdatesEnabled=\"false\"/>\n  </Response>\n </LSX>\n";
                const std::string language_reply=" <LSX>\n  <Response id=\"language\" sender=\"EbisuSDK\">\n    <GetSettingResponse Setting=\"en_US\"/>\n  </Response>\n </LSX>\n";
                const std::string connected_reply=" <LSX>\n  <Response id=\"connected\" sender=\"EbisuSDK\">\n    <InternetConnectedState connected=\""+yes+"\"/>\n  </Response>\n </LSX>\n";
                if(!receive_exact(*client,settings_reply+language_reply+connected_reply))return 44;
                const std::string stateless_request="<LSX><Request id='chunk'><QueryChunkStatus/></Request></LSX><LSX><Request id='group'><GetGroupInfo/></Request></LSX><LSX><Request id='friends'><QueryFriends/></Request></LSX>";
                asio::write(*client,asio::buffer(stateless_request),ec);if(ec)return 45;
                const std::string stateless_reply=" <LSX>\n  <Response id=\"group\" sender=\"EbisuSDK\">\n    <ErrorSuccess Description=\"User not in a party, create one.\" Code=\"-1577058296\"/>\n  </Response>\n </LSX>\n <LSX>\n  <Response id=\"friends\" sender=\"EbisuSDK\">\n    <QueryFriendsResponse Return=\"Success\"/>\n  </Response>\n </LSX>\n";
                if(!receive_exact(*client,stateless_reply))return 46;
                const std::string profile_request="<LSX><Request id='profile'><GetProfile index='2'/></Request></LSX><LSX><Request id='presence'><GetPresence/></Request></LSX>";
                asio::write(*client,asio::buffer(profile_request),ec);if(ec)return 47;
                // Missing config uses constructor defaults; every explicitly
                // loaded config has empty-on-miss User profile fields.
                const bool defaults=test.config==nullptr;
                const std::string country=defaults?"US":"",persona=defaults?"1144668899":"",level=defaults?"2":"",name=defaults?"SURGE":"",user=defaults?"1000200030000":"",currency=defaults?"USD":"",subscriber=defaults?"true":"",under=defaults?"false":"",steam=defaults?"false":"";
                const std::string profile_reply=" <LSX>\n  <Response id=\"profile\" sender=\"EbisuSDK\">\n    <GetProfileResponse Country=\""+country+"\" GeoCountry=\""+country+"\" PersonaId=\""+persona+"\" IsTrialSubscriber=\"false\" SubscriberLevel=\""+level+"\" Persona=\""+name+"\" UserId=\""+user+"\" CommerceCountry=\""+country+"\" CommerceCurrency=\""+currency+"\" AvatarId=\"\" IsSubscriber=\""+subscriber+"\" UserIndex=\"2\" IsUnderAge=\""+under+"\" IsSteamSubscriber=\""+steam+"\"/>\n  </Response>\n </LSX>\n";
                const std::string presence_reply=" <LSX>\n  <Response id=\"presence\" sender=\"EbisuSDK\">\n    <GetPresenceResponse Title=\"\" SessionId=\"\" MultiplayerId=\"\" GroupId=\"\" TitleId=\"\" Presence=\"INGAME\" GamePresence=\"\" Group=\"\" RichPresence=\"Game\" UserId=\"0\"/>\n  </Response>\n </LSX>\n";
                if(!receive_exact(*client,profile_reply+presence_reply))return 48;
                // Keep this socket open through DllShutdown: the server must
                // cancel/drain its outstanding receive before module unload.
            }
            if(test.drop_host_ref){
                if(!FreeLibrary(module))return 13;
                // The worker's counted reference must keep this exact image
                // loaded. Reacquire a host reference before calling shutdown.
                if(!GetModuleHandleW(path.c_str()))return 14;
                module=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
                if(!module || (status()&255u)!=2)return 15;
            }
            if(test.faults){
                faults(1,1);
                if(shutdown(10000)!=7 || shutdown(10000)!=8)return 16;
            }
            if(shutdown(10000)!=0 || shutdown(0)!=0)return 17;
            if(client && !peer_closed(*client))return 38;
            if(read_port_value()!=std::optional<std::string>{"RF03_HOST_ORIGINAL"})return 41;
            if(!FreeLibrary(module) || GetModuleHandleW(path.c_str()))return 18;
            std::printf("NATIVE_CASE_PASS:%ls\n",test.name);
        }
        // Cancellation may win before initialization. A zero-budget timeout
        // must preserve ownership for the finite-budget retry.
        for(unsigned i=0;i<10;++i){
            const auto dir=workspace.root/(L"early_stop_"+std::to_wstring(i));std::filesystem::create_directory(dir);
            const auto path=dir/L"preloader_l_native_test.dll";std::filesystem::copy_file(source,path);
            auto module=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
            if(!module)return 20;
            auto shutdown=reinterpret_cast<Shutdown>(GetProcAddress(module,"DllShutdown"));if(!shutdown)return 21;
            const auto result=shutdown(0);if(result!=0 && result!=2 && result!=10)return 22;
            if(shutdown(10000)!=0 || !FreeLibrary(module) || GetModuleHandleW(path.c_str()))return 23;
            if(read_port_value()!=std::optional<std::string>{"RF03_HOST_ORIGINAL"})return 42;
        }
        std::puts("NATIVE_EARLY_STOP_PASS:10");
        std::puts("PRELOADER_L_NATIVE_ACCEPTANCE_PASS");
        return 0;
    }catch(const std::exception& error){std::fprintf(stderr,"NATIVE_FAIL:%s\n",error.what());return 30;}
}
