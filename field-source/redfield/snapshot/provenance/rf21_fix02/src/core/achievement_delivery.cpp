// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/achievement_delivery.hpp"
#include "preloader_l/runtime/service_state.hpp"
#include "stream_response_support.hpp"
#include <utility>
namespace redfield { namespace grant_wire { namespace {
struct StreamWriter: redfield::StreamWriter {
    using redfield::StreamWriter::StreamWriter;
    void append_semantic(preloader_l::runtime::ServiceTextView v) noexcept {redfield::StreamWriter::append_semantic({v.data,v.size});}
    bool finish(size_t* written) noexcept {
        if(!out){if(written)*written=pos;return valid && written;}
        return redfield::StreamWriter::finish(written);
    }
};
Span semantic_span(preloader_l::runtime::ServiceTextView v) noexcept { return Span{v.data,v.size,SpanEncoding::semantic_attribute}; }
HandlerStatus format_parts(const preloader_l::xml::RequestView& r,const Span* parts,size_t count,char* out,size_t cap,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!out&&cap))return HandlerStatus::invalid_request;
    if(!safe_attr(r.id)||!safe_attr(preloader_l::xml::response_sender_policy(r)))return HandlerStatus::invalid_request;
    size_t n=0;
    for(size_t i=0;i<count;++i){
        size_t part_size=parts[i].size;
        if(parts[i].encoding!=SpanEncoding::raw){
            const preloader_l::xml::TextSpan value{parts[i].data,parts[i].size};
            const auto st=parts[i].encoding==SpanEncoding::semantic_attribute
                ? preloader_l::xml::escape_attribute_value(value,nullptr,0,&part_size)
                : preloader_l::xml::normalize_source_attribute_value(value,nullptr,0,&part_size);
            if(st!=preloader_l::xml::ValueCodecStatus::ok)
                return parts[i].encoding==SpanEncoding::source_attribute ? HandlerStatus::invalid_request : HandlerStatus::response_error;
        }
        if(!add(n,part_size))return HandlerStatus::response_error;
    }
    *written=n;
    if(!out && cap==0)return HandlerStatus::ok; // Validated measure only.
    size_t with0=0; if(!add(with0,n) || !add(with0,1) || !out || cap<with0) return HandlerStatus::response_error;
    char* d=out;
    for(size_t i=0;i<count;++i){
        if(parts[i].encoding==SpanEncoding::raw){ cp(d,parts[i].data,parts[i].size); continue; }
        size_t encoded=0;
        const preloader_l::xml::TextSpan value{parts[i].data,parts[i].size};
        const auto st=parts[i].encoding==SpanEncoding::semantic_attribute
            ? preloader_l::xml::escape_attribute_value(value,d,cap-static_cast<size_t>(d-out),&encoded)
            : preloader_l::xml::normalize_source_attribute_value(value,d,cap-static_cast<size_t>(d-out),&encoded);
        if(st!=preloader_l::xml::ValueCodecStatus::ok)
            return parts[i].encoding==SpanEncoding::source_attribute ? HandlerStatus::invalid_request : HandlerStatus::response_error;
        d+=encoded;
    }
    *d='\0'; *written=n; return HandlerStatus::ok;
}
size_t i32_text(int32_t v,char* out) noexcept {
    uint32_t mag=v<0?static_cast<uint32_t>(-(int64_t)v):static_cast<uint32_t>(v); char tmp[16]{}; size_t n=0;
    do{tmp[n++]=static_cast<char>('0'+mag%10u);mag/=10u;}while(mag);
    size_t pos=0;if(v<0)out[pos++]='-';for(size_t i=0;i<n;++i)out[pos+i]=tmp[n-1u-i];pos+=n;out[pos]='\0';return pos;
}
bool safe_service_text(preloader_l::runtime::ServiceTextView v) noexcept {
    return safe_semantic_attr_value(preloader_l::xml::TextSpan{v.data,v.size});
}
bool parse_i32(preloader_l::xml::TextSpan v,int32_t* out) noexcept {
    if(!out||!v.data||!v.size)return false;
    size_t i=0; bool neg=false;
    if(v.data[0]=='-'||v.data[0]=='+'){ neg=v.data[0]=='-'; if(++i==v.size)return false; }
    uint32_t x=0;const uint32_t limit=neg?UINT32_C(2147483648):UINT32_C(2147483647);
    for(;i<v.size;++i){const unsigned char c=static_cast<unsigned char>(v.data[i]);if(c<'0'||c>'9')return false;const uint32_t d=c-'0';if(x>(limit-d)/10u)return false;x=x*10u+d;}
    *out=neg?(x==UINT32_C(2147483648)?INT32_MIN:-static_cast<int32_t>(x)):static_cast<int32_t>(x);return true;
}
bool build_completion_event(const preloader_l::runtime::AchievementRecordView& a,
                            char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||!safe_service_text(a.id)||!safe_service_text(a.name)||
       !safe_service_text(a.description)||!safe_service_text(a.how_to)||
       !safe_service_text(a.image_id)||!safe_service_text(a.grant_date)||
       !safe_service_text(a.expiration))return false;
    char total[32]{},count[32]{},progress[32]{};
    const size_t tn=i32_text(a.total,total),cn=i32_text(a.count,count),pn=i32_text(a.progress,progress);
    StreamWriter w(output,capacity);
    w.append("<LSX><Event sender=\"EbisuSDK\"><Achievement Id=\"");w.append_semantic(a.id);
    w.append("\" Total=\"");w.append(total,tn);w.append("\" Count=\"");w.append(count,cn);
    w.append("\" Progress=\"");w.append(progress,pn);w.append("\" GrantDate=\"");w.append_semantic(a.grant_date);
    w.append("\" Expiration=\"");w.append_semantic(a.expiration);w.append("\" ImageId=\"");w.append_semantic(a.image_id);
    w.append("\" Name=\"");w.append_semantic(a.name);w.append("\" Description=\"");w.append_semantic(a.description);
    w.append("\" HowTo=\"");w.append_semantic(a.how_to);w.append("\"/></Event></LSX>");
    return w.finish(written);
}

HandlerStatus format_grant_achievement_record(const preloader_l::xml::RequestView& r,
                                              const preloader_l::runtime::AchievementRecordView& rec,
                                              char* out,size_t cap,size_t* written) noexcept {
    if(!safe_service_text(rec.id)||!safe_service_text(rec.name)||!safe_service_text(rec.description)||
       !safe_service_text(rec.how_to)||!safe_service_text(rec.image_id)||!safe_service_text(rec.grant_date)||
       !safe_service_text(rec.expiration)){if(written)*written=0;return HandlerStatus::response_error;}
    char total[32]{},count[32]{},progress[32]{};
    const size_t tn=i32_text(rec.total,total),cn=i32_text(rec.count,count),pn=i32_text(rec.progress,progress);
    const Span parts[]={lit(" <LSX>\n  <Achievement Id=\""),semantic_span(rec.id),lit("\" Total=\""),Span{total,tn},lit("\" Count=\""),Span{count,cn},lit("\" Progress=\""),Span{progress,pn},lit("\" GrantDate=\""),semantic_span(rec.grant_date),lit("\" Expiration=\""),semantic_span(rec.expiration),lit("\" ImageId=\""),semantic_span(rec.image_id),lit("\" Name=\""),semantic_span(rec.name),lit("\" Description=\""),semantic_span(rec.description),lit("\" HowTo=\""),semantic_span(rec.how_to),lit("\"/>\n </LSX>\n")};
    return grant_wire::format_parts(r,parts,sizeof(parts)/sizeof(parts[0]),out,cap,written);
}

HandlerStatus select(const preloader_l::xml::RequestView& r,preloader_l::xml::SemanticValue& semantic_id,int32_t& increment,size_t* written) noexcept {
    preloader_l::xml::TextSpan id{}; preloader_l::xml::TextSpan progress_text{};
    const auto id_status=preloader_l::xml::find_direct_child_text(r.command_content,"AchievementId",&id);
    if(id_status==preloader_l::xml::ChildTextStatus::malformed||id_status==preloader_l::xml::ChildTextStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(id_status!=preloader_l::xml::ChildTextStatus::found){if(written)*written=0;return HandlerStatus::unsupported;}
    const auto id_decode=semantic_id.assign_parsed(id);
    if(id_decode==preloader_l::xml::ValueCodecStatus::malformed_entity||id_decode==preloader_l::xml::ValueCodecStatus::invalid_character||id_decode==preloader_l::xml::ValueCodecStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(id_decode!=preloader_l::xml::ValueCodecStatus::ok){if(written)*written=0;return HandlerStatus::response_error;}
    increment=1;
    const auto progress_status=preloader_l::xml::find_direct_child_text(r.command_content,"Progress",&progress_text);
    if(progress_status==preloader_l::xml::ChildTextStatus::malformed||progress_status==preloader_l::xml::ChildTextStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    preloader_l::xml::SemanticValue semantic_progress;
    if(progress_status==preloader_l::xml::ChildTextStatus::found){
        const auto progress_decode=semantic_progress.assign_parsed(progress_text);
        if(progress_decode==preloader_l::xml::ValueCodecStatus::malformed_entity||progress_decode==preloader_l::xml::ValueCodecStatus::invalid_character||progress_decode==preloader_l::xml::ValueCodecStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
        if(progress_decode!=preloader_l::xml::ValueCodecStatus::ok){if(written)*written=0;return HandlerStatus::response_error;}
        if(!parse_i32(semantic_progress.view(),&increment)){if(written)*written=0;return HandlerStatus::unsupported;}
    }

    return HandlerStatus::ok;
}
preloader_l::runtime::AchievementRecordView record_view(const AchievementRecord& a) noexcept {
    return {{a.id.data(),a.id.size()},{a.name.data(),a.name.size()},{a.description.data(),a.description.size()},{a.how_to.data(),a.how_to.size()},{a.image_id.data(),a.image_id.size()},{a.grant_date.data(),a.grant_date.size()},{a.expiration.data(),a.expiration.size()},a.total,a.count,a.progress};
}
} } }
namespace redfield {
PreparedAchievementDelivery::PreparedAchievementDelivery(PreparedAchievementDelivery&& other) noexcept
    :transaction_(std::move(other.transaction_)),response_(std::move(other.response_)),event_(std::move(other.event_)) {
    other.response_.clear();other.event_.clear();
}
PreparedAchievementDelivery& PreparedAchievementDelivery::operator=(PreparedAchievementDelivery&& other) noexcept {
    if(this!=&other){transaction_=std::move(other.transaction_);response_=std::move(other.response_);event_=std::move(other.event_);other.response_.clear();other.event_.clear();}return *this;
}
bool PreparedAchievementDelivery::commit() noexcept {
    if(transaction_.commit())return true;
    response_.clear();event_.clear();return false;
}
void PreparedAchievementDelivery::rollback() noexcept {transaction_.rollback();response_.clear();event_.clear();}
GrantDeliveryResult prepare_achievement_delivery(AchievementSession& owner,const xml::Request& request,std::string_view timestamp,GrantWireBudget budget,PreparedAchievementDelivery& output) noexcept {
    if(output.active() || !output.response_.empty() || !output.event_.empty())return GrantDeliveryResult::rejected;
    if(!preloader_l::xml::span_equals(request.command,"GrantAchievement"))return GrantDeliveryResult::no_response;
    if(!safe_attr(request.id)||!safe_attr(preloader_l::xml::response_sender_policy(request)))return GrantDeliveryResult::rejected;
    if(budget.response_capacity>PreparedAchievementDelivery::max_frame_capacity || budget.event_capacity>PreparedAchievementDelivery::max_frame_capacity || budget.total_bytes>2*PreparedAchievementDelivery::max_frame_capacity)return GrantDeliveryResult::limit;
    try {
        preloader_l::xml::SemanticValue id;int32_t increment=1;size_t parsed=0;
        const auto selected=grant_wire::select(request,id,increment,&parsed);
        if(selected==HandlerStatus::unsupported || selected==HandlerStatus::wrong_command)return GrantDeliveryResult::no_response;
        if(selected!=HandlerStatus::ok)return GrantDeliveryResult::rejected;
        PreparedAchievementDelivery staged;const auto value=id.view();
        const auto prepared=owner.prepare({value.data,value.size},increment,timestamp,staged.transaction_);
        using P=AchievementSession::PrepareResult;
        if(prepared!=P::prepared){
            if(prepared==P::busy)return GrantDeliveryResult::busy;
            if(prepared==P::closed || prepared==P::not_initialized)return GrantDeliveryResult::not_ready;
            if(prepared==P::limit)return GrantDeliveryResult::limit;
            if(prepared==P::resource)return GrantDeliveryResult::resource;
            return GrantDeliveryResult::rejected;
        }
        const auto snapshot=staged.transaction_.projected();const auto index=staged.transaction_.index();
        if(!snapshot || index>=snapshot->achievements.size())return GrantDeliveryResult::rejected;
        const auto record=grant_wire::record_view(snapshot->achievements[index]);
        size_t response_n=0,event_n=0;
        if(grant_wire::format_grant_achievement_record(request,record,nullptr,0,&response_n)!=HandlerStatus::ok)return GrantDeliveryResult::rejected;
        if(staged.transaction_.completed() && !grant_wire::build_completion_event(record,nullptr,0,&event_n))return GrantDeliveryResult::rejected;
        if(response_n>=budget.response_capacity || (staged.transaction_.completed() && event_n>=budget.event_capacity))return GrantDeliveryResult::limit;
        size_t combined=0;if(!preloader_l::core::checked_add_size(response_n,event_n,combined) || combined>budget.total_bytes)return GrantDeliveryResult::limit;
        staged.response_.resize(response_n+1);size_t actual=0;
        if(grant_wire::format_grant_achievement_record(request,record,staged.response_.data(),staged.response_.size(),&actual)!=HandlerStatus::ok || actual!=response_n)return GrantDeliveryResult::rejected;
        staged.response_.resize(response_n);
        if(staged.transaction_.completed()){
            staged.event_.resize(event_n+1);actual=0;
            if(!grant_wire::build_completion_event(record,staged.event_.data(),staged.event_.size(),&actual) || actual!=event_n)return GrantDeliveryResult::rejected;
            staged.event_.resize(event_n);
        }
        output=std::move(staged);return GrantDeliveryResult::prepared;
    }catch(...){return GrantDeliveryResult::resource;}
}
}
