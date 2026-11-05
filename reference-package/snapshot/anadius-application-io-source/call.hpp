#pragma once
#include "../anadius-session-memory-source/memory.hpp"
#include "../anadius-content-handlers-source/content_handlers.hpp"
#include <stdexcept>

namespace anadius::application_io {
// Immutable source authority for one actual child actor. The caller retains
// the checkpoint's raw backing through every view's explicit normal close.
// Copying a permit extends neither its Operation nor the raw checkpoint life.
class CallContext final {
    using Parent=socket_recovered::SessionMemoryDomain;
    struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
public:
    CallContext(Parent& parent,const Parent::JobPermit& permit,
                content_handlers_recovered::CallCheckpoint checkpoint)
        :parent_(&parent),permit_(permit),checkpoint_(checkpoint),
         thread_(::GetCurrentThreadId()),fiber_(::GetCurrentFiber()){
        Error error;
        if(!checkpoint_.context||!checkpoint_.check)
            throw std::invalid_argument("Complete borrowed provider-call checkpoint required");
        validate(); // metadata/actual actor checks only; no caller callback
    }
    CallContext(const CallContext&)=default;
    CallContext& operator=(const CallContext&)=delete;

    Parent& parent()const noexcept{return *parent_;}
    const Parent::JobPermit& permit()const noexcept{return permit_;}
    DWORD thread()const noexcept{return thread_;}
    void* fiber()const noexcept{return fiber_;}
    bool actor()const noexcept{
        return thread_==::GetCurrentThreadId()&&fiber_==::GetCurrentFiber();
    }
    void validate()const{
        Error error;
        if(!actor())throw std::logic_error("Provider call belongs to another thread/fiber actor");
        permit_.check(*parent_);
        (void)parent_->buffer_context();
        permit_.check(*parent_);
    }
    void check()const{
        // Admission is metadata, including its borrowed checkpoint. Preserve
        // the preceding source SDK result; real provider effects run outside.
        Error error;
        validate();
        checkpoint_.check(checkpoint_.context);
        validate();
    }
private:
    Parent* const parent_;
    const Parent::JobPermit permit_;
    const content_handlers_recovered::CallCheckpoint checkpoint_;
    const DWORD thread_;
    void* const fiber_;
};
} // namespace anadius::application_io
