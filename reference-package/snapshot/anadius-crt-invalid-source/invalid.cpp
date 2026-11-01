#include "invalid.hpp"
#include <bit>
#include <cstring>

namespace anadius::crt_invalid_recovered {
namespace {
template<class T> T read_at(void* p, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<std::byte*>(p) + offset, sizeof value);
    return value;
}
template<class T> void write_at(void* p, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof value);
}
}
Context make_context(Effects& effects) {
    Context context;
    if (effects.locale_changed_0a52d8() == 0) {
        context.locale_snapshot = effects.locale_pair_0a1938();
        context.locale_mode = 1;
    }
    return context;
}
void* optional_context_ptd_05c5d0(Context& context, Effects& effects) {
    auto* observed = context.ptd;
    if (observed == nullptr) {
        auto saved_error = effects.last_error();
        std::uint64_t index = 0;
        if (context.state_index_ready == 0) {
            context.state_index = 0;
            context.state_index_ready = 1;
        } else {
            index = context.state_index;
        }
        observed = effects.optional_ptd_064dd0(&saved_error, index);
        context.ptd = observed;
        effects.set_last_error(saved_error);
    }
    return observed;
}
void* required_context_ptd_052500(Context& context, Effects& effects) {
    if (context.ptd == nullptr) {
        if (optional_context_ptd_05c5d0(context, effects) == nullptr) {
            effects.missing_ptd_05f03c();
        }
    }
    return context.ptd;
}
std::uint64_t state_index_05c63c(Context& context, Effects& effects) {
    if (context.state_index_ready != 0) return context.state_index;
    const auto saved_error = effects.last_error();
    context.state_index = 0;
    context.state_index_ready = 1;
    effects.set_last_error(saved_error);
    return 0;
}
void release_context(Context& context, Effects& effects) {
    if (context.locale_mode == 2) {
        const auto flags = read_at<std::uint32_t>(context.ptd, 0x3a8);
        write_at(context.ptd, 0x3a8, flags & ~std::uint32_t{2});
    }
    if (context.restore_errno != 0) {
        const auto saved = context.saved_errno;
        auto* ptd = required_context_ptd_052500(context, effects);
        write_at(ptd, 0x20, saved);
    }
    if (context.restore_doserrno != 0) {
        const auto saved = context.saved_doserrno;
        auto* ptd = required_context_ptd_052500(context, effects);
        write_at(ptd, 0x24, saved);
    }
}
void select_handler_05c888(const Arguments& args, Context& context, Effects& effects) {
    Address handler = 0;
    if (auto* ptd = optional_context_ptd_05c5d0(context, effects)) {
        handler = read_at<Address>(ptd, 0x3b8);
    }
    if (handler == 0) {
        const auto index = state_index_05c63c(context, effects);
        const auto encoded = effects.encoded_global_handler_0a4f00(index);
        const auto cookie = effects.security_cookie_0a1580();
        handler = std::rotr(encoded ^ cookie, static_cast<int>(cookie & 63));
        if (handler == 0) effects.watson_05c9a4(args);
    }
    effects.invoke_handler(handler, args);
}
void invalid_parameter_05c7ec(const Arguments& args, Effects& effects) {
    auto context = make_context(effects);
    select_handler_05c888(args, context, effects);
    // Explicit normal-return cleanup. Native unwind/SEH is not established.
    release_context(context, effects);
}
void invalid_parameter_noinfo_05c954(Effects& effects) {
    invalid_parameter_05c7ec({}, effects);
}
[[noreturn]] void invalid_parameter_noreturn_05c974(Effects& effects) {
    invalid_parameter_05c7ec({}, effects);
    effects.watson_05c9a4({});
}
} // namespace anadius::crt_invalid_recovered
