// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/achievement_delivery.hpp"
#include "redfield/achievement_response.hpp"
#include "redfield/tcp_ingress.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <iostream>
using namespace redfield;using asio::ip::tcp;using namespace std::chrono_literals;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
const std::string grant="<LSX><Request id='g'><GrantAchievement><AchievementId>A</AchievementId></GrantAchievement></Request></LSX>";
struct Delivery final:FrameDelivery {
    PreparedAchievementDelivery prepared;
    std::string_view response() const noexcept override{return prepared.response();}
    std::string_view completion_event() const noexcept override{return prepared.completion_event();}
    bool commit() noexcept override{return prepared.commit();}
};
struct Fixture {
    std::shared_ptr<ConfigSnapshot> initial=std::make_shared<ConfigSnapshot>();AchievementSession owner;TcpIngress server;asio::io_context io;tcp::socket client{io};std::string received;
    Fixture(){initial->achievements.push_back({"A",std::string(9000,'N'),"","","","0000-00-00T00:00:00","0000-00-00T00:00:00",1,0,0});}
    static bool prepare(void* p,std::string_view input,char* out,std::size_t cap,std::size_t* n,std::unique_ptr<FrameDelivery>& delivery) noexcept {
        try{auto& f=*static_cast<Fixture*>(p);xml::Request r;if(xml::parse(input,r)!=xml::Result::ok)return false;
            if(preloader_l::xml::span_equals(r.command,"QueryAchievements")){const auto snapshot=f.owner.snapshot();return snapshot && dispatch_achievement_response(r,*snapshot,out,cap,n)!=ResponseResult::rejected;}
            auto d=std::make_unique<Delivery>();const auto result=prepare_achievement_delivery(f.owner,r,"2026-10-20T12:00:00",{},d->prepared);
            if(result==GrantDeliveryResult::no_response)return true;
            if(result!=GrantDeliveryResult::prepared)return false;
            delivery=std::move(d);return true;
        }catch(...){return false;}
    }
    bool start(){if(owner.initialize(initial)!=AchievementSession::InitializeResult::initialized || server.start({this,nullptr,nullptr,prepare})!=TcpStartResult::started)return false;client.connect({asio::ip::address_v4::loopback(),server.snapshot().port});client.non_blocking(true);return true;}
    template<class P>bool until(P p){const auto end=std::chrono::steady_clock::now()+3s;while(!p() && std::chrono::steady_clock::now()<end){server.poll_for(0ms);char b[4096];asio::error_code e;const auto n=client.read_some(asio::buffer(b),e);received.append(b,n);}return p();}
    bool stop(){for(unsigned i=0;i<3000;++i)if(server.stop_for(1ms))return true;return false;}
    ~Fixture(){stop();}
};
int main(){
    {Fixture f;CHECK(f.start());asio::write(f.client,asio::buffer(grant));CHECK(f.until([&]{return f.server.snapshot().bytes_written==4096;}));CHECK(f.owner.snapshot()==f.initial);CHECK(f.stop());CHECK(f.owner.snapshot()==f.initial && f.server.snapshot().committed_deliveries==0 && f.server.snapshot().events==0);AchievementSession::PreparedGrant retry;CHECK(f.owner.prepare("A",1,"retry",retry)==AchievementSession::PrepareResult::prepared);retry.rollback();}
    {Fixture f;CHECK(f.start());asio::write(f.client,asio::buffer(grant));CHECK(f.until([&]{return f.server.snapshot().committed_deliveries==1;}));const auto committed=f.owner.snapshot();CHECK(committed!=f.initial && committed->achievements[0].count==1 && f.server.snapshot().events==0);CHECK(f.stop());CHECK(f.owner.snapshot()==committed && f.server.snapshot().events==0 && committed->achievements[0].progress==1 && f.initial->achievements[0].count==0);}
    {Fixture f;CHECK(f.start());asio::write(f.client,asio::buffer(grant));CHECK(f.until([&]{return f.server.snapshot().bytes_written==4096;}));f.owner.close();CHECK(f.until([&]{return f.server.snapshot().completed==1;}));CHECK(f.server.snapshot().responses==1 && f.server.snapshot().committed_deliveries==0 && f.server.snapshot().events==0 && f.server.snapshot().failures==1 && !f.owner.snapshot());}
    {Fixture f;f.initial->achievements[0].name.assign(20000,'N');CHECK(f.start());asio::write(f.client,asio::buffer(grant));CHECK(f.until([&]{return f.server.snapshot().completed==1;}));CHECK(f.received.empty() && f.owner.snapshot()==f.initial && f.server.snapshot().malformed==1 && f.server.snapshot().bytes_written==0);}
    {Fixture f;CHECK(f.start());AchievementSession::PreparedGrant held;CHECK(f.owner.prepare("A",1,"held",held)==AchievementSession::PrepareResult::prepared);asio::write(f.client,asio::buffer(grant));CHECK(f.until([&]{return f.server.snapshot().completed==1;}));CHECK(f.received.empty() && f.owner.snapshot()==f.initial && f.server.snapshot().malformed==1);held.rollback();CHECK(f.owner.prepare("A",1,"retry",held)==AchievementSession::PrepareResult::prepared);}
    {Fixture f;CHECK(f.start());const std::string input="<LSX><Request id='s'><GrantAchievement/></Request></LSX><LSX><Request id='s'><GrantAchievement><AchievementId>A</AchievementId><Progress>bad</Progress></GrantAchievement></Request></LSX><LSX><Request id='q'><QueryAchievements/></Request></LSX>";asio::write(f.client,asio::buffer(input));asio::error_code e;f.client.shutdown(tcp::socket::shutdown_send,e);CHECK(f.until([&]{return f.server.snapshot().completed==1;}));CHECK(f.server.snapshot().frames==3 && f.server.snapshot().responses==1 && f.server.snapshot().committed_deliveries==0 && f.owner.snapshot()==f.initial && f.server.snapshot().malformed==0 && f.received.find("<Achievement Id=")==std::string::npos);}
    std::cout<<"TCP_REAL_GRANT_PARTIAL_CANCEL_COMMIT_EVENT_CANCEL_OWNER_CLOSE_LIMIT_BUSY_PASS\n";
}
