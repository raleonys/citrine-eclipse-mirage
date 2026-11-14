// SPDX-License-Identifier: GPL-3.0-only
#include "application_probe.hpp"
#include "laureondo/runtime/application.hpp"
#include "laureondo/runtime/event_queue.hpp"
#include "laureondo/codec/crypto.hpp"
#include <asio.hpp>
#include <pugixml.hpp>
#include <windows.h>
#include <psapi.h>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <latch>
#include <locale>
#include <map>
#include <set>

using namespace laureondo;
namespace {
namespace fs=std::filesystem;
using benchmark::Clock;
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
std::uint64_t ns(Clock::duration duration){return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();}
unsigned number(std::string_view value){
    unsigned result=0;auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
    require(parsed.ec==std::errc{}&&parsed.ptr==value.data()+value.size(),"invalid integer");return result;
}
std::string award(unsigned client){return "award"+std::to_string(client);}
std::vector<std::string> content_order;
std::string id(unsigned client,unsigned sequence){return std::to_string(client)+"-"+std::to_string(sequence);}
std::string envelope(std::string_view request_id,std::string_view body,bool response=false){
    const std::string tag=response?"Response":"Request",direction=response?"sender":"recipient";
    return "<LSX><"+tag+" id='"+std::string(request_id)+"' "+direction+"='EbisuSDK'>"+
        std::string(body)+"</"+tag+"></LSX>";
}
std::string achievement(std::string_view name,unsigned progress,std::string_view date){
    return "<Achievement Id='"+std::string(name)+"' Total='1000000' Count='0' Progress='"+std::to_string(progress)+
        "' GrantDate='"+std::string(date)+"' Expiration='0000-00-00T00:00:00' ImageId='' Name='Load award' Description='' HowTo=''/>";
}
// Independent expected fixture values, not a second call through the router.
std::string expected_body(unsigned command,unsigned clients){
    if(command==0)return "<GetProfileResponse Country='US' GeoCountry='US' PersonaId='42' IsTrialSubscriber='false' SubscriberLevel='2' Persona='load-user' UserId='43' CommerceCountry='US' CommerceCurrency='USD' AvatarId='' IsSubscriber='true' UserIndex='0' IsUnderAge='false' IsSteamSubscriber='false'/>";
    if(command==1)return "<GetSettingsResponse Environment='production' IsIGOAvailable='false' IsTelemetryEnabled='false' IsManualOffline='false' Language='en_US' IsIGOEnabled='false'/>";
    if(command==2){
        std::string result="<QueryContentResponse>";
        for(const auto& content:content_order)result+="<Game progressValue='0' contentID='"+content+"' availableVersion='1' displayName='Load content' state='INSTALLED' installedVersion='1'/>";
        return result+"</QueryContentResponse>";
    }
    std::set<std::string> names;for(unsigned i=0;i<clients;++i)names.insert(award(i));
    std::string result="<AchievementSets><AchievementSet GameName='Load game' Name='load-set'>";
    for(const auto& name:names)result+=achievement(name,0,"0000-00-00T00:00:00");
    return result+"</AchievementSet></AchievementSets>";
}
bool equal_xml(pugi::xml_node actual,pugi::xml_node expected){
    if(actual.type()!=expected.type()||std::string_view(actual.name())!=expected.name()||std::string_view(actual.value())!=expected.value())return false;
    std::map<std::string,std::string> left,right;
    for(auto attribute:actual.attributes())if(!left.emplace(attribute.name(),attribute.value()).second)return false;
    for(auto attribute:expected.attributes())right.emplace(attribute.name(),attribute.value());
    if(left!=right)return false;
    auto a=actual.first_child(),b=expected.first_child();
    for(;a&&b;a=a.next_sibling(),b=b.next_sibling())if(!equal_xml(a,b))return false;
    return !a&&!b;
}
void verify_xml(std::string_view actual,std::string_view expected){
    pugi::xml_document a,b;
    require(a.load_buffer(actual.data(),actual.size())&&b.load_buffer(expected.data(),expected.size())&&equal_xml(a,b),
            "application response validation failed");
}
bool timestamp(std::string_view value){
    if(value.size()!=19||value=="0000-00-00T00:00:00")return false;
    for(unsigned i=0;i<value.size();++i){
        const char separator=i==4||i==7?'-':i==10?'T':i==13||i==16?':':'\0';
        if(separator?value[i]!=separator:value[i]<'0'||value[i]>'9')return false;
    }
    return true;
}
void verify_response(std::string_view response,unsigned client,unsigned sequence,unsigned command,unsigned clients,bool grant){
    std::string body;
    if(grant){
        pugi::xml_document parsed;
        require(parsed.load_buffer(response.data(),response.size()),"application response validation failed");
        const auto date=parsed.child("LSX").child("Response").child("Achievement").attribute("GrantDate").value();
        require(timestamp(date),"application response validation failed");
        body=achievement(award(client),sequence+1,date);
    }else body=expected_body(command,clients);
    verify_xml(response,envelope(id(client,sequence),body,true));
}
struct Peer {
    asio::io_context io;asio::ip::tcp::socket socket{io};std::string input;
    crypto::Cipher cipher;bool encrypted;
    Peer(std::uint16_t port,unsigned client,bool use_cipher):encrypted(use_cipher){
        socket.connect({asio::ip::address_v4::loopback(),port});socket.set_option(asio::ip::tcp::no_delay(true));
        DWORD timeout=10000;
        for(auto option:{SO_RCVTIMEO,SO_SNDTIMEO})require(setsockopt(socket.native_handle(),SOL_SOCKET,option,
            reinterpret_cast<const char*>(&timeout),sizeof(timeout))==0,"peer timeout");
        pugi::xml_document greeting;auto bytes=frame();
        require(greeting.load_string(bytes.c_str()),"Application greeting parses");
        auto challenge=greeting.child("LSX").child("Event").child("Challenge");
        const std::string key=challenge.attribute("key").value();
        require(key.size()==32&&key.find_first_not_of("0123456789abcdef")==key.npos,"Application challenge key");
        verify_xml(bytes,"<LSX><Event sender='EALS'><Challenge version='10,6,2,31' build='release' key='"+key+"'/></Event></LSX>");
        if(!encrypted)return;
        const std::string secret="load-peer-"+std::to_string(client);
        const unsigned version=client%2?3:2;
        send(envelope("handshake","<ChallengeResponse key='"+secret+"' version='"+std::to_string(version)+"'/>")+ '\0');
        crypto::Cipher fixed;fixed.set_fixed_key_084c98();std::string accepted(secret);
        require(fixed.encrypt_042610(accepted),"reference challenge encryption");messages::hex_encode_03d190(accepted);
        verify_xml(frame(),envelope("handshake","<ChallengeAccepted response='"+accepted+"'/>",true));
        if(version==2)cipher.set_fixed_key_084c98();
        else{crypto::WindowsTimeSource time;crypto::Random random(time);cipher.set_key_042780(random.key_from_response_034c30(accepted));}
    }
    void send(std::string_view bytes){asio::write(socket,asio::buffer(bytes));}
    std::string frame(){
        for(;;){const auto end=input.find('\0');if(end!=input.npos){auto result=input.substr(0,end);input.erase(0,end+1);return result;}
            char data[16384];const auto count=socket.read_some(asio::buffer(data));require(count>0,"empty Application receive");input.append(data,count);
            require(input.size()<1024*1024,"unbounded Application response");}
    }
    std::string encode(std::string value){
        if(encrypted){require(cipher.encrypt_042610(value),"request encryption");messages::hex_encode_03d190(value);}
        value.push_back('\0');return value;
    }
    std::string receive(){
        auto value=frame();
        if(encrypted){
            require(value.size()%2==0&&value.find_first_not_of("0123456789abcdef")==value.npos,"application response validation failed");
            messages::hex_decode_03ce30(value);require(cipher.decrypt_042480(value),"application response validation failed");
        }
        return value;
    }
};
unsigned command_for(std::string_view workload,unsigned sequence){return workload=="app_mixed_encrypted"?sequence%4:0;}
std::string command(unsigned which){
    static constexpr const char* commands[]={"<GetProfile index='0'/>","<GetSettings/>","<QueryContent/>","<QueryAchievements All='true'/>"};
    return commands[which];
}
std::string request(std::string_view workload,unsigned client,unsigned sequence){
    return envelope(id(client,sequence),workload=="app_achievement_encrypted"
        ?"<GrantAchievement AchievementId='"+award(client)+"' Progress='1'/>"
        :command(command_for(workload,sequence)));
}
void write_config(const fs::path& scratch,unsigned clients){
    std::ofstream out(scratch/"surge.cfg",std::ios::binary);
    out<<R"("Config2" {"Game" {"ContentId" "load-id" "Name" "Load game"} "User" {"Username" "load-user" "PersonaId" "42" "UserId" "43"} "Content" {)";
    for(unsigned i=0;i<8;++i)out<<'"'<<i<<R"(" {"Name" "Load content" "Version" "1"})";
    out<<R"(} "Achievements" {"AchievementsSet" "load-set" "SteamAppId" "12345" "AchievementNames" {)";
    for(unsigned i=0;i<clients;++i)out<<'"'<<award(i)<<R"(" "Load award")";
    out<<R"(} "AchievementTotals" {)";
    for(unsigned i=0;i<clients;++i)out<<'"'<<award(i)<<R"(" "1000000")";
    out<<"}}}";out.close();require(out.good(),"fixture configuration write");
}
std::string read_file(const fs::path& path){std::ifstream stream(path,std::ios::binary);require(stream.good(),"persisted file missing");return {std::istreambuf_iterator<char>(stream),{}};}
void verify_persistence(const fs::path& scratch,const configuration::State& state,unsigned clients,unsigned count){
    require(state.achievements.size()==clients,"unexpected persisted achievement count");
    std::string xml="<Achievements GameName='Load game' ContentId='load-id'>",ini;
    for(const auto& [name,a]:state.achievements){
        require(a.field00==1000000&&a.field04==0&&a.field08==static_cast<int>(count)&&timestamp(a.timestamp)&&a.field70>0,
            "persisted state differs from acknowledged progress");
        xml+="<Achievement Id='"+name+"' Count='0' Progress='"+std::to_string(count)+"' Grant='"+a.timestamp+"' Name='Load award'/>";
        ini+="["+name+"]\r\nAchieved=0\r\nCurProgress="+std::to_string(count)+"\r\nMaxProgress=1000000\r\nUnlockTime="+std::to_string(a.field70)+"\r\n\r\n";
    }
    verify_xml(read_file(scratch/"data/achievement-load-set.xml"),xml+"</Achievements>");
    require(read_file(scratch/"data/achievement_watcher/12345/achievements.ini")==ini,"persisted INI differs from acknowledged progress");
}
void response_controls(){
    for(unsigned i=0;i<8;++i)content_order.push_back(std::to_string(i));
    for(unsigned c=0;c<4;++c)verify_response(envelope("0-4",expected_body(c,2),true),0,4,c,2,false);
    const auto good=envelope("0-4",achievement("award0",5,"2026-01-01T00:00:01"),true);
    verify_response(good,0,4,0,2,true);
    const std::vector<std::string> bad={
        envelope("wrong-id",expected_body(0,2),true),
        envelope("0-4","<GetProfileResponse Persona='wrong'/>",true),
        envelope("0-4",expected_body(0,2)+"<Unexpected/>",true),
        envelope("0-4",achievement("award0",4,"2026-01-01T00:00:01"),true),
        envelope("0-4",achievement("award1",5,"2026-01-01T00:00:01"),true),
        envelope("0-4",achievement("award0",5,"0000-00-00T00:00:00"),true)};
    for(unsigned i=0;i<bad.size();++i){bool rejected=false;
        try{verify_response(bad[i],0,4,0,2,i>=3);}catch(const std::runtime_error&){rejected=true;}
        require(rejected,"Application oracle accepted a bad response");}
    std::cout<<"PASS: complete Application response, ID, command, progress and timestamp controls\n";
}
std::uint64_t cpu(){FILETIME a,b,k,u;require(GetProcessTimes(GetCurrentProcess(),&a,&b,&k,&u)!=0,"process times");return ((std::uint64_t(k.dwHighDateTime)<<32)+k.dwLowDateTime+(std::uint64_t(u.dwHighDateTime)<<32)+u.dwLowDateTime)*100;}
std::uint64_t cycles(){ULONG64 value=0;require(QueryProcessCycleTime(GetCurrentProcess(),&value)!=0,"process cycles");return value;}
void values(std::string_view name,const std::vector<std::uint64_t>& data){std::cout<<",\""<<name<<"\":[";bool first=true;for(auto value:data){if(!first)std::cout<<',';first=false;std::cout<<value;}std::cout<<']';}
void run(unsigned clients,unsigned requests,unsigned pipeline,unsigned bytes,const std::string& workload,const fs::path& scratch){
    require(clients>=1&&clients<=32&&requests>=1&&requests<=4096&&pipeline>=1&&pipeline<=8&&bytes==0,"bounded Application workload required");
    require(workload=="app_profile"||workload=="app_profile_encrypted"||workload=="app_mixed_encrypted"||workload=="app_achievement_encrypted","unknown Application workload");
    require(!fs::exists(scratch)&&fs::create_directory(scratch),"scratch must be a new directory");write_config(scratch,clients);
    // Child-local startup policy prevents console rebinding or inherited LSX
    // tracing. Application Options confine config, avatars and files to scratch.
    require(SetEnvironmentVariableA("OriginEmuCmdArgs","-noConsole")!=0,"isolated console policy");
    application::Application app({(scratch/"host.exe").u16string(),(scratch/"data").u16string(),false,(scratch/"program").u16string()});
    const auto starting=Clock::now();app.start();const auto startup=ns(Clock::now()-starting);
    require(app.configuration().content.size()==8,"fixture content count");
    std::set<std::string> seen;
    for(const auto& content:app.configuration().content){
        require(number(content.id)<8&&seen.insert(content.id).second&&content.name=="Load content"&&content.version=="1"&&content.state=="INSTALLED","fixture content values");
        content_order.push_back(content.id);
    }
    const auto port=app.configuration().server_port;
    // Consume and verify the one shared DOWN event before timing any peers.
    {Peer seed(port,999,true);seed.send(seed.encode(envelope("seed",command(0))));
        verify_xml(seed.receive(),envelope("seed",expected_body(0,clients),true));
        verify_xml(seed.receive(),event_queue::initial_down_event);}
    const bool encrypted=workload!="app_profile",grant=workload=="app_achievement_encrypted";
    std::vector<std::unique_ptr<Peer>> peers;
    for(unsigned i=0;i<clients;++i){auto peer=std::make_unique<Peer>(port,i,encrypted);
        for(unsigned j=0;j<4;++j){peer->send(peer->encode(request(workload,i,j)));verify_response(peer->receive(),i,j,command_for(workload,j),clients,grant);}
        peers.push_back(std::move(peer));}
    benchmark::ApplicationMeasurements measured(clients*requests);
    std::vector<std::vector<std::uint64_t>> timings(clients);
    std::vector<std::exception_ptr> failures(clients);std::vector<std::thread> threads;
    std::latch arrived{clients},go{1};
    struct Join {std::latch& go;std::vector<std::thread>& threads;bool released=false;
        void release(){if(!released){released=true;go.count_down();}}
        ~Join(){release();for(auto& thread:threads)if(thread.joinable())thread.join();benchmark::application_measurements=nullptr;}
    } join{go,threads};
    for(unsigned i=0;i<clients;++i)threads.emplace_back([&,i]{arrived.count_down();go.wait();try{
        timings[i].reserve(requests);
        for(unsigned sent=0;sent<requests;){
            const auto count=std::min(pipeline,requests-sent);std::string batch;
            for(unsigned j=0;j<count;++j)batch+=peers[i]->encode(request(workload,i,4+sent+j));
            const auto start=Clock::now();peers[i]->send(batch);
            for(unsigned j=0;j<count;++j){auto response=peers[i]->receive();const auto elapsed=ns(Clock::now()-start);
                verify_response(response,i,4+sent+j,command_for(workload,4+sent+j),clients,grant);timings[i].push_back(elapsed);}
            sent+=count;
        }
    }catch(...){failures[i]=std::current_exception();}});
    arrived.wait();const auto before_created=benchmark::created_message_threads.load();
    benchmark::application_measurements=&measured;
    const auto before_cpu=cpu(),before_cycles=cycles();const auto start=Clock::now();join.release();
    for(auto& thread:threads)thread.join();const auto elapsed=ns(Clock::now()-start),used_cycles=cycles()-before_cycles,used_cpu=cpu()-before_cpu;
    benchmark::application_measurements=nullptr;
    for(auto& failure:failures)if(failure)std::rethrow_exception(failure);
    const auto created=benchmark::created_message_threads.load()-before_created;
    require(measured.used==clients*requests&&measured.errors==0&&measured.active==0,"missing or failed Application handler");
    require(measured.peak<=std::min(clients,RUNTIME_BENCHMARK_WORKERS?unsigned(RUNTIME_BENCHMARK_WORKERS):clients),"Application worker bound");
    require(created==(RUNTIME_BENCHMARK_WORKERS?0u:clients*requests),"Application message thread count");
    PROCESS_MEMORY_COUNTERS_EX memory{};memory.cb=sizeof(memory);require(GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),sizeof(memory))!=0,"process memory");
    peers.clear();const auto stopping=Clock::now();app.stop();const auto shutdown=ns(Clock::now()-stopping);
    if(grant){
        const auto* fault=std::getenv("RUNTIME_BENCHMARK_FAULT");
        if(fault&&std::string_view(fault)=="corrupt-persistence"){
            std::ofstream damaged(scratch/"data/achievement-load-set.xml",std::ios::binary);damaged<<"<Achievements/>";
        }
        if(fault&&std::string_view(fault)=="corrupt-ini"){
            std::ofstream damaged(scratch/"data/achievement_watcher/12345/achievements.ini",std::ios::binary);damaged<<"[stale]\r\nCurProgress=0\r\n";
        }
        try{verify_persistence(scratch,app.configuration(),clients,4+requests);}
        catch(const std::exception&){throw std::runtime_error("Application persistence validation failed");}
    }
    std::vector<std::uint64_t> rtt,queue,handler,wait,hold,calls;std::set<std::thread::id> ids;unsigned lock_calls=0;
    for(const auto& timing:timings)rtt.insert(rtt.end(),timing.begin(),timing.end());
    for(const auto& record:measured.records){
        require(record.lock_calls==1,"each Application request must observe exactly one handler lock");
        queue.push_back(record.queue);handler.push_back(record.handler);wait.push_back(record.lock_wait);hold.push_back(record.lock_hold);calls.push_back(record.lock_calls);ids.insert(record.thread);lock_calls+=record.lock_calls;
    }
    require(lock_calls==clients*requests,"missing actual handler lock observations");
    std::cout<<"{\"profile\":\""<<(RUNTIME_BENCHMARK_WORKERS?"pool-"+std::to_string(RUNTIME_BENCHMARK_WORKERS):"thread-per-message")
        <<"\",\"created_message_threads\":"<<created<<",\"workers\":"<<RUNTIME_BENCHMARK_WORKERS<<",\"clients\":"<<clients<<",\"requests_per_client\":"<<requests<<",\"pipeline\":"<<pipeline
        <<",\"payload_bytes\":"<<bytes<<",\"workload\":\""<<workload<<"\",\"completed\":"<<rtt.size()<<",\"verified_responses\":"<<rtt.size()<<",\"errors\":0,\"application_routed\":true,\"persistence_verified\":"<<(grant?"true":"false")
        <<",\"handler_lock_calls\":"<<lock_calls<<",\"elapsed_ns\":"<<elapsed<<",\"cpu_ns\":"<<used_cpu<<",\"cpu_cycles\":"<<used_cycles<<",\"startup_ns\":"<<startup<<",\"shutdown_ns\":"<<shutdown
        <<",\"peak_working_set_bytes\":"<<memory.PeakWorkingSetSize<<",\"private_bytes\":"<<memory.PrivateUsage<<",\"peak_active_handlers\":"<<measured.peak<<",\"observed_worker_threads\":"<<ids.size();
    values("rtt_ns",rtt);values("queue_ns",queue);values("handler_ns",handler);values("lock_wait_ns",wait);values("lock_hold_ns",hold);values("lock_calls",calls);std::cout<<"}\n";
}
}
int main(int argc,char** argv)try{
    std::cout.imbue(std::locale::classic());
    if(argc==2&&std::string_view(argv[1])=="--verify-controls"){response_controls();return 0;}
    require(argc==8,"usage: clients requests pipeline payload workload scratch expected-workers");
    require(number(argv[7])==RUNTIME_BENCHMARK_WORKERS,"wrong Application candidate");
    run(number(argv[1]),number(argv[2]),number(argv[3]),number(argv[4]),argv[5],fs::path(argv[6]));return 0;
}catch(const std::exception& error){std::cerr<<"BENCHMARK FAILED: "<<error.what()<<'\n';return 1;}
