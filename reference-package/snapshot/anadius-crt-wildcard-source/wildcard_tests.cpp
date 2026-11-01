#include "wildcard.hpp"
#include "../anadius-crt-argv-bridge-source/test_fixtures.hpp"
#include <limits>
namespace wr=anadius::crt_wildcard_recovered;
namespace ws=anadius::crt_wildcard_storage;
namespace sr=anadius::crt_sort_recovered;
namespace at=anadius::crt_argv_test;
namespace {
using f::check;
struct Errors final:er::Effects {void* optional_ptd_064d08()override{return nullptr;}};
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
struct Effects final:wr::SupportEffects {
    using Address=wr::Address;
    f::Memory& memory;at::Heap& heap;av::Effects* argv=nullptr;
    std::map<Address,std::size_t> borrowed;
    std::map<std::string,std::vector<std::u16string>> names{
        {"*.txt",{u".",u"..",u"z.txt",u"A.txt",u"m.txt",u"b.txt",u"Q.txt",u"w.txt",u"c.txt",u"L.txt",u"a.txt",u"D.txt",u"n.txt",u"B.txt"}},
        {"dir\\?.dat",{u"z.dat",u"a.dat"}}, {"*.onlydots",{u".",u".."}}
    };
    struct Session{const std::vector<std::u16string>* files;std::size_t index;};
    std::map<Address,Session> handles;Address serial=0x700000;
    unsigned updates=0,clears=0,ansi_calls=0,sort_writes=0,closed=0,copies=0;
    std::uint32_t flags=0xa5a5a5a7,locale_cp=65001,ansi=1;
    bool deny_final=false,deny_recalloc=false,fail_final_copy=false,has_final=false;
    Effects(f::Memory& m,at::Heap& h):memory(m),heap(h){for(const auto& [key,files]:names){(void)key;for(const auto& file:files)borrowed[reinterpret_cast<Address>(file.c_str())]=(file.size()+1)*2;}}
    std::byte* resolve(Address address,std::uint64_t bytes){
        for(const auto& [p,size]:memory.blocks){const auto begin=reinterpret_cast<Address>(p);if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return static_cast<std::byte*>(p)+static_cast<std::size_t>(address-begin);}
        for(const auto& [begin,size]:borrowed)if(address>=begin&&bytes<=size&&address-begin<=size-bytes)return reinterpret_cast<std::byte*>(begin)+static_cast<std::size_t>(address-begin);
        throw std::out_of_range("synthetic wildcard storage bounds");
    }
    std::uint64_t length(Address source)override{std::uint64_t count=0;while(read_u8(source+count)!=0)++count;return count;}
    Address calloc_066848(std::uint64_t count,std::uint64_t size)override{return reinterpret_cast<Address>(argv->calloc_066848(count,size).base);}
    Address recalloc_070434(Address old,std::uint64_t count,std::uint64_t size)override{
        const auto previous=heap.deny;heap.deny=deny_recalloc;const auto result=calloc_066848(count,size);heap.deny=previous;
        if(!result)return 0;
        const auto old_size=memory.blocks.at(reinterpret_cast<void*>(old));const auto bytes=std::min<std::uint64_t>(old_size,count*size);
        std::memcpy(resolve(result,bytes),resolve(old,bytes),static_cast<std::size_t>(bytes));free_0668c0(old);return result;
    }
    void free_0668c0(Address address)override{argv->free_0668c0(reinterpret_cast<void*>(address));}
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
    std::uint32_t file_api_ansi_066c1c()override{++ansi_calls;return ansi;}
    std::string narrow(Address address){const auto size=length(address);return{reinterpret_cast<char*>(resolve(address,size+1)),static_cast<std::size_t>(size)};}
    std::u16string wide(Address address){std::u16string result;for(;;address+=2){const auto c=read_u16(address);if(!c)return result;result.push_back(static_cast<char16_t>(c));}}
    anadius::crt_wildcard_search::Conversion to_wide_06004c(Address source,std::uint32_t cp)override{
        check(cp==(locale_cp==65001?65001U:ansi?0U:1U));const auto text=narrow(source);const auto address=calloc_066848(text.size()+1,2);check(address!=0);
        auto* output=::new(static_cast<void*>(resolve(address,(text.size()+1)*2))) char16_t[text.size()+1]{};
        for(std::size_t i=0;i<text.size();++i)output[i]=static_cast<unsigned char>(text[i]);return{address,true,0};
    }
    anadius::crt_wildcard_search::Conversion to_narrow_06e430(Address source,std::uint32_t cp)override{
        check(cp==(locale_cp==65001?65001U:ansi?0U:1U));const auto text=wide(source);const auto address=calloc_066848(text.size()+1,1);check(address!=0);
        auto* output=::new(static_cast<void*>(resolve(address,text.size()+1))) char[text.size()+1]{};
        for(std::size_t i=0;i<text.size();++i){check(text[i]<128);output[i]=static_cast<char>(text[i]);}return{address,true,0};
    }
    Address find_first(Address pattern,Address& filename)override{
        const auto text=wide(pattern);const std::string key(text.begin(),text.end());auto item=names.find(key);
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
    er::State errors;Errors error_fx;f::Memory memory;at::Heap heap_fx{errors,error_fx,memory};h::State heap{0xaaa,0,0,0};Invalid invalid;
    Effects effects{memory,heap_fx};Policy policy{effects};ag::ArgvEffects argv_fx{heap,heap_fx,errors,error_fx,invalid,policy};bc::ParserEffects parser{effects};
    Harness(){effects.argv=&argv_fx;}
};
std::vector<std::string> values(const av::State& state){std::vector<std::string> result;for(int i=0;i<state.argc;++i)result.emplace_back(state.argv[i]);check(state.argv[state.argc]==nullptr);return result;}
void integrated(unsigned variant){
    Harness f;f.effects.locale_cp=variant?1252:65001;f.effects.ansi=variant==2?0:0x100;
    av::State state;const std::string command="app.exe *.txt dir\\?.dat no-match*.bin";state.command_line={command.c_str(),command.size()+1};
    check(av::configure_0638e8(state,1,f.argv_fx,f.parser)==0);auto* original=state.argv;
    f.effects.has_final=false;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==0);
    const std::vector<std::string> expected{"app.exe","z.txt","A.txt","m.txt","b.txt","Q.txt","w.txt","c.txt","L.txt","a.txt","D.txt","n.txt","B.txt","dir\\z.dat","dir\\a.dat","no-match*.bin"};
    check(values(state)==expected);check(f.effects.sort_writes==0&&f.effects.closed==2&&f.effects.handles.empty());
    check(f.effects.updates==f.effects.clears&&(f.effects.flags&2)==0);check(variant?f.effects.ansi_calls>0:f.effects.ansi_calls==0);
    f.argv_fx.free_0668c0(original);check(f.memory.blocks.size()==1);
    auto* published=state.argv;const auto argc=state.argc;
    f.effects.has_final=false;f.effects.deny_recalloc=true;check(av::configure_0638e8(state,2,f.argv_fx,f.parser)==12);
    check(state.argv==published&&state.argc==argc&&f.memory.blocks.size()==1&&f.effects.handles.empty());f.effects.deny_recalloc=false;
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
int main(){try{for(unsigned variant=0;variant<3;++variant)integrated(variant);previous_and_bounds();copy_failure();std::cout<<"PASS "<<f::checks<<" wildcard/configure/storage/search/sort/locale composition checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
