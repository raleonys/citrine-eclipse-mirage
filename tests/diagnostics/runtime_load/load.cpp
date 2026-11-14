// SPDX-License-Identifier: GPL-3.0-only
#include "probe.hpp"
#include "laureondo/net/asio_service.hpp"
#include "laureondo/platform/execution_windows.hpp"
#include "laureondo/core/binary_io.hpp"
#include "laureondo/xml/reader.hpp"
#include <asio.hpp>
#include <windows.h>
#include <psapi.h>
#include <algorithm>
#include <atomic>
#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <latch>
#include <locale>
#include <mutex>
#include <numeric>
#include <set>
#include <thread>
#include <vector>
using namespace laureondo;
namespace {
using Clock=benchmark::Clock;
namespace fs=std::filesystem;
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
std::uint64_t ns(Clock::duration d){return std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();}
unsigned number(std::string_view value){unsigned n=0;auto r=std::from_chars(value.data(),value.data()+value.size(),n);require(r.ec==std::errc{}&&r.ptr==value.data()+value.size(),"invalid integer");return n;}
unsigned attribute(std::string_view s,std::string_view name){
    const auto marker=std::string(name)+"='";const auto first=s.find(marker);
    require(first!=s.npos,"missing generated attribute");const auto begin=first+marker.size(),end=s.find('\'',begin);
    require(end!=s.npos,"unterminated generated attribute");return number(s.substr(begin,end-begin));
}
struct Record {std::uint64_t queue=0,service=0;DWORD thread=0;};
struct Factory final:net::HandlerFactory {
    std::string workload;fs::path scratch;
    file::NativeFactory files;
    std::mutex shared_state;
    std::atomic<bool> measuring{false};
    std::atomic<unsigned> used{0},active{0},peak{0},errors{0};
    std::vector<Record> records;
    Factory(std::string w,fs::path dir,unsigned count):workload(std::move(w)),scratch(std::move(dir)),records(count){}
    struct Handler final:server::Handler {
        Factory& f;unsigned expected=0;
        explicit Handler(Factory& owner):f(owner){}
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view request)override {
            const auto begin=Clock::now();const auto queue=benchmark::queue_wait_ns;
            const auto count=++f.active;auto peak=f.peak.load();while(count>peak&&!f.peak.compare_exchange_weak(peak,count)){}
            struct Leave {Factory& f;~Leave(){--f.active;}} leave{f};
            require(attribute(request,"seq")==expected++,"session FIFO violated");
            if(f.workload!="echo") {
                std::unique_lock<std::mutex> lock(f.shared_state,std::defer_lock);
                if(f.workload=="xml_locked")lock.lock();
                xml_reader_bridge::Backend xml;
                // Repeated actual parse/view/printer work exposes CPU and lock
                // sensitivity without substituting a sleep for CPU work.
                for(unsigned i=0;i<16;++i){
                    auto parsed=xml.parse_c_string(request);require(!parsed.has_error,"XML parse failure");
                    auto* root=parsed.document->first_element();require(root&&root->name()=="B","XML root mismatch");
                    require(root->attribute("seq")==std::to_string(expected-1),"XML sequence mismatch");
                    auto printer=xml.printer();printer->open("Response");printer->attribute("seq",std::to_string(expected-1));printer->close();
                    require(printer->buffer_including_terminator().find("Response")!=std::string::npos,"XML printer failure");
                }
                if(f.workload=="atomic_pair"){
                    auto name=std::to_string(attribute(request,"session"));
                    file::FileWrite writes[]={{(f.scratch/(name+".xml")).u16string(),request},
                                              {(f.scratch/(name+".ini")).u16string(),request}};
                    f.files.replace_atomic(writes);
                }
            }
            std::string response(request);response.push_back('\0');
            if(f.measuring&&std::getenv("RUNTIME_BENCHMARK_FAULT"))response[0]='!';
            if(f.measuring){const auto slot=f.used.fetch_add(1);require(slot<f.records.size(),"extra measured handler call");f.records[slot]={queue,ns(Clock::now()-begin),GetCurrentThreadId()};}
            return response;
        }
    };
    std::unique_ptr<server::Handler> create()override{return std::make_unique<Handler>(*this);}
    void error(std::string_view,std::exception_ptr)noexcept override{++errors;}
};
struct Peer {
    asio::io_context io;asio::ip::tcp::socket socket{io};std::string input;
    explicit Peer(std::uint16_t port){
        socket.connect({asio::ip::address_v4::loopback(),port});socket.set_option(asio::ip::tcp::no_delay(true));
        DWORD timeout=10000;
        require(setsockopt(socket.native_handle(),SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout))==0,"receive timeout");
        require(setsockopt(socket.native_handle(),SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout))==0,"send timeout");
        require(frame()=="hello","greeting mismatch");
    }
    void send(std::string_view bytes){asio::write(socket,asio::buffer(bytes));}
    std::string frame(){
        for(;;){const auto end=input.find('\0');if(end!=std::string::npos){auto result=input.substr(0,end);input.erase(0,end+1);return result;}
            char data[16384];const auto count=socket.read_some(asio::buffer(data));require(count>0,"empty receive");input.append(data,count);
            require(input.size()<1024*1024,"unbounded response");}
    }
};
std::string request(unsigned session,unsigned seq,unsigned bytes){return "<B seq='"+std::to_string(seq)+"' session='"+std::to_string(session)+"'><Text>"+std::string(bytes,'x')+"</Text></B>";}
std::uint64_t cpu(){FILETIME a,b,k,u;require(GetProcessTimes(GetCurrentProcess(),&a,&b,&k,&u)!=0,"process times");
    return (std::uint64_t(k.dwHighDateTime)<<32)+k.dwLowDateTime+(std::uint64_t(u.dwHighDateTime)<<32)+u.dwLowDateTime;}
std::uint64_t cycles(){ULONG64 value=0;require(QueryProcessCycleTime(GetCurrentProcess(),&value)!=0,"process cycles");return value;}
void values(std::string_view name,const std::vector<std::uint64_t>& data){std::cout<<",\""<<name<<"\":[";bool first=true;for(auto v:data){if(!first)std::cout<<',';first=false;std::cout<<v;}std::cout<<']';}
void run(unsigned clients,unsigned requests,unsigned pipeline,unsigned bytes,const std::string& workload,const fs::path& scratch){
    require(clients>=1&&clients<=64&&requests>=1&&requests<=4096&&pipeline>=1&&pipeline<=32&&bytes<=65536,"bounded workload required");
    require(workload=="echo"||workload=="xml"||workload=="xml_locked"||workload=="atomic_pair","unknown workload");
    require(!fs::exists(scratch)&&fs::create_directory(scratch),"scratch must be a new directory");
    Factory factory(workload,scratch,clients*requests);
    execution::MutexState mutex{};execution::initialize_mutex_04a698(mutex,2);
    std::uint8_t ready=0,b=0,c=0;std::uint16_t port=0;
    execution::WindowsPlatform platform(+[](int){throw std::runtime_error("thread error");});
    auto frequency=platform.query_performance_frequency();execution::Execution execution(mutex,{ready,b,c},frequency,platform);
    const auto start_begin=Clock::now();
    net::AsioService service(factory,execution,port,ready);service.start();
    const auto startup=ns(Clock::now()-start_begin);
    std::vector<std::unique_ptr<Peer>> peers;
    // Warm every connection and the real XML/storage paths before measuring.
    for(unsigned i=0;i<clients;++i){auto p=std::make_unique<Peer>(port);for(unsigned j=0;j<4;++j){auto s=request(i,j,bytes);p->send(s+'\0');require(p->frame()==s,"warmup response mismatch");}peers.push_back(std::move(p));}
    factory.peak=0;
    std::vector<std::vector<std::uint64_t>> timings(clients);
    std::vector<std::exception_ptr> failures(clients);
    std::latch go{1},arrived{clients};std::vector<std::thread> threads;
    struct Join {
        std::latch& go;std::vector<std::thread>& threads;bool released=false;
        void release(){if(!released){released=true;go.count_down();}}
        ~Join(){release();for(auto& thread:threads)if(thread.joinable())thread.join();}
    } join{go,threads};
        for(unsigned i=0;i<clients;++i)threads.emplace_back([&,i]{
            arrived.count_down();go.wait();try{
                auto& out=timings[i];out.reserve(requests);
                for(unsigned sent=0;sent<requests;){
                    const auto count=std::min(pipeline,requests-sent);std::string batch;
                    for(unsigned j=0;j<count;++j){batch+=request(i,4+sent+j,bytes);batch.push_back('\0');}
                    const auto begin=Clock::now();peers[i]->send(batch);
                    for(unsigned j=0;j<count;++j){auto response=peers[i]->frame();auto elapsed=ns(Clock::now()-begin);require(response==request(i,4+sent+j,bytes),"loss, corruption or response reordering");out.push_back(elapsed);}
                    sent+=count;
                }
            }catch(...){failures[i]=std::current_exception();}
        });
    arrived.wait();
    const auto before_created=benchmark::created_message_threads.load();
    factory.measuring=true;const auto before_cpu=cpu(),before_cycles=cycles();const auto begin=Clock::now();join.release();
    for(auto& t:threads)t.join();const auto elapsed=ns(Clock::now()-begin);const auto used_cycles=cycles()-before_cycles;const auto cpu_ticks=cpu()-before_cpu;factory.measuring=false;
    for(auto& failure:failures)if(failure)std::rethrow_exception(failure);
    require(factory.errors==0&&factory.used==clients*requests,"handler error or missing measured request");
    require(factory.active==0&&factory.peak<=(RUNTIME_BENCHMARK_WORKERS?RUNTIME_BENCHMARK_WORKERS:clients),"worker bound/drain violated");
    PROCESS_MEMORY_COUNTERS_EX memory{};memory.cb=sizeof(memory);require(GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),sizeof(memory))!=0,"process memory");
    if(workload=="atomic_pair")for(unsigned i=0;i<clients;++i)for(auto extension:{".xml",".ini"}){
        std::ifstream stream(scratch/(std::to_string(i)+extension),std::ios::binary);std::string actual{std::istreambuf_iterator<char>(stream),{}};
        require(actual==request(i,3+requests,bytes),"atomic final file does not match acknowledged request");
    }
    peers.clear();const auto stop_begin=Clock::now();service.stop();const auto stop=ns(Clock::now()-stop_begin);
    factory.files.recover_atomic();require(factory.errors==0,"service retirement error");
    std::vector<std::uint64_t> rtt,queue,work;std::set<DWORD> worker_ids;
    for(auto& v:timings)rtt.insert(rtt.end(),v.begin(),v.end());
    for(auto& v:factory.records){queue.push_back(v.queue);work.push_back(v.service);worker_ids.insert(v.thread);}
    require(worker_ids.size()<=(RUNTIME_BENCHMARK_WORKERS?RUNTIME_BENCHMARK_WORKERS:clients*requests)&&!worker_ids.contains(0),"invalid worker observation");
    const auto created=benchmark::created_message_threads.load()-before_created;
    require(created==(RUNTIME_BENCHMARK_WORKERS?0u:clients*requests),"per-message thread count mismatch");
    const auto profile=RUNTIME_BENCHMARK_WORKERS?("pool-"+std::to_string(RUNTIME_BENCHMARK_WORKERS)):std::string("thread-per-message");
    std::cout<<"{\"profile\":\""<<profile<<"\",\"created_message_threads\":"<<created<<",\"workers\":"<<RUNTIME_BENCHMARK_WORKERS<<",\"clients\":"<<clients<<",\"requests_per_client\":"<<requests<<",\"pipeline\":"<<pipeline
      <<",\"payload_bytes\":"<<bytes<<",\"workload\":\""<<workload<<"\",\"completed\":"<<rtt.size()<<",\"errors\":0,\"elapsed_ns\":"<<elapsed
      <<",\"cpu_ns\":"<<cpu_ticks*100<<",\"cpu_cycles\":"<<used_cycles<<",\"startup_ns\":"<<startup<<",\"shutdown_ns\":"<<stop<<",\"peak_working_set_bytes\":"<<memory.PeakWorkingSetSize<<",\"private_bytes\":"<<memory.PrivateUsage
      <<",\"peak_active_handlers\":"<<factory.peak<<",\"observed_worker_threads\":"<<worker_ids.size();
    values("rtt_ns",rtt);values("queue_ns",queue);values("handler_ns",work);std::cout<<"}\n";
}
}
int main(int argc,char** argv)try{
    std::cout.imbue(std::locale::classic());require(argc==8,"usage: clients requests pipeline payload workload scratch expected-workers");
    require(number(argv[7])==RUNTIME_BENCHMARK_WORKERS,"wrong candidate binary");
    run(number(argv[1]),number(argv[2]),number(argv[3]),number(argv[4]),argv[5],fs::path(argv[6]));return 0;
}catch(const std::exception& e){std::cerr<<"BENCHMARK FAILED: "<<e.what()<<'\n';return 1;}
