#include "context.hpp"
#include <cstdlib>
#include <stdexcept>
#include <type_traits>

namespace anadius::native_context {
namespace {
class Control {
    bool& value_;
public:
    explicit Control(bool& value):value_(value){if(value_)throw std::logic_error("Native Context control reentry");value_=true;}
    ~Control(){value_=false;}
};
std::unique_ptr<native_recovered::OriginalEffects> policies(PolicyView view,PolicyFactory& factory){
    if(!factory)throw std::invalid_argument("Explicit native callback policy factory required");
    auto result=factory(view);
    if(!result||result->original_slots_identity()!=&view.backend.runtime.originals)
        throw std::invalid_argument("Native callback policy must reference exact moved runtime original slots");
    return result;
}
Quiescence checked(Quiescence value){if(!value)throw std::invalid_argument("Explicit callback quiescence proof required");return value;}
}
Owner::Owner(bootstrap_backend::CallbackStateView state,CallbackSeed callbacks,PolicyFactory factory,
    native_recovered::Errors& errors,Quiescence quiescence,std::unique_ptr<native_recovered::Admission> admission)
    :state_(state),callbacks_(std::move(callbacks)),quiescence_(checked(std::move(quiescence))),errors_(errors),
     admission_(std::move(admission)),effects_(policies({state_,callbacks_},factory)),
     context_{{state_.runtime.registry_values,callbacks_.client,callbacks_.language,callbacks_.fabricated,state_.configuration.registry_spoof},
        {callbacks_.ipv4_length,callbacks_.ipv6_length,state_.configuration.server_port,callbacks_.attempts},
        *effects_,*effects_,*effects_,admission_.get()}{}
Owner::~Owner() noexcept{
    // Never silently unpublish an enabled callback domain or destroy its state.
    if(control_busy_||binding_)std::abort();
}
void Owner::publish(){
    Control control(control_busy_);
    if(closed_)throw std::logic_error("Closed native Context cannot republish");
    if(published_)return;
    binding_.emplace(context_,errors_);published_=true;
}
void Owner::host_setup_begin(){
    Control control(control_busy_);if(closed_)throw std::logic_error("Closed callback setup");
    if(admission_)admission_->host_setup_begin();
    if(auto* provider=effects_->original_provider())provider->setup_begin();
}
void Owner::host_setup_complete(){
    Control control(control_busy_);if(closed_)throw std::logic_error("Closed callback setup");
    if(auto* provider=effects_->original_provider())provider->setup_complete();
    if(admission_)admission_->host_setup_complete();
}
bool Owner::pause_before_cleanup(){
    Control control(control_busy_);
    if(closed_||!published_)return true;
    if(admission_&&!admission_->pause_before_cleanup())return false;
    if(native_recovered::active_calls()!=0)return false;
    if(auto* provider=effects_->original_provider())provider->stop();
    return true;
}
bool Owner::close_after_cleanup(){
    Control control(control_busy_);
    if(closed_)return true;
    if(!published_){if(auto* provider=effects_->original_provider())provider->stop();closed_=true;return true;}
    if(!quiescence_())return false;
    if(native_recovered::active_calls()!=0)return false;
    binding_->close(); // native gate rechecks; a faulty quiescence promise may throw
    if(auto* provider=effects_->original_provider())provider->stop();
    binding_.reset();closed_=true;return true;
}
bootstrap_backend::CallbackAttachmentFactory make_factory(CallbackSeed seed,PolicyFactory factory,
    native_recovered::Errors& errors,Quiescence quiescence,std::unique_ptr<native_recovered::Admission> admission){
    if(!factory||!quiescence)throw std::invalid_argument("Explicit policy factory and quiescence required");
    struct State {
        CallbackSeed seed;PolicyFactory factory;Quiescence quiescence;native_recovered::Errors& errors;
        std::unique_ptr<native_recovered::Admission> admission;bool used=false;
    };
    auto state=std::make_shared<State>(State{std::move(seed),std::move(factory),std::move(quiescence),errors,std::move(admission)});
    return [state](bootstrap_backend::CallbackStateView view)->std::unique_ptr<bootstrap_backend::CallbackAttachment>{
        if(state->used)throw std::logic_error("Native callback attachment factory already consumed");
        state->used=true;
        return std::make_unique<Owner>(view,std::move(state->seed),state->factory,state->errors,state->quiescence,std::move(state->admission));
    };
}
static_assert(!std::is_abstract_v<Owner>&&!std::is_move_constructible_v<Owner>);
} // namespace anadius::native_context
