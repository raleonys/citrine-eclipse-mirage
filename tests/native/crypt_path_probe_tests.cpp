// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/platform/crypt_query.hpp"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>

namespace {
thread_local bool count_allocations=false;
thread_local std::size_t allocations=0;
}
// Test-only observation of C++ heap requests; the application uses standard CRT.
void* operator new(std::size_t size){if(count_allocations)++allocations;if(auto* p=std::malloc(size?size:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t size){return ::operator new(size);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}

namespace hooks=preloader_l::hooks;
namespace platform=preloader_l::platform;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct AllocationScope {
    AllocationScope(){allocations=0;count_allocations=true;}
    ~AllocationScope(){count_allocations=false;}
};
struct Memory {
    std::u16string text;
    std::uintptr_t base=0x10000;
    std::size_t calls=0,fail_at=std::numeric_limits<std::size_t>::max();
    bool mutate_at_nul=false;
    explicit Memory(std::u16string value):text(std::move(value)){}
    static bool read(std::uintptr_t address,char16_t& unit,void* context)noexcept {
        auto& self=*static_cast<Memory*>(context);++self.calls;
        if(address<self.base||(address-self.base)%2)return false;
        const auto index=(address-self.base)/2;
        if(index>self.text.size()||index==self.fail_at)return false;
        unit=index==self.text.size()?0:self.text[index];
        if(!unit&&self.mutate_at_nul&&!self.text.empty())self.text.back()=u'X';
        return true;
    }
    hooks::PathProbe probe() {
        hooks::PathProbe result;
        {AllocationScope scope;result=hooks::probe_path(reinterpret_cast<const void*>(base),&read,this);}
        require(allocations==0,"streaming probe requests no C++ heap storage");return result;
    }
};
void selection_and_streaming() {
    for(std::size_t prefix=0;prefix<34;++prefix) {
        Memory memory(std::u16string(prefix,u'x')+std::u16string(hooks::target_suffix));
        const auto p=memory.probe();require(p.valid&&p.suffix_matches,"suffix ring preserves every tested rotation and source prefix rule");
        require(memory.calls==memory.text.size()+1,"each path unit read once including NUL");
        require(hooks::select_object(1,false,p)==hooks::Selection::companion,"valid case-sensitive suffix selected");
    }
    for(const auto* text:{u"",u"PRELOADER_L.DLL",u"preloader_l.dll.old",u"preloader_l.dl"}) {
        Memory memory(text);const auto p=memory.probe();require(p.valid&&!p.suffix_matches,"nonmatching paths remain original");
    }
    Memory unicode(std::u16string{char16_t{0xd83d},char16_t{0xde00}}+std::u16string(hooks::target_suffix));
    require(unicode.probe().suffix_matches,"valid surrogate pair prefix preserved");
    for(auto text:{std::u16string{char16_t{0xd800}},std::u16string{char16_t{0xdc00}},std::u16string{char16_t{0xd800},u'x'}}) {
        Memory invalid(std::move(text));const auto p=invalid.probe();require(!p.valid&&hooks::select_object(1,false,p)==hooks::Selection::original,"invalid UTF16 falls back without a trap");
    }
    Memory embedded(std::u16string{u'x',0}+std::u16string(hooks::target_suffix));
    const auto short_path=embedded.probe();require(short_path.valid&&!short_path.suffix_matches&&embedded.calls==2,"embedded NUL prevents later suffix inspection");
    Memory changed(u"preloader_l.dll");changed.mutate_at_nul=true;
    require(changed.probe().suffix_matches&&changed.text.back()==u'X',"decision uses copied units rather than rereading changed caller memory");
    const hooks::PathProbe matching{true,true,16};
    require(hooks::select_object(2,false,matching)==hooks::Selection::original&&hooks::select_object(1,true,{})==hooks::Selection::companion,"non-file and null-file rules unchanged");
}
void scan_boundaries() {
    Memory exact(std::u16string(hooks::max_path_units-1,u'x'));
    exact.text.replace(exact.text.size()-hooks::target_suffix.size(),hooks::target_suffix.size(),hooks::target_suffix);
    const auto p=exact.probe();require(p.valid&&p.suffix_matches&&p.units_examined==hooks::max_path_units&&exact.calls==hooks::max_path_units,"NUL at final permitted unit is valid");
    Memory too_long(std::u16string(hooks::max_path_units,u'x'));
    const auto q=too_long.probe();require(!q.valid&&too_long.calls==hooks::max_path_units,"terminator one unit beyond cap is never read");
    Memory unreadable(u"preloader_l.dll");unreadable.fail_at=7;
    const auto r=unreadable.probe();require(!r.valid&&unreadable.calls==8&&r.units_examined==7,"partial read fails without claiming a valid suffix");
    Memory overflow(u"xx");overflow.base=std::numeric_limits<std::uintptr_t>::max();
    require(!overflow.probe().valid&&overflow.calls==0,"entire UTF16 unit checked before overflowing address range");
    Memory last_unit(u"xx");last_unit.base=std::numeric_limits<std::uintptr_t>::max()-1;
    require(!last_unit.probe().valid&&last_unit.calls==1,"increment cannot wrap past final readable unit");
}
void native_reader() {
    platform::WindowsCryptQueryHost host;
    std::u16string longest(hooks::max_path_units-1,u'x');
    longest.replace(longest.size()-hooks::target_suffix.size(),hooks::target_suffix.size(),hooks::target_suffix);
    hooks::PathProbe p;
    {AllocationScope scope;p=host.inspect_path(longest.c_str());}
    require(p.valid&&p.suffix_matches&&allocations==0,"real protected Windows reader accepts longest valid path with no C++ heap allocation");
    require(!host.inspect_path(reinterpret_cast<const void*>(1)).valid,"inaccessible native pointer rejected safely");
    SYSTEM_INFO info{};GetSystemInfo(&info);
    auto* pages=static_cast<std::byte*>(VirtualAlloc(nullptr,info.dwPageSize*2,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    require(pages!=nullptr,"allocate native page boundary");
    DWORD previous=0;require(VirtualProtect(pages+info.dwPageSize,info.dwPageSize,PAGE_NOACCESS,&previous)!=FALSE,"guard following page");
    constexpr char16_t suffix[]=u"preloader_l.dll";
    auto* terminated=pages+info.dwPageSize-sizeof(suffix);std::memcpy(terminated,suffix,sizeof(suffix));
    require(host.inspect_path(terminated).suffix_matches,"NUL in last readable code unit is accepted");
    auto* truncated=pages+info.dwPageSize-(sizeof(suffix)-2);std::memcpy(truncated,suffix,sizeof(suffix)-2);
    require(!host.inspect_path(truncated).valid,"suffix crossing inaccessible terminator is not a match");
    require(VirtualFree(pages,0,MEM_RELEASE)!=FALSE,"release owned native pages");
}
}
int main()try{selection_and_streaming();scan_boundaries();native_reader();std::cout<<"PASS: "<<checks<<" bounded heap-free protected path/UTF16/suffix checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
