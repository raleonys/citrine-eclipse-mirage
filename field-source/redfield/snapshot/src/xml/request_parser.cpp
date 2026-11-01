// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/lsx_framing.hpp"
#include "tinyxml2_dom.hpp"
#include <cstring>

namespace preloader_l { namespace xml {
namespace {
void clear(RequestView* out) noexcept { *out=RequestView{}; }
bool no_second_element(const tinyxml2::XMLElement* element) noexcept {
    return !element || element->NextSiblingElement()==nullptr;
}
bool project_attr(const tinyxml2::XMLElement* element,const char* name,
    const std::shared_ptr<const tinyxml2::XMLDocument>& doc,TextSpan* target) noexcept {
    return dom::decoded(element->Attribute(name),doc,target);
}
} // namespace

bool span_equals(TextSpan span,const char* literal) noexcept {
    if (!literal || (!span.data && span.size)) return false;
    const size_t length=std::strlen(literal);
    return span.size==length && (!length || std::memcmp(span.data,literal,length)==0);
}
TextSpan response_sender_policy(const RequestView& request) noexcept {
    if (request.recipient.data && request.recipient.size) return request.recipient;
    static constexpr char fallback[]="EbisuSDK";
    return {fallback,sizeof(fallback)-1};
}
RequestStatus parse_request_view(const char* input,size_t length,RequestView* out) noexcept {
    if (!out || (!input && length)) return RequestStatus::invalid_argument;
    clear(out);
    const auto frame=scan_lsx_frame(input,length);
    if (frame.status==FrameStatus::need_more) return RequestStatus::incomplete_frame;
    if (frame.status!=FrameStatus::frame_ready) return RequestStatus::malformed;
    try {
        auto owned=std::make_shared<tinyxml2::XMLDocument>(true,tinyxml2::PRESERVE_WHITESPACE);
        if (!dom::parse_document(input,frame.frame_length,owned.get())) return RequestStatus::malformed;
        const std::shared_ptr<const tinyxml2::XMLDocument> doc=owned;
        const tinyxml2::XMLElement* root=doc->RootElement();
        if (!root || std::strcmp(root->Name(),"LSX") || !no_second_element(root))
            return RequestStatus::malformed;
        const tinyxml2::XMLElement* envelope=root->FirstChildElement();
        if (!envelope) return RequestStatus::no_command;
        if (!no_second_element(envelope)) return RequestStatus::malformed;
        const tinyxml2::XMLElement* command=envelope;
        if (std::strcmp(envelope->Name(),"Request")==0) {
            out->has_request_wrapper=true;
            if (!project_attr(envelope,"id",doc,&out->id) ||
                !project_attr(envelope,"sender",doc,&out->sender) ||
                !project_attr(envelope,"recipient",doc,&out->recipient)) {
                    clear(out); return RequestStatus::malformed;
                }
            command=envelope->FirstChildElement();
            if (!command) return RequestStatus::no_command;
            if (!no_second_element(command)) { clear(out); return RequestStatus::malformed; }
        }
        if (!dom::decoded(command->Name(),doc,&out->command)) {
            clear(out); return RequestStatus::malformed;
        }
        // No regenerated input markup: these views carry DOM identity and
        // lifetime only. Attributes and children are consumed on the same tree.
        out->command_open_tag=TextSpan{nullptr,0,doc,command};
        out->command_content=TextSpan{nullptr,0,doc,command};
        // Cached once on the owned DOM, including the comment/text policy.
        // Direct-child iteration must not rescan siblings on every call.
        out->command_content.direct_children=dom::child_index(command);
        return RequestStatus::ok;
    } catch (...) { clear(out); return RequestStatus::malformed; }
}
} }
