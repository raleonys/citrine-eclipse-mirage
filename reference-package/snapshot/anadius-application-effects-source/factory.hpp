#pragma once
#include "composition.hpp"
#include "../anadius-information-effects-source/effects.hpp"
#include "../anadius-content-effects-source/effects.hpp"
#include "../anadius-licensing-effects-source/effects.hpp"

namespace anadius::application_effects {
struct Options {
    // Own backing first so every failed-construction/copy/move prefix also
    // destroys opaque leaf captures before their borrowed attempt cells.
    std::shared_ptr<licensing_effects::AttemptState> attempts;
    information_effects::Options information;
    content_effects::Options content;
    licensing_effects::Options licensing;
    // Explicit HOST owner for the native attempt/block projection. No guessed
    // constructor defaults, resetting or process-global sharing is introduced.
};
// Controller owns immutable provider settings; compose creates all3 concrete
// worker-affine providers on the actual worker, exactly once. Own no worker
// outputs here: normal retirement and unknown-prefix retention belong to Worker.
class Factory final:public Composer {
public:
    explicit Factory(Options);
    ~Factory()noexcept override;
    Factory(const Factory&)=delete;Factory& operator=(const Factory&)=delete;
    void compose(Providers&,const Context&)override;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
}
