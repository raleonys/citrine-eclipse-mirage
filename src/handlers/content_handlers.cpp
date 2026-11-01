// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/handlers/content_handlers.hpp"
#include <bit>
#include <limits>

namespace preloader_l::content_handlers {
namespace {
class Guard {
    Mutex& mutex;
public:
    explicit Guard(Mutex& m):mutex(m) {
        if(mutex.lock_04a6ac()!=0) mutex.thread_error_04a648(5);
        if(mutex.count_4c()==std::numeric_limits<std::int32_t>::max()) {
            mutex.count_4c()=std::numeric_limits<std::int32_t>::max()-1;
            mutex.thread_error_04a648(6);
        }
    }
    ~Guard(){mutex.unlock_04a6b4();}
    Guard(const Guard&)=delete;
    Guard& operator=(const Guard&)=delete;
};
void open(Xml& x,const char* name){x.open_004080(name,false);}
void close(Xml& x){x.close_virtual_48(false);}
void attr(Xml& x,const char* key,std::string_view value) {
    const std::string owned(value);
    x.attribute_005560(key,owned.c_str());
}
void number(Xml& x,const char* key,std::int32_t value){attr(x,key,std::to_string(value));}
std::int32_t wrap_add(std::int32_t a,std::int32_t b) {
    return std::bit_cast<std::int32_t>(std::bit_cast<std::uint32_t>(a)+std::bit_cast<std::uint32_t>(b));
}
void achievement_persistence(Xml& x,StateReferences s) {
    open(x,"Achievements");
    attr(x,"GameName",s.game_0a3d88["DISPLAY_NAME"]);
    attr(x,"ContentId",s.content_id_0a3af8);
    for(const auto& [id,a]:s.achievements_0a3e58) {
        // Original tests signed progress != 0, including negative progress.
        if(a.field08==0) continue;
        open(x,"Achievement");attr(x,"Id",id);
        number(x,"Count",a.field04);number(x,"Progress",a.field08);
        attr(x,"Grant",a.timestamp);attr(x,"Name",a.display_name);close(x);
    }
    close(x);
}
}
const std::string* RequestTree::attribute(std::string_view key) const {
    for(const auto& [name,value]:attributes) if(name==key)return &value;
    return nullptr;
}
bool Filters::empty()const{return offers.empty()&&groups.empty()&&items.empty();}

Filters parse_entitlement_filters_02c6a0(const RequestTree& request,Effects& effects) {
    Filters result;
    for(const auto& [name,value]:request.attributes) {
        if(value.empty())continue;
        if(name=="UserId"||name=="includeChildGroups"||name=="includeExpiredTrialDLC"||name=="version")continue;
        // Preserve original ItemId -> first set (offers), rather than "fixing" it.
        if(name=="OfferId"||name=="ItemId")result.offers.insert(value);
        else if(name=="Group")result.groups.insert(value);
        else effects.unknown_entitlement_field("QueryEntitlements unknown attr: "+name+"|"+value);
    }
    for(const auto& child:request.children) {
        if(!child.name||!child.text||child.text->empty())continue;
        if(*child.name=="FilterOffers")result.offers.insert(*child.text);
        else if(*child.name=="FilterGroups")result.groups.insert(*child.text);
        else if(*child.name=="FilterItems")result.items.insert(*child.text);
        else effects.unknown_entitlement_field("QueryEntitlements unknown child: "+*child.name+"|"+*child.text);
    }
    return result;
}
bool query_content_030ee0(Xml& x,StateReferences s) {
    open(x,"QueryContentResponse");
    for(const auto& c:s.content_0a3d98) {
        open(x,"Game");attr(x,"progressValue","0");attr(x,"contentID",c.id);
        attr(x,"availableVersion",c.version);attr(x,"displayName",c.name);
        attr(x,"state",c.state);attr(x,"installedVersion",c.version);close(x);
    }
    close(x);return true;
}
bool query_entitlements_031030(const RequestTree& request,Xml& x,StateReferences s,Effects& effects) {
    const auto filters=parse_entitlement_filters_02c6a0(request,effects);
    open(x,"QueryEntitlementsResponse");
    std::uint64_t entitlement_id=1000000000000ULL;
    for(const auto& e:s.entitlements_0a3db0) {
        ++entitlement_id; // Counts ALL entries, including filtered-out entries.
        if(!filters.empty()&&!filters.offers.contains(e.id)&&!filters.groups.contains(e.group)&&!filters.items.contains(e.tag))continue;
        open(x,"Entitlement");attr(x,"ItemId",e.id);attr(x,"Type",e.type);
        attr(x,"EntitlementId",std::to_string(entitlement_id));attr(x,"EntitlementTag",e.tag);
        attr(x,"Group",e.group);attr(x,"ResourceId","");attr(x,"UseCount","0");
        attr(x,"Expiration","0000-00-00T00:00:00");attr(x,"GrantDate","0000-00-00T00:00:00");
        attr(x,"LastModifiedDate","0000-00-00T00:00:00");attr(x,"Version",e.version);close(x);
    }
    close(x);return true;
}
void achievement_set_header_02bf60(Xml& x,StateReferences s) {
    open(x,"AchievementSets");open(x,"AchievementSet");
    // map::operator[] inserts DISPLAY_NAME when absent in the original, too.
    attr(x,"GameName",s.game_0a3d88["DISPLAY_NAME"]);attr(x,"Name",s.achievements_set_0a3dc8);
}
void achievement_element_02c080(Xml& x,std::string_view id,const Achievement& a) {
    open(x,"Achievement");attr(x,"Id",id);number(x,"Total",a.field00);
    number(x,"Count",a.field04);number(x,"Progress",a.field08);attr(x,"GrantDate",a.timestamp);
    attr(x,"Expiration","0000-00-00T00:00:00");attr(x,"ImageId","");attr(x,"Name",a.display_name);
    attr(x,"Description","");attr(x,"HowTo","");close(x);
}
bool query_achievements_030d30(const RequestTree& request,Xml& x,StateReferences s) {
    Guard guard(s.achievements_mutex_0a3e08);
    const auto all=request.attribute("All");
    achievement_set_header_02bf60(x,s);
    for(const auto& [id,a]:s.achievements_0a3e58)
        if((all&&*all=="true")||a.field04!=0)achievement_element_02c080(x,id,a);
    close(x);close(x);return true;
}
bool grant_achievement_030010(const RequestTree& request,Xml& response,StateReferences s,Effects& effects) {
    const auto id=request.attribute("AchievementId");
    if(!id)return false;
    const auto progress=request.attribute("Progress");
    const auto increment=progress?effects.atoi_05ee08(*progress):1;
    Guard guard(s.achievements_mutex_0a3e08);
    auto found=s.achievements_0a3e58.find(*id);
    const bool inserted=found==s.achievements_0a3e58.end();
    std::optional<Achievement> previous;
    if(!inserted)previous=found->second; // Allocate rollback state before mutation.
    if(inserted) {
        // For an unknown ID the first requested increment also becomes total.
        found=s.achievements_0a3e58.emplace(*id,Achievement{increment,0,0,"0000-00-00T00:00:00",{},{},0}).first;
    }
    struct Rollback {
        AchievementMap& map;AchievementMap::iterator entry;
        std::optional<Achievement>& previous;bool committed=false;
        ~Rollback(){
            if(committed)return;
            if(previous){using std::swap;swap(entry->second,*previous);}
            else map.erase(entry);
        }
    } rollback{s.achievements_0a3e58,found,previous};
    auto& a=found->second;
    if(a.field08<a.field00) {
        a.field08=wrap_add(a.field08,increment); // Preserve x86 progress arithmetic.
        a.timestamp=effects.utc_time_041b60(a.field70);
        std::optional<std::string> event_bytes;
        if(a.field08>=a.field00) {
            a.field04=1;a.field08=a.field00;
            auto event=effects.printer_0019b0();auto& x=event->xml();
            open(x,"LSX");open(x,"Event");attr(x,"sender","EbisuSDK");
            achievement_set_header_02bf60(x,s);achievement_element_02c080(x,*id,a);
            close(x);close(x);close(x);close(x);
            event_bytes=event->c_string_without_nul();
        }
        std::string xml;
        if(!s.achievement_file_0a3de8.empty()) {
            auto document=effects.printer_0019b0();achievement_persistence(document->xml(),s);
            xml=document->c_string_without_nul();
        }
        const auto persist=[&]{effects.persist_achievement(s.achievement_file_0a3de8,xml,s.achievements_0a3e58);};
        if(event_bytes) {
            Guard event_guard(s.events.mutex_02c620());
            if(!s.events.initialized_locked())
                s.events.initialize_locked("<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>");
            s.events.push_persisted_locked(std::move(*event_bytes),persist);
        } else persist();
    }
    rollback.committed=true;
    achievement_element_02c080(response,*id,a);return true;
}

bool query_image_0314f0(const RequestTree& request,Xml& x,StateReferences s,Effects& effects) {
    const auto width=request.attribute("Width"),height=request.attribute("Height"),id=request.attribute("ImageId");
    if(!width||!height||!id)return false;
    Path result=s.avatar_0a3a90;
    const auto separator=id->find(':');
    if(separator!=std::string::npos) {
        const auto owner=id->substr(separator+1);
        const bool own=owner==s.user_id_0a3c18;
        if(!own)result=s.default_avatar_0a3ab0;
        if(s.online_0a3c3b!=0) {
            const auto size=effects.atoi_05ee08(*width)/208;
            const auto filename="_AVATAR_"+owner+"_"+*width;
            const auto cache=effects.path_append_016b50(s.user_directory_0a3a00,effects.filesystem_widen_006d70(filename));
            if(effects.online_avatar_virtual_28(cache)) {
                result=cache;
                if(own&&s.saved_avatar_size_0a1130<size&&effects.copy_overwrite_048d20(cache,s.last_avatar_0a3a60)) {
                    s.saved_avatar_size_0a1130=size;
                    effects.log_saved_size_085530(size);
                }
            }
        }
    }
    open(x,"QueryImageResponse");open(x,"Image");attr(x,"Width",*width);attr(x,"Height",*height);
    attr(x,"ResourcePath",effects.path_to_utf8_031ca0(result));attr(x,"ImageId",*id);
    close(x);close(x);return true;
}
} // namespace preloader_l::content_handlers
