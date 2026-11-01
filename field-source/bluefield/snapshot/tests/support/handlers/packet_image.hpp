// Test-only literal packet QueryImage body. Not linked into production.
#pragma once
#include "src/handlers/content.hpp"
namespace preloader_l::test_packet_avatar {
using content::Path; using content::RequestTree; using content::StateReferences; using content::Xml;
// Packet C++ models the virtual avatar call with a path-only effect. Production
// has explicit owner/size ABI arguments; this test adapter forwards them and
// discloses that signature projection rather than claiming native ABI proof.
struct Effects {
    content::Effects& target;
    std::string owner;
    std::int32_t width=0;
    std::int32_t atoi_05ee08(std::string_view v){width=target.atoi_05ee08(v);return width;}
    Path filesystem_widen_006d70(std::string_view v){return target.filesystem_widen_006d70(v);}
    Path path_append_016b50(const Path& a,const Path& b){return target.path_append_016b50(a,b);}
    bool online_avatar_virtual_28(const Path& p){return target.online_avatar_virtual_28(p,owner,width/208);}
    bool copy_overwrite_048d20(const Path& a,const Path& b){return target.copy_overwrite_048d20(a,b);}
    void log_saved_size_085530(std::int32_t n){target.log_saved_size_085530(n);}
    std::string path_to_utf8_031ca0(const Path& p){return target.path_to_utf8_031ca0(p);}
};
void open(Xml& x,const char* name){x.open_004080(name,false);}
void close(Xml& x){x.close_virtual_48(false);}
void attr(Xml& x,const char* key,std::string_view value) {
    const std::string owned(value);
    x.attribute_005560(key,owned.c_str());
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
} // namespace preloader_l::test_packet_avatar
