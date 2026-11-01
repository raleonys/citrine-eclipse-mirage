#include "factory.hpp"
#include "../anadius-path-source/paths.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <algorithm>
#include <climits>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <system_error>
#include <thread>
#include <type_traits>

namespace {
using namespace anadius;
namespace cf=configuration_factory;
using Path=configuration::Path;
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
struct Buffer:std::stringbuf {unsigned flushes=0;int sync()override{++flushes;return 0;}};
struct Stream:Buffer,std::ostream {std::function<void()> on_retire;Stream():std::ostream(static_cast<Buffer*>(this)){}~Stream(){if(on_retire)on_retire();} };
struct Call {std::uint32_t code,flags;int size,capacity;bool fill,has_default;};
struct State {
    std::shared_ptr<Stream> stream=std::make_shared<Stream>();
    std::wstring module=L"C:\\synthetic\\host.exe",program=L"PROGRAM",local=L"LOCAL";
    std::vector<DWORD> module_caps,env_caps,incoming;
    std::vector<std::wstring> env_names;
    std::vector<Call> narrow,wide;
    std::vector<std::string> prefixes,events,licenses;
    std::vector<Path> documents,creates,enumerations,removes,reads,writes;
    std::map<Path,std::string> input,output;
    std::vector<std::wstring> names{L"z",L".",L"..",L"a"};std::size_t cursor=0;
    unsigned sdk_calls=0,cp_calls=0,ansi_calls=0,lower_calls=0,space_calls=0,find_nexts=0,find_closes=0,file_closes=0;
    int lower_arg=0,space_arg=0;std::uint32_t cp=65001,convert_error=1113;
    bool ansi=true,short_fill=false,fail_convert=false,fail_fill=false,used_default=false,fail_once_flags=false;
    bool module_zero=false,env_zero=false,env_success=false,bad_output=false,throw_write=false,throw_close=false,throw_license=false,throw_document=false;
    DWORD enumeration_error=0,next_error=ERROR_NO_MORE_FILES,create_error=0,remove_error=0;
    std::vector<std::pair<std::string,std::string>> environments;
    std::shared_ptr<std::vector<std::string>> retirement=std::make_shared<std::vector<std::string>>();
    std::function<void()> on_wide;
    std::function<void(std::string_view)> on_retire;
    ~State(){if(on_retire)on_retire("backing");retirement->push_back("backing");}
};
State* current=nullptr;
DWORD WINAPI module_name(HMODULE module,LPWSTR out,DWORD capacity){
    auto& s=*current;++s.sdk_calls;require(!module,"module lookup selects current image");s.module_caps.push_back(capacity);s.incoming.push_back(::GetLastError());
    ::SetLastError(0x700+static_cast<DWORD>(s.module_caps.size()));if(s.module_zero)return 0;
    const auto n=std::min<std::size_t>(s.module.size(),capacity);std::copy_n(s.module.data(),n,out);if(n<capacity)out[n]=0;return static_cast<DWORD>(n);
}
DWORD WINAPI environment_path(LPCWSTR name,LPWSTR out,DWORD capacity){
    auto& s=*current;++s.sdk_calls;s.env_names.emplace_back(name);s.env_caps.push_back(capacity);s.incoming.push_back(::GetLastError());
    ::SetLastError(0x800+static_cast<DWORD>(s.env_caps.size()));if(s.env_zero)return 0;
    const auto& text=std::wstring_view(name)==L"ProgramData"?s.program:s.local;
    if(text.size()>=capacity)return static_cast<DWORD>(text.size()+1);std::copy(text.begin(),text.end(),out);out[text.size()]=0;return static_cast<DWORD>(text.size());
}
int space(int value){auto& s=*current;++s.space_calls;s.space_arg=value;return value==' '?8:0;}
int lower(int value){auto& s=*current;++s.lower_calls;s.lower_arg=value;return value>='A'&&value<='Z'?value+32:value;}
BOOL WINAPI set_environment(LPCSTR key,LPCSTR value){auto& s=*current;++s.sdk_calls;s.environments.emplace_back(key,value);return s.env_success?TRUE:FALSE;}
BOOL WINAPI create_directory(LPCWSTR path,LPSECURITY_ATTRIBUTES security){auto& s=*current;++s.sdk_calls;require(!security,"mkdir keeps null security");s.creates.emplace_back(path,path+std::wcslen(path));::SetLastError(s.create_error);return s.create_error?FALSE:TRUE;}
BOOL WINAPI attributes(LPCWSTR,GET_FILEEX_INFO_LEVELS,LPVOID data){++current->sdk_calls;*static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(data)={};static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(data)->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;return TRUE;}
HANDLE WINAPI find_first(LPCWSTR,LPWIN32_FIND_DATAW out){++current->sdk_calls;*out={};out->dwFileAttributes=FILE_ATTRIBUTE_DIRECTORY;return reinterpret_cast<HANDLE>(0x151);}
void filename(WIN32_FIND_DATAW& out,const std::wstring& name){out={};std::copy(name.begin(),name.end(),out.cFileName);}
HANDLE WINAPI find_first_ex(LPCWSTR path,FINDEX_INFO_LEVELS level,LPVOID data,FINDEX_SEARCH_OPS search,LPVOID filter,DWORD flags){
    auto& s=*current;++s.sdk_calls;s.enumerations.emplace_back(path,path+std::wcslen(path));require(level==FindExInfoBasic&&search==FindExSearchNameMatch&&!filter&&!flags,"enumeration preserves exact SDK operands");
    ::SetLastError(s.enumeration_error);if(s.enumeration_error)return INVALID_HANDLE_VALUE;
    if(s.names.empty()){::SetLastError(ERROR_NO_MORE_FILES);return INVALID_HANDLE_VALUE;}filename(*static_cast<WIN32_FIND_DATAW*>(data),s.names.front());s.cursor=1;return reinterpret_cast<HANDLE>(0x151);
}
BOOL WINAPI find_next(HANDLE h,LPWIN32_FIND_DATAW data){auto& s=*current;++s.sdk_calls;++s.find_nexts;require(h==reinterpret_cast<HANDLE>(0x151),"advance retains owned find handle");if(s.cursor==s.names.size()){::SetLastError(s.next_error);return FALSE;}filename(*data,s.names[s.cursor++]);return TRUE;}
BOOL WINAPI find_close(HANDLE h){auto& s=*current;++s.sdk_calls;++s.find_closes;require(h==reinterpret_cast<HANDLE>(0x151),"close retains owned find handle");::SetLastError(0x909);return TRUE;}
HANDLE WINAPI open_file(LPCWSTR path,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES security,DWORD disposition,DWORD flags,HANDLE model){
    auto& s=*current;++s.sdk_calls;s.removes.emplace_back(path,path+std::wcslen(path));require(access==0x10180&&share==7&&!security&&disposition==OPEN_EXISTING&&flags==0x02200000&&!model,"remove preserves native open access/share/disposition");
    ::SetLastError(s.remove_error);return s.remove_error?INVALID_HANDLE_VALUE:reinterpret_cast<HANDLE>(0x152);
}
BOOL WINAPI get_info(HANDLE,FILE_INFO_BY_HANDLE_CLASS,LPVOID,DWORD){++current->sdk_calls;return FALSE;}
BOOL WINAPI set_info(HANDLE h,FILE_INFO_BY_HANDLE_CLASS level,LPVOID data,DWORD bytes){++current->sdk_calls;require(h==reinterpret_cast<HANDLE>(0x152)&&static_cast<int>(level)==0x15&&bytes==4&&*static_cast<DWORD*>(data)==0x13,"remove uses retained extended disposition13");return TRUE;}
BOOL WINAPI close_handle(HANDLE h){++current->sdk_calls;require(h==reinterpret_cast<HANDLE>(0x152),"remove closes only acquired handle");return TRUE;}
config_storage::FilesystemApi fs_api(){return {create_directory,attributes,find_first,find_first_ex,find_next,find_close,open_file,get_info,set_info,close_handle,::GetLastError,::SetLastError};}
struct Encoding final:encoding_recovered::Calls {
    State& s;explicit Encoding(State& value):s(value){}~Encoding(){if(s.on_retire)s.on_retire("encoding");s.retirement->push_back("encoding");}
    std::uint32_t locale_codepage_0629ec()override{++s.cp_calls;return s.cp;}
    bool are_file_apis_ansi()override{++s.ansi_calls;return s.ansi;}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* p,int n,char16_t* out,int cap)override{
        s.narrow.push_back({cp,flags,n,cap,out!=nullptr,false});if(s.fail_convert||(out&&s.fail_fill))return 0;
        const int count=out&&s.short_fill?n-1:n;if(out){require(cap==n,"widen fill uses first pass capacity");for(int i=0;i<count;++i)out[i]=static_cast<unsigned char>(p[i]);}return count;
    }
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* p,int n,char* out,int cap,bool* used)override{
        s.wide.push_back({cp,flags,n,cap,out!=nullptr,used!=nullptr});s.prefixes.push_back(s.stream->str());if(s.on_wide)s.on_wide();if(used)*used=s.used_default;
        if(s.fail_once_flags&&flags){s.fail_once_flags=false;s.convert_error=0x3ec;return 0;}
        if(s.fail_convert||(out&&s.fail_fill))return 0;
        const int count=out&&s.short_fill?n-1:n;if(out){require(cap==n,"narrow fill uses first pass capacity");for(int i=0;i<count;++i)out[i]=static_cast<char>(p[i]);}return count;
    }
    std::uint32_t last_error()override{return s.convert_error;}
};
struct Errors final:encoding_recovered::Errors {
    State& s;explicit Errors(State& value):s(value){}~Errors(){if(s.on_retire)s.on_retire("errors");s.retirement->push_back("errors");}
    [[noreturn]]void runtime_00cd70(int value)override{throw std::system_error(value,std::generic_category());}
    [[noreturn]]void filesystem_00cdb0(std::uint32_t value)override{throw std::system_error(static_cast<int>(value),std::system_category());}
};
struct Input final:file_recovered::Input {
    std::string bytes;bool exists;std::size_t position=0;
    Input(std::string text,bool found):bytes(std::move(text)),exists(found){}
    std::uint32_t initial_state()const override{return exists?0:2;}
    void seek_03b610(std::int64_t offset,file_recovered::Seek mode)override{require(!offset,"binary persistence seeks zero");position=mode==file_recovered::Seek::end?bytes.size():0;}
    std::uint64_t position_sum_03ba50()override{return position;}
    std::int32_t peek_043790()override{throw std::logic_error("unexpected text read in binary persistence");}
    std::size_t read_03b2a0(std::span<std::uint8_t> out)override{auto n=std::min(out.size(),bytes.size()-position);std::copy_n(bytes.data()+position,n,out.data());position+=n;return n;}
    void close_03ae70()override{}
};
struct Output final:file_recovered::Output {
    State& s;Path path;Output(State& state,Path p):s(state),path(std::move(p)){}
    std::uint32_t initial_state()const override{return s.bad_output?2:0;}
    void write_03be80(std::string_view bytes)override{s.events.push_back("write");if(s.throw_write)throw std::runtime_error("write-prefix");s.output[path]=std::string(bytes);}
    void close_03ae70()override{++s.file_closes;s.events.push_back("close");if(s.throw_close)throw std::runtime_error("close-prefix");}
};
struct Files final:file_recovered::Factory {
    State& s;explicit Files(State& value):s(value){}~Files(){if(s.on_retire)s.on_retire("files");s.retirement->push_back("files");}
    std::unique_ptr<file_recovered::Input> open_input_039f70(const Path& p,std::uint32_t mode,std::uint32_t share)override{require(mode==0x20&&share==0x40,"binary input exact modes");s.reads.push_back(p);const auto i=s.input.find(p);return std::make_unique<Input>(i==s.input.end()?std::string{}:i->second,i!=s.input.end());}
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const Path& p,std::uint32_t mode,std::uint32_t share)override{require(mode==0x20&&share==0x40,"binary output exact modes");s.writes.push_back(p);s.events.push_back("open");return std::make_unique<Output>(s,p);}
};
struct Stats final:status_recovered::Stats {status_recovered::StatsResult query(const Path&,std::uint32_t flags)override{require(flags==3,"persistence status uses source flags3");return {ERROR_FILE_NOT_FOUND,0,{}};}};
struct License final:cf::LicenseEffects {
    State& s;explicit License(State& value):s(value){}~License(){if(s.on_retire)s.on_retire("license");s.retirement->push_back("license");}
    void apply(std::string_view value)override{s.licenses.emplace_back(value);if(s.throw_license)throw std::runtime_error("license-prefix");}
};
struct Document final:cf::LegacyDocumentReader {
    State& s;explicit Document(State& value):s(value){}~Document(){if(s.on_retire)s.on_retire("documents");s.retirement->push_back("documents");}
    configuration::Node read(const Path& path)override{s.documents.push_back(path);if(s.throw_document)throw std::runtime_error("document-prefix");configuration::Node n;n.value="legacy";n.children["b"].value="second";n.children["a"].value="first";n.iteration_order={"b","a"};return n;}
};
cf::Inputs inputs(const std::shared_ptr<State>& owner){
    current=owner.get();auto& s=*owner;cf::Inputs in;in.lifetime=owner;in.stream=s.stream;in.filesystem=std::make_shared<config_storage::Filesystem>(fs_api());
    in.encoding=std::make_unique<Encoding>(s);in.errors=std::make_unique<Errors>(s);in.crt={space,lower,set_environment};in.platform={module_name,environment_path,::GetLastError,::SetLastError};
    in.persistence.files=std::make_unique<Files>(s);in.persistence.stats=std::make_unique<Stats>();in.persistence.encoding=std::make_unique<Encoding>(s);in.persistence.errors=std::make_unique<Errors>(s);return in;
}
struct Fixture {
    std::shared_ptr<State> state=std::make_shared<State>();std::unique_ptr<cf::Configuration> config;
    explicit Fixture(achievement_storage::Paths paths={}){config=std::make_unique<cf::Configuration>(inputs(state),std::move(paths),std::make_unique<License>(*state),std::make_unique<Document>(*state));}
    ~Fixture(){config.reset();current=nullptr;}
};
void passive_and_required(){
    static_assert(!std::is_abstract_v<cf::Configuration>);
    static_assert(!std::is_copy_constructible_v<cf::Configuration>&&!std::is_move_constructible_v<cf::Configuration>);
    {Fixture f({u"seed.xml",u"seed.ini"});auto& s=*f.state;require(!s.sdk_calls&&s.narrow.empty()&&s.wide.empty()&&s.reads.empty()&&s.writes.empty()&&s.documents.empty()&&s.licenses.empty()&&s.stream->str().empty(),"construction is passive across every supplied effect");require(f.config->paths().xml_path==u"seed.xml"&&f.config->paths().ini_path==u"seed.ini","explicit initial persistence paths retained");}
    using Mutation=std::function<void(cf::Inputs&)>;
    const std::vector<Mutation> mutations={
        [](auto& i){i.stream.reset();},[](auto& i){i.filesystem.reset();},[](auto& i){i.encoding.reset();},[](auto& i){i.errors.reset();},
        [](auto& i){i.crt.space=nullptr;},[](auto& i){i.crt.lower=nullptr;},[](auto& i){i.crt.environment=nullptr;},
        [](auto& i){i.platform.module_filename=nullptr;},[](auto& i){i.platform.environment=nullptr;},[](auto& i){i.platform.last_error=nullptr;},[](auto& i){i.platform.set_last_error=nullptr;},
        [](auto& i){i.persistence.files.reset();},[](auto& i){i.persistence.xml.reset();},[](auto& i){i.persistence.numbers.reset();},[](auto& i){i.persistence.parser.reset();},[](auto& i){i.persistence.stats.reset();},[](auto& i){i.persistence.encoding.reset();},[](auto& i){i.persistence.errors.reset();}};
    for(const auto& change:mutations){auto s=std::make_shared<State>();auto in=inputs(s);change(in);rejects<std::invalid_argument>([&]{cf::Configuration invalid(std::move(in),{},std::make_unique<License>(*s),std::make_unique<Document>(*s));},"each required owned provider/import rejected before use");require(!s->sdk_calls&&s->reads.empty()&&s->writes.empty(),"invalid input has no construction I/O prefix");}
    auto s=std::make_shared<State>();rejects<std::invalid_argument>([&]{cf::Configuration invalid(inputs(s),{},{},std::make_unique<Document>(*s));},"license effect mandatory");
    rejects<std::invalid_argument>([&]{cf::Configuration invalid(inputs(s),{},std::make_unique<License>(*s),{});},"legacy reader mandatory");current=nullptr;
}
void paths_crt_environment(){
    Fixture f;auto& c=*f.config;auto& s=*f.state;
    ::SetLastError(0x53);require(c.executable_path_03c6d0()==u"C:\\synthetic\\host.exe","executable route returns supplied current-module path");require(s.module_caps==std::vector<DWORD>{260}&&s.incoming==std::vector<DWORD>{0x53}&&::GetLastError()==0x701,"module initial capacity and final LastError preserved");
    s.module_caps.clear();s.incoming.clear();s.module.assign(790,L'x');::SetLastError(0x54);require(c.executable_path_03c6d0()==Path(790,u'x'),"long module path retained");require(s.module_caps==std::vector<DWORD>({260,520,780,1040})&&s.incoming==std::vector<DWORD>({0x54,0x701,0x702,0x703}),"fixed0x104 growth forwards previous SDK status");
    s.module_zero=true;require(c.executable_path_03c6d0().empty(),"module zero remains empty");s.program.assign(800,L'p');s.env_caps.clear();require(c.program_data_03c920()==Path(800,u'p')&&s.env_caps==std::vector<DWORD>({260,520,780,1040}),"environment required size still uses fixed increments");
    require(s.env_names==std::vector<std::wstring>(4,L"ProgramData")&&c.local_app_data_03c6f0()==u"LOCAL"&&s.env_names.back()==L"LocalAppData","exact source environment names selected");s.env_zero=true;require(c.program_data_03c920().empty()&&c.local_app_data_03c6f0().empty(),"missing environment paths no guessed fallback");
    require(c.parent_path_019820(u"C:\\a\\file")==u"C:\\a"&&c.join_016b00(u"C:\\a",u"b")==u"C:\\a\\b"&&c.filename(u"C:\\a\\file")==u"file","actual parent join and filename routes");
    require(c.join_016b00(u"C:\\a",u"\\root")==u"C:\\root","rooted join retains drive");
    require(c.crt_is_space(' ')&&!c.crt_is_space(0xff)&&s.space_arg==255,"space SDK receives unsigned byte unchanged");require(c.crt_tolower('Q')=='q'&&c.crt_tolower(EOF)==EOF&&s.lower_arg==EOF,"tolower retains EOF and live CRT result");
    const auto prior=s.lower_calls;rejects<std::domain_error>([&]{c.crt_tolower(-2);},"negative nonEOF lower argument rejected");rejects<std::domain_error>([&]{c.crt_tolower(256);},"oversize lower argument rejected");require(s.lower_calls==prior,"invalid signed lower input never reaches CRT");
    require(!c.set_environment_variable(std::string_view("K\0tail",6),std::string_view("V\0tail",6))&&s.environments.back()==std::pair<std::string,std::string>{"K","V"},"setenv preserves original C-string truncation and FALSE status");s.env_success=true;require(c.set_environment_variable("K","V"),"setenv TRUE result returned");
}
void encoding_and_logging(){
    Fixture f;auto& c=*f.config;auto& s=*f.state;
    require(c.widen_0145d0(std::string_view("a\0B",3))==Path(u"a\0B",3),"UTF8 conversion keeps explicit embeddedNUL bytes");require(s.narrow.size()==2&&s.narrow[0].code==65001&&s.narrow[0].flags==8&&s.narrow[1].capacity==3&&!s.cp_calls,"UTF8 route two-pass flags8 no filesystem query");
    s.short_fill=true;require(c.widen_0145d0("abcd")==Path(u"abc\0",4),"UTF8 short fill retains first pass size");s.short_fill=false;
    const auto conversions=s.narrow.size();require(c.widen_0145d0({}).empty()&&s.narrow.size()==conversions,"empty UTF8 conversion has no calls");
    s.cp=1252;s.ansi=false;s.narrow.clear();require(c.filesystem_widen_010540("oem")==u"oem"&&s.narrow.front().code==1&&s.ansi_calls==1,"filesystem widening distinct OEM selection");s.ansi=true;s.narrow.clear();require(c.filesystem_widen_010540("ansi")==u"ansi"&&s.narrow.front().code==0,"filesystem ANSI selection");
    const auto cp_queries=s.cp_calls;require(c.filesystem_widen_010540({}).empty()&&s.cp_calls==cp_queries+1,"empty filesystem widen still selects codepage");
    s.cp=65001;s.wide.clear();require(c.narrow_path(Path(u"a\0B",3))==std::string("a\0B",3)&&s.wide[0].code==65001&&s.wide[0].flags==0x80&&!s.wide[0].has_default,"filesystem narrowing explicit length and strict UTF8 flag");
    s.cp=1252;s.used_default=true;bool error=false;try{c.narrow_path(u"lossy");}catch(const std::system_error& e){error=e.code().value()==0x459;}require(error,"default substitution remains error459");s.used_default=false;s.cp=65001;s.fail_once_flags=true;s.wide.clear();require(c.narrow_path(u"fallback")=="fallback"&&s.wide.size()==3&&s.wide[0].flags==0x80&&s.wide[1].flags==0&&s.wide[2].flags==0x80,"invalid-flags fallback retains independent fill pass");
    require(c.resolve_path_0192d0("a\\b",u"C:\\synthetic\\host.exe",true)==u"C:\\synthetic\\a\\b"&&c.resolve_path_0192d0("D:\\b",u"C:\\x\\host.exe",true)==u"D:\\b"&&c.resolve_path_0192d0("b",u"C:\\x\\host.exe",false)==u"b","resolution uses actual recovered absolute and parent rules");
    s.stream->str({});s.prefixes.clear();s.wide.clear();s.cp=1252;c.log_config_path(Path(u"C:\\synthetic\\a\0b",16));
    require(s.stream->str()==std::string("Parsing a\0b\n",12)&&s.stream->flushes==1,"logging counted filename bytes and newline flush");require(s.prefixes.size()==2&&s.prefixes[0]=="Parsing "&&s.prefixes[1]=="Parsing "&&s.wide[0].code==65001,"Parsing prefix precedes each UTF8 conversion with no filesystem codepage");
    s.stream->str({});s.fail_convert=true;error=false;try{c.log_config_path(u"C:\\bad");}catch(const std::system_error& e){error=e.code().value()==s.convert_error;}require(error&&s.stream->str()=="Parsing "&&s.stream->flushes==1,"conversion exception preserves visible prefix and suppresses suffix/flush");s.fail_convert=false;s.fail_fill=true;rejects<std::system_error>([&]{c.widen_0145d0("bad");},"UTF8 second-pass failure propagates");
}
void files_and_streaming(){
    Fixture f;auto& c=*f.config;auto& s=*f.state;c.create_directories_018e00(u"A\\B");require(s.creates==std::vector<Path>({u"A",u"A\\B"}),"mkdir actual retained prefix walk");
    require(c.directory_entries(u"DIR")==std::vector<Path>({u"DIR\\z",u"DIR\\a"})&&s.find_nexts==4&&s.find_closes==1,"vector enumeration preserves SDK order and exact dot omissions");
    struct Visit {cf::Configuration* c;State* s;std::vector<unsigned> nexts;std::vector<Path> paths;} v{&c,&s,{}, {}};
    s.find_nexts=0;c.visit_directory_entries(u"DIR",{&v,[](void* raw,const Path& p){auto& x=*static_cast<Visit*>(raw);x.nexts.push_back(x.s->find_nexts);x.paths.push_back(p);x.c->remove_04948c(p);}});
    require(v.nexts==std::vector<unsigned>({0,3})&&v.paths==std::vector<Path>({u"DIR\\z",u"DIR\\a"})&&s.removes==v.paths&&s.find_closes==2,"stream visitor deletes each entry before FindNext instead of eager collection");
    rejects<std::invalid_argument>([&]{c.visit_directory_entries(u"DIR",{});},"missing streaming visitor rejected");
    s.next_error=ERROR_ACCESS_DENIED;const auto oldcloses=s.find_closes;rejects<std::system_error>([&]{c.directory_entries(u"DIR");},"enumeration failure propagated");require(s.find_closes==oldcloses+1,"known enumeration failure closes owned cursor");s.next_error=ERROR_NO_MORE_FILES;
    s.enumeration_error=ERROR_NO_MORE_FILES;require(c.directory_entries(u"DIR").empty(),"empty SDK stream returns empty");s.enumeration_error=0;
    c.write_embedded_png_03c020(u"user.png",configuration::EmbeddedPng::default_user_1800809d0);const auto expected=config_storage::embedded_png(configuration::EmbeddedPng::default_user_1800809d0);
    require(s.output[u"user.png"]==std::string(reinterpret_cast<const char*>(expected.data()),expected.size())&&s.file_closes==1,"PNG route actual exact embedded payload through source binary writer");
    c.write_embedded_png_03c020(u"avatar.png",configuration::EmbeddedPng::default_avatar_180082710);require(s.output[u"avatar.png"].size()==config_storage::embedded_png(configuration::EmbeddedPng::default_avatar_180082710).size(),"second embedded identity routed");
    s.bad_output=true;const auto writes=s.output.size();const auto closes=s.file_closes;c.write_embedded_png_03c020(u"bad.png",configuration::EmbeddedPng::default_user_1800809d0);require(s.output.size()==writes&&s.file_closes==closes,"PNG void leaf retains ignored write_binary FALSE");s.bad_output=false;s.throw_write=true;rejects<std::runtime_error>([&]{c.write_embedded_png_03c020(u"throw.png",configuration::EmbeddedPng::default_user_1800809d0);},"PNG writer exception preserved");s.throw_write=false;
    s.throw_close=true;rejects<std::runtime_error>([&]{c.write_embedded_png_03c020(u"close.png",configuration::EmbeddedPng::default_user_1800809d0);},"PNG close exception preserved");s.throw_close=false;
}
void managed_log_boundary(){
    auto s=std::make_shared<State>();auto buffer=std::make_shared<Buffer>();auto stream=shared_log::make_stream(buffer);auto in=inputs(s);in.stream=stream;
    s->on_wide=[stream]{auto lease=shared_log::acquire(*stream);*stream<<"[convert]";};
    auto c=cf::make_configuration(std::move(in),{},std::make_unique<License>(*s),std::make_unique<Document>(*s));
    c->log_config_path(u"DIR\\file");require(buffer->str()=="Parsing [convert][convert]file\n"&&buffer->flushes==1,"managed stream lease is released before both opaque conversion calls");
    s->on_wide={};c.reset();current=nullptr;
}
void narrow_effects_and_storage(){
    Fixture f;auto& c=*f.config;auto& s=*f.state;
    const auto doc=c.load_document(u"synthetic.cfg");require(doc.value=="legacy"&&doc.iteration_order==std::vector<std::string>({"b","a"})&&s.documents==std::vector<Path>{u"synthetic.cfg"},"legacy narrow reader result keeps explicit nonlexical traversal");
    c.apply_license_key_02a830(std::string_view("key\0tail",8));require(s.licenses==std::vector<std::string>{std::string("key\0tail",8)},"license narrow effect receives complete counted input");
    s.throw_license=true;rejects<std::runtime_error>([&]{c.apply_license_key_02a830("fail");},"license exception prefix propagates");s.throw_document=true;rejects<std::runtime_error>([&]{c.load_document(u"fail.cfg");},"document exception propagates without fake document");
    configuration::State state{};state.user_directory=u"USER";state.achievements_set="set";s.input[u"USER\\achievement-set.xml"]="<Achievements><Achievement Id=\"id\" Count=\"3\" Progress=\"4\" Name=\"name\" Grant=\"2026-01-02T03:04:05\"/></Achievements>";
    c.update_achievement_storage_01eb00(state);require(c.paths().xml_path==u"USER\\achievement-set.xml"&&s.reads==std::vector<Path>{c.paths().xml_path},"actual achievementStorage derives and reads configured XML path");require(state.achievements.contains("id")&&state.achievements.at("id").field04==3&&state.achievements.at("id").field08==4&&state.achievements.at("id").display_name=="name","actual XMLBackend and storage mutate achievement values");
    state.steam_app_id="42";c.update_achievement_storage_01eb00(state);require(c.paths().ini_path==u"USER\\achievement_watcher\\42\\achievements.ini"&&s.output.contains(c.paths().ini_path)&&s.output.at(c.paths().ini_path).find("Achieved=3")!=std::string::npos,"actual persistence writer serializes achievement into derived INI");
}
void installation_lifetime_and_thread(){
    auto owner=std::make_shared<State>();auto retired=owner->retirement;std::weak_ptr<State> weak=owner;auto built=cf::make_configuration(inputs(owner),{},std::make_unique<License>(*owner),std::make_unique<Document>(*owner));
    auto* raw=owner.get();owner.reset();require(!weak.expired(),"Inputs lifetime alone retains raw capture backing");
    std::exception_ptr failure;std::thread worker([&]{try{require(built->crt_tolower('Z')=='z'&&built->widen_0145d0("worker")==u"worker","serialized runtime calls admit a different construction thread");}catch(...){failure=std::current_exception();}});worker.join();if(failure)std::rethrow_exception(failure);
    bootstrap_backend::Resources resources{};resources.configuration=std::move(built);auto other=std::make_shared<State>();current=other.get();auto in=inputs(other);auto* identity=resources.configuration.get();rejects<std::invalid_argument>([&]{cf::install_configuration(resources,std::move(in),{},std::make_unique<License>(*other),std::make_unique<Document>(*other));},"mixed existing configuration slot rejected");require(resources.configuration.get()==identity&&!other->sdk_calls,"failed mixed install preserves original owner and passive prefix");current=raw;
    resources.configuration.reset();require(weak.expired()&&!retired->empty()&&retired->back()=="backing","narrow effects, persistence and conversion owners retire before Inputs backing");
    auto position=[&](std::string_view name){return std::find(retired->begin(),retired->end(),name)-retired->begin();};require(position("license")<position("files")&&position("documents")<position("files")&&position("files")<position("backing"),"normal destruction ordering retains persistence through both narrow effect destructors");
    auto s=std::make_shared<State>();bootstrap_backend::Resources empty{};auto invalid=inputs(s);invalid.crt.lower=nullptr;rejects<std::invalid_argument>([&]{cf::install_configuration(empty,std::move(invalid),{},std::make_unique<License>(*s),std::make_unique<Document>(*s));},"invalid installation fails before publication");require(!empty.configuration&&!s->sdk_calls,"invalid install leaves slot empty");
    cf::install_configuration(empty,inputs(s),{},std::make_unique<License>(*s),std::make_unique<Document>(*s));require(empty.configuration&&empty.configuration->filename(u"A\\B")==u"B","successful install publishes concrete all-route owner");empty.configuration.reset();current=nullptr;
}
void attached_retirement(){
    auto owner=std::make_shared<State>();auto* raw=owner.get();auto config=std::make_unique<cf::Configuration>(inputs(owner),achievement_storage::Paths{},std::make_unique<License>(*owner),std::make_unique<Document>(*owner));auto* facade=config.get();
    struct Result {unsigned phases=0,attempts=0,rejected=0;bool unexpected=false,document=false,license=false,backing=false,stream=false;};auto result=std::make_shared<Result>();
    // Every callback executes while this exact facade's destructor is active;
    // none retains or calls it after destructor return. The backing's final
    // callback runs in its destructor body before its metadata members die.
    auto probe=[result](auto&& f){++result->attempts;try{f();}catch(const std::logic_error&){++result->rejected;}catch(...){result->unexpected=true;}};
    owner->on_retire=[facade,result,probe](std::string_view phase){
        ++result->phases;probe([&]{(void)facade->paths();});probe([&]{(void)facade->widen_0145d0("stop");});probe([&]{(void)facade->parent_path_019820(u"A\\B");});probe([&]{(void)facade->join_016b00(u"A",u"B");});probe([&]{(void)facade->filename(u"A\\B");});
        if(phase=="license")result->license=true;if(phase=="backing")result->backing=true;
        if(phase!="documents")return;result->document=true;
        // The first opaque destructor still has all providers alive; all21
        // actual methods get valid arguments, so only the retirement seal is
        // a valid reason for these calls to reject.
        probe([&]{(void)facade->executable_path_03c6d0();});probe([&]{(void)facade->filesystem_widen_010540("x");});probe([&]{(void)facade->narrow_path(u"x");});probe([&]{(void)facade->resolve_path_0192d0("x",u"C:\\host.exe",true);});
        probe([&]{(void)facade->program_data_03c920();});probe([&]{(void)facade->local_app_data_03c6f0();});probe([&]{(void)facade->load_document(u"x");});probe([&]{facade->log_config_path(u"x");});probe([&]{(void)facade->crt_is_space(' ');});probe([&]{(void)facade->crt_tolower('A');});probe([&]{facade->apply_license_key_02a830("x");});
        probe([&]{facade->create_directories_018e00(u"A");});probe([&]{facade->write_embedded_png_03c020(u"x",configuration::EmbeddedPng::default_user_1800809d0);});probe([&]{(void)facade->directory_entries(u"A");});probe([&]{facade->visit_directory_entries(u"A",{nullptr,[](void*,const Path&){}});});probe([&]{facade->remove_04948c(u"x");});probe([&]{configuration::State state{};facade->update_achievement_storage_01eb00(state);});probe([&]{(void)facade->set_environment_variable("k","v");});
    };
    owner->stream->on_retire=[facade,result,probe]{++result->phases;result->stream=true;probe([&]{(void)facade->paths();});probe([&]{(void)facade->widen_0145d0("stop");});};
    // Stream now belongs only to Inputs, backing only to Inputs::lifetime.
    // All ordinary provider destructors therefore run before backing retires.
    owner->stream.reset();std::weak_ptr<State> weak=owner;owner.reset();require(raw->sdk_calls==0,"retirement fixture begins without SDK effects");config.reset();
    require(result->phases==9&&result->document&&result->license&&result->stream&&result->backing&&weak.expired(),"both narrow effects, persistence/core providers, stream and backing actually retire under attached facade");
    require(!result->unexpected&&result->attempts==60&&result->rejected==result->attempts,"all21 methods plus paths and each retirement phase reject reentry before provider use");current=nullptr;
}
}
int main(){try{passive_and_required();paths_crt_environment();encoding_and_logging();files_and_streaming();managed_log_boundary();narrow_effects_and_storage();installation_lifetime_and_thread();attached_retirement();std::cout<<"PASS: "<<checks<<" configuration-factory core checks; all21 required routes and streaming, owned synthetic effects, passive validation, prefix/errors and serialized cross-thread lifetime; original targets never loaded\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
