// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/entitlement_response.hpp"
#include "redfield/config.hpp"
#include "redfield/runtime/record_views.hpp"
#include "stream_response_support.hpp"
#include "../../third_party/tinyxml2/tinyxml2.h"
namespace redfield { namespace entitlement_detail { namespace {
struct StreamWriter: redfield::StreamWriter {
    using redfield::StreamWriter::StreamWriter;
    void append_semantic(redfield::runtime::ServiceTextView value) noexcept {redfield::StreamWriter::append_semantic({value.data,value.size});}
    bool finish(size_t* written) noexcept {
        if(!out){if(written)*written=pos;return valid && written;}
        return redfield::StreamWriter::finish(written);
    }
};
size_t u64_text(uint64_t v,char* out) noexcept {
    char tmp[32]{}; size_t n=0; do{tmp[n++]=static_cast<char>('0'+v%10u);v/=10u;}while(v);
    for(size_t i=0;i<n;++i)out[i]=tmp[n-1u-i];
    out[n]='\0';
    return n;
}
bool safe_service_text(redfield::runtime::ServiceTextView v) noexcept {
    return safe_semantic_attr_value(redfield::xml::TextSpan{v.data,v.size});
}
bool source_equals_service(redfield::xml::TextSpan source,redfield::runtime::ServiceTextView target,bool* malformed) noexcept {
    if(malformed)*malformed=false;
    redfield::xml::ValueCodecStatus status=redfield::xml::ValueCodecStatus::ok;
    const bool match=redfield::xml::source_value_equals(source,{target.data,target.size},&status);
    if(status!=redfield::xml::ValueCodecStatus::ok){if(malformed)*malformed=true;return false;}
    return match;
}

bool validate_source_span(redfield::xml::TextSpan source) noexcept {
    redfield::xml::SemanticValue value{};
    return value.assign_parsed(source)==redfield::xml::ValueCodecStatus::ok;
}
bool validate_entitlement_filters(const redfield::xml::RequestView& r) noexcept {
    const char* attrs[]={"OfferId","ItemId","Group"};
    for(const char* name:attrs){
        redfield::xml::TextSpan value{};
        const auto st=redfield::xml::find_attribute(r.command_open_tag,name,&value);
        if(st==redfield::xml::AttributeStatus::malformed||st==redfield::xml::AttributeStatus::invalid_argument) return false;
        if(st==redfield::xml::AttributeStatus::found&&!validate_source_span(value)) return false;
    }
    size_t pos=0;
    for(;;){
        redfield::xml::DirectChildView child{};
        const auto st=redfield::xml::next_direct_child(r.command_content,&pos,&child);
        if(st==redfield::xml::DirectChildStatus::end) break;
        if(st!=redfield::xml::DirectChildStatus::found) return false;
        const bool filter=redfield::xml::span_equals(child.name,"FilterOffers")||
                          redfield::xml::span_equals(child.name,"FilterGroups")||
                          redfield::xml::span_equals(child.name,"FilterItems");
        if(!filter) continue;
        const bool nested=child.content.element && child.content.element->FirstChildElement();
        if(!nested&&!validate_source_span(child.content)) return false;
    }
    return true;
}
struct FilterProbe { bool active; bool match; bool malformed; };
FilterProbe entitlement_filter_probe(const redfield::xml::RequestView& r,const char* child_name,
                                     redfield::runtime::ServiceTextView target,
                                     const char* direct_a,const char* direct_b=nullptr) noexcept {
    FilterProbe out{false,false,false};
    redfield::xml::TextSpan av{};
    if(direct_a){const auto st=redfield::xml::find_attribute(r.command_open_tag,direct_a,&av);if(st==redfield::xml::AttributeStatus::malformed||st==redfield::xml::AttributeStatus::invalid_argument){out.malformed=true;return out;}if(st==redfield::xml::AttributeStatus::found){out.active=true;bool malformed=false;if(source_equals_service(av,target,&malformed))out.match=true;if(malformed){out.malformed=true;return out;}}}
    if(direct_b){const auto st=redfield::xml::find_attribute(r.command_open_tag,direct_b,&av);if(st==redfield::xml::AttributeStatus::malformed||st==redfield::xml::AttributeStatus::invalid_argument){out.malformed=true;return out;}if(st==redfield::xml::AttributeStatus::found){out.active=true;bool malformed=false;if(source_equals_service(av,target,&malformed))out.match=true;if(malformed){out.malformed=true;return out;}}}
    size_t pos=0;
    for(;;){
        redfield::xml::DirectChildView child{};
        const auto st=redfield::xml::next_direct_child(r.command_content,&pos,&child);
        if(st==redfield::xml::DirectChildStatus::end)break;
        if(st!=redfield::xml::DirectChildStatus::found){out.malformed=true;return out;}
        if(!redfield::xml::span_equals(child.name,child_name))continue;
        // Behavioral grammar consumes the text of the direct Filter* child itself.
        // A nested element is not a filter item and is therefore ignored here.
        const bool nested=child.content.element && child.content.element->FirstChildElement();
        if(nested)continue;
        out.active=true;
        bool malformed=false;if(source_equals_service(child.content,target,&malformed))out.match=true;if(malformed){out.malformed=true;return out;}
    }
    return out;
}
bool include_entitlement(const redfield::xml::RequestView& r,const redfield::runtime::EntitlementRecordView& e,bool* malformed) noexcept {
    if(malformed)*malformed=false;
    const auto offers=entitlement_filter_probe(r,"FilterOffers",e.item_id,"OfferId","ItemId");
    const auto groups=entitlement_filter_probe(r,"FilterGroups",e.group,"Group");
    const auto items=entitlement_filter_probe(r,"FilterItems",e.entitlement_tag,nullptr);
    if(offers.malformed||groups.malformed||items.malformed){if(malformed)*malformed=true;return false;}
    return (!offers.active||offers.match)&&(!groups.active||groups.match)&&(!items.active||items.match);
}
class EntitlementView final {
    const ConfigSnapshot& snapshot_;
    size_t entitlement_record_count_locked() const noexcept {return snapshot_.entitlements.size();}
    bool entitlement_record_at_locked(size_t i,redfield::runtime::EntitlementRecordView* out) const noexcept {
        if(!out || i>=snapshot_.entitlements.size())return false;
        const auto& r=snapshot_.entitlements[i];
        *out={{r.item_id.data(),r.item_id.size()},{r.group.data(),r.group.size()},{r.version.data(),r.version.size()},{r.type.data(),r.type.size()},{r.entitlement_tag.data(),r.entitlement_tag.size()}};
        return true;
    }
public:
    explicit EntitlementView(const ConfigSnapshot& snapshot) noexcept:snapshot_(snapshot){}
    HandlerStatus handle_query_entitlements(const redfield::xml::RequestView&,char*,size_t,size_t*) noexcept;
};
HandlerStatus EntitlementView::handle_query_entitlements(const redfield::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"QueryEntitlements")){if(written)*written=0;return HandlerStatus::wrong_command;}
    if(written)*written=0;
    if(!written||(!out&&cap)||!safe_attr(r.id)||!safe_attr(redfield::xml::response_sender_policy(r))||!validate_entitlement_filters(r))return HandlerStatus::invalid_request;
    StreamWriter w(out,cap);w.append(" <LSX>\n  <Response id=\"");w.append_source(r.id);w.append("\" sender=\"");if(r.recipient.data&&r.recipient.size)w.append_source(r.recipient);else w.append("EbisuSDK");w.append("\">\n    <QueryEntitlementsResponse");
    const size_t count=entitlement_record_count_locked();
    // Phase 225 exact behavioral counter: base is 1,000,000,000,000 and the
    // counter advances once per policy record before filter evaluation.
    // Therefore the first record is 1000000000001 and filtered records still
    // consume their sequence position. EntitlementId is never record-owned.
    bool opened=false;uint64_t sequence=UINT64_C(1000000000000);
    static constexpr char zero_date[]="0000-00-00T00:00:00";
    for(size_t i=0;i<count;++i){
        redfield::runtime::EntitlementRecordView e{};if(!entitlement_record_at_locked(i,&e))return HandlerStatus::response_error;
        const redfield::runtime::ServiceTextView values[]={e.item_id,e.type,e.entitlement_tag,e.group,e.version};for(const auto& v:values)if(!safe_service_text(v))return HandlerStatus::response_error;
        ++sequence;
        bool malformed=false;if(!include_entitlement(r,e,&malformed)){if(malformed)return HandlerStatus::invalid_request;continue;}
        if(!opened){w.append(">\n");opened=true;}
        char synthetic[32]{};const size_t eid_n=u64_text(sequence,synthetic);
        w.append("      <Entitlement ItemId=\"");w.append_semantic(e.item_id);w.append("\" Type=\"");w.append_semantic(e.type);w.append("\" EntitlementId=\"");w.append(synthetic,eid_n);w.append("\" EntitlementTag=\"");w.append_semantic(e.entitlement_tag);w.append("\" Group=\"");w.append_semantic(e.group);w.append("\" ResourceId=\"\" GrantDate=\"");w.append(zero_date,sizeof(zero_date)-1u);w.append("\" Expiration=\"");w.append(zero_date,sizeof(zero_date)-1u);w.append("\" LastModifiedDate=\"");w.append(zero_date,sizeof(zero_date)-1u);w.append("\" Version=\"");w.append_semantic(e.version);w.append("\" UseCount=\"0\"/>\n");
    }
    if(!opened)w.append("/>\n  </Response>\n </LSX>\n");else w.append("    </QueryEntitlementsResponse>\n  </Response>\n </LSX>\n");
    return w.finish(written)?HandlerStatus::ok:HandlerStatus::response_error;
}
} } }
namespace redfield {
ResponseResult dispatch_entitlement_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;if(!output || !capacity)return ResponseResult::rejected;
    entitlement_detail::EntitlementView view(snapshot);
    std::size_t measured=0;
    const auto measure=view.handle_query_entitlements(request,nullptr,0,&measured);
    if(measure==HandlerStatus::wrong_command || measure==HandlerStatus::unsupported)return ResponseResult::no_response;
    if(measure!=HandlerStatus::ok || measured>=capacity)return ResponseResult::rejected;
    std::size_t actual=0;
    const auto render=view.handle_query_entitlements(request,output,capacity,&actual);
    if(render!=HandlerStatus::ok || actual!=measured || actual>=capacity)return ResponseResult::rejected;
    *written=actual;return ResponseResult::response;
}
}
