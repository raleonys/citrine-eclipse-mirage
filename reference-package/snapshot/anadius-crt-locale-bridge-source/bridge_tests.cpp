#include "bridge.hpp"
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
struct Lower final:cb::MemoryEffects,ca::Effects,ca::FileApiEffects,r::Effects,lb::Memory {
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
    bool export_present=true,fail_query=false,fail_fill=false;
    explicit Lower(f::Memory& m):memory(m){constexpr char16_t path[]=u"fallback.exe";std::copy(std::begin(path),std::end(path),module_path.begin());borrowed[reinterpret_cast<Address>(module_path.data())]=sizeof(module_path);}
    std::u16string wide(Address a){std::u16string result;for(;;a+=2){const auto value=read_u16(a);if(!value)return result;result.push_back(static_cast<char16_t>(value));}}
    std::byte* resolve(Address address,std::uint64_t bytes){
        for(const auto& [p,size]:memory.blocks){const auto begin=reinterpret_cast<Address>(p);if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return static_cast<std::byte*>(p)+static_cast<std::size_t>(address-begin);}
        for(const auto& [begin,size]:borrowed)if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return reinterpret_cast<std::byte*>(begin)+static_cast<std::size_t>(address-begin);
        throw std::out_of_range("bounded conversion fixture storage");
    }
    std::uint8_t read_u8(Address a)override{if(native_find&&native_find->contains(a,1))return native_find->read_u8(a);return std::to_integer<std::uint8_t>(*resolve(a,1));}
    std::uint16_t read_u16(Address a)override{if(native_find&&native_find->contains(a,2))return native_find->read_u16(a);std::uint16_t value;std::memcpy(&value,resolve(a,2),2);return value;}
    void write_zero_u8(Address a)override{*resolve(a,1)=std::byte{0};}
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
    r::Module load_library_ex_w(std::u16string_view name,r::Module file,std::uint32_t flags)override{check(name==u"kernel32"&&file==0&&flags==0x800);++loads;return 0x4000;}
    bool free_library(r::Module)override{throw std::runtime_error("unexpected fixture module free");}
    r::Address get_proc_address(r::Module module,std::string_view name)override{check(module==0x4000&&name=="AreFileApisANSI");++lookups;return export_present?0x5000:0;}
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
struct Resources final:pd::Effects {
    Lower& memory;lb::Globals& globals;unsigned locks=0,unlocks=0,adds=0,releases=0,frees=0;std::array<unsigned,6> depths{};
    Resources(Lower& m,lb::Globals& g):memory(m),globals(g){}
    void lock_0625f8(std::uint32_t id)override{check(id==4||id==5);++depths[id];++locks;}
    void unlock_06264c(std::uint32_t id)override{check(id<depths.size()&&depths[id]>0);--depths[id];++unlocks;}
    std::uint32_t atomic_fetch_add_32(pd::Address a,std::uint32_t increment)override{check(depths[5]>0);const auto old=memory.read_u32(a);memory.write_u32(a,old+increment);return old;}
    void free_0668c0(pd::Address)override{throw std::runtime_error("thread bridge must use actual heap free");}
    pd::Address current_locale_0a52e0()override{return globals.locale_0a52e0;}
    void release_locale_ref_0708c0(pd::Address a)override{check(depths[4]>0);++releases;memory.write_u32(a+0x10,memory.read_u32(a+0x10)-1);}
    std::uint32_t locale_ref_count_10(pd::Address a)override{return memory.read_u32(a+0x10);}
    void free_locale_0706c0(pd::Address)override{++frees;}
    void add_locale_ref_070634(pd::Address a)override{check(depths[4]>0);++adds;memory.write_u32(a+0x10,memory.read_u32(a+0x10)+1);}
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
struct Policy final:fb::Policy {
    unsigned initialized=0;
    Policy(wr::Effects& fx,mp::Effects& path):fb::Policy(fx,path){}
    void initialize_multibyte_06f5f8()override{++initialized;}

};
struct Harness {
    er::State errors;Errors error_fx;f::Memory memory;Heap heap_fx{errors,error_fx,memory};h::State heap{0xaaa,0,0,0};Invalid invalid;
    rb::Allocator allocator{heap,heap_fx};Lower lower{memory};nf::Session find{fixture_api()};r::State runtime{0x1234};
    std::array<std::byte,0x40> locale_a{},locale_b{};std::array<std::byte,0x120> mb_a{},mb_b{};p::Ptd ptd;p::State ptd_state{7};PtdFixture ptd_fx{ptd,lower};lb::Globals globals;
    Resources resources{lower,globals};lb::ThreadEffects thread{globals,lower,ptd_state,ptd_fx,resources,heap,heap_fx};lb::UpdateEffects locale{globals,lower,thread};
    cb::BufferEffects conversion{heap,heap_fx,errors,error_fx,lower,lower};Effects effects{memory,heap_fx,allocator,conversion,runtime,lower,find,locale};
    std::array<WCHAR,264> path_scratch{};fb::SdkPathProvider module_api{path_scratch,fixture_module};fb::PathEffects path{conversion,effects,module_api};Policy policy{effects,path};ag::ArgvEffects argv_fx{heap,heap_fx,errors,error_fx,invalid,policy};bc::ParserEffects parser{effects};
    std::uint64_t ptd_address(){return reinterpret_cast<std::uint64_t>(&ptd);}
    template<class T>void borrow(T& value){lower.borrowed[reinterpret_cast<std::uint64_t>(&value)]=sizeof(value);}
    void set_code_page(std::uint32_t cp){lower.write_u32(reinterpret_cast<std::uint64_t>(locale_a.data())+0xc,cp);lower.write_u32(reinterpret_cast<std::uint64_t>(locale_b.data())+0xc,cp);}
    Harness(){
        effects.argv=&argv_fx;lower.native_find=&find;active_lower=&lower;
        borrow(ptd);borrow(globals);borrow(locale_a);borrow(locale_b);borrow(mb_a);borrow(mb_b);borrow(path_scratch);
        globals.changed_0a52d8=1;globals.mask_0a1c6c=1;globals.pair_0a1938={reinterpret_cast<std::uint64_t>(locale_a.data()),reinterpret_cast<std::uint64_t>(mb_a.data())};
        globals.locale_0a52e0=globals.initial_locale_0a17e0=globals.pair_0a1938[0];globals.multibyte_0a59d0=globals.initial_multibyte_0a1c70=globals.pair_0a1938[1];
        lower.write_address(ptd_address()+0x90,globals.locale_0a52e0);lower.write_address(ptd_address()+0x88,globals.multibyte_0a59d0);lower.write_u32(ptd_address()+0x3a8,1);
        set_code_page(65001);effects.flags_address=ptd_address()+0x3a8;
        for(auto* block:{&mb_a,&mb_b}){const auto a=reinterpret_cast<std::uint64_t>(block->data());lower.write_u32(a,1);lower.write_u32(a+8,1);(*block)[0x19+0x81]=std::byte{4};}
        for(auto* block:{&locale_a,&locale_b})lower.write_u32(reinterpret_cast<std::uint64_t>(block->data())+0x10,1);
    }
};
std::vector<std::string> values(const av::State& state){std::vector<std::string> result;for(int i=0;i<state.argc;++i)result.emplace_back(state.argv[i]);check(state.argv[state.argc]==nullptr);return result;}
void integrated(unsigned variant){
    Harness f;f.heap_fx.inplace=variant==1;f.set_code_page(variant?1252:65001);f.lower.ansi=variant==2?0:0x100;f.lower.expected_cp=variant?variant==2?1:0:65001;
    av::State state;f.lower.borrowed[reinterpret_cast<std::uint64_t>(state.module_filename.data())]=state.module_filename.size();const std::string command="app.exe *.txt dir\\?.dat no-match*.bin";state.command_line={command.c_str(),command.size()+1};
    check(av::configure_0638e8(state,1,f.argv_fx,f.parser)==0);auto* original=state.argv;
    f.effects.has_final=false;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0);
    const std::vector<std::string> expected{"app.exe","z.txt","A.txt","m.txt","b.txt","Q.txt","w.txt","c.txt","L.txt","a.txt","D.txt","n.txt","B.txt","dir\\z.dat","dir\\a.dat","no-match*.bin"};
    check(values(state)==expected);check(f.effects.sort_writes==0&&f.lower.closed==2&&f.lower.handles.empty()&&f.find.active_count()==0);
    check(f.ptd_fx.gets==f.effects.clears&&(f.lower.read_u32(f.ptd_address()+0x3a8)&2)==0);check(variant?f.lower.file_calls>0:f.lower.file_calls==0);
    check(f.lower.mb_calls>0&&f.lower.wide_calls>0);check(variant?(f.lower.loads==1&&f.lower.lookups==1):f.lower.loads==0);f.argv_fx.free_0668c0(original);check(f.memory.blocks.size()==1);
    auto* published=state.argv;const auto argc=state.argc;
    f.effects.has_final=false;f.heap_fx.deny_realloc=true;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==12);
    check(state.argv==published&&state.argc==argc&&f.memory.blocks.size()==1&&f.lower.handles.empty());f.heap_fx.deny_realloc=false;
    f.effects.has_final=false;f.effects.deny_final=true;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0xffffffffU);
    check(state.argv==published&&state.argc==argc&&f.memory.blocks.size()==1&&f.lower.handles.empty());f.effects.deny_final=false;
    ag::CleanupState cleanup{state};check(ag::cleanup_064538(cleanup,f.argv_fx)&&f.memory.blocks.empty());
    const std::string dots="app *.onlydots";state.command_line={dots.c_str(),dots.size()+1};f.effects.has_final=false;
    check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0&&values(state)==std::vector<std::string>{"app"});
    check(ag::cleanup_064538(cleanup,f.argv_fx)&&f.memory.blocks.empty());
}
void previous_and_bounds(){
    Harness f;
    for(unsigned leads=0;leads<33;++leads){
        const auto address=f.effects.calloc_066848(leads+2,1);for(unsigned i=0;i<leads;++i)*f.effects.resolve(address+i,1)=std::byte{0x81};*f.effects.resolve(address+leads,1)=std::byte{'X'};
        std::uint64_t cursor=0,expected=0;while(cursor<leads+1){expected=cursor;cursor+=cursor<leads?2:1;}
        check(wr::previous_locale_077170(address,address+leads+1,reinterpret_cast<std::uint64_t>(f.globals.pair_0a1938.data()),f.effects)==address+expected);
        check(wr::previous_077168(address,address,f.effects)==0);f.effects.free_0668c0(address);
    }
    const auto updates=f.ptd_fx.gets;check(wr::previous_077168(0,10,f.effects)==0&&f.errors.errno_fallback==22&&f.invalid.handled==1&&f.ptd_fx.gets==updates);
    check(wr::expand_06e5bc(0,nullptr,f.effects)==22&&f.invalid.handled==2);
    const auto array=f.effects.calloc_066848(1,8);wr::Result result;
    f.effects.has_final=false;check(wr::expand_06e5bc(array,&result,f.effects)==0&&result.pointer_slots==1&&f.effects.read_word(result.base)==0);
    f.effects.free_0668c0(result.base);f.effects.free_0668c0(array);check(f.memory.blocks.empty());
    result={99,99};bool caught=false;try{(void)wr::expand_06e5bc(0,&result,f.effects);}catch(const std::out_of_range&){caught=true;}
    check(caught&&result.base==0&&result.pointer_slots==0&&f.memory.blocks.empty());
}
void copy_failure(){
    Harness f;av::State state;f.lower.borrowed[reinterpret_cast<std::uint64_t>(state.module_filename.data())]=state.module_filename.size();const std::string command="literal";state.command_line={command.c_str(),command.size()+1};
    check(av::configure_0638e8(state,1,f.argv_fx,f.parser)==0);
    wr::Result result{99,99};f.effects.has_final=false;f.effects.fail_final_copy=true;bool caught=false;
    try{(void)wr::expand_06e5bc(reinterpret_cast<wr::Address>(state.argv),&result,f.effects);}catch(const std::runtime_error& error){caught=std::string(error.what())=="synthetic copy Watson";}
    check(caught&&result.base==0&&result.pointer_slots==0&&f.memory.blocks.size()==4); // input + temporary string/vector + final allocation; no invented cleanup
}
}
using WildcardFixture=Effects;
struct ExitEffects final:rb::OnexitHeapEffects {
    WildcardFixture& memory;ox::State& state;unsigned depth=0,maximum_depth=0;
    std::vector<std::uint64_t> callbacks;bool recursive=true;
    ExitEffects(rb::Allocator& allocator,WildcardFixture& m,ox::State& s):OnexitHeapEffects(allocator),memory(m),state(s){}
    void lock_0625f8(std::uint32_t index)override{check(index==2);maximum_depth=std::max(maximum_depth,++depth);}
    void unlock_06264c(std::uint32_t index)override{check(index==2&&depth>0);--depth;}
    std::uint64_t read_word(std::uint64_t address)override{return memory.read_word(address);}
    void write_word(std::uint64_t address,std::uint64_t value)override{memory.write_word(address,value);}
    void invoke_callback(std::uint64_t address)override{
        callbacks.push_back(address);
        if(recursive&&address==65){check(ox::atexit_04b770(1000,state,*this)==0);recursive=false;}
    }
};
void shared_heap_onexit(bool inplace){
    Harness f;f.heap_fx.inplace=inplace;
    ox::State state{0x1234,{},{}};
    check(ox::initialize_onexit_table_0643f8(&state.application_0a47a8,state)==0);
    ExitEffects exit(f.allocator,f.effects,state);
    for(unsigned i=1;i<=65;++i)check(ox::atexit_04b770(i,state,exit)==0);
    check(f.heap_fx.reallocations==2&&f.heap_fx.sizes==2&&f.heap_fx.zeroes==3);
    // Wildcard and onexit allocations coexist in the exact same state/backend.
    const auto extra=f.effects.calloc_066848(8,1);check(extra!=0&&f.heap_fx.reported.size()==2);
    check(ox::execute_onexit_table_0643b4(&state.application_0a47a8,state,exit)==0);
    std::vector<std::uint64_t> expected{65,1000};for(unsigned i=64;i!=0;--i)expected.push_back(i);
    check(exit.callbacks==expected&&exit.maximum_depth==2&&exit.depth==0);
    check(f.heap_fx.reported.size()==1&&state.application_0a47a8.begin==state.cookie_0a1580);
    f.effects.free_0668c0(extra);check(f.memory.blocks.empty()&&f.heap_fx.reported.empty());
}
void failed_onexit_growth(){
    Harness f;ox::State state{0x1234,{},{}};ox::initialize_onexit_table_0643f8(&state.application_0a47a8,state);
    ExitEffects exit(f.allocator,f.effects,state);exit.recursive=false;
    for(unsigned i=1;i<=32;++i)check(ox::atexit_04b770(i,state,exit)==0);
    const auto before=state.application_0a47a8;const auto calls=f.heap_fx.reallocations;
    f.heap_fx.deny_realloc=true;f.errors.errno_fallback=0;
    check(ox::atexit_04b770(33,state,exit)==-1&&f.errors.errno_fallback==12);
    check(state.application_0a47a8.begin==before.begin&&state.application_0a47a8.end==before.end&&state.application_0a47a8.capacity_end==before.capacity_end);
    check(f.heap_fx.reallocations==calls+2&&f.heap_fx.reported.size()==1); // main growth + fallback
    check(ox::execute_onexit_table_0643b4(&state.application_0a47a8,state,exit)==0);
    check(exit.callbacks.size()==32&&exit.callbacks.front()==32&&exit.callbacks.back()==1&&f.memory.blocks.empty());
}
void capacity_and_failure(bool inplace){
    Harness f;f.heap_fx.inplace=inplace;f.heap_fx.padding=true;
    auto old=f.allocator.calloc(32,1);auto* bytes=f.effects.resolve(old,40);
    std::memset(bytes,0x55,32);check(bytes[32]==std::byte{0xcc});
    const auto grown=f.allocator.recalloc(old,64,1);check(grown!=0&&(inplace?(grown==old):(grown!=old)));
    bytes=f.effects.resolve(grown,64);
    for(unsigned i=0;i<64;++i)check(bytes[i]==(i<32?std::byte{0x55}:i<40?std::byte{0xcc}:std::byte{0}));
    f.heap_fx.deny_realloc=true;const auto zeroes=f.heap_fx.zeroes;
    check(f.allocator.recalloc(grown,128,1)==0&&f.heap_fx.zeroes==zeroes&&f.heap_fx.reported.contains(reinterpret_cast<void*>(grown)));
    check(bytes[0]==std::byte{0x55}&&bytes[63]==std::byte{0});
    check(f.allocator.recalloc(grown,0,8)==0&&f.memory.blocks.empty());
    const auto allocations=f.heap_fx.allocations;const auto sizes=f.heap_fx.sizes;
    check(f.allocator.recalloc(0,~std::uint64_t{0},2)==0&&f.heap_fx.allocations==allocations&&f.heap_fx.sizes==sizes);
}
void conversion_errors_and_reuse(bool ptd){
    Harness f;std::array<std::byte,0x28> storage{};if(ptd)f.error_fx.ptd=storage.data();
    const std::string source="fixture";const auto input=reinterpret_cast<std::uint64_t>(source.c_str());f.lower.borrowed[input]=source.size()+1;
    cv::Buffer buffer;const auto allocations=f.heap_fx.allocations;
    f.lower.fail_query=true;check(cv::to_wide_06004c(input,buffer,65001,f.conversion)==22);
    check(buffer.data==0&&f.heap_fx.allocations==allocations);
    check(f.conversion.errno_cell_05d724().load()==22&&er::dos_error_cell_05d704(f.errors,f.error_fx).load()==87);
    if(ptd)check(f.errors.errno_fallback==12&&f.errors.dos_fallback==8);
    f.lower.fail_query=false;f.lower.fail_fill=true;
    auto converted=f.effects.to_wide_06004c(input,65001);
    check(converted.status==22&&converted.owned&&converted.data!=0);f.effects.free_0668c0(converted.data);
    f.lower.fail_fill=false;f.heap_fx.deny=true;
    converted=f.effects.to_wide_06004c(input,65001);
    check(converted.status==12&&!converted.owned&&converted.data==0&&f.conversion.errno_cell_05d724().load()==12);f.heap_fx.deny=false;
    check(cv::to_wide_06004c(input,buffer,65001,f.conversion)==0&&buffer.length==source.size()&&buffer.capacity==source.size()+1);
    const auto first=buffer.data;const auto allocated=f.heap_fx.allocations;
    check(cv::to_wide_06004c(input,buffer,65001,f.conversion)==0&&buffer.data==first&&f.heap_fx.allocations==allocated);
    check(cv::to_wide_06004c(0,buffer,65001,f.conversion)==0&&buffer.data==0&&buffer.capacity==0&&buffer.owned==0&&f.memory.blocks.empty());
}
void failed_pattern_conversion(){
    Harness f;av::State state;f.lower.borrowed[reinterpret_cast<std::uint64_t>(state.module_filename.data())]=state.module_filename.size();const std::string command="app *.txt";state.command_line={command.c_str(),command.size()+1};
    f.lower.fail_fill=true;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0);
    check(values(state)==std::vector<std::string>{"app","*.txt"}&&f.lower.null_patterns==1&&f.lower.handles.empty()&&f.memory.blocks.size()==1);
    check(f.conversion.errno_cell_05d724().load()==22);
    ag::CleanupState cleanup{state};check(ag::cleanup_064538(cleanup,f.argv_fx)&&f.memory.blocks.empty());
}
void missing_file_api(){
    Harness f;f.set_code_page(1252);f.lower.expected_cp=0;f.lower.export_present=false;
    av::State state;f.lower.borrowed[reinterpret_cast<std::uint64_t>(state.module_filename.data())]=state.module_filename.size();const std::string command="app *.txt";state.command_line={command.c_str(),command.size()+1};
    check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0&&state.argc==13);
    check(f.lower.loads==1&&f.lower.lookups==1&&f.lower.file_calls==0);
    check(r::decode_pointer(f.runtime.procedures[0].load(),f.runtime.cookie)==r::missing);
    ag::CleanupState cleanup{state};check(ag::cleanup_064538(cleanup,f.argv_fx)&&f.memory.blocks.empty());
}
void module_fallback(){
    Harness f;av::State state;f.lower.borrowed[reinterpret_cast<std::uint64_t>(state.module_filename.data())]=state.module_filename.size();
    check(av::configure_0638e8(state,1,f.argv_fx,f.parser)==0&&values(state)==std::vector<std::string>{"fallback.exe"});
    check(f.lower.module_calls==1&&f.ptd_fx.gets==f.effects.clears&&state.program_name==state.module_filename.data());
    ag::CleanupState cleanup{state};check(ag::cleanup_064538(cleanup,f.argv_fx)&&f.memory.blocks.empty());
    const auto updates=f.ptd_fx.gets;f.lower.module_fail=true;f.lower.error=87;const auto old=state.module_filename;
    check(f.policy.module_filename_06ecdc(state.module_filename)==0&&state.module_filename==old&&f.errors.errno_fallback==22&&f.errors.dos_fallback==87&&f.ptd_fx.gets==updates);
    f.lower.module_fail=false;std::fill(f.lower.module_path.begin(),f.lower.module_path.end(),u'X');f.lower.module_path[260]=0;state.module_filename.fill('Q');
    check(f.policy.module_filename_06ecdc(state.module_filename)==0&&f.errors.errno_fallback==34&&std::all_of(state.module_filename.begin(),state.module_filename.end(),[](char c){return c=='Q';}));
}
void locale_transitions(){
    Harness f;f.lower.write_u32(f.ptd_address()+0x3a8,0);
    const auto old_locale=f.globals.locale_0a52e0,old_mb=f.globals.multibyte_0a59d0;
    f.globals.locale_0a52e0=reinterpret_cast<std::uint64_t>(f.locale_b.data());f.globals.multibyte_0a59d0=reinterpret_cast<std::uint64_t>(f.mb_b.data());
    const auto context=f.effects.locale_update_058b04(0);
    check(context.ptd==f.ptd_address()&&context.locale_info==f.globals.locale_0a52e0&&context.multibyte_info==f.globals.multibyte_0a59d0&&context.clear_thread_flag==1);
    check(f.ptd_fx.gets==3&&f.resources.locks==2&&f.resources.unlocks==2&&f.resources.adds==1&&f.resources.releases==1&&f.resources.frees==0);
    check(f.lower.read_u32(old_locale+0x10)==0&&f.lower.read_u32(old_mb)==0&&f.lower.read_u32(context.locale_info+0x10)==2&&f.lower.read_u32(context.multibyte_info)==2);
    check(f.lower.read_address(f.ptd_address()+0x90)==context.locale_info&&f.lower.read_address(f.ptd_address()+0x88)==context.multibyte_info);
    f.effects.write_u32(f.ptd_address()+0x3a8,1);const auto locks=f.resources.locks;
    f.globals.locale_0a52e0=old_locale;f.globals.multibyte_0a59d0=old_mb;
    check(wr::select_code_page_region(f.effects)==65001&&f.resources.locks==locks&&f.lower.read_address(f.ptd_address()+0x90)==context.locale_info&&f.lower.read_u32(f.ptd_address()+0x3a8)==1);
}
void nested_and_fast_locale(){
    Harness f;f.lower.error=0x8877;
    const auto outer=f.effects.locale_update_058b04(0);check(outer.clear_thread_flag==1&&f.lower.error==0x8877);
    const auto nested=f.effects.locale_update_058b04(0);check(nested.clear_thread_flag==0&&f.lower.read_u32(f.ptd_address()+0x3a8)==3);
    check(bc::is_lead_06f9a8(0x81,f.effects)==1&&f.effects.clears==0&&f.lower.read_u32(f.ptd_address()+0x3a8)==3);
    f.effects.write_u32(f.ptd_address()+0x3a8,f.lower.read_u32(f.ptd_address()+0x3a8)&~2U);check(f.lower.read_u32(f.ptd_address()+0x3a8)==1);
    f.globals.changed_0a52d8=0;const auto gets=f.ptd_fx.gets;auto context=f.effects.locale_update_058b04(0);
    check(context.ptd==0&&context.clear_thread_flag==0&&context.locale_info==f.globals.pair_0a1938[0]&&f.ptd_fx.gets==gets);
    std::array<std::uint64_t,2> pair{reinterpret_cast<std::uint64_t>(f.locale_b.data()),reinterpret_cast<std::uint64_t>(f.mb_b.data())};f.borrow(pair);
    context=f.effects.locale_update_058b04(reinterpret_cast<std::uint64_t>(pair.data()));check(context.locale_info==pair[0]&&context.multibyte_info==pair[1]&&context.clear_thread_flag==0&&f.ptd_fx.gets==gets);
    f.globals.changed_0a52d8=1;f.ptd_fx.missing=true;bool caught=false;
    try{(void)f.effects.locale_update_058b04(0);}catch(const std::runtime_error& e){caught=std::string(e.what())=="synthetic required PTD abort";}
    check(caught&&f.lower.error==0x8877&&f.resources.locks==0);
}
void release_multibyte_through_heap(){
    Harness f;f.lower.write_u32(f.ptd_address()+0x3a8,0);const auto old=f.allocator.calloc(0x120,1);f.lower.write_u32(old,1);f.lower.write_address(f.ptd_address()+0x88,old);
    f.globals.multibyte_0a59d0=reinterpret_cast<std::uint64_t>(f.mb_b.data());
    check(lt::update_thread_multibyte_06f658(f.thread)==f.globals.multibyte_0a59d0&&f.memory.blocks.empty()&&f.heap_fx.reported.empty());
    check(f.resources.locks==1&&f.resources.unlocks==1&&f.lower.read_u32(f.globals.multibyte_0a59d0)==2);
}
int main(){try{for(unsigned variant=0;variant<3;++variant)integrated(variant);previous_and_bounds();copy_failure();for(bool inplace:{false,true}){shared_heap_onexit(inplace);capacity_and_failure(inplace);}failed_onexit_growth();conversion_errors_and_reuse(false);conversion_errors_and_reuse(true);failed_pattern_conversion();missing_file_api();module_fallback();locale_transitions();nested_and_fast_locale();release_multibyte_through_heap();std::cout<<"PASS "<<f::checks<<" locale/PTD/argv/find/path/conversion/heap composition checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}




