#include "stream_test_fixture.hpp"

namespace {
using namespace b20_test;
void ownership_and_metadata(){
    rejects<std::invalid_argument>([]{sl::make_stream({});},"null streambuf owner rejected");
    std::ostringstream legacy;::SetLastError(111);
    {const auto scope=sl::acquire(legacy);require(!scope.managed()&&::GetLastError()==111,"plain legacy stream has no managed scope");::SetLastError(222);}
    require(::GetLastError()==222,"legacy scope leaves body LastError visible");
    auto buffer=std::make_shared<Buffer>();auto stream=sl::make_stream(buffer);std::weak_ptr<sl::Stream> weak=stream;std::weak_ptr<Buffer> weak_buffer=buffer;
    ::SetLastError(111);
    {const auto scope=sl::acquire(*stream);require(scope.managed()&&::GetLastError()==111,"managed acquire restores entry LastError before body");
     auto* raw=stream.get();stream.reset();buffer.reset();require(!weak.expired()&&!weak_buffer.expired(),"lease retains stream and its buffer after outside owners reset");
     *raw<<"retained";::SetLastError(222);}
    require(weak.expired()&&weak_buffer.expired()&&::GetLastError()==222,"release and final owner destruction preserve outgoing LastError");
    auto owned=std::make_shared<Buffer>();sl::Stream stack(owned);
    rejects<std::invalid_argument>([&]{const auto scope=sl::acquire(stack);},"selected stack stream has no shared lifetime owner");
}
void rejection_and_unwind(){
    auto a_buffer=std::make_shared<Buffer>(),b_buffer=std::make_shared<Buffer>();auto a=sl::make_stream(a_buffer),b=sl::make_stream(b_buffer);
    {const auto scope=sl::acquire(*a);
     rejects<std::logic_error>([&]{const auto nested=sl::acquire(*a);},"same-stream nested scope rejected before ostream access");
     rejects<std::logic_error>([&]{const auto nested=sl::acquire(*b);},"cross-stream nested scope rejected before second gate");}
    std::ostringstream legacy;
    {const auto scope=sl::acquire(*a);rejects<std::logic_error>([&]{const auto nested=sl::acquire(legacy);},"legacy adapter reentry rejected before its private gate");}
    require(a_buffer->text().empty()&&b_buffer->text().empty(),"nested rejections emitted no bytes");
    a->tie(b.get());::SetLastError(123);
    rejects<std::logic_error>([&]{const auto scope=sl::acquire(*a);},"tied managed stream rejected before external flush");
    require(::GetLastError()==123&&b_buffer->syncs==0,"tie rejection preserves LastError and external stream untouched");a->tie(nullptr);
    a->rdbuf(b_buffer.get());
    rejects<std::logic_error>([&]{const auto scope=sl::acquire(*a);},"external rdbuf replacement rejected before source output");
    require(a_buffer->text().empty()&&b_buffer->text().empty(),"buffer mismatch emits no bytes to either buffer");a->rdbuf(a_buffer.get());
    const std::runtime_error controlled("own body failure");
    try{const auto scope=sl::acquire(*a);::SetLastError(242);throw controlled;}catch(const std::runtime_error&){}
    require(::GetLastError()==242,"exception unwind preserves body LastError");
    {const auto scope=sl::acquire(*a);*a<<"after";}
    require(a_buffer->text()=="after","exception releases gate and thread scope");
    unsigned rejected=0;a_buffer->on_write=[&]{try{const auto nested=sl::acquire(*b);}catch(const std::logic_error&){++rejected;}};
    {const auto scope=sl::acquire(*a);*a<<"outer";}
    require(rejected==1&&b_buffer->text().empty(),"streambuf nested adapter rejects before touching other stream");
    a_buffer->on_write=[&]{const auto nested=sl::acquire(*a);};
    {const auto scope=sl::acquire(*a);*a<<"blocked";}
    require(a->bad()&&a_buffer->text()=="afterouter","ostream may swallow nested rejection into badbit without rollback");
    a_buffer->on_write={};{const auto scope=sl::acquire(*a);a->clear();*a<<"ok";}
    require(a_buffer->text()=="afterouterok","gate remains usable after swallowed streambuf failure");
}
void formatting_and_failure(){
    auto managed_buffer=std::make_shared<Buffer>(),plain_buffer=std::make_shared<Buffer>();auto managed=sl::make_stream(managed_buffer);std::ostream plain(plain_buffer.get());
    for(auto* stream:{static_cast<std::ostream*>(managed.get()),&plain}){
        {const auto scope=sl::acquire(*stream);*stream<<std::hex<<std::setfill('_')<<std::setw(6);}
        {const auto scope=sl::acquire(*stream);*stream<<std::uint16_t{0x23};*stream<<std::string_view("A\0B",3)<<std::endl;}
    }
    require(managed_buffer->text()==plain_buffer->text()&&managed_buffer->text()==std::string("____23A\0B\n",10),"persistent numeric formatting, width and counted NUL agree with compiler ostream");
    require(managed_buffer->syncs==plain_buffer->syncs&&managed->width()==0,"endl flush and consumed width agree");
    auto short_buffer=std::make_shared<Buffer>();short_buffer->maximum=3;auto short_stream=sl::make_stream(short_buffer);
    {const auto scope=sl::acquire(*short_stream);*short_stream<<"Parsing ";}
    require(short_stream->bad()&&short_buffer->text()=="Par","short insertion retains partial prefix and shared badbit");
    {const auto scope=sl::acquire(*short_stream);*short_stream<<"later";}
    require(short_buffer->text()=="Par","another operation observes same fail state rather than isolated frontend");
    short_buffer->maximum=-1;short_buffer->fail_sync=true;
    {const auto scope=sl::acquire(*short_stream);short_stream->clear();short_stream->exceptions(std::ios::badbit);}
    rejects<std::ios_base::failure>([&]{const auto scope=sl::acquire(*short_stream);*short_stream<<"kept"<<std::endl;},"flush failure propagates selected exception mask");
    require(short_buffer->text()=="Parkept\n"&&short_stream->bad(),"flush exception retains bytes and fail state");
    {const auto scope=sl::acquire(*short_stream);short_stream->exceptions(std::ios::goodbit);short_stream->clear();}
}
void concurrent_operations(){
    auto buffer=std::make_shared<Buffer>();auto stream=sl::make_stream(buffer);std::vector<std::thread> workers;std::atomic<unsigned> failures=0;
    for(unsigned i=0;i<6;++i)workers.emplace_back([&]{try{for(unsigned j=0;j<100;++j){const auto scope=sl::acquire(*stream);*stream<<"entry"<<std::endl;}}catch(...){++failures;}});
    for(auto& thread:workers)thread.join();
    require(failures==0&&stream->good(),"six own callers finish managed formatted operations");
    require(buffer->overlap==0&&buffer->inside==0,"no concurrent streambuf invocation through shared gate");
    require(buffer->text().size()==3600&&buffer->syncs==600,"all owned operation bytes and flushes retained");
}
struct Erasure {std::weak_ptr<Buffer> buffer;bool called=false,alive=false;};
void erased(std::ios_base::event event,std::ios_base& stream,int index){if(event==std::ios_base::erase_event){auto& state=*static_cast<Erasure*>(stream.pword(index));state.called=true;state.alive=!state.buffer.expired();}}
void buffer_outlives_ios(){
    auto buffer=std::make_shared<Buffer>();auto stream=sl::make_stream(buffer);Erasure state{buffer};const auto index=std::ios_base::xalloc();
    {const auto scope=sl::acquire(*stream);stream->pword(index)=&state;stream->register_callback(erased,index);}
    buffer.reset();stream.reset();require(state.called&&state.alive&&state.buffer.expired(),"buffer owner survives ostream erase callback then releases");
}
}
int main(){try{ownership_and_metadata();rejection_and_unwind();formatting_and_failure();concurrent_operations();buffer_outlives_ios();std::cout<<"PASS: "<<b20_test::checks<<" shared log core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
