// Stage02 structural/behavioral test using the actual production frame-step,
// wire scanner and AES/hex codec; the handshake parser and host binding are
// deliberately deterministic test doubles (NOT native handshake acceptance).
#include "../../src/protocol/socket_exchange_internal.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
namespace p=preloader_l::protocol;
namespace w=preloader_l::protocol::detail;
static unsigned binds=0u;
namespace preloader_l { namespace runtime {
bool bind_host_social_profile(LocalSocialSession* out) noexcept {
    ++::binds;
    return out!=nullptr;
}
} namespace protocol {
HandshakeStatus process_challenge_request(const char* input,size_t length,char* response,
                                          size_t cap,HandshakeResult* result) noexcept {
    if(!input || !response || !result || cap<9u) return HandshakeStatus::invalid_argument;
    const std::string s(input,length);
    if(s.find("missing")!=std::string::npos) return HandshakeStatus::missing_key;
    if(s.find("fatal")!=std::string::npos) return HandshakeStatus::codec_error;
    const char reply[]="reply";
    std::memcpy(response,reply,sizeof(reply));
    result->response_length=sizeof(reply)-1u;
    for(unsigned i=0;i<16u;++i)result->session_key[i]=static_cast<uint8_t>(i+1u);
    return HandshakeStatus::ok;
}
} }
static unsigned checks=0;
static void require(bool b){++checks; if(!b)std::abort();}
static void append(p::ExchangeState& state,const std::string& data){
    require(state.wire_size+data.size()<=sizeof(state.wire_response));
    std::memcpy(state.wire_response+state.wire_size,data.data(),data.size());
    state.wire_size+=data.size();
}
int main(){
    p::ExchangeState state{};
    uint32_t streak=3u;
    w::WireFrameView frame{};
    const std::string handshake="<LSX><Request id=\"1\"><ChallengeResponse/></Request></LSX>";
    append(state,handshake+std::string(1,'\0')+"junk");
    require(w::scan_available_wire_frame(&state,&frame));
    const auto candidate=w::classify_received_wire_frame(&state,frame,&streak);
    require(candidate.kind==w::WireStepKind::handshake_reply);
    require(!state.established && state.session_key[0]==0u && binds==0u);
    require(streak==3u && state.wire_size==handshake.size()+1u+4u);
    require(candidate.handshake.response_length==5u);
    // A failed/partial async write does not call commit_sent_handshake.
    require(!state.established && binds==0u);
    require(w::commit_sent_handshake(&state,frame,candidate.handshake,&streak));
    require(state.established && state.session_key[0]==1u && state.session_key[15]==16u);
    require(streak==0u && binds==1u && state.wire_size==4u);
    require(w::scan_available_wire_frame(&state,&frame));
    require(w::classify_received_wire_frame(&state,frame,&streak).kind==w::WireStepKind::discarded);
    require(streak==1u && state.wire_size==0u);
    // Decode real shipping codec output, including trailing XML bytes behavior.
    const std::string xml="<LSX><Request id=\"2\"><GetSetting/></Request></LSX>TRAIL";
    size_t required=0u;
    const auto measured=preloader_l::codec::encode_response(state.session_key,16,
        reinterpret_cast<const uint8_t*>(xml.data()),xml.size(),nullptr,0,&required);
    require(measured==preloader_l::codec::MessageCodecStatus::capacity && required>0u);
    std::string enc(required,'\0'); size_t n=0u;
    require(preloader_l::codec::encode_response(state.session_key,16,
        reinterpret_cast<const uint8_t*>(xml.data()),xml.size(),enc.data(),enc.size(),&n)==
        preloader_l::codec::MessageCodecStatus::ok);
    require(n==required);
    append(state,enc);
    require(w::scan_available_wire_frame(&state,&frame));
    const auto decoded=w::classify_received_wire_frame(&state,frame,&streak);
    require(decoded.kind==w::WireStepKind::xml_request);
    require(decoded.xml_length==xml.find("</LSX>")+6u);
    require(streak==1u);
    w::consume_wire_frame(&state,frame.consume_length);
    require(state.wire_size==0u);
    // A second valid handshake on an established session must defer key commit.
    const std::string again="<LSX><Request id=\"3\"><ChallengeResponse/></Request></LSX>";
    append(state,again);
    require(w::scan_available_wire_frame(&state,&frame));
    const auto second=w::classify_received_wire_frame(&state,frame,&streak);
    require(second.kind==w::WireStepKind::handshake_reply && binds==1u);
    require(streak==1u); // a prepare is not a commit
    // Bad pending proof must not commit / consume.
    p::HandshakeResult invalid{};
    require(!w::commit_sent_handshake(&state,frame,invalid,&streak));
    require(state.wire_size==again.size() && binds==1u);
    require(w::commit_sent_handshake(&state,frame,second.handshake,&streak));
    require(binds==2u && streak==0u && state.wire_size==0u);
    // Ignored handshake takes existing malformed-frame budget.
    const std::string ignored="<LSX><Request id=\"4\"><missing/></Request></LSX>";
    append(state,ignored);
    require(w::scan_available_wire_frame(&state,&frame));
    require(w::classify_received_wire_frame(&state,frame,&streak).kind==w::WireStepKind::discarded);
    require(streak==1u && binds==2u);
    const std::string fatal="<LSX><Request id=\"5\"><fatal/></Request></LSX>";
    append(state,fatal);
    require(w::scan_available_wire_frame(&state,&frame));
    const auto error=w::classify_received_wire_frame(&state,frame,&streak);
    require(error.kind==w::WireStepKind::terminal && error.status==p::ExchangeStatus::handshake_error);
    require(state.wire_size==fatal.size() && streak==1u);
    require(w::classify_received_wire_frame(nullptr,frame,&streak).status==p::ExchangeStatus::invalid_argument);
    require(w::classify_received_wire_frame(&state,frame,nullptr).status==p::ExchangeStatus::invalid_argument);
    uint32_t bound=32u;
    require(!w::tolerate_malformed_frame(&bound) && bound==32u);
    bound=31u; require(w::tolerate_malformed_frame(&bound) && bound==32u);
    std::cout<<"HF12_STAGE02_SOURCE_LINKED_STEP PASS checks="<<checks<<"\n";
}
