#pragma once
#include "../anadius-native-context-source/context.hpp"
#include <mutex>
#include <stdexcept>

namespace anadius::callback_admission {
enum class Fault {ForeignThread,Reentry,Stopped};
class Rejected final:public std::logic_error {
public:
    explicit Rejected(Fault);
    const Fault fault;
};
// A bounded SOURCE policy enforcing the B16 single-controller/no-reentry
// contract at all ten native entries. This is not foreign-thread PTD support
// or a general concurrent state lock. It never waits while a callback body,
// original function, provider or Execution wait runs. Direct core/Effects calls
// and other map/config/stream users still require external serialization.
class Controller final:public native_recovered::Admission {
public:
    Controller();
    ~Controller() noexcept override;
    Controller(const Controller&)=delete;Controller& operator=(const Controller&)=delete;
    Controller(Controller&&)=delete;Controller& operator=(Controller&&)=delete;
    void enter(native_recovered::Callback) override;
    void leave(native_recovered::Callback) noexcept override;
    // Controller only. Active callback returns false without stopping. A
    // successful stop is permanent. It does not disable hooks or prove external
    // quiescence, stop workers, drain cleanup, close Binding or unload anything.
    bool stop();
    bool stopped() const;
    bool active() const;
private:
    const DWORD controller_;
    mutable std::mutex gate_;
    bool stopped_=false,active_=false;
    native_recovered::Callback callback_{};
};
// Construct/consume/publish/close on the same controller. Native Context owns
// admission through successful close. Proof false/throw retains an OPEN gate;
// only a successful external proof is followed by stop(), then native close.
// Existing B15 factory remains available without the new optional policy.
bootstrap_backend::CallbackAttachmentFactory make_factory(native_context::CallbackSeed,
    native_context::PolicyFactory,native_recovered::Errors& permanent_errors,native_context::Quiescence);
} // namespace anadius::callback_admission
