// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/file_copy.hpp"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cstring>
#include <stdexcept>

namespace metadata_probe {
struct State {
    bool armed=false,valid=true,known_cells=false;
    DWORD modern_error=0,legacy_error=0;
    unsigned modern=0,legacy=0;
} state;
BOOL WINAPI extended(HANDLE handle,FILE_INFO_BY_HANDLE_CLASS kind,LPVOID output,DWORD size){
    if(!state.armed)return ::GetFileInformationByHandleEx(handle,kind,output,size);
    ++state.modern;
    state.valid=state.valid&&handle&&handle!=INVALID_HANDLE_VALUE&&output&&
        kind==static_cast<FILE_INFO_BY_HANDLE_CLASS>(0x12)&&size==24;
    SetLastError(state.modern_error);return FALSE;
}
BOOL WINAPI legacy(HANDLE handle,LPBY_HANDLE_FILE_INFORMATION output){
    if(!state.armed)return ::GetFileInformationByHandle(handle,output);
    ++state.legacy;state.valid=state.valid&&handle&&handle!=INVALID_HANDLE_VALUE&&output;
    if(state.legacy_error){SetLastError(state.legacy_error);return FALSE;}
    if(state.known_cells){
        *output={};output->dwVolumeSerialNumber=0xf0e1d2c3;
        output->nFileIndexHigh=0x12345678;output->nFileIndexLow=0x9abcdef0;return TRUE;
    }
    return ::GetFileInformationByHandle(handle,output);
}
}
// Source-include the actual Windows adapter. Only these two metadata boundaries
// are intercepted; unarmed calls and ordinary file operations use real Win32.
#define GetFileInformationByHandleEx metadata_probe::extended
#define GetFileInformationByHandle metadata_probe::legacy
#include "../../src/platform/file_copy_windows.cpp"
#undef GetFileInformationByHandle
#undef GetFileInformationByHandleEx

namespace {
namespace fs=std::filesystem;
namespace copy=laureondo::file_copy;
unsigned checks=0;
void require(bool value,const char* what) {++checks;if(!value)throw std::runtime_error(what);}
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
struct MetadataFault {
    explicit MetadataFault(DWORD modern,DWORD legacy=0,bool known=false){
        require(!metadata_probe::state.armed,"metadata probes do not overlap");
        metadata_probe::state={};metadata_probe::state.armed=true;
        metadata_probe::state.modern_error=modern;metadata_probe::state.legacy_error=legacy;
        metadata_probe::state.known_cells=known;
    }
    ~MetadataFault(){metadata_probe::state.armed=false;}
};
struct MetadataHandle {
    copy::NativeSystemCalls system;copy::Handle value;
    explicit MetadataHandle(const fs::path& path):value(system.open_attributes(path.u16string(),FILE_SHARE_READ).handle){
        require(value!=copy::invalid_handle,"borrow an actual metadata handle");
    }
    ~MetadataHandle(){if(value!=copy::invalid_handle)system.close(value);}
    HANDLE native()const{return reinterpret_cast<HANDLE>(value);}
};
copy::Identity legacy_oracle(HANDLE handle){
    BY_HANDLE_FILE_INFORMATION raw{};
    require(::GetFileInformationByHandle(handle,&raw)!=FALSE,"independent real legacy metadata query");
    copy::Identity result;result.volume=raw.dwVolumeSerialNumber;
    // Windows x64 stores each DWORD little-endian. Snapshot the two distinct
    // fields independently; do not derive this answer from the tested adapter.
    std::memcpy(result.file_id.data(),&raw.nFileIndexHigh,4);
    std::memcpy(result.file_id.data()+4,&raw.nFileIndexLow,4);
    return result;
}
void native_identity_fallback(){
    Directory directory;
    const auto source=directory.path/"fallback-source.bin",target=directory.path/"fallback-target.bin";
    const auto link=directory.path/"fallback-hard-link.bin";
    write(source,std::string("source\0bytes",12));write(target,"retained-target");
    require(CreateHardLinkW(link.c_str(),source.c_str(),nullptr)!=FALSE,"own the fallback hard-link fixture");
    MetadataHandle original(source),alias(link);
    const auto expected=legacy_oracle(original.native());
    require(legacy_oracle(alias.native())==expected,"raw legacy oracle recognizes the physical hard link");
    for(const DWORD unsupported:{DWORD{50},DWORD{87}}){
        {
            MetadataFault fault(unsupported);
            copy::Identity actual;actual.volume=~std::uint64_t{0};actual.file_id.fill(0xa5);
            copy::Identity linked;
            require(copy::file_identity_04895c(original.value,actual,original.system)==0&&actual==expected,
                "forced FileIdInfo fallback reproduces exact real legacy bytes including zero extension");
            require(copy::file_identity_04895c(alias.value,linked,alias.system)==0&&linked==actual,
                "legacy fallback retains physical hard-link identity");
            require(metadata_probe::state.valid&&metadata_probe::state.modern==2&&metadata_probe::state.legacy==2,
                "two valid FileIdInfo class18/24-byte requests reach two real legacy calls");
        }
        {
            MetadataFault fault(unsupported,0,true);
            copy::Identity actual;actual.volume=~std::uint64_t{0};actual.file_id.fill(0xa5);
            const copy::Identity cells{0xf0e1d2c3,{0x78,0x56,0x34,0x12,0xf0,0xde,0xbc,0x9a,0,0,0,0,0,0,0,0}};
            require(copy::file_identity_04895c(original.value,actual,original.system)==0&&actual==cells,
                "native SDK legacy fields retain DWORD width, high/low order and all eight zero tail bytes");
            require(metadata_probe::state.valid&&metadata_probe::state.modern==1&&metadata_probe::state.legacy==1,
                "known-cell mapping still crosses exactly one typed native fallback boundary");
        }
        for(const DWORD denied:{DWORD{ERROR_ACCESS_DENIED},DWORD{ERROR_SHARING_VIOLATION}}){
            MetadataFault fault(unsupported,denied);copy::Identity actual;
            require(copy::file_identity_04895c(original.value,actual,original.system)==denied,
                "native fallback error is propagated instead of fabricating identity success");
            require(metadata_probe::state.valid&&metadata_probe::state.modern==1&&metadata_probe::state.legacy==1,
                "failed legacy call is not retried or replaced by another metadata route");
        }
        {
            MetadataFault fault(unsupported);const auto result=run(source,target,1);
            require(!result.copied&&result.error==0&&read(source)==std::string("source\0bytes",12)&&read(target)=="retained-target",
                "complete skip-existing copy uses fallback identities without overwriting either owned file");
            require(metadata_probe::state.valid&&metadata_probe::state.modern==2&&metadata_probe::state.legacy==2,
                "complete copy helper sends both physical handles through the native fallback");
        }
        require(legacy_oracle(original.native())==expected,"identity helpers leave the caller's borrowed handle usable");
    }
    for(const DWORD denied:{DWORD{ERROR_INVALID_FUNCTION},DWORD{ERROR_ACCESS_DENIED},DWORD{ERROR_CALL_NOT_IMPLEMENTED}}){
        MetadataFault fault(denied);copy::Identity actual;
        require(copy::file_identity_04895c(original.value,actual,original.system)==denied,
            "other modern metadata errors remain unchanged");
        require(metadata_probe::state.valid&&metadata_probe::state.modern==1&&metadata_probe::state.legacy==0,
            "errors outside 50/87 cannot invoke the legacy fallback");
    }
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
        // A live reader that denies write/delete sharing must prevent overwrite
        // without mutating the existing target; release it before retrying.
        const auto locked=CreateFileW(target.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,
                                      OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        require(locked!=INVALID_HANDLE_VALUE,"locked target fixture open failed");
        write(source,"retry-after-lock");
        r=run(source,target,2);
        const auto lock_error=r.error;
        const auto released=CloseHandle(locked);
        require(released!=FALSE,"locked target fixture close failed");
        require(!r.copied&&lock_error==ERROR_SHARING_VIOLATION &&
                read(target)=="new-source","locked target must reject overwrite without mutation");
        r=run(source,target,2);
        require(r.copied&&r.error==0&&read(target)=="retry-after-lock",
                "released target must permit retry of the same copy route");
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
        native_identity_fallback();
        std::cout<<"file copy Windows integration: PASS ("<<checks<<" checks; Unicode/binary, modes, hard links, native identity fallback)\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
