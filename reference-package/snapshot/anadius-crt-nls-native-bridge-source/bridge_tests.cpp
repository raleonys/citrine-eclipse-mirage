#include "bridge.hpp"
#include <functional>
#include <exception>
#include <memory>
#include "../anadius-crt-locale-resource-bridge-source/bridge.hpp"
#include "../anadius-crt-argv-bridge-source/test_fixtures.hpp"
#include <limits>
namespace wr=anadius::crt_wildcard_recovered;
namespace ws=anadius::crt_wildcard_storage;
namespace sr=anadius::crt_sort_recovered;
namespace at=anadius::crt_argv_test;
namespace {
using f::check;
namespace rb=anadius::crt_recalloc_bridge;
namespace cb=anadius::crt_conversion_bridge;
namespace fb=anadius::crt_find_bridge;
namespace lb=anadius::crt_locale_bridge;
namespace lr=anadius::crt_locale_resource_bridge;
namespace mb=anadius::crt_multibyte_bridge;
namespace mr=anadius::crt_multibyte_record_bridge;
namespace nb=anadius::crt_nls_bridge;
namespace nd=anadius::crt_nls_dispatch_bridge;
namespace nn=anadius::crt_nls_native;
namespace dn=anadius::crt_nls_dispatch_native;
namespace np=anadius::crt_nls_native_bridge;
namespace mc=anadius::crt_multibyte_control_recovered;
namespace ms=anadius::crt_multibyte_support_recovered;
namespace lf=anadius::crt_locale_free_recovered;
namespace lref=anadius::crt_locale_ref_recovered;
namespace lu=anadius::crt_locale_update_recovered;
namespace lt=anadius::crt_locale_thread_recovered;
namespace pd=anadius::crt_ptd_data_recovered;
namespace nf=anadius::crt_find_native;
namespace mp=anadius::crt_module_path_recovered;
namespace ca=anadius::crt_conversion_api;
namespace cv=anadius::crt_conversion_recovered;
namespace rc=anadius::crt_recalloc_recovered;
struct Heap final:rc::Effects {
    er::State& errors;er::Effects& error_fx;f::Memory& memory;
    std::map<void*,std::size_t> reported;
    bool deny=false,deny_realloc=false,inplace=false,padding=false;
    unsigned allocations=0,reallocations=0,sizes=0,zeroes=0;
    Heap(er::State& s,er::Effects& e,f::Memory& m):errors(s),error_fx(e),memory(m){}
    h::Heap get_process_heap()override{return 0xaaa;}
    void* heap_alloc(h::Heap heap,std::uint32_t flags,std::uint64_t bytes)override{
        check(heap==0xaaa&&(flags==0||flags==8));++allocations;if(deny)return nullptr;
        check(bytes>0&&bytes<65536);const auto capacity=static_cast<std::size_t>(bytes)+(padding?8:0);
        const auto extent=std::max<std::size_t>(512,capacity);auto* p=::operator new(extent);
        std::memset(p,0xcc,extent);if(flags==8)std::memset(p,0,static_cast<std::size_t>(bytes));
        memory.blocks[p]=extent;reported[p]=capacity;return p;
    }
    void* heap_realloc(h::Heap heap,std::uint32_t flags,void* old,std::uint64_t bytes)override{
        check(heap==0xaaa&&flags==0&&reported.contains(old));++reallocations;
        if(deny_realloc)return nullptr;check(bytes>0&&bytes<65536);
        const auto old_size=reported.at(old);
        if(inplace&&bytes<=memory.blocks.at(old)){reported[old]=static_cast<std::size_t>(bytes);return old;}
        auto* replacement=heap_alloc(heap,0,bytes);if(!replacement)return nullptr;
        std::memcpy(replacement,old,std::min<std::size_t>(old_size,static_cast<std::size_t>(bytes)));
        check(heap_free(heap,0,old));return replacement;
    }
    std::uint64_t heap_size(h::Heap heap,std::uint32_t flags,void* p)override{check(heap==0xaaa&&flags==0);++sizes;return reported.at(p);}
    bool heap_free(h::Heap heap,std::uint32_t flags,void* p)override{check(reported.erase(p)==1);return memory.heap_free(heap,flags,p);}
    void zero_bytes_0793d0(void* begin,std::uint64_t bytes)override{
        const auto address=reinterpret_cast<std::uintptr_t>(begin);bool valid=false;
        for(auto [p,extent]:memory.blocks){const auto start=reinterpret_cast<std::uintptr_t>(p);if(address>=start&&bytes<=extent&&address-start<=extent-bytes)valid=true;}
        check(valid);++zeroes;std::memset(begin,0,static_cast<std::size_t>(bytes));
    }
    void invalid_parameter_05c954()override{throw std::runtime_error("unexpected invalid heap parameter");}
    std::uint32_t last_error()override{throw std::runtime_error("unexpected heap last error");}
    std::uint32_t errno_from_os_error_05d650(std::uint32_t value)override{return er::errno_from_os_error_05d650(value);}
    void write_error_slot_05d724(std::uint32_t value)override{er::errno_cell_05d724(errors,error_fx).store(value);}
    void lock_0625f8(std::uint32_t)override{throw std::runtime_error("unexpected new handler lock");}
    void unlock_06264c(std::uint32_t)override{throw std::runtime_error("unexpected new handler unlock");}
    std::int32_t invoke_new_handler(std::uintptr_t,std::uint64_t)override{throw std::runtime_error("unexpected new handler");}
};
struct Errors final:er::Effects {void* ptd=nullptr;void* optional_ptd_064d08()override{return ptd;}};
struct Invalid final:iv::Effects {
    unsigned handled=0;std::uint32_t error=0x1234;
    std::uint32_t last_error()override{return error;}
    void set_last_error(std::uint32_t value)override{error=value;}
    void* optional_ptd_064dd0(std::uint32_t*,std::uint64_t index)override{check(index==0);return nullptr;}
    std::uint32_t locale_changed_0a52d8()override{return 0;}
    std::array<iv::Address,2> locale_pair_0a1938()override{return{0,0};}
    iv::Address encoded_global_handler_0a4f00(std::uint64_t index)override{check(index==0);return r::encode_pointer(0x500,0x1234);}
    iv::Address security_cookie_0a1580()override{return 0x1234;}
    void invoke_handler(iv::Address address,const iv::Arguments& args)override{check(address==0x500&&args==iv::Arguments{});++handled;}
    [[noreturn]]void watson_05c9a4(const iv::Arguments&)override{throw std::runtime_error("invalid Watson");}
    [[noreturn]]void missing_ptd_05f03c()override{throw std::runtime_error("missing PTD");}
};
struct Lower;
inline Lower* active_lower=nullptr;
HANDLE WINAPI fixture_first(LPCWSTR,FINDEX_INFO_LEVELS,LPVOID,FINDEX_SEARCH_OPS,LPVOID,DWORD);
BOOL WINAPI fixture_next(HANDLE,LPWIN32_FIND_DATAW);
BOOL WINAPI fixture_close(HANDLE);
DWORD WINAPI fixture_module(HMODULE,LPWSTR,DWORD);
nf::Api fixture_api(){return{fixture_first,fixture_next,fixture_close};}
struct Lower final:cb::MemoryEffects,ca::Effects,ca::FileApiEffects,r::Effects,nb::Memory {
    using Address=std::uint64_t;
    f::Memory& memory;std::map<Address,std::size_t> borrowed;
    std::map<std::string,std::vector<std::u16string>> names{
        {"*.txt",{u".",u"..",u"z.txt",u"A.txt",u"m.txt",u"b.txt",u"Q.txt",u"w.txt",u"c.txt",u"L.txt",u"a.txt",u"D.txt",u"n.txt",u"B.txt"}},
        {"dir\\?.dat",{u"z.dat",u"a.dat"}}, {"*.onlydots",{u".",u".."}}
    };
    struct Session{const std::vector<std::u16string>* files;std::size_t index;};
    std::map<Address,Session> handles;Address serial=0x700000;
    nf::Session* native_find=nullptr;std::array<char16_t,264> module_path{};
    unsigned closed=0,null_patterns=0,module_calls=0;bool module_fail=false;
    std::uint32_t ansi=0x100,expected_cp=65001,error=87;
    unsigned file_calls=0,loads=0,lookups=0,mb_calls=0,wide_calls=0;
    bool export_present=true,fail_query=false,fail_fill=false,map_export_present=true,lcid_export_present=true;std::vector<std::string> lookup_names;
    explicit Lower(f::Memory& m):memory(m){constexpr char16_t path[]=u"fallback.exe";std::copy(std::begin(path),std::end(path),module_path.begin());borrowed[reinterpret_cast<Address>(module_path.data())]=sizeof(module_path);}
    struct Borrow {Address address;std::size_t old_size;bool existed;};
    std::map<std::uint64_t,Borrow> input_borrows;std::uint64_t borrow_serial=0;
    std::uint64_t borrow_readable(Address a,std::uint64_t n)override{
        const auto old=borrowed.find(a);const auto ticket=++borrow_serial;
        input_borrows.emplace(ticket,Borrow{a,old==borrowed.end()?0:old->second,old!=borrowed.end()});borrowed[a]=static_cast<std::size_t>(n);return ticket;
    }
    void release_readable(std::uint64_t ticket)noexcept override{
        const auto it=input_borrows.find(ticket);if(it==input_borrows.end())std::terminate();
        if(it->second.existed)borrowed[it->second.address]=it->second.old_size;else borrowed.erase(it->second.address);input_borrows.erase(it);
    }
    std::u16string wide(Address a){std::u16string result;for(;;a+=2){const auto value=read_u16(a);if(!value)return result;result.push_back(static_cast<char16_t>(value));}}
    std::byte* resolve(Address address,std::uint64_t bytes){
        for(const auto& [p,size]:memory.blocks){const auto begin=reinterpret_cast<Address>(p);if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return static_cast<std::byte*>(p)+static_cast<std::size_t>(address-begin);}
        for(const auto& [begin,size]:borrowed)if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return reinterpret_cast<std::byte*>(begin)+static_cast<std::size_t>(address-begin);
        throw std::out_of_range("bounded conversion fixture storage");
    }
    std::uint8_t read_u8(Address a)override{if(native_find&&native_find->contains(a,1))return native_find->read_u8(a);return std::to_integer<std::uint8_t>(*resolve(a,1));}
    std::uint16_t read_u16(Address a)override{if(native_find&&native_find->contains(a,2))return native_find->read_u16(a);std::uint16_t value;std::memcpy(&value,resolve(a,2),2);return value;}
    void write_u8(Address a,std::uint8_t v)override{*resolve(a,1)=static_cast<std::byte>(v);}
    void write_zero_u8(Address a)override{*resolve(a,1)=std::byte{0};}
    void write_u16(Address a,std::uint16_t v)override{std::memcpy(resolve(a,2),&v,2);}
    void write_zero_u16(Address a)override{std::memset(resolve(a,2),0,2);}
    std::uint64_t read_address(Address a)override{std::uint64_t v;std::memcpy(&v,resolve(a,8),8);return v;}
    void write_address(Address a,Address v)override{std::memcpy(resolve(a,8),&v,8);}
    std::uint32_t read_u32(Address a)override{std::uint32_t v;std::memcpy(&v,resolve(a,4),4);return v;}
    void write_u32(Address a,std::uint32_t v)override{std::memcpy(resolve(a,4),&v,4);}
    std::uint32_t last_error()override{return error;}
    std::int32_t multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,Address src,std::int32_t count,Address dst,std::int32_t capacity)override{
        ++mb_calls;check(cp==expected_cp&&flags==(cp==65001?8U:9U)&&count==-1);
        if(dst==0&&fail_query)return 0;if(dst!=0&&fail_fill)return 0;
        std::size_t bytes=0;while(read_u8(src+bytes)!=0){check(read_u8(src+bytes)<128);++bytes;}
        ++bytes;if(!dst){check(capacity==0);return static_cast<std::int32_t>(bytes);}
        check(capacity>=static_cast<std::int32_t>(bytes));
        for(std::size_t i=0;i<bytes;++i){const std::uint16_t c=read_u8(src+i);std::memcpy(resolve(dst+i*2,2),&c,2);}
        return static_cast<std::int32_t>(bytes);
    }
    std::int32_t wide_char_to_multi_byte(std::uint32_t cp,std::uint32_t flags,Address src,std::int32_t count,Address dst,std::int32_t capacity,Address def,Address used)override{
        ++wide_calls;check(cp==expected_cp&&flags==0&&count==-1&&def==0&&used==0);
        if(dst==0&&fail_query)return 0;if(dst!=0&&fail_fill)return 0;
        std::size_t characters=0;while(read_u16(src+characters*2)!=0){check(read_u16(src+characters*2)<128);++characters;}
        ++characters;if(!dst){check(capacity==0);return static_cast<std::int32_t>(characters);}
        check(capacity>=static_cast<std::int32_t>(characters));
        for(std::size_t i=0;i<characters;++i)*resolve(dst+i,1)=static_cast<std::byte>(read_u16(src+i*2));
        return static_cast<std::int32_t>(characters);
    }
    void write_bool32(Address a,std::int32_t value)override{std::memcpy(resolve(a,4),&value,4);}
    r::Module load_library_ex_w(std::u16string_view name,r::Module file,std::uint32_t flags)override{check((name==u"kernel32"||name==u"api-ms-win-core-localization-l1-2-1")&&file==0&&flags==0x800);++loads;return 0x4000;}
    bool free_library(r::Module)override{throw std::runtime_error("unexpected fixture module free");}
    r::Address get_proc_address(r::Module module,std::string_view name)override{check(module==0x4000);++lookups;lookup_names.emplace_back(name);if(name=="AreFileApisANSI")return export_present?0x5000:0;if(name=="LCMapStringEx")return map_export_present?0x6000:0;check(name=="LocaleNameToLCID");return lcid_export_present?0x7000:0;}
    std::uint32_t call_are_file_apis_ansi(Address a)override{check(a==0x5000);++file_calls;return ansi;}
    std::uint32_t process_parameters_flags()override{throw std::runtime_error("unexpected PEB fixture");}
    std::int32_t call_ro_initialize(r::Address,std::uint32_t)override{throw std::runtime_error("unexpected RO fixture");}
    void call_ro_uninitialize(r::Address)override{throw std::runtime_error("unexpected RO fixture");}
    std::uint32_t call_thread_initialization_policy(r::Address,std::uintptr_t,std::int32_t*)override{throw std::runtime_error("unexpected thread policy fixture");}
};
struct PtdFixture final:p::Effects {
    p::Ptd& value;Lower& lower;unsigned gets=0;bool missing=false;
    PtdFixture(p::Ptd& v,Lower& l):value(v),lower(l){}
    p::Ptd* fls_get_value(std::uint32_t index)override{check(index==7);++gets;lower.error=0xdead;return missing?p::construction_sentinel():&value;}
    bool fls_set_value(std::uint32_t,p::Ptd*)override{throw std::runtime_error("unexpected PTD allocation");}
    std::uint32_t fls_alloc(p::FlsCallback)override{throw std::runtime_error("unexpected FLS allocate");}
    bool fls_free(std::uint32_t)override{throw std::runtime_error("unexpected FLS free");}
    std::uint32_t last_error()override{return lower.error;}
    void set_last_error(std::uint32_t v)override{lower.error=v;}
    p::Ptd* calloc_ptd_066848()override{throw std::runtime_error("unexpected PTD calloc");}
    void free_0668c0(p::Ptd*)override{throw std::runtime_error("unexpected PTD free");}
    void construct_ptd_array_0648fc(p::Ptd*)override{throw std::runtime_error("unexpected PTD construct");}
    void destroy_ptd_array_0649ec(p::Ptd*)override{throw std::runtime_error("unexpected PTD destroy");}
    [[noreturn]]void abort_05f03c()override{throw std::runtime_error("synthetic required PTD abort");}
};
struct Resources final:lr::Resources {
    Lower& memory;unsigned locks=0,unlocks=0,atomic_calls=0;std::array<unsigned,6> depths{};
    Resources(Lower& m,lb::Globals& g,const lref::Identities& ids,const lf::Defaults& defaults,h::State& heap,h::Effects& heap_fx):lr::Resources(g,m,ids,defaults,heap,heap_fx),memory(m){}
    void lock_0625f8(std::uint32_t id)override{check(id==4||id==5);++depths[id];++locks;}
    void unlock_06264c(std::uint32_t id)override{check(id<depths.size()&&depths[id]>0);--depths[id];++unlocks;}
    std::uint32_t atomic_fetch_add_32(pd::Address a,std::uint32_t increment)override{++atomic_calls;const auto old=memory.read_u32(a);memory.write_u32(a,old+increment);return old;}
};
void fill_record(WIN32_FIND_DATAW& data,const std::u16string& name){
    check(name.size()<std::size(data.cFileName));std::fill(std::begin(data.cFileName),std::end(data.cFileName),L'\0');
    for(std::size_t i=0;i<name.size();++i)data.cFileName[i]=static_cast<WCHAR>(name[i]);
    data.dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY; // original search does not filter directories
}
HANDLE WINAPI fixture_first(LPCWSTR pattern,FINDEX_INFO_LEVELS info,LPVOID output,FINDEX_SEARCH_OPS search,LPVOID filter,DWORD flags){
    check(active_lower&&info==FindExInfoStandard&&search==FindExSearchNameMatch&&!filter&&!flags);auto& lower=*active_lower;
    if(!pattern){++lower.null_patterns;return INVALID_HANDLE_VALUE;}
    const auto text=lower.wide(reinterpret_cast<std::uint64_t>(pattern));const std::string key(text.begin(),text.end());
    const auto item=lower.names.find(key);if(item==lower.names.end())return INVALID_HANDLE_VALUE;
    const auto handle=++lower.serial;lower.handles.emplace(handle,Lower::Session{&item->second,0});
    fill_record(*static_cast<WIN32_FIND_DATAW*>(output),item->second[0]);return reinterpret_cast<HANDLE>(handle);
}
BOOL WINAPI fixture_next(HANDLE handle,LPWIN32_FIND_DATAW output){
    check(active_lower);auto& lower=*active_lower;auto& session=lower.handles.at(reinterpret_cast<std::uint64_t>(handle));
    if(++session.index==session.files->size())return 0;fill_record(*output,(*session.files)[session.index]);return 0x100;
}
BOOL WINAPI fixture_close(HANDLE handle){check(active_lower);auto& lower=*active_lower;check(lower.handles.erase(reinterpret_cast<std::uint64_t>(handle))==1);++lower.closed;return 1;}
DWORD WINAPI fixture_module(HMODULE module,LPWSTR destination,DWORD capacity){
    check(active_lower&&module==nullptr&&capacity==261);auto& lower=*active_lower;++lower.module_calls;
    if(lower.module_fail)return 0;
    DWORD length=0;while(length<capacity){destination[length]=static_cast<WCHAR>(lower.module_path[length]);if(destination[length]==0)return length;++length;}
    throw std::runtime_error("fixture module path missing supported terminator");
}
struct Effects final:lb::WildcardEffects {
    using Address=wr::Address;
    f::Memory& memory;Heap& heap;Lower& lower;av::Effects* argv=nullptr;
    std::map<Address,std::size_t>& borrowed;
    unsigned clears=0,sort_writes=0,copies=0;
    std::uint64_t flags_address=0;
    bool deny_final=false,fail_final_copy=false,has_final=false;
    Effects(f::Memory& m,Heap& h,rb::Allocator& allocator,cv::Effects& conversion,r::State& runtime,Lower& l,nf::Session& session,lu::Effects& locale):WildcardEffects(allocator,conversion,runtime,l,l,session,locale),memory(m),heap(h),lower(l),borrowed(l.borrowed){}
    std::byte* resolve(Address address,std::uint64_t bytes){
        for(const auto& [p,size]:memory.blocks){const auto begin=reinterpret_cast<Address>(p);if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return static_cast<std::byte*>(p)+static_cast<std::size_t>(address-begin);}
        for(const auto& [begin,size]:borrowed)if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return reinterpret_cast<std::byte*>(begin)+static_cast<std::size_t>(address-begin);
        throw std::out_of_range("synthetic wildcard storage bounds");
    }
    std::uint64_t length(Address source)override{std::uint64_t count=0;while(read_u8(source+count)!=0)++count;return count;}
    std::uint32_t memcpy_s_0767fc(Address dst,std::uint64_t capacity,Address src,std::uint64_t count)override{
        ++copies;if(fail_final_copy&&has_final)return 22;
        if(!dst||!src||count>capacity)return 22;
        std::memcpy(resolve(dst,count),resolve(src,count),static_cast<std::size_t>(count));return 0;
    }
    std::uint64_t read_word(Address address)override{std::uint64_t value;std::memcpy(&value,resolve(address,8),8);return value;}
    void write_word(Address address,std::uint64_t value)override{std::memcpy(resolve(address,8),&value,8);}
    [[noreturn]]void watson_05c9a4()override{throw std::runtime_error("synthetic copy Watson");}
    std::uint8_t read_u8(Address address)override{

        return lower.read_u8(address);
    }
    void write_u8(Address address,std::uint8_t value)override{++sort_writes;*resolve(address,1)=static_cast<std::byte>(value);}
    er::Cell32 errno_cell_05d724()override{return argv->errno_05d724();}
    void invalid_parameter_05c954()override{argv->invalid_parameter_05c954();}

    std::uint16_t read_u16(Address address)override{return lower.read_u16(address);}
    std::uint32_t read_u32(Address address)override{return lower.read_u32(address);}
    Address read_address(Address address)override{return read_word(address);}
    void write_u32(Address address,std::uint32_t value)override{check(address==flags_address);lower.write_u32(address,value);++clears;}
    std::string narrow(Address address){const auto size=length(address);return{reinterpret_cast<char*>(resolve(address,size+1)),static_cast<std::size_t>(size)};}
    std::u16string wide(Address address){std::u16string result;for(;;address+=2){const auto c=read_u16(address);if(!c)return result;result.push_back(static_cast<char16_t>(c));}}
    Address allocate_buffer_063888(std::uint64_t pointers,std::uint64_t characters,std::uint64_t element)override{
        const auto previous=heap.deny;heap.deny=deny_final;auto allocation=av::allocate_buffer_063888(pointers,characters,element,*argv);heap.deny=previous;
        if(!allocation.base)return 0;
        (void)::new(allocation.base) char*[static_cast<std::size_t>(pointers)]{};
        (void)::new(static_cast<void*>(allocation.storage.data()+pointers*8)) char[static_cast<std::size_t>(characters)]{};
        has_final=true;return reinterpret_cast<Address>(allocation.base);
    }
};
// Synthetic native/remaining lower-wrapper boundary. The record, SBCS and case
// algorithms themselves now come from the recovered source, not this fixture.
struct Native final:nb::NativeEffects,nd::NativeCalls {
    Lower& memory;std::uint32_t acp=1252,oem=437,valid_result=0x100,helper_result=0x100,dynamic_dbcs=0;
    bool deny_cpinfo=false,undefined_types=false;
    unsigned acp_calls=0,oem_calls=0,type_calls=0,map_calls=0,exchanges=0,valid_calls=0,info_calls=0;
    explicit Native(Lower& m):memory(m){}
    std::uint32_t get_acp()override{++acp_calls;return acp;}
    std::uint32_t get_oemcp()override{++oem_calls;return oem;}
    std::uint32_t is_valid_code_page(std::uint32_t cp)override{check(cp<=0xffff);++valid_calls;return valid_result;}
    std::uint32_t get_cp_info(std::uint32_t cp,mr::CPInfo& out)override{
        ++info_calls;if(deny_cpinfo)return 0;out.max_char_size=1;out.lead_bytes.fill(0);
        if(cp==932){out.max_char_size=2;out.lead_bytes={0x81,0x9f,0xe0,0xfc,0,0,0,0,0,0,0,0};}
        else if(cp==936||cp==949||cp==950||cp==dynamic_dbcs){out.max_char_size=2;out.lead_bytes={0x81,0xfe,0,0,0,0,0,0,0,0,0,0};}
        else if(cp==1361){out.max_char_size=2;out.lead_bytes={0x81,0xd3,0xd8,0xde,0xe0,0xf9,0,0,0,0,0,0};}
        return 0x100;
    }
    struct Scratch {std::unique_ptr<std::byte[]> bytes;std::uint64_t address;};
    std::vector<Scratch> scratch;unsigned frames=0,maximum_frames=0,stack_allocations=0,tail_reads=0,mb_queries=0,mb_fills=0,wide_queries=0,wide_fills=0;
    bool fail_native_map=false;std::uint8_t tail=0xcc;std::uint32_t last_query_flags=0;
    std::uint64_t begin_scratch_frame()override{maximum_frames=std::max(maximum_frames,++frames);return scratch.size();}
    void end_scratch_frame(std::uint64_t marker)noexcept override{
        while(scratch.size()>marker){memory.borrowed.erase(scratch.back().address);scratch.pop_back();}
        if(!frames)std::terminate();--frames;
    }
    std::uint64_t stack_allocate(std::uint64_t n)override{
        check(n>0&&n<=1024);++stack_allocations;auto data=std::make_unique_for_overwrite<std::byte[]>(static_cast<std::size_t>(n));
        std::memset(data.get(),0xcc,static_cast<std::size_t>(n));const auto a=reinterpret_cast<std::uint64_t>(data.get());memory.borrowed[a]=static_cast<std::size_t>(n);scratch.push_back({std::move(data),a});return a;
    }
    std::uint8_t read_input_tail(std::uint64_t)override{++tail_reads;return tail;}
    std::int32_t multi_byte_to_wide(std::uint32_t,std::uint32_t flags,std::uint64_t src,std::int32_t n,std::uint64_t dst,std::int32_t cap)override{
        check(n>0&&(flags==0||flags==1||flags==8||flags==9));if(!dst){++mb_queries;last_query_flags=flags;check(cap==0);return n;}
        ++mb_fills;check(cap>=n);for(int i=0;i<n;++i)memory.write_u16(dst+static_cast<unsigned>(i)*2,memory.read_u8(src+static_cast<unsigned>(i)));return n;
    }
    std::int32_t wide_char_to_multi_byte(std::uint32_t,std::uint32_t flags,std::uint64_t src,std::int32_t n,std::uint64_t dst,std::int32_t cap,std::uint64_t def,std::uint64_t used,std::span<nb::ByteOutput> tracked)override{
        check(n>0&&flags==0&&def==0&&used==0);if(!dst){++wide_queries;check(cap==0);return n;}++wide_fills;check(cap>=n);
        for(int i=0;i<n;++i){const auto c=memory.read_u16(src+static_cast<unsigned>(i)*2);check(c<=255);if(!tracked.empty()){check(static_cast<std::size_t>(i)<tracked.size());tracked[static_cast<std::size_t>(i)].set(static_cast<std::uint8_t>(c));}else memory.write_u8(dst+static_cast<unsigned>(i),static_cast<std::uint8_t>(c));}return n;
    }
    std::uint32_t get_string_type_w(std::uint32_t type,std::uint64_t src,std::int32_t n,std::uint64_t,std::span<nb::WordOutput> out)override{
        check(type==1&&n>0&&static_cast<std::size_t>(n)<=out.size());++type_calls;
        if(!undefined_types)for(int i=0;i<n;++i){const auto c=memory.read_u16(src+static_cast<unsigned>(i)*2);out[static_cast<std::size_t>(i)].set(c>='A'&&c<='Z'?1:c>='a'&&c<='z'?2:0);}
        return helper_result;
    }
    std::int32_t map_native(std::uint64_t,std::uint32_t flags,std::uint64_t src,std::int32_t n,std::uint64_t dst,std::int32_t cap,std::uint64_t v,std::uint64_t r,std::uint64_t h,std::span<nb::ByteOutput> tracked){
        check(n>0&&v==0&&r==0&&h==0);++map_calls;if(fail_native_map)return 0;
        if(!dst){check(cap==0);return n;}check(cap>=n);
        for(int i=0;i<n;++i){auto c=memory.read_u16(src+static_cast<unsigned>(i)*2);if(flags&0x400){check(static_cast<std::size_t>(i)<tracked.size());tracked[static_cast<std::size_t>(i)].set(static_cast<std::uint8_t>(c));}
            else{if(flags==0x100&&c>='A'&&c<='Z')c+=32;else if(flags==0x200&&c>='a'&&c<='z')c-=32;memory.write_u16(dst+static_cast<unsigned>(i)*2,c);}}
        return n;
    }
    std::int32_t lc_map_string_0671d4(std::uint64_t,std::uint32_t,std::uint64_t,std::int32_t,std::uint64_t,std::int32_t,std::uint64_t,std::uint64_t,std::uint64_t,std::span<nb::ByteOutput>)override{
        throw std::logic_error("old unresolved dispatch fixture must not be called");
    }
    struct ExArguments {std::uint64_t function,name,source,destination,version,reserved,sort_handle;std::uint32_t flags;std::int32_t count,capacity;};
    ExArguments last_ex{};std::uint32_t last_w_lcid=0,native_lcid=0x12340804,last_lcid_flags=0;
    std::uint64_t last_lcid_name=0;unsigned ex_calls=0,w_calls=0,locale_calls=0;
    std::function<void()> reenter;bool force_ex_result=false;std::int32_t ex_result=0;
    std::int32_t lc_map_string_ex(std::uint64_t fn,std::uint64_t name,std::uint32_t flags,std::uint64_t src,std::int32_t n,std::uint64_t dst,std::int32_t cap,std::uint64_t v,std::uint64_t r,std::uint64_t h,std::span<nb::ByteOutput> tracked)override{
        check(fn==0x6000);++ex_calls;last_ex={fn,name,src,dst,v,r,h,flags,n,cap};
        if(reenter){auto callback=std::move(reenter);reenter={};callback();}
        if(force_ex_result)return ex_result;
        return map_native(name,flags,src,n,dst,cap,0,0,0,tracked);
    }
    std::int32_t lc_map_string_w(std::uint32_t id,std::uint32_t flags,std::uint64_t src,std::int32_t n,std::uint64_t dst,std::int32_t cap,std::span<nb::ByteOutput> tracked)override{
        ++w_calls;last_w_lcid=id;return map_native(0,flags,src,n,dst,cap,0,0,0,tracked);
    }
    std::uint32_t locale_name_to_lcid(std::uint64_t fn,std::uint64_t name,std::uint32_t flags)override{
        check(fn==0x7000);++locale_calls;last_lcid_name=name;last_lcid_flags=flags;return native_lcid;
    }
    std::uint32_t exchange_32(std::uint64_t a,std::uint32_t v)override{
        ++exchanges;const auto old=memory.read_u32(a);memory.write_u32(a,v);return old;
    }
};
struct Harness {
    er::State errors;Errors error_fx;f::Memory memory;Heap heap_fx{errors,error_fx,memory};h::State heap{0xaaa,0,0,0};Invalid invalid;
    rb::Allocator allocator{heap,heap_fx};Lower lower{memory};nf::Session find{fixture_api()};r::State runtime{0x1234};
    std::array<std::byte,0x160> locale_a{},locale_b{};std::array<std::byte,0x228> mb_a{},mb_b{};p::Ptd ptd;p::State ptd_state{7};PtdFixture ptd_fx{ptd,lower};lb::Globals globals;lref::Identities ids{};lf::Defaults defaults;std::array<std::byte,0x2c0> initial_time{};std::array<std::byte,0x98> initial_lconv{};std::array<char16_t,1> category_name{};
    Resources resources{lower,globals,ids,defaults,heap,heap_fx};lb::ThreadEffects thread{globals,lower,ptd_state,ptd_fx,resources,heap,heap_fx};lb::UpdateEffects locale{globals,lower,thread};
    std::array<std::uint8_t,257> initial_type{};std::array<std::uint8_t,256> initial_case{};
    mc::Identities mb_ids{};mb::Globals mb_globals;Native native{lower};mr::Identities record_ids{};
    std::array<std::u16string,4> locale_names{u"ja-JP",u"zh-CN",u"ko-KR",u"zh-TW"};
    nd::Effects dispatch{native,native,runtime,lower,lower};
    nb::Effects nls{{lower,locale,heap,heap_fx,dispatch}};
    mr::RecordNative record_native{lower,mb_globals,record_ids,nls};
    mb::Effects multibyte{{globals,mb_globals,lower,ptd_state,ptd_fx,heap,heap_fx,errors,error_fx,invalid,thread,locale,resources,record_native,mb_ids}};
    cb::BufferEffects conversion{heap,heap_fx,errors,error_fx,lower,lower};Effects effects{memory,heap_fx,allocator,conversion,runtime,lower,find,locale};
    std::array<WCHAR,264> path_scratch{};fb::SdkPathProvider module_api{path_scratch,fixture_module};fb::PathEffects path{conversion,effects,module_api};mb::Policy policy{effects,path,mb_ids,multibyte};ag::ArgvEffects argv_fx{heap,heap_fx,errors,error_fx,invalid,policy};bc::ParserEffects parser{effects};
    std::uint64_t ptd_address(){return reinterpret_cast<std::uint64_t>(&ptd);}
    template<class T>void borrow(T& value){lower.borrowed[reinterpret_cast<std::uint64_t>(&value)]=sizeof(value);}
    void set_code_page(std::uint32_t cp){lower.write_u32(reinterpret_cast<std::uint64_t>(locale_a.data())+0xc,cp);lower.write_u32(reinterpret_cast<std::uint64_t>(locale_b.data())+0xc,cp);}
    Harness(){
        effects.argv=&argv_fx;lower.native_find=&find;active_lower=&lower;
        borrow(ptd);borrow(globals);borrow(locale_a);borrow(locale_b);borrow(mb_a);borrow(mb_b);borrow(path_scratch);borrow(initial_time);borrow(initial_lconv);borrow(category_name);
        borrow(initial_type);borrow(initial_case);borrow(mb_globals);
        mb_ids={reinterpret_cast<std::uint64_t>(mb_a.data()),reinterpret_cast<std::uint64_t>(initial_type.data()),reinterpret_cast<std::uint64_t>(initial_case.data())};
        record_ids.initial_multibyte_0a1c70=mb_ids.initial_multibyte_0a1c70;
        for(unsigned i=0;i<4;++i){record_ids.locale_names[i]=reinterpret_cast<std::uint64_t>(locale_names[i].data());lower.borrowed[record_ids.locale_names[i]]=(locale_names[i].size()+1)*2;}
        record_native.bind_system_cp(multibyte);
        ids.category_name_0a1948=defaults.category_name_0a1948=reinterpret_cast<std::uint64_t>(category_name.data());
        ids.initial_time_08ad20=defaults.initial_time_08ad20=reinterpret_cast<std::uint64_t>(initial_time.data());defaults.lconv_0a1700=reinterpret_cast<std::uint64_t>(initial_lconv.data());
        globals.changed_0a52d8=1;globals.mask_0a1c6c=1;globals.pair_0a1938={reinterpret_cast<std::uint64_t>(locale_a.data()),reinterpret_cast<std::uint64_t>(mb_a.data())};
        globals.locale_0a52e0=globals.initial_locale_0a17e0=globals.pair_0a1938[0];globals.multibyte_0a59d0=globals.initial_multibyte_0a1c70=globals.pair_0a1938[1];
        lower.write_address(ptd_address()+0x90,globals.locale_0a52e0);lower.write_address(ptd_address()+0x88,globals.multibyte_0a59d0);lower.write_u32(ptd_address()+0x3a8,1);
        set_code_page(65001);effects.flags_address=ptd_address()+0x3a8;
        for(auto* block:{&mb_a,&mb_b}){const auto a=reinterpret_cast<std::uint64_t>(block->data());lower.write_u32(a,1);lower.write_u32(a+4,1252);lower.write_u32(a+8,1);(*block)[0x19+0x81]=std::byte{4};}
        for(auto* block:{&locale_a,&locale_b})lower.write_u32(reinterpret_cast<std::uint64_t>(block->data())+0x10,1);
    }
};
struct Context final:np::ContextEffects {
    Native& state;explicit Context(Native& s):state(s){}
    std::uint32_t exchange_32(std::uint64_t a,std::uint32_t v)override{return state.exchange_32(a,v);}
    std::uint64_t begin_scratch_frame()override{return state.begin_scratch_frame();}
    void end_scratch_frame(std::uint64_t n)noexcept override{state.end_scratch_frame(n);}
    std::uint64_t stack_allocate(std::uint64_t n)override{return state.stack_allocate(n);}
    std::uint8_t read_input_tail(std::uint64_t a)override{return state.read_input_tail(a);}
};
// Only the two fixed system-module names requested by the recovered source.
// This fixture owns every LoadLibrary reference and releases it after calls.
struct Loader final:r::Effects {
    std::vector<HMODULE> owned;unsigned loads=0,lookups=0;bool no_ex=false,no_locale=false;
    Loader(){owned.reserve(32);}
    ~Loader(){const auto error=::GetLastError();for(auto h:owned)(void)::FreeLibrary(h);::SetLastError(error);}
    r::Module load_library_ex_w(std::u16string_view name,r::Module file,std::uint32_t flags)override{
        check(file==0&&(name==u"api-ms-win-core-localization-l1-2-1"||name==u"kernel32"));++loads;
        const auto handle=::LoadLibraryExW(reinterpret_cast<LPCWSTR>(name.data()),nullptr,flags);const auto error=::GetLastError();
        if(handle)owned.push_back(handle);::SetLastError(error);return reinterpret_cast<r::Module>(handle);
    }
    std::uint32_t last_error()override{return ::GetLastError();}
    bool free_library(r::Module value)override{
        auto handle=reinterpret_cast<HMODULE>(value);const auto it=std::find(owned.begin(),owned.end(),handle);check(it!=owned.end());
        const auto result=::FreeLibrary(handle);const auto error=::GetLastError();owned.erase(it);::SetLastError(error);return result!=0;
    }
    r::Address get_proc_address(r::Module module,std::string_view name)override{
        check(name=="LCMapStringEx"||name=="LocaleNameToLCID");++lookups;
        if((name=="LCMapStringEx"&&no_ex)||(name=="LocaleNameToLCID"&&no_locale)){::SetLastError(ERROR_PROC_NOT_FOUND);return 0;}
        return reinterpret_cast<r::Address>(::GetProcAddress(reinterpret_cast<HMODULE>(module),name.data()));
    }
    std::uint32_t process_parameters_flags()override{throw std::logic_error("NLS fixture has no PEB policy");}
    std::int32_t call_ro_initialize(r::Address,std::uint32_t)override{throw std::logic_error("NLS fixture has no RO call");}
    void call_ro_uninitialize(r::Address)override{throw std::logic_error("NLS fixture has no RO call");}
    std::uint32_t call_thread_initialization_policy(r::Address,std::uintptr_t,std::int32_t*)override{throw std::logic_error("NLS fixture has no thread policy");}
};
struct NativeHarness:Harness {
    Loader loader;nn::Calls api;dn::Calls native_calls;Context context{native};np::Backend backend{api,context};
    nd::Effects actual_dispatch{backend,native_calls,runtime,loader,lower};
    nb::Effects actual_nls{{lower,locale,heap,heap_fx,actual_dispatch}};
    mr::RecordNative actual_record{lower,mb_globals,record_ids,actual_nls};
    mb::Effects actual_multibyte{{globals,mb_globals,lower,ptd_state,ptd_fx,heap,heap_fx,errors,error_fx,invalid,thread,locale,resources,actual_record,mb_ids}};
    NativeHarness(){actual_record.bind_system_cp(actual_multibyte);}
    ~NativeHarness(){check(native.frames==0&&native.scratch.empty()&&lower.input_borrows.empty()&&memory.blocks.empty()&&heap_fx.reported.empty());}
};
void real_nls_chain(bool fallback){
    NativeHarness f;f.loader.no_ex=fallback;f.loader.no_locale=fallback;
    const std::u16string name=u"en-US";const auto locale=reinterpret_cast<std::uint64_t>(name.data());f.lower.borrowed[locale]=(name.size()+1)*2;
    std::array<std::uint8_t,3> utf8{'A',0xc3,0x84};std::array<nb::WordOutput,8> types{};
    check(f.actual_nls.get_string_type_06e210(0,CT_CTYPE1,utf8,types,65001,1)!=0);
    check((types[0].get()&C1_UPPER)!=0&&(types[1].get()&C1_UPPER)!=0&&!types[2].defined());
    std::array<nb::ByteOutput,16> lower{};const auto count=f.actual_nls.map_string_06cc30(0,locale,LCMAP_LOWERCASE,utf8,lower,65001,1);
    check(count==3&&lower[0].get()=='a'&&lower[1].get()==0xc3&&lower[2].get()==0xa4&&!lower[3].defined());
    const auto lookup_count=f.loader.lookups;
    check(f.actual_nls.map_string_06cc30(0,locale,LCMAP_UPPERCASE,utf8,lower,65001,1)==3&&f.loader.lookups==lookup_count);
    const auto cached=r::decode_pointer(f.runtime.procedures[0x11].load(),f.runtime.cookie);
    check(fallback?cached==r::missing:cached!=0&&cached!=r::missing);
    if(fallback)check(f.actual_dispatch.locale_name_to_lcid_0672c0(locale,0xffffffff)==0x409);
    std::array<nb::ByteOutput,64> key{};const auto key_size=f.actual_nls.map_string_06cc30(0,locale,LCMAP_SORTKEY,utf8,key,65001,0);
    check(key_size>0&&key_size<key.size()&&key[key_size-1].get()==0&&!key[key_size].defined());
    std::array<std::uint8_t,1> invalid{0xff};std::array<nb::WordOutput,1> failed{};
    check(f.actual_nls.get_string_type_06e210(0,CT_CTYPE1,invalid,failed,65001,1)==0&&!failed[0].defined());
    check((f.lower.read_u32(f.ptd_address()+0x3a8)&2)==0&&f.native.frames==0&&f.native.scratch.empty());
}
void real_allocation_edges(){
    NativeHarness f;for(unsigned size:{504U,505U}){
        std::vector<std::uint8_t> input(size,'A');std::vector<nb::WordOutput> type(size);std::vector<nb::ByteOutput> mapped(size);
        const auto before=f.heap_fx.allocations;
        check(f.actual_nls.get_string_type_06e210(0,CT_CTYPE1,input,type,65001,0)!=0&&f.heap_fx.allocations==before+(size==505?1:0));
        for(auto& cell:type)check((cell.get()&C1_UPPER)!=0);
        const auto after=f.heap_fx.allocations;
        check(f.actual_nls.map_string_06cc30(0,0,LCMAP_LOWERCASE,input,mapped,65001,0)==size&&f.heap_fx.allocations==after+(size==505?2:0));
        for(auto& cell:mapped)check(cell.get()=='a');
        check(f.memory.blocks.empty()&&f.heap_fx.reported.empty());
    }
    std::vector<std::uint8_t> input(505,'A');std::vector<nb::WordOutput> out(505);f.heap_fx.deny=true;
    check(f.actual_nls.get_string_type_06e210(0,CT_CTYPE1,input,out,65001,0)==0&&f.errors.errno_fallback==12&&!out[0].defined());
}
void real_record_builder(){
    NativeHarness f;std::array<std::byte,0x228> record{};f.borrow(record);const auto address=reinterpret_cast<std::uint64_t>(record.data());
    check(f.actual_record.build_multibyte_06f674(932,address)==0&&f.lower.read_u32(address+4)==932&&f.lower.read_u32(address+8)==1);
    check(f.lower.read_u8(address+0x119+'A')=='a'&&f.lower.read_u8(address+0x119+'a')=='A'&&(f.lower.read_u8(address+0x19+0x81)&4)!=0);
    const auto retained=f.lower.read_u8(address+0x19+0x81);check(f.actual_record.build_multibyte_06f674(65001,address)==0&&f.lower.read_u32(address+8)==0&&f.lower.read_u8(address+0x19+0x81)==retained);
    check(f.actual_record.build_multibyte_06f674(0,address)==0&&f.lower.read_u32(address+4)==0);
    check(f.backend.get_acp()==::GetACP()&&f.backend.get_oemcp()==::GetOEMCP()&&f.backend.is_valid_code_page(65001)!=0);
}
}
int main(){try{real_nls_chain(false);real_nls_chain(true);real_allocation_edges();real_record_builder();std::cout<<"PASS "<<f::checks<<" actual native-NLS/UTF8/record/source integration checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}