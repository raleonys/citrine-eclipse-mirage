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
struct Lower final:cb::MemoryEffects,ca::Effects,ca::FileApiEffects,r::Effects {
    using Address=std::uint64_t;
    f::Memory& memory;std::map<Address,std::size_t> borrowed;
    std::uint32_t ansi=0x100,expected_cp=65001,error=87;
    unsigned file_calls=0,loads=0,lookups=0,mb_calls=0,wide_calls=0;
    bool export_present=true,fail_query=false,fail_fill=false;
    explicit Lower(f::Memory& m):memory(m){}
    std::byte* resolve(Address address,std::uint64_t bytes){
        for(const auto& [p,size]:memory.blocks){const auto begin=reinterpret_cast<Address>(p);if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return static_cast<std::byte*>(p)+static_cast<std::size_t>(address-begin);}
        for(const auto& [begin,size]:borrowed)if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return reinterpret_cast<std::byte*>(begin)+static_cast<std::size_t>(address-begin);
        throw std::out_of_range("bounded conversion fixture storage");
    }
    std::uint8_t read_u8(Address a)override{return std::to_integer<std::uint8_t>(*resolve(a,1));}
    std::uint16_t read_u16(Address a)override{std::uint16_t value;std::memcpy(&value,resolve(a,2),2);return value;}
    void write_zero_u8(Address a)override{*resolve(a,1)=std::byte{0};}
    void write_zero_u16(Address a)override{std::memset(resolve(a,2),0,2);}
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
struct Effects final:cb::WildcardEffects {
    using Address=wr::Address;
    f::Memory& memory;Heap& heap;Lower& lower;av::Effects* argv=nullptr;
    std::map<Address,std::size_t>& borrowed;
    std::map<std::string,std::vector<std::u16string>> names{
        {"*.txt",{u".",u"..",u"z.txt",u"A.txt",u"m.txt",u"b.txt",u"Q.txt",u"w.txt",u"c.txt",u"L.txt",u"a.txt",u"D.txt",u"n.txt",u"B.txt"}},
        {"dir\\?.dat",{u"z.dat",u"a.dat"}}, {"*.onlydots",{u".",u".."}}
    };
    struct Session{const std::vector<std::u16string>* files;std::size_t index;};
    std::map<Address,Session> handles;Address serial=0x700000;
    unsigned updates=0,clears=0,sort_writes=0,closed=0,copies=0,null_patterns=0;
    std::uint32_t flags=0xa5a5a5a7,locale_cp=65001;
    bool deny_final=false,fail_final_copy=false,has_final=false;
    Effects(f::Memory& m,Heap& h,rb::Allocator& allocator,cv::Effects& conversion,r::State& runtime,Lower& l):WildcardEffects(allocator,conversion,runtime,l,l),memory(m),heap(h),lower(l),borrowed(l.borrowed){for(const auto& [key,files]:names){(void)key;for(const auto& file:files)borrowed[reinterpret_cast<Address>(file.c_str())]=(file.size()+1)*2;}}
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
        if(address>=0x3019&&address<0x3119)return address==0x3019+0x81?4:0;
        return std::to_integer<std::uint8_t>(*resolve(address,1));
    }
    void write_u8(Address address,std::uint8_t value)override{++sort_writes;*resolve(address,1)=static_cast<std::byte>(value);}
    er::Cell32 errno_cell_05d724()override{return argv->errno_05d724();}
    void invalid_parameter_05c954()override{argv->invalid_parameter_05c954();}
    bc::LocaleContext locale_update_058b04(Address locale)override{check(locale==0||locale==0x55);++updates;flags|=2;return{0x1000,0x2000,0x3000,1};}
    std::uint16_t read_u16(Address address)override{std::uint16_t value;std::memcpy(&value,resolve(address,2),2);return value;}
    std::uint32_t read_u32(Address address)override{if(address==0x13a8)return flags;if(address==0x200c)return locale_cp;check(address==0x3008);return 1;}
    Address read_address(Address address)override{return read_word(address);}
    void write_u32(Address address,std::uint32_t value)override{check(address==0x13a8);flags=value;++clears;}
    std::string narrow(Address address){const auto size=length(address);return{reinterpret_cast<char*>(resolve(address,size+1)),static_cast<std::size_t>(size)};}
    std::u16string wide(Address address){std::u16string result;for(;;address+=2){const auto c=read_u16(address);if(!c)return result;result.push_back(static_cast<char16_t>(c));}}
    Address find_first(Address pattern,Address& filename)override{
        if(pattern==0){++null_patterns;return std::numeric_limits<Address>::max();}const auto text=wide(pattern);const std::string key(text.begin(),text.end());auto item=names.find(key);
        if(item==names.end())return std::numeric_limits<Address>::max();
        const auto handle=++serial;handles.emplace(handle,Session{&item->second,0});filename=reinterpret_cast<Address>(item->second[0].c_str());return handle;
    }
    std::int32_t find_next(Address handle,Address& filename)override{auto& session=handles.at(handle);if(++session.index==session.files->size())return 0;filename=reinterpret_cast<Address>((*session.files)[session.index].c_str());return -1;}
    void find_close(Address handle)override{check(handles.erase(handle)==1);++closed;}
    Address allocate_buffer_063888(std::uint64_t pointers,std::uint64_t characters,std::uint64_t element)override{
        const auto previous=heap.deny;heap.deny=deny_final;auto allocation=av::allocate_buffer_063888(pointers,characters,element,*argv);heap.deny=previous;
        if(!allocation.base)return 0;
        (void)::new(allocation.base) char*[static_cast<std::size_t>(pointers)]{};
        (void)::new(static_cast<void*>(allocation.storage.data()+pointers*8)) char[static_cast<std::size_t>(characters)]{};
        has_final=true;return reinterpret_cast<Address>(allocation.base);
    }
};
struct Policy final:wr::ExpansionPolicy {
    unsigned initialized=0;
    explicit Policy(wr::Effects& fx):ExpansionPolicy(fx){}
    void initialize_multibyte_06f5f8()override{++initialized;}
    std::uint32_t module_filename_06ecdc(std::span<char,260> out)override{constexpr char value[]="fallback.exe";std::copy(std::begin(value),std::end(value),out.begin());return 12;}
};
struct Harness {
    er::State errors;Errors error_fx;f::Memory memory;Heap heap_fx{errors,error_fx,memory};h::State heap{0xaaa,0,0,0};Invalid invalid;
    rb::Allocator allocator{heap,heap_fx};Lower lower{memory};r::State runtime{0x1234};cb::BufferEffects conversion{heap,heap_fx,errors,error_fx,lower,lower};Effects effects{memory,heap_fx,allocator,conversion,runtime,lower};Policy policy{effects};ag::ArgvEffects argv_fx{heap,heap_fx,errors,error_fx,invalid,policy};bc::ParserEffects parser{effects};
    Harness(){effects.argv=&argv_fx;}
};
std::vector<std::string> values(const av::State& state){std::vector<std::string> result;for(int i=0;i<state.argc;++i)result.emplace_back(state.argv[i]);check(state.argv[state.argc]==nullptr);return result;}
void integrated(unsigned variant){
    Harness f;f.heap_fx.inplace=variant==1;f.effects.locale_cp=variant?1252:65001;f.lower.ansi=variant==2?0:0x100;f.lower.expected_cp=variant?variant==2?1:0:65001;
    av::State state;const std::string command="app.exe *.txt dir\\?.dat no-match*.bin";state.command_line={command.c_str(),command.size()+1};
    check(av::configure_0638e8(state,1,f.argv_fx,f.parser)==0);auto* original=state.argv;
    f.effects.has_final=false;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0);
    const std::vector<std::string> expected{"app.exe","z.txt","A.txt","m.txt","b.txt","Q.txt","w.txt","c.txt","L.txt","a.txt","D.txt","n.txt","B.txt","dir\\z.dat","dir\\a.dat","no-match*.bin"};
    check(values(state)==expected);check(f.effects.sort_writes==0&&f.effects.closed==2&&f.effects.handles.empty());
    check(f.effects.updates==f.effects.clears&&(f.effects.flags&2)==0);check(variant?f.lower.file_calls>0:f.lower.file_calls==0);
    check(f.lower.mb_calls>0&&f.lower.wide_calls>0);check(variant?(f.lower.loads==1&&f.lower.lookups==1):f.lower.loads==0);f.argv_fx.free_0668c0(original);check(f.memory.blocks.size()==1);
    auto* published=state.argv;const auto argc=state.argc;
    f.effects.has_final=false;f.heap_fx.deny_realloc=true;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==12);
    check(state.argv==published&&state.argc==argc&&f.memory.blocks.size()==1&&f.effects.handles.empty());f.heap_fx.deny_realloc=false;
    f.effects.has_final=false;f.effects.deny_final=true;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0xffffffffU);
    check(state.argv==published&&state.argc==argc&&f.memory.blocks.size()==1&&f.effects.handles.empty());f.effects.deny_final=false;
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
        check(wr::previous_locale_077170(address,address+leads+1,0x55,f.effects)==address+expected);
        check(wr::previous_077168(address,address,f.effects)==0);f.effects.free_0668c0(address);
    }
    const auto updates=f.effects.updates;check(wr::previous_077168(0,10,f.effects)==0&&f.errors.errno_fallback==22&&f.invalid.handled==1&&f.effects.updates==updates);
    check(wr::expand_06e5bc(0,nullptr,f.effects)==22&&f.invalid.handled==2);
    const auto array=f.effects.calloc_066848(1,8);wr::Result result;
    f.effects.has_final=false;check(wr::expand_06e5bc(array,&result,f.effects)==0&&result.pointer_slots==1&&f.effects.read_word(result.base)==0);
    f.effects.free_0668c0(result.base);f.effects.free_0668c0(array);check(f.memory.blocks.empty());
    result={99,99};bool caught=false;try{(void)wr::expand_06e5bc(0,&result,f.effects);}catch(const std::out_of_range&){caught=true;}
    check(caught&&result.base==0&&result.pointer_slots==0&&f.memory.blocks.empty());
}
void copy_failure(){
    Harness f;av::State state;const std::string command="literal";state.command_line={command.c_str(),command.size()+1};
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
    Harness f;av::State state;const std::string command="app *.txt";state.command_line={command.c_str(),command.size()+1};
    f.lower.fail_fill=true;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0);
    check(values(state)==std::vector<std::string>{"app","*.txt"}&&f.effects.null_patterns==1&&f.effects.handles.empty()&&f.memory.blocks.size()==1);
    check(f.conversion.errno_cell_05d724().load()==22);
    ag::CleanupState cleanup{state};check(ag::cleanup_064538(cleanup,f.argv_fx)&&f.memory.blocks.empty());
}
void missing_file_api(){
    Harness f;f.effects.locale_cp=1252;f.lower.expected_cp=0;f.lower.export_present=false;
    av::State state;const std::string command="app *.txt";state.command_line={command.c_str(),command.size()+1};
    check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0&&state.argc==13);
    check(f.lower.loads==1&&f.lower.lookups==1&&f.lower.file_calls==0);
    check(r::decode_pointer(f.runtime.procedures[0].load(),f.runtime.cookie)==r::missing);
    ag::CleanupState cleanup{state};check(ag::cleanup_064538(cleanup,f.argv_fx)&&f.memory.blocks.empty());
}
int main(){try{for(unsigned variant=0;variant<3;++variant)integrated(variant);previous_and_bounds();copy_failure();for(bool inplace:{false,true}){shared_heap_onexit(inplace);capacity_and_failure(inplace);}failed_onexit_growth();conversion_errors_and_reuse(false);conversion_errors_and_reuse(true);failed_pattern_conversion();missing_file_api();std::cout<<"PASS "<<f::checks<<" wildcard/argv/onexit/conversion/API/resolver/heap/error composition checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}


