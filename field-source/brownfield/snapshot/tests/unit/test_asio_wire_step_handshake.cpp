// Actual source-linked TinyXML2->challenge handshake->AES/hex->wire framing
// continuation. Only the external host Config2 social binding is stubbed;
// this is portable execution, NOT Windows IOCP/native/game acceptance.
#include "../../src/protocol/socket_exchange_internal.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
namespace p=preloader_l::protocol;
namespace w=preloader_l::protocol::detail;
static unsigned binding_calls=0;
namespace preloader_l { namespace config {
ValueView runtime_configuration_setting(const char*) noexcept { return ValueView{nullptr,0u,false}; }
} namespace runtime {
bool bind_host_social_profile(LocalSocialSession* out) noexcept {
    ++::binding_calls;
    return out!=nullptr;
}
} }
static unsigned checks=0;
static void verify(bool good,const char* name){++checks; if(!good){std::cerr<<"FAIL "<<name<<"\n";std::abort();}}
static void append(p::ExchangeState& s,const std::string& v){
    verify(s.wire_size+v.size()<=sizeof(s.wire_response),"bounded buffer");
    std::memcpy(s.wire_response+s.wire_size,v.data(),v.size());
    s.wire_size+=v.size();
}
int main(){
    p::ExchangeState state{};
    uint32_t streak=2u;
    w::WireFrameView frame{};
    const std::string raw="<LSX><Request id=\"7\"><ChallengeResponse key=\"a\" version=\"3\"/></Request></LSX>";
    append(state,raw+std::string(1,'\0'));
    verify(w::scan_available_wire_frame(&state,&frame),"source wire scanner");
    auto pending=w::classify_received_wire_frame(&state,frame,&streak);
    verify(pending.kind==w::WireStepKind::handshake_reply,"real TinyXML2 handshake parsed");
    verify(pending.handshake.response_length>10,"real response built");
    verify(!state.established && binding_calls==0 && streak==2u,"no pre-send session commit");
    verify(std::string(state.plain_response).find("<ChallengeAccepted response=\"fe123486508b0eb503a8468388ef5812\"/>")!=std::string::npos,
           "original handshake response serialization");
    const uint8_t known_key[16]={0x4e,0x54,0x1f,0x16,0x5f,0x6a,0x47,0xa9,0xc5,0x18,0xd6,0x66,0x8a,0x14,0x45,0x37};
    verify(std::memcmp(pending.handshake.session_key,known_key,16)==0,"exact original key semantics");
    // The actual IOCP async_write completion will invoke this only after all bytes.
    verify(w::commit_sent_handshake(&state,frame,pending.handshake,&streak),"commit after complete reply");
    verify(state.established && state.wire_size==0u && binding_calls==1u && streak==0u,"commit and consume");
    verify(std::memcmp(state.session_key,known_key,16)==0,"key published");
    const std::string xml="<LSX><Request id=\"8\"><GetSetting/></Request></LSX>";
    size_t required=0u;
    verify(preloader_l::codec::encode_response(state.session_key,16,reinterpret_cast<const uint8_t*>(xml.data()),xml.size(),nullptr,0,&required)==preloader_l::codec::MessageCodecStatus::capacity,"real codec size");
    std::string wire(required,'\0');size_t written=0u;
    verify(preloader_l::codec::encode_response(state.session_key,16,reinterpret_cast<const uint8_t*>(xml.data()),xml.size(),wire.data(),wire.size(),&written)==preloader_l::codec::MessageCodecStatus::ok,"real encode");
    verify(written==required,"exact ciphertext wire size");
    append(state,wire);
    verify(w::scan_available_wire_frame(&state,&frame),"encrypted frame scan");
    const auto decoded=w::classify_received_wire_frame(&state,frame,&streak);
    verify(decoded.kind==w::WireStepKind::xml_request && decoded.xml_length==xml.size(),"source AES/hex decode into original XML");
    verify(std::memcmp(state.decoded,xml.data(),xml.size())==0,"decoded bytes identical");
    w::consume_wire_frame(&state,frame.consume_length);
    verify(state.wire_size==0u,"frame consumed exactly");
    const std::string malformed="<LSX><Request id=\"9\"><ChallengeResponse version=\"3\"/></Request></LSX>";
    append(state,malformed);
    verify(w::scan_available_wire_frame(&state,&frame),"malformed handshake scan");
    verify(w::classify_received_wire_frame(&state,frame,&streak).kind==w::WireStepKind::discarded,"missing-key ignored policy");
    verify(streak==1u && binding_calls==1u,"malformed budget and binding retention");
    std::cout<<"HF12_STAGE02_REAL_HANDSHAKE_PORTABLE PASS checks="<<checks<<"\n";
}
