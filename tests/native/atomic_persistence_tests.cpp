// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/core/binary_io.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include <windows.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <type_traits>

using namespace preloader_l;
namespace io=file;
namespace fs=std::filesystem;
static_assert(!std::is_copy_constructible_v<io::AtomicFiles>,
    "Atomic recovery ownership must not be copied");
static_assert(!std::is_copy_assignable_v<io::AtomicFiles>,
    "Atomic recovery ownership must not be copy-assigned");
static_assert(!std::is_move_constructible_v<io::AtomicFiles>,
    "Atomic recovery must retain its stable owner");
static_assert(!std::is_move_assignable_v<io::AtomicFiles>,
    "Atomic recovery ownership must not be move-assigned");
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>void rejects(F fn,const char* why){bool failed=false;try{fn();}catch(const std::exception&){failed=true;}require(failed,why);}
struct Ops final:io::AtomicFileOps {
    std::map<configuration::Path,std::string> files{{u"xml","old-xml"},{u"ini","old-ini"}};
    unsigned next=0;bool fail_stage=false,fail_ini=false,fail_rollback=false,fail_cleanup=false;
    configuration::Path stage(const configuration::Path& target,std::string_view bytes)override {
        if(fail_stage&&target==u"ini")throw std::runtime_error("staging write/flush failed");
        const auto suffix=std::to_string(++next);auto path=target+u".stage";path.append(suffix.begin(),suffix.end());files[path]=bytes;return path;
    }
    bool exists(const configuration::Path& p)override{return files.contains(p);}
    configuration::Path backup(const configuration::Path& p)override{return stage(p+u".backup",files.at(p));}
    void replace(const configuration::Path& from,const configuration::Path& to)override {
        if((fail_ini&&to==u"ini")||(fail_rollback&&from.find(u"backup")!=std::u16string::npos))throw std::runtime_error("replace denied");
        files[to]=files.at(from);files.erase(from);
    }
    void remove(const configuration::Path& p)override {
        if(fail_cleanup&&p.find(u"backup")!=std::u16string::npos)throw std::runtime_error("cleanup denied");
        files.erase(p);
    }
};
void injected_failures() {
    const std::array<io::FileWrite,2> writes{{{u"xml","new-xml"},{u"ini","new-ini"}}};
    {Ops ops;io::AtomicFiles files(ops);ops.fail_stage=true;rejects([&]{files.replace(writes);},"staging failure is visible");
     require(ops.files.size()==2&&ops.files.at(u"xml")=="old-xml"&&ops.files.at(u"ini")=="old-ini","stage failure preserves both originals and cleans staging");}
    {Ops ops;io::AtomicFiles files(ops);ops.fail_ini=true;rejects([&]{files.replace(writes);},"second replacement failure is visible");
     require(ops.files.size()==2&&ops.files.at(u"xml")=="old-xml"&&ops.files.at(u"ini")=="old-ini","partial commit rolls back first file");
     require(!files.recovery_pending(),"successful rollback releases recovery ownership");}
    {Ops ops;io::AtomicFiles files(ops);ops.fail_ini=ops.fail_rollback=true;
     rejects([&]{files.replace(writes);},"incomplete rollback propagates");require(files.recovery_pending(),"failed rollback retains exact backup ownership");
     const auto count=ops.files.size();rejects([&]{files.replace(writes);},"new update waits for previous recovery");
     require(ops.files.size()==count,"blocked update does not create or overwrite staging");
     ops.fail_ini=ops.fail_rollback=false;files.recover();
     require(ops.files.size()==2&&ops.files.at(u"xml")=="old-xml"&&ops.files.at(u"ini")=="old-ini","retry restores exact old group");
     files.replace(writes);require(ops.files.size()==2&&ops.files.at(u"xml")=="new-xml"&&ops.files.at(u"ini")=="new-ini","new update succeeds after recovery");}
    {Ops ops;ops.files.erase(u"xml");io::AtomicFiles files(ops);ops.fail_ini=true;
     rejects([&]{files.replace(writes);},"new file partial commit failed");require(!ops.files.contains(u"xml")&&ops.files.at(u"ini")=="old-ini","rollback restores original absence");}
    {Ops ops;io::AtomicFiles files(ops);ops.fail_cleanup=true;files.replace(writes);
     require(!files.recovery_pending()&&ops.files.at(u"xml")=="new-xml","cleanup failure does not roll back committed state");
     ops.fail_cleanup=false;files.recover();require(ops.files.size()==2,"later cleanup removes retained owned backups");}
}
std::string read(const fs::path& path){std::ifstream in(path,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
void native_failures() {
    const auto root=fs::temp_directory_path()/("surge-atomic-"+std::to_string(GetCurrentProcessId()));
    fs::remove_all(root);fs::create_directories(root);
    struct Cleanup{fs::path root;~Cleanup(){std::error_code e;fs::remove_all(root,e);}} cleanup{root};
    const auto xml=root/fs::path(u"kayit-\u0131.xml"),ini=root/"achievements.ini";
    {std::ofstream out(xml,std::ios::binary);out<<"old-xml";}{std::ofstream out(ini,std::ios::binary);out<<"old-ini";}
    io::NativeFactory files;const std::array<io::FileWrite,2> writes{{{xml.u16string(),"new-xml"},{ini.u16string(),"new-ini"}}};
    HANDLE held=CreateFileW(ini.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
    require(held!=INVALID_HANDLE_VALUE,"hold destination without delete sharing");
    rejects([&]{files.replace_atomic(writes);},"native locked second replacement fails");CloseHandle(held);
    require(read(xml)=="old-xml"&&read(ini)=="old-ini","real locked destination restores first file and preserves second");
    files.replace_atomic(writes);require(read(xml)=="new-xml"&&read(ini)=="new-ini","real retry replaces Unicode XML and INI");
    std::size_t members=0;for(const auto& p:fs::directory_iterator(root)){(void)p;++members;}
    require(members==2,"successful batch leaves no staging or backup files");
    const std::array<io::FileWrite,2> missing{{{xml.u16string(),"discard"},{(root/"absent"/"file.ini").u16string(),"fail"}}};
    rejects([&]{files.replace_atomic(missing);},"missing second parent fails staging");
    require(read(xml)=="new-xml","native staging failure never truncates existing first file");
}
void event_publication() {
    event_queue::OwnedStorage storage;storage.construct_seed_copy("seed");storage.pop_front_locked();
    std::string value="achievement";
    rejects([&]{storage.push_persisted_locked(value,[&]{require(!storage.nonempty_relaxed(),"reserved event unpublished during persistence");throw std::runtime_error("disk failure");});},"persistence failure propagated");
    require(!storage.nonempty_locked()&&!storage.nonempty_relaxed(),"failed persistence discards reserved event");
    value="achievement";storage.push_persisted_locked(value,[&]{require(!storage.nonempty_relaxed(),"successful commit also starts unpublished");});
    require(storage.copy_front_locked()=="achievement"&&storage.nonempty_relaxed(),"event becomes visible only after persistence succeeds");
}
}
int main()try{injected_failures();native_failures();event_publication();std::cout<<"PASS: "<<checks<<" atomic persistence/recovery/publication checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
