#include "messages.hpp"
#include "control.hpp"
#include <limits>

namespace anadius::messages_recovered {
Cipher& reset_02bc70(Cipher& cipher,std::uint8_t& encrypted_0c4,std::uint8_t& unknown_0c5) {
    cipher.construct_0423d0_in_place();
    encrypted_0c4=0;unknown_0c5=0;
    return cipher;
}
std::span<const Registration> registrations_032d00() {
    static constexpr Registration entries[] = {
        {"GetConfig",0x02e5f0},{"GetProfile",0x02f530},{"GetInternetConnectedState",0x02f2d0},
        {"GetSetting",0x02f7c0},{"GetSettings",0x02fa60},{"QueryContent",0x030ee0},
        {"QueryEntitlements",0x031030},{"QueryOffers",0x031ec0},{"GetGameInfo",0x02efd0},
        {"GetAllGameInfo",0x02da90},{"QueryImage",0x0314f0},{"GoOnline",0x02ffb0},
        {"GetAuthCode",0x02e3b0},{"ShowIGO",0x032b10},{"ShowIGOWindow",0x032b10},
        {"GetUTCTime",0x02fea0},{"StartDownload",0x032ca0},{"SetPresenceVisibility",0x02ffb0},
        {"SetPresence",0x02ffb0},{"GetPresenceVisibility",0x02f4e0},{"GetPresence",0x02f330},
        {"QueryPresence",0x031ef0},{"GetGroupInfo",0x02f270},
        {"IsProgressiveInstallationAvailable",0x030cd0},{"QueryChunkStatus",0x030eb0},
        {"AreChunksInstalled",0x02d720},{"SetDownloaderUtilization",0x02ffb0},
        {"QueryAchievements",0x030d30},{"GrantAchievement",0x030010},{"QueryFriends",0x0314a0},
        {"GetBlockList",0x02e5a0},{"GetVoipStatus",0x02da30},{"EnableVoip",0x02da30},
        {"SelectStore",0x02ffb0},{"GetWalletBalance",0x02ff60},{"RestartGame",0x032ab0},
        {"CheckPermission",0x02d9e0},{"RequestLicense",0x0320c0},{"InvalidateLicense",0x030c50}
    };
    return entries;
}

void hex_encode_03d190(std::string& value) {
    static constexpr char digits[] = "0123456789abcdef";
    const auto count=value.size();
    value.resize(count*2);
    for(auto i=count;i!=0;--i) {
        const auto byte=static_cast<unsigned char>(value[i-1]);
        value[(i-1)*2]=digits[byte>>4];
        value[(i-1)*2+1]=digits[byte&15];
    }
}
static unsigned nibble(unsigned char value) {
    if(value>='0' && value<='9')return value-'0';
    if(value>='A' && value<='F')return value-'A'+10;
    if(value>='a' && value<='f')return value-'a'+10;
    return 0; // original silently treats invalid digits as zero
}
void hex_decode_03ce30(std::string& value) {
    const auto count=value.size()/2;
    for(std::size_t i=0;i<count;++i)
        value[i]=static_cast<char>((nibble(static_cast<unsigned char>(value[i*2]))<<4)
                                  |nibble(static_cast<unsigned char>(value[i*2+1])));
    value.resize(count); // final unmatched character is discarded
}

std::string initial_message_02d540(XmlBackend& xml,Random& random,std::string_view version) {
    auto printer=xml.printer();
    printer->open("LSX");printer->open("Event");printer->attribute("sender","EALS");
    printer->open("Challenge");printer->attribute("version",version);printer->attribute("build","release");
    const auto random_bytes=random.challenge_bytes_034cf0();
    std::string challenge(reinterpret_cast<const char*>(random_bytes.data()),random_bytes.size());
    hex_encode_03d190(challenge);
    printer->attribute("key",challenge);
    printer->close();printer->close();printer->close();
    return printer->buffer_including_terminator();
}

bool challenge_response_02d7e0(const XmlNode& node,XmlPrinter& printer,Cipher& state,
                              CryptoFactory& factory,Random& random,Effects& effects) {
    const auto key=node.attribute("key");
    if(!key)return false;
    std::string response(*key);
    auto fixed=factory.create_0423d0();
    fixed->set_fixed_key_084c98();
    (void)fixed->encrypt_042610(response); // original ignores this result
    hex_encode_03d190(response);
    const auto version=node.attribute("version");
    if(!version || effects.atoi_05ee08(*version)>2)
        state.set_key_042780(random.key_from_response_034c30(response));
    else
        state.set_fixed_key_084c98();
    printer.open("ChallengeAccepted");printer.attribute("response",response);printer.close();
    return true;
}

std::string dispatch_032d00(const XmlDocument& document,Cipher& state,XmlBackend& xml,
                            CryptoFactory& factory,Random& random,Effects& effects) {
    const auto* root=document.first_element();
    if(!root || root->name()!="LSX")effects.invalid_message_02bd30_04d228();
    const auto* request=root->first_element();
    if(!request || request->name()!="Request" || request->next_element())
        effects.invalid_message_02bd30_04d228();
    const auto id=request->attribute("id");
    const auto recipient=request->attribute("recipient");
    const auto* command=request->first_element();
    if(!id || !recipient || !command)effects.invalid_message_02bd30_04d228();
    const Registration* selected=nullptr;
    for(const auto& registration:registrations_032d00())
        if(registration.name==command->name()){selected=&registration;break;}
    if(!selected && command->name()!="ChallengeResponse")effects.invalid_message_02bd30_04d228();
    auto printer=xml.printer();
    printer->open("LSX");printer->open("Response");
    printer->attribute("id",*id);printer->attribute("sender",*recipient);
    const bool accepted=selected
        ?effects.registered_handler(selected->rva,*command,*printer)
        :challenge_response_02d7e0(*command,*printer,state,factory,random,effects);
    if(!accepted)effects.invalid_message_02bd30_04d228();
    printer->close();printer->close();
    const auto buffer=printer->buffer_including_terminator();
    return std::string(buffer.c_str()); // 009fd0 copies through first NUL
}

std::string process_message_033d70(State state,std::string& input,XmlBackend& xml,
                                  CryptoFactory& factory,Random& random,Effects& effects) {
    bool retried_version=false;
    for(;;) {
        if(state.encrypted_0c4) {
            hex_decode_03ce30(input);
            if(!state.cipher.decrypt_042480(input)) {
                effects.log_decryption_failure(input);
                return {};
            }
        }
        // tinyxml2 gets a C string with size_t(-1), so embedded NUL ends parsing.
        auto parsed=xml.parse_c_string(std::string_view(input.c_str()));
        if(!parsed.has_error) {
            if(state.show_lsx_0a3ef0)effects.log_received(input);
            std::string response;
            try{response=dispatch_032d00(*parsed.document,state.cipher,xml,factory,random,effects);}
            catch(const NoResponseRequiredError&){return response;}
            catch(const UnknownMessageError&){
                auto* diagnostic=dynamic_cast<UnknownMessageEffects*>(&effects);if(!diagnostic)throw;
                struct Input final:UnknownMessageInput {std::string& input;explicit Input(std::string& s):input(s){}std::string_view view()const override{return input;}} request(input);
                diagnostic->unknown_message_07bd70(request);return response;
            }
            if(state.show_lsx_0a3ef0)effects.log_response(response);
            if(!state.encrypted_0c4) {
                response.push_back('\0');
                return response;
            }
            if(!state.cipher.encrypt_042610(response))return {};
            hex_encode_03d190(response);response.push_back('\0');
            if(state.events.nonempty_relaxed()) {
                if(state.events.lock_04a6ac()!=0)effects.thread_error_04a648(5);
                auto& count=state.events.lock_count_4c();
                if(count==std::numeric_limits<std::int32_t>::max()) {
                    --count;effects.thread_error_04a648(6);
                }
                if(state.events.nonempty_locked()) {
                    auto event=state.events.copy_front_locked();
                    if(state.show_lsx_0a3ef0)effects.log_event(event);
                    if(state.cipher.encrypt_042610(event)) {
                        hex_encode_03d190(event);event.push_back('\0');response+=event;
                    }
                    // Failed event encryption still consumes exactly one event.
                    state.events.pop_front_locked();
                }
                state.events.unlock_04a6b4();
            }
            return response;
        }
        if(parsed.returned_code==8 && !state.encrypted_0c4 && input.find("<LSX>")==std::string::npos) {
            state.encrypted_0c4=true;
            continue;
        }
        if(parsed.returned_code==7 && !retried_version) {
            const auto position=input.find("version=\"\"");
            if(position!=std::string::npos)input.erase(position,10);
            retried_version=true;
            // Retry enters the original loop head, including decryption again.
            continue;
        }
        effects.log_bad_xml(parsed.document->error_name(),input);
        return {};
    }
}
} // namespace anadius::messages_recovered
