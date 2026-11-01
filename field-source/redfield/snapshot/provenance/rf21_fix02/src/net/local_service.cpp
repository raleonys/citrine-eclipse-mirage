// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/local_service.hpp"
#include "redfield/response_dispatch.hpp"
#include "redfield/achievement_delivery.hpp"
#include "redfield/achievement_response.hpp"
#include "preloader_l/runtime/utc_time.hpp"
namespace redfield {
namespace {
class GrantFrame final: public FrameDelivery {
public:
    PreparedAchievementDelivery prepared;
    std::string_view response() const noexcept override{return prepared.response();}
    std::string_view completion_event() const noexcept override{return prepared.completion_event();}
    bool commit() noexcept override{return prepared.commit();}
};
}
bool LocalService::start(std::uint16_t port) {
    if(session_.start()!=StartResult::started)return false;
    if(achievements_.initialize(session_.configuration())!=AchievementSession::InitializeResult::initialized)return false;
    if(transport_.start({this,nullptr,nullptr,frame},port)!=TcpStartResult::started)return false;
    return publication_.publish(transport_.snapshot().port);
}
bool LocalService::stop_for(std::chrono::milliseconds budget) {
    if(!publication_.restore())return false;
    if(!transport_.stop_for(budget))return false;
    achievements_.close();
    auth_code_.close();
    return session_.stop(std::chrono::milliseconds(0))==StopResult::stopped;
}
bool LocalService::frame(void* context,std::string_view source,char* output,std::size_t capacity,std::size_t* written,std::unique_ptr<FrameDelivery>& delivery) noexcept {
    try {
        auto& self=*static_cast<LocalService*>(context);
        const auto configuration=self.session_.configuration();
        if(!configuration)return false;
        xml::Request request;
        if(self.session_.parse_request(source,request)!=SessionParseResult::parsed)return false;
        if(preloader_l::xml::span_equals(request.command,"GrantAchievement")){
            char timestamp[20];std::size_t length=0;
            if(!preloader_l::runtime::current_utc_timestamp(timestamp,sizeof(timestamp),&length))return false;
            auto grant=std::make_unique<GrantFrame>();
            const auto result=prepare_achievement_delivery(self.achievements_,request,{timestamp,length},{},grant->prepared);
            if(result==GrantDeliveryResult::no_response)return true;
            if(result!=GrantDeliveryResult::prepared)return false;
            delivery=std::move(grant);return true;
        }
        if(preloader_l::xml::span_equals(request.command,"QueryAchievements")){
            const auto committed=self.achievements_.snapshot();
            return committed && dispatch_achievement_response(request,*committed,output,capacity,written)!=ResponseResult::rejected;
        }
        return dispatch_response_with_session(request,*configuration,self.auth_code_,output,capacity,written)!=ResponseResult::rejected;
    }catch(...){return false;}
}
}
