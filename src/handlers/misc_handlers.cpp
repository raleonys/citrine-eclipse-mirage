// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/handlers/misc_handlers.hpp"
#include <exception>
#include <limits>

namespace preloader_l::misc_handlers {
static bool response(Xml& xml,const char* element,const char* key,const char* value) {
    xml.open_004080(element,false);
    xml.attribute_005560(key,value);
    xml.close_virtual_48(false);
    return true;
}
static bool result(Xml& xml,const char* code,const char* description) {
    xml.open_004080("ErrorSuccess",false);
    xml.attribute_005560("Code",code);
    xml.attribute_005560("Description",description);
    xml.close_virtual_48(false);
    return true;
}
bool success_02ffb0(Xml& xml) {return result(xml,"0","");}
bool presence_visibility_02f4e0(Xml& xml) {
    return response(xml,"GetPresenceVisibilityResponse","Visible","false");
}
bool presence_02f330(Xml& xml,Attributes& game) {
    xml.open_004080("GetPresenceResponse",false);
    for(const auto* key:{"Title","SessionId","MultiplayerId","GroupId","TitleId"})
        xml.attribute_005560(key,"");
    xml.attribute_005560("Presence","INGAME");
    xml.attribute_005560("GamePresence","");
    xml.attribute_005560("Group","");
    xml.attribute_005560("RichPresence",game["DISPLAY_NAME"].c_str());
    xml.attribute_005560("UserId","0");
    xml.close_virtual_48(false);
    return true;
}
bool query_presence_031ef0(Request request,Xml& xml,Elements& elements,const std::string& username) {
    xml.open_004080("QueryPresenceResponse",false);
    for(auto child=elements.first_child_element_003300(request);child;
        child=elements.next_sibling_element_003fe0(child)) {
        xml.open_004080("Friend",false);
        for(const auto* key:{"AvatarId","Title","GroupId","MultiplayerId","TitleId"})
            xml.attribute_005560(key,"");
        xml.attribute_005560("Presence","UNKNOWN");
        xml.attribute_005560("PersonaId","0");
        xml.attribute_005560("GamePresence","");
        xml.attribute_005560("Group","");
        xml.attribute_005560("Persona",username.c_str());
        xml.attribute_005560("RichPresence","");
        xml.attribute_005560("UserId",elements.get_text_003860(child));
        xml.attribute_005560("State","NONE");
        xml.close_virtual_48(false);
    }
    xml.close_virtual_48(false);
    return true;
}
bool group_info_02f270(Xml& xml) {
    return result(xml,"-1577058296","User not in a party, create one.");
}
bool progressive_available_030cd0(Xml& xml) {
    xml.open_004080("IsProgressiveInstallationAvailableResponse",false);
    xml.attribute_005560("ItemId","");
    xml.attribute_005560("Available","false");
    xml.close_virtual_48(false);
    return true;
}
[[noreturn]] void query_chunk_status_030eb0(NoResponse& effect) {
    effect.throw_no_response_02bcd0_04d228();
    // GCC/Clang do not consistently infer noreturn from a virtual override.
    std::terminate();
}
bool chunks_installed_02d720(Request request,Xml& xml,Elements& elements) {
    xml.open_004080("AreChunksInstalledResponse",false);
    xml.attribute_005560("ItemId","");
    xml.attribute_005560("Installed","true");
    for(auto child=elements.first_child_element_003300(request);child;
        child=elements.next_sibling_element_003fe0(child)) {
        xml.open_004080("ChunkIds",false);
        elements.push_text_0056b0(elements.get_text_003860(child),false);
        xml.close_virtual_48(false);
    }
    xml.close_virtual_48(false);
    return true;
}
bool query_friends_0314a0(Xml& xml) {return response(xml,"QueryFriendsResponse","Return","Success");}
bool block_list_02e5a0(Xml& xml) {return response(xml,"GetBlockListResponse","Return","Success");}
bool voip_02da30(Xml& xml) {
    return result(xml,"-1593835518","The function you are calling is not implemented.");
}
bool wallet_02ff60(Xml& xml) {return response(xml,"GetWalletBalanceResponse","Balance","0");}
bool restart_game_032ab0(Xml& xml) {
    return result(xml,"1","Origin is waiting for the game to shutdown.");
}
bool permission_02d9e0(Xml& xml) {return response(xml,"CheckPermissionResponse","Access","GRANTED");}
bool utc_time_02fea0(Xml& xml,handlers::Effects& effects) {
    xml.open_004080("GetUTCTimeResponse",false);
    const auto time=effects.current_time_041b60();
    xml.attribute_005560("Time",time.c_str());
    xml.close_virtual_48(false);
    return true;
}
bool show_igo_032b10(Xml& xml,IgoEffects& effects) {
    (void)success_02ffb0(xml);
    if(effects.lock_02c620_04a6ac()!=0)effects.thread_error_04a648(5);
    auto& count=effects.lock_count_4c();
    if(count==std::numeric_limits<std::int32_t>::max()) {
        --count;effects.thread_error_04a648(6);
    }
    struct Unlock {IgoEffects& effects;~Unlock(){effects.unlock_04a6b4();}} unlock{effects};
    effects.acquire_event_queue_02c400();
    effects.enqueue_034660("<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>");
    return true;
}
bool start_download_032ca0(Xml& xml) {
    return result(xml,"0","Requested a download of a DLC offer.");
}
bool query_offers_031ec0(Xml& xml) {
    xml.open_004080("QueryOffersResponse",false);
    xml.close_virtual_48(false);
    return true;
}
} // namespace preloader_l::misc_handlers
