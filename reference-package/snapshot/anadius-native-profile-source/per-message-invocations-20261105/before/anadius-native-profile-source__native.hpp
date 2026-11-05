#pragma once
#include "../anadius-session-profile-source/profile.hpp"
#include "../anadius-socket-source/native_message.hpp"
#include "../anadius-messages-source/messages.hpp"

namespace anadius::native_profile {
namespace sp=session_profile;namespace s=socket_recovered;namespace m=messages_recovered;
// Exact typed queue/mutex lower. The provider owns queue slots and their string
// prefixes for the entire invocation, including lock/log/crypto callbacks.
// A cancellation request or an empty queue is not a lifetime/drain proof.
class Events {
public:
    virtual ~Events()=default;
    virtual s::NativeMessageEventQueue& queue()=0;
    virtual s::NativeMessageMutex& mutex()=0;
    virtual int lock(s::NativeMessageMutex&)=0;
    virtual void unlock(s::NativeMessageMutex&)=0;
};
struct Dependencies {
    std::shared_ptr<void> lifetime;
    std::shared_ptr<m::XmlBackend> xml;
    std::shared_ptr<m::CryptoFactory> factory;
    std::shared_ptr<m::Random> random;
    std::shared_ptr<m::Effects> effects;
    std::shared_ptr<Events> events;
    std::uint8_t show_lsx=0;
    std::string version;
};
// Optional independent HOST interface; existing Logic/SessionLogic virtual
// slots are unchanged. Caller keeps exact job claim and both string prefixes
// alive, including any retained unknown prefix. No scheduling/job completion.
class JobLogic {
public:
    virtual ~JobLogic()=default;
    virtual void process_native_job(s::SessionMemoryDomain&,sp::Strings&,
        s::ReceiveBufferString& output,s::ReceiveBufferString& request,
        const s::SessionMemoryDomain::JobPermit&,std::function<void()> job_admission)=0;
};
// Concrete optional SessionLogic route.033d70, string growth/hex and prefix AES
// use recovered typed bodies. XML documents/032d00 use selected existing C++
// interfaces, NOT native DOM/printer/registry ABI. No native EH rollback.
class Logic:public sp::SessionLogic {
public:
    Logic(sp::SessionView,Dependencies);
    ~Logic()noexcept override;
    std::string greeting()override;
    virtual std::string greeting_native(s::SessionMemoryDomain&,sp::Strings&,std::function<void()> checkpoint);
    std::string process(std::string&)final; // typed route must not silently stage
    // Existing whole Parent Operation and source admission required. Checkpoint
    // runs before/after every opaque provider. Unknown prefixes retain the owned
    // invocation and reject reuse; destruction then fails closed.
    void process_native(s::SessionMemoryDomain&,sp::Strings&,
        s::ReceiveBufferString& output,s::ReceiveBufferString& request,
        std::function<void()> checkpoint);
    bool retained_prefix()const noexcept;
    bool matches(sp::SessionView)const noexcept;
private:
    struct Invocation;
    void validate_owner(s::SessionMemoryDomain&)const;
    sp::SessionView session_;Dependencies deps_;
    std::unique_ptr<Invocation> invocation_;
    bool poisoned_=false,retiring_=false;
};
} // namespace anadius::native_profile
