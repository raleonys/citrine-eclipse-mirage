#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <string_view>

namespace anadius::library_effects {
struct DialogApi {
    decltype(&::MultiByteToWideChar) convert;
    decltype(&::MessageBoxW) message;
};
DialogApi native_dialog_api() noexcept;
// Source counterpart of038bc0 for callers that discard its return mapping.
// This owns no window. Native SDK callbacks follow their nonthrowing Win32
// contracts; injected functions must keep borrowed inputs live for the call.
// The selected source domain bounds each input to INT_MAX explicit bytes.
// Original0452d0 instead truncates length to int; that huge-input domain is not
// emulated. No original string/object/EH ABI is adopted.
class Dialog final {
public:
    explicit Dialog(DialogApi=native_dialog_api());
    void show(std::string_view title,std::string_view text,std::uint32_t flags) const;
private:
    std::wstring widen(std::string_view) const;
    DialogApi api_;
};
} // namespace anadius::library_effects
