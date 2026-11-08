// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/platform/file_copy.hpp"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
namespace fs=std::filesystem;
namespace copy=citrine_eclipse_mirage::file_copy;
void require(bool value,const char* what) {if(!value)throw std::runtime_error(what);}
struct Directory {
    fs::path path;
    Directory() {
        const auto prefix="surge-copy-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64());
        for(unsigned i=0;i<32;++i) {
            auto p=fs::temp_directory_path()/(prefix+"-"+std::to_string(i));
            if(fs::create_directory(p)){path=std::move(p);return;}
        }
        throw std::runtime_error("could not create owned copy-test directory");
    }
    ~Directory(){std::error_code error;fs::remove_all(path,error);}
};
void write(const fs::path& p,const std::string& bytes) {
    std::ofstream out(p,std::ios::binary);out.write(bytes.data(),static_cast<std::streamsize>(bytes.size()));
    require(bool(out),"fixture write failed");
}
std::string read(const fs::path& p) {std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
void set_time(const fs::path& p,std::uint64_t value) {
    const auto h=CreateFileW(p.c_str(),FILE_WRITE_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    require(h!=INVALID_HANDLE_VALUE,"time fixture open failed");
    FILETIME time{static_cast<DWORD>(value),static_cast<DWORD>(value>>32)};
    const auto result=SetFileTime(h,nullptr,nullptr,&time);const auto closed=CloseHandle(h);
    require(result&&closed,"time fixture update failed");
}
copy::Result run(const fs::path& a,const fs::path& b,std::uint32_t options) {
    return copy::copy_file_windows_048d20(a.u16string(),b.u16string(),options);
}
}
int main() {
    try {
        Directory directory;const auto source=directory.path/fs::path(u"kaynak-\u00e7\u0131.bin");
        const auto target=directory.path/fs::path(u"hedef-\u015f.bin");
        const std::string original("A\0B\r\nZ",6);write(source,original);
        auto r=run(source,target,0);require(r.copied&&r.error==0&&read(target)==original,"new Unicode/binary copy failed");
        r=run(source,target,0);require(!r.copied&&r.error==ERROR_FILE_EXISTS,"existing destination error differs");
        write(source,"new-source");r=run(source,target,1);
        require(!r.copied&&r.error==0&&read(target)==original,"skip-existing changed destination");
        r=run(source,target,2);require(r.copied&&r.error==0&&read(target)=="new-source","overwrite failed");
        r=run(source,source,1);require(!r.copied&&r.error==ERROR_SHARING_VIOLATION,"same-file copy must fail");
        const auto link=directory.path/"hard-link.bin";
        require(CreateHardLinkW(link.c_str(),source.c_str(),nullptr)!=FALSE,"hard-link fixture failed");
        r=run(source,link,1);require(!r.copied&&r.error==ERROR_SHARING_VIOLATION,"hard-link identity not recognized");
        constexpr std::uint64_t epoch2020=132223104000000000ULL;
        write(source,"newer-content");write(target,"older-content");
        set_time(source,epoch2020+100000000);set_time(target,epoch2020);
        r=run(source,target,4);require(r.copied&&r.error==0&&read(target)=="newer-content","update-newer failed");
        write(source,"older-source");write(target,"newer-target");
        set_time(source,epoch2020);set_time(target,epoch2020+100000000);
        r=run(source,target,4);require(!r.copied&&r.error==0&&read(target)=="newer-target","update-older overwrote target");
        r=run(directory.path/"missing.bin",target,0);
        require(!r.copied&&r.error==ERROR_FILE_NOT_FOUND,"missing source error differs");
        copy::NativeSystemCalls system;const auto opened=system.open_attributes(source.u16string(),FILE_SHARE_READ);
        require(opened.handle!=copy::invalid_handle,"native metadata open failed");
        copy::Identity id{};copy::LegacyIdentity legacy{};std::int64_t timestamp{};
        const auto id_error=copy::file_identity_04895c(opened.handle,id,system);
        const auto legacy_error=system.legacy_file_info(opened.handle,legacy);
        const auto time_error=system.last_write(opened.handle,timestamp);
        const auto closed=system.close(opened.handle);
        require(id_error==0&&legacy_error==0&&time_error==0&&closed,"native metadata helpers failed");
        require(static_cast<std::uint32_t>(id.volume)==legacy.volume&&timestamp==static_cast<std::int64_t>(epoch2020),"native identity/time data differ");
        std::cout<<"file copy Windows integration: PASS (Unicode/binary, modes, hard links, metadata)\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
