#pragma once
#include "../anadius-native-source/native_callbacks.hpp"
#include "../anadius-bootstrap-backend-source/backend.hpp"
#include <functional>
#include <memory>
#include <optional>

namespace anadius::native_context {
// Explicit caller seed for the remaining callback state. These are source
// values, not guessed native image initializers. The owner retains these sets
// and counters at stable addresses through successful binding shutdown.
struct CallbackSeed {
    registry_recovered::Handles client,language,fabricated;
    std::int32_t ipv4_length,ipv6_length;
    std::uint32_t attempts;
};
struct PolicyView {bootstrap_backend::CallbackStateView backend;CallbackSeed& callbacks;};
using PolicyFactory=std::function<std::unique_ptr<native_recovered::OriginalEffects>(PolicyView)>;
// True must establish disabled hooks and exclude ALL new native callback
// admissions, including cached entry pointers. In-flight leases may remain;
// they keep close false. A count snapshot or lifetime token is not this proof.
using Quiescence=std::function<bool()>;
class Owner final:public bootstrap_backend::CallbackAttachment {
public:
    // Optional source admission is owned at a stable address through native
    // Binding/Context shutdown. Context layout and this constructor extension
    // require rebuilding all consumers; SDK entry signatures are unchanged.
    Owner(bootstrap_backend::CallbackStateView,CallbackSeed,PolicyFactory,
        native_recovered::Errors& permanent_errors,Quiescence,
        std::unique_ptr<native_recovered::Admission> = {});
    ~Owner() noexcept override;
    Owner(const Owner&)=delete;Owner& operator=(const Owner&)=delete;
    Owner(Owner&&)=delete;Owner& operator=(Owner&&)=delete;
    void publish() override;
    void host_setup_begin() override;
    void host_setup_complete() override;
    bool pause_before_cleanup() override;
    bool close_after_cleanup() override;
private:
    bootstrap_backend::CallbackStateView state_;
    CallbackSeed callbacks_;
    Quiescence quiescence_;
    native_recovered::Errors& errors_;
    std::unique_ptr<native_recovered::Admission> admission_;
    std::unique_ptr<native_recovered::OriginalEffects> effects_;
    native_recovered::Context context_;
    std::optional<native_recovered::Binding> binding_;
    bool published_=false,closed_=false,control_busy_=false;
};
// One-shot factory: copied function objects share the same consumption guard.
// Permanent errors outlive all future callback entry attempts, even after close.
bootstrap_backend::CallbackAttachmentFactory make_factory(CallbackSeed,PolicyFactory,
    native_recovered::Errors& permanent_errors,Quiescence,
    std::unique_ptr<native_recovered::Admission> = {});
} // namespace anadius::native_context
