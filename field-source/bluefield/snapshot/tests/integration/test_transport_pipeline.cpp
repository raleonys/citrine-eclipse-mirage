// SPDX-License-Identifier: GPL-3.0-only
#include "tests/support/transport/packet_session.hpp"
#include "tests/support/io.hpp"
#include "src/transport/service.hpp"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>
#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
namespace {
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
namespace packet=preloader_l::test_packet;
struct Scheduling:packet::Scheduling {
 std::deque<std::function<void()>> io,workers,writes;
 void post_io(std::function<void()> f) override{io.push_back(std::move(f));}
 void launch_detached(std::function<void()> f) override{workers.push_back(std::move(f));}
 void dispatch_write(std::function<void()> f) override{writes.push_back(std::move(f));}
 void log_error(std::string_view,packet::Error) override{throw std::runtime_error("unexpected packet error");}
 [[noreturn]] void raise_write_error(packet::Error) override{throw std::runtime_error("packet greeting error");}
 static void one(std::deque<std::function<void()>>& q){require(!q.empty(),"source job queued");auto f=std::move(q.front());q.pop_front();f();}
};
struct Handler:packet::Handler {
 unsigned resets=0;std::vector<std::string> seen;
 void reset_02bc70() override{++resets;}
 std::string initial_message_02d540() override{return std::string("HELLO\0",6);}
 std::string process_message_033d70(std::string_view value) override{seen.emplace_back(value);return "reply:"+std::string(value)+'\0';}
};
struct Transport:packet::Transport {
 std::function<void(packet::Error,std::string)> receive;
 packet::Completion send;unsigned reads=0,sends=0;std::string bytes;
 packet::WriteResult synchronous_send(std::span<const char> value) override{return {{},value.size()};}
 void async_receive(std::size_t,std::function<void(packet::Error,std::string)> f) override{require(!receive,"single packet read");++reads;receive=std::move(f);}
 void async_send(std::span<const char> value,packet::Completion f) override{require(!send,"single packet write");++sends;bytes.assign(value.data(),value.size());send=std::move(f);}
 void frame(std::string value){require(bool(receive),"source read rearmed with pending write");auto f=std::move(receive);receive={};value.push_back('\0');f({},std::move(value));}
 void complete(){require(bool(send),"packet write active");auto f=std::move(send);send={};f({},bytes.size());}
};
void source_oracle(){
 Scheduling q;Transport t;Handler h;packet::Session s{t,q,h};s.start_025720();
 require(h.resets==2,"packet constructor/start reset sequence");
 t.frame("one");Scheduling::one(q.io);Scheduling::one(q.workers);Scheduling::one(q.writes);
 require(s.queued_messages()==1&&t.sends==1,"source first write pending");
 t.frame("two");Scheduling::one(q.io);Scheduling::one(q.workers);Scheduling::one(q.writes);
 require(h.seen==std::vector<std::string>({"one","two"}),"source processes second frame before first write completion");
 require(t.reads==3&&s.queued_messages()==2&&t.sends==1,"source receive/response queues are independent");
 require(t.bytes==std::string("reply:one\0",10),"source front response stable");t.complete();
 require(t.sends==2&&t.bytes==std::string("reply:two\0",10),"source output queue preserves dispatch order");t.complete();require(s.queued_messages()==0,"source queue drained");
}
#if defined(__linux__)
std::atomic<unsigned> controlled_accepts{0};std::atomic<int> observed_send_buffer{0};
struct Client {
 int socket=-1;std::string pending;
 explicit Client(unsigned short port){
  socket=::socket(AF_INET,SOCK_STREAM,0);if(socket<0)throw std::runtime_error("client socket");
  int buffer=64*1024;require(::setsockopt(socket,SOL_SOCKET,SO_RCVBUF,&buffer,sizeof buffer)==0,"fixture receiver buffer set");
  timeval timeout{5,0};require(::setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof timeout)==0,"fixture receive timeout");
  sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons(port);
  if(::connect(socket,reinterpret_cast<sockaddr*>(&a),sizeof a)!=0){::close(socket);socket=-1;throw std::runtime_error("client connect");}
 }
 ~Client(){if(socket>=0)::close(socket);}
 void send(std::string_view value){while(!value.empty()){const auto n=::send(socket,value.data(),value.size(),MSG_NOSIGNAL);if(n<=0)throw std::runtime_error("client send");value.remove_prefix(static_cast<std::size_t>(n));}}
 std::string frame(){for(;;){auto n=pending.find('\0');if(n!=std::string::npos){auto result=pending.substr(0,n);pending.erase(0,n+1);return result;}char buffer[65536];const auto got=::recv(socket,buffer,sizeof buffer,0);if(got<=0)throw std::runtime_error("client receive");pending.append(buffer,static_cast<std::size_t>(got));}}
};
std::string request(const char* id,const char* body){return "<LSX><Request id=\""+std::string(id)+"\" recipient=\"EbisuSDK\">"+body+"</Request></LSX>"+'\0';}
void live(const std::filesystem::path& root,bool drain){
 namespace fs=std::filesystem;using namespace preloader_l;fs::create_directories(root);
 const std::string large(4*1024*1024,'P');Environment env{RuntimePaths::fixture(root/"fixture.exe",root/"surge.cfg",root,root/"programdata")};
 write_file(model_path(env.configuration_file),"\"Config2\"{\"Game\"{\"Name\" \""+large+"\"}}");
 Service service{env};service.start();const auto before=controlled_accepts.load();Client client{service.port()};
 require(client.frame().find("Challenge")!=std::string::npos,"greeting before application responses");
 require(controlled_accepts.load()>before&&observed_send_buffer.load()<=65536,"actual accepted socket send buffer constrained");
 require(!fs::exists(env.paths.default_avatar()),"avatar is lazy before second request");
 client.send(request("blocked","<GetGameInfo GameInfoId=\"DISPLAY_NAME\"/>")+request("side-effect","<QueryImage Width=\"208\" Height=\"208\" ImageId=\"avatar:1000200030000\"/>")+request("third","<GetProfile index=\"0\"/>"));
 const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(!fs::exists(env.paths.default_avatar())&&std::chrono::steady_clock::now()<deadline)std::this_thread::sleep_for(std::chrono::milliseconds(5));
 require(fs::exists(env.paths.default_avatar()),"second request executes while first TCP response is backpressured");
 if(drain){
  const auto first=client.frame();require(first.find("id=\"blocked\"")!=std::string::npos&&first.find(large)!=std::string::npos,"large queued front remains intact");
  const auto second=client.frame();require(second.find("id=\"side-effect\"")!=std::string::npos&&second.find("ResourcePath")!=std::string::npos,"second response follows front");
  const auto third=client.frame();require(third.find("id=\"third\"")!=std::string::npos&&third.find("SURGE")!=std::string::npos,"third request/response order retained");
 }
 const auto begin=std::chrono::steady_clock::now();service.stop();require(std::chrono::steady_clock::now()-begin<std::chrono::seconds(2),"stop drains backpressured read/write/work owners");
 require(!service.is_ready(),"stopped service is not ready");
}
#endif
}
#if defined(__linux__)
// Test executable only. Actual pinned Asio still accepts/completes real TCP IO.
extern "C" int accept(int fd,sockaddr* address,socklen_t* length){
 const auto result=static_cast<int>(::syscall(SYS_accept,fd,address,length));
 if(result>=0){int size=4096;socklen_t n=sizeof size;if(::setsockopt(result,SOL_SOCKET,SO_SNDBUF,&size,sizeof size)==0&&::getsockopt(result,SOL_SOCKET,SO_SNDBUF,&size,&n)==0){observed_send_buffer.store(size);controlled_accepts.fetch_add(1);}}
 return result;
}
#endif
int main(){try{
 source_oracle();
#if defined(__linux__)
 namespace fs=std::filesystem;const auto root=fs::temp_directory_path()/("preloader_l_pipeline_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 struct Cleanup{fs::path path;~Cleanup(){std::error_code e;fs::remove_all(path,e);}} cleanup{root};
 live(root/"drain",true);live(root/"stop",false);
 std::cout<<"Real TCP backpressure: PASS on Linux; Windows IOCP: NOT_RUN\n";
#else
 std::cout<<"Real TCP controlled-send-buffer fixture: NOT_RUN outside Linux\n";
#endif
 std::cout<<"PASS transport pipeline checks="<<checks<<"; literal packet session oracle + actual owned Asio transport\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL transport pipeline: "<<error.what()<<'\n';return 1;}}
