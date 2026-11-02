// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/router/router.hpp"
#include <deque>
#include <stdexcept>

namespace preloader_l::dispatch {
namespace {
using Node=messages::XmlNode;
using Request=handlers::Request;
// Request IDs here refer exclusively to caller-owned Node objects in this source
// model. They are never original binary VAs and never invoke original machine code.
Request handle(const Node* node){return reinterpret_cast<Request>(node);}
const Node* node(Request value){return reinterpret_cast<const Node*>(value);}
class View final:public handlers::Xml,public misc_handlers::Elements {
public:
    View(xml::MemoryPrinter& writer,NodeAccess& access):writer_(writer),access_(access){}
    const char* attribute_002cc0(Request value,const char* key) override {
        const auto result=node(value)->attribute(key);
        if(!result)return nullptr;
        // Stable copies across subsequent attribute lookups. Additional source
        // allocations are not claimed to reproduce MSVC XMLDocument string ABI.
        strings_.emplace_back(*result);
        return strings_.back().c_str();
    }
    void open_004080(const char* name,bool compact) override {writer_.open_element(name,compact);}
    void attribute_005560(const char* key,const char* value) override {
        if(!key||!value)throw std::invalid_argument("Missing XML attribute text");
        writer_.attribute(key,value);
    }
    void close_virtual_48(bool compact) override {writer_.close_element(compact);}
    Request first_child_element_003300(Request value) override {return handle(node(value)->first_element());}
    Request next_sibling_element_003fe0(Request value) override {return handle(node(value)->next_element());}
    const char* get_text_003860(Request value) override {return access_.text_003860(*node(value));}
    void push_text_0056b0(const char* value,bool cdata) override {
        if(!value)throw std::invalid_argument("Missing XML element text");
        writer_.push_text(value,cdata);
    }
private:
    xml::MemoryPrinter& writer_;
    NodeAccess& access_;
    std::deque<std::string> strings_;
};
content_handlers::RequestTree request_tree(const Node& request,NodeAccess& access) {
    content_handlers::RequestTree result;
    result.attributes=access.attributes_in_order(request);
    for(auto child=request.first_element();child;child=child->next_element()) {
        const char* text=access.text_003860(*child);
        result.children.push_back({std::string(child->name()),text?std::optional<std::string>(text):std::nullopt});
    }
    return result;
}
}
bool Router::registered_handler(std::uint32_t rva,const Node& request,messages::XmlPrinter& abstract_printer) {
    // This integration route requires the concrete serialization model;
    // an unrelated XmlPrinter implementation is outside this adapter's contract.
    auto* printer=dynamic_cast<xml::MemoryPrinter*>(&abstract_printer);
    if(!printer)throw std::invalid_argument("Router requires a memory XML printer");
    View xml(*printer,deps_.nodes);
    const auto input=handle(&request);
    namespace info=handlers;
    namespace misc=misc_handlers;
    namespace content=content_handlers;
    switch(rva) {
    case 0x02e5f0:return info::get_config_02e5f0(xml);
    case 0x02f530:return info::get_profile_02f530(input,xml,information_,deps_.information);
    case 0x02f2d0:return info::get_connected_02f2d0(xml,information_);
    case 0x02f7c0:return info::get_setting_02f7c0(input,xml,information_);
    case 0x02fa60:return info::get_settings_02fa60(xml,information_);
    case 0x02efd0:return info::get_game_info_02efd0(input,xml,information_);
    case 0x02da90:return info::get_all_game_info_02da90(xml,information_,deps_.information);
    case 0x030ee0:return content::query_content_030ee0(xml,content_);
    case 0x031030:return content::query_entitlements_031030(request_tree(request,deps_.nodes),xml,content_,deps_.content);
    case 0x030d30:return content::query_achievements_030d30(request_tree(request,deps_.nodes),xml,content_);
    case 0x030010:return content::grant_achievement_030010(request_tree(request,deps_.nodes),xml,content_,deps_.content);
    case 0x0314f0:return content::query_image_0314f0(request_tree(request,deps_.nodes),xml,content_,deps_.content);
    case 0x02ffb0:return misc::success_02ffb0(xml);
    case 0x02f4e0:return misc::presence_visibility_02f4e0(xml);
    case 0x02f330:return misc::presence_02f330(xml,information_.game_0a3d88);
    case 0x031ef0:return misc::query_presence_031ef0(input,xml,xml,information_.profile.username);
    case 0x02f270:return misc::group_info_02f270(xml);
    case 0x030cd0:return misc::progressive_available_030cd0(xml);
    case 0x030eb0:misc::query_chunk_status_030eb0(deps_.no_response);
    case 0x02d720:return misc::chunks_installed_02d720(input,xml,xml);
    case 0x0314a0:return misc::query_friends_0314a0(xml);
    case 0x02e5a0:return misc::block_list_02e5a0(xml);
    case 0x02da30:return misc::voip_02da30(xml);
    case 0x02ff60:return misc::wallet_02ff60(xml);
    case 0x032ab0:return misc::restart_game_032ab0(xml);
    case 0x02d9e0:return misc::permission_02d9e0(xml);
    case 0x02fea0:return misc::utc_time_02fea0(xml,deps_.information);
    case 0x032b10:return misc::show_igo_032b10(xml,deps_.igo);
    case 0x032ca0:return misc::start_download_032ca0(xml);
    case 0x031ec0:return misc::query_offers_031ec0(xml);
    case 0x02e3b0:return deps_.licensing.auth_code_02e3b0(request,xml);
    case 0x0320c0:return deps_.licensing.request_license_0320c0(request,xml);
    case 0x030c50:return deps_.licensing.invalidate_license_030c50(request,xml);
    default:throw std::invalid_argument("Unregistered handler RVA passed to source integration adapter");
    }
}
} // namespace preloader_l::dispatch
