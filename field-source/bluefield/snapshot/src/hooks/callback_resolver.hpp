// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/hooks/setup.hpp"
namespace hook_engine {
class CallbackResolver {
public:
    virtual ~CallbackResolver()=default;
    virtual std::uintptr_t rebuilt_address(preloader_l::host::Callback)=0;
};
}
