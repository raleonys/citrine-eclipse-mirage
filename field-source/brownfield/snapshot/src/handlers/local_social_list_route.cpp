#include "preloader_l/handlers/local_social_list_route.hpp"
#include "preloader_l/xml/response_builder.hpp"
namespace preloader_l { namespace handlers { namespace {
// Static, bounded, no heap allocation and no speculative network persona.
struct Writer {
    char* p;size_t used;size_t limit;bool valid;
    explicit Writer(char* dest,size_t capacity) noexcept:p(dest),used(0),limit(capacity),valid(true){}
    void append(const char* text) noexcept {
        if(!valid||!text)return;
        for(size_t i=0;text[i];++i){
            if(used>=limit){valid=false;return;}
            p[used++]=text[i];
        }
    }
    void decimal(uint64_t n) noexcept {
        char tmp[21]{};size_t count=0;
        do {tmp[count++]=static_cast<char>('0'+n%10u);n/=10u;}while(n);
        while(count){if(used>=limit){valid=false;return;}p[used++]=tmp[--count];}
    }
};
} // namespace
HandlerStatus render_local_social_list(const xml::RequestView& request,
    router::RouteId route,const runtime::LocalSocialSnapshot& snap,
    char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity!=0)||!request.id.data||!request.id.size)
        return HandlerStatus::invalid_request;
    const bool friends=route==router::RouteId::query_friends;
    if(!friends && route!=router::RouteId::get_block_list)return HandlerStatus::wrong_command;
    if(!xml::span_equals(request.command,friends?"QueryFriends":"GetBlockList"))
        return HandlerStatus::wrong_command;
    if(!snap.actor)return HandlerStatus::invalid_request;
    const size_t count=friends?snap.friends_n:snap.blocked_n;
    if(count>runtime::LocalSocialStore::edge_limit)return HandlerStatus::unsupported;
    // This is explicitly a LOCAL-ONLY OriginFriendT projection patterned on
    // the separately recovered QueryPresence Friend attributes. Game-native
    // populated-response acceptance is a mandatory *release gate*.
    char body[4096]{};Writer w(body,sizeof(body));
    w.append(friends?"<QueryFriendsResponse Return=\"Success\">":"<GetBlockListResponse Return=\"Success\">");
    for(size_t i=0;i<count;++i){
        const uint64_t uid=friends?snap.friends[i]:snap.blocked[i];
        if(!uid)return HandlerStatus::invalid_request;
        w.append("<Friend AvatarId=\"\" Title=\"\" GroupId=\"\" MultiplayerId=\"\" TitleId=\"\" Presence=\"UNKNOWN\" PersonaId=\"0\" GamePresence=\"\" Group=\"\" Persona=\"\" RichPresence=\"\" UserId=\"");
        w.decimal(uid);w.append("\" State=\"NONE\"/>");
    }
    w.append(friends?"</QueryFriendsResponse>":"</GetBlockListResponse>");
    if(!w.valid)return HandlerStatus::response_error;
    auto status=xml::build_response_envelope_from_source_attributes(
        request.id,xml::response_sender_policy(request),{body,w.used},output,capacity,written);
    return status==xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}
} }
