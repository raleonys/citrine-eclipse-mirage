#pragma once
#include "../anadius-application-graph-source/graph.hpp"
#include "../anadius-achievement-storage-source/path_binding.hpp"
#include "../anadius-license-source/auth_state.hpp"

namespace anadius::application_effects {
// Backend lends its actual owned component before Worker publication. Borrow
// ends only after worker-affine providers retire and the worker is joined.
class OnlineBinding {
public:
    virtual ~OnlineBinding()=default;
    virtual void bind_effects_online(bootstrap_backend::OnlineComponent&)=0;
};
struct Providers {
    std::shared_ptr<handlers_recovered::Effects> information;
    std::shared_ptr<content_handlers_recovered::Effects> content;
    std::shared_ptr<dispatch_recovered::LicenseHandlers> licensing;
};
struct Context {
    bootstrap_backend::CallbackStateView state;
    bootstrap_backend::OnlineComponent& online;
    const achievement_storage::Paths& paths;
    std::ostream& stream;
    content_handlers_recovered::CallCheckpoint checkpoint;
};
class Composer {
public:
    virtual ~Composer()=default;
    // Worker owns these output slots BEFORE the call. Adopt each constructed
    // provider before a checkpoint; leave partial output in place on failure.
    // Passive setup only: startup fields may still be concurrently populated.
    // Context is borrowed synchronously and must not escape this call.
    virtual void compose(Providers&,const Context&)=0;
};
}
