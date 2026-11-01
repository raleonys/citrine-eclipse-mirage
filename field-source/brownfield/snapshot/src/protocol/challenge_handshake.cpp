#include "preloader_l/protocol/challenge_handshake.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/response_builder.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "preloader_l/config/runtime_configuration.hpp"

namespace preloader_l { namespace protocol {
namespace {
bool safe_version_char(char c) noexcept {
    return static_cast<unsigned char>(c)>=0x20u;
}

uint16_t msvc_rand15(uint32_t& state) noexcept {
    state = state * UINT32_C(0x343fd) + UINT32_C(0x269ec3);
    return static_cast<uint16_t>((state >> 16) & UINT32_C(0x7fff));
}

void clear(HandshakeResult* r) noexcept {
    if (!r) return;
    for (unsigned i=0;i<16;++i) r->session_key[i]=0;
    r->response_length=0;
}
}

GreetingStatus build_challenge_greeting(uint32_t unix_seconds_seed, char* output,
                                        size_t capacity, size_t* written) noexcept {
    if (written) *written=0;
    if (!written || (!output && capacity)) return GreetingStatus::invalid_argument;

    char key_hex[33]{};
    constexpr char hex[]="0123456789abcdef";
    uint32_t state=unix_seconds_seed;
    for (size_t i=0;i<16;++i) {
        const uint8_t b=static_cast<uint8_t>(msvc_rand15(state));
        key_hex[i*2]=hex[b>>4]; key_hex[i*2+1]=hex[b&0x0f];
    }

    const auto cfg=preloader_l::config::runtime_configuration_setting("SDK");
    // Approved quality promotion: behavioral Phase 137 closes missing/empty SDK as
    // a valid empty normalized version. Do not synthesize a compatibility SDK.
    const size_t sdk_n=cfg.present ? cfg.size : 0u;
    if (sdk_n && !cfg.data) return GreetingStatus::invalid_argument;
    const char* sdk=sdk_n ? cfg.data : "";
    char version[96]{};
    if (sdk_n >= sizeof(version)) return GreetingStatus::capacity;
    for(size_t i=0;i<sdk_n;++i) {
        if(!safe_version_char(sdk[i])) return GreetingStatus::invalid_argument;
        version[i]=(sdk[i]=='.')?',':sdk[i];
    }

    constexpr char a[]=" <LSX>\n  <Event sender=\"EALS\">\n   <Challenge key=\"";
    constexpr char b[]="\" build=\"release\" version=\"";
    constexpr char c[]="\"/>\n  </Event>\n </LSX>\n";
    size_t escaped_version_n=0;
    if(preloader_l::xml::escape_attribute_value({version,sdk_n},nullptr,0,&escaped_version_n)!=preloader_l::xml::ValueCodecStatus::ok)
        return GreetingStatus::invalid_argument;
    size_t need=(sizeof(a)-1)+32+(sizeof(b)-1);
    if(escaped_version_n>static_cast<size_t>(-1)-need)return GreetingStatus::capacity;
    need+=escaped_version_n;
    if((sizeof(c)-1)>static_cast<size_t>(-1)-need)return GreetingStatus::capacity;
    need+=sizeof(c)-1;
    *written=need;
    if (!output || capacity < need+1) return GreetingStatus::capacity;
    size_t p=0;
    auto append=[&](const char* x,size_t n) noexcept { for(size_t i=0;i<n;++i) output[p++]=x[i]; };
    append(a,sizeof(a)-1); append(key_hex,32); append(b,sizeof(b)-1);
    size_t escaped_written=0;
    if(preloader_l::xml::escape_attribute_value({version,sdk_n},output+p,capacity-p,&escaped_written)!=preloader_l::xml::ValueCodecStatus::ok||escaped_written!=escaped_version_n)
        return GreetingStatus::capacity;
    p+=escaped_written;
    append(c,sizeof(c)-1);
    output[p]='\0';
    return GreetingStatus::ok;
}

HandshakeStatus process_challenge_request(const char* input, size_t length,
                                          char* response, size_t response_capacity,
                                          HandshakeResult* result) noexcept {
    if (!result || !response || response_capacity==0 || (!input && length!=0)) return HandshakeStatus::invalid_argument;
    clear(result);
    preloader_l::xml::RequestView req{};
    if (preloader_l::xml::parse_request_view(input,length,&req)!=preloader_l::xml::RequestStatus::ok)
        return HandshakeStatus::malformed_request;
    if (!req.has_request_wrapper) return HandshakeStatus::malformed_request;
    if (!req.id.data || req.id.size == 0) return HandshakeStatus::missing_id;
    if (!preloader_l::xml::span_equals(req.command,"ChallengeResponse")) return HandshakeStatus::wrong_command;
    preloader_l::xml::TextSpan key{}, version{};
    const auto key_attr=preloader_l::xml::find_attribute(req.command_open_tag,"key",&key);
    if (key_attr!=preloader_l::xml::AttributeStatus::found) return HandshakeStatus::missing_key;
    const auto version_attr=preloader_l::xml::find_attribute(req.command_open_tag,"version",&version);
    if (version_attr==preloader_l::xml::AttributeStatus::malformed || version_attr==preloader_l::xml::AttributeStatus::invalid_argument)
        return HandshakeStatus::malformed_request;

    preloader_l::xml::SemanticValue semantic_key;
    preloader_l::xml::SemanticValue semantic_version;
    const auto key_status=semantic_key.assign_parsed(key);
    if(key_status==preloader_l::xml::ValueCodecStatus::malformed_entity||key_status==preloader_l::xml::ValueCodecStatus::invalid_character||key_status==preloader_l::xml::ValueCodecStatus::invalid_argument)
        return HandshakeStatus::malformed_request;
    if(key_status!=preloader_l::xml::ValueCodecStatus::ok) return HandshakeStatus::response_error;

    const char* version_data=nullptr; size_t version_size=0;
    if(version_attr==preloader_l::xml::AttributeStatus::found){
        const auto version_status=semantic_version.assign_parsed(version);
        if(version_status==preloader_l::xml::ValueCodecStatus::malformed_entity||version_status==preloader_l::xml::ValueCodecStatus::invalid_character||version_status==preloader_l::xml::ValueCodecStatus::invalid_argument)
            return HandshakeStatus::malformed_request;
        if(version_status!=preloader_l::xml::ValueCodecStatus::ok) return HandshakeStatus::response_error;
        const auto version_value=semantic_version.view(); version_data=version_value.data; version_size=version_value.size;
    }
    const auto key_value=semantic_key.view();
    char reply[1025]{}; size_t reply_written=0;
    if (key_value.size>256 || preloader_l::codec::make_challenge_reply(reinterpret_cast<const uint8_t*>(key_value.data),key_value.size,
            version_data,version_size,result->session_key,reply,sizeof(reply),&reply_written)!=preloader_l::codec::SessionStatus::ok)
        return HandshakeStatus::codec_error;
    char body[1100]{};
    const char a[]="<ChallengeAccepted response=\""; const char b[]="\"/>";
    size_t p=0;
    auto append=[&](const char* s,size_t n)->bool{ if(p+n>=sizeof(body)) return false; for(size_t i=0;i<n;++i) body[p++]=s[i]; return true; };
    if (!append(a,sizeof(a)-1) || !append(reply,reply_written) || !append(b,sizeof(b)-1)) return HandshakeStatus::response_error;
    body[p]='\0';
    const preloader_l::xml::TextSpan sender{"EALS",4};
    const preloader_l::xml::TextSpan body_span{body,p};
    size_t out=0;
    if (preloader_l::xml::build_response_envelope_from_source_attributes(req.id,sender,body_span,response,response_capacity,&out)!=preloader_l::xml::ResponseStatus::ok)
        return HandshakeStatus::response_error;
    result->response_length=out;
    return HandshakeStatus::ok;
}
} }
