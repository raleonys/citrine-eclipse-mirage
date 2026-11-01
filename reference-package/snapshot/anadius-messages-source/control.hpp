#pragma once
#include <string_view>

namespace anadius::messages_recovered {
// Selected HOST C++ control type for the observed NoResponseRequiredError
// catch in033d70. Not the original MSVC object, RTTI or exception ABI. Only the
// checked registered-handler/dispatch path admits this as known control flow.
struct NoResponseRequiredError final {};
struct UnknownMessageError final {};
// Borrow the ORIGINAL request object synchronously. Re-read after prefix/flush
// and again after the counted log/flush before making the dialog's owned copy.
class UnknownMessageInput {
public:
    virtual ~UnknownMessageInput()=default;
    virtual std::string_view view()const=0;
};
// Optional diagnostic boundary; old Effects/NativeMessageApi vtables stay fixed.
// Missing diagnostics cannot be substituted with a successful empty response.
class UnknownMessageEffects {
public:
    virtual ~UnknownMessageEffects()=default;
    virtual void unknown_message_07bd70(const UnknownMessageInput&)=0;
};
}
