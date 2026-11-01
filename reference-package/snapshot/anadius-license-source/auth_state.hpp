#pragma once
#include <string>

namespace anadius::license_recovered {
// Owned +138/a3b18 semantic cell. 036d20 writes these exact forty static
// bytes, length0x28/capacity0x2f; std::string host layout/capacity may differ.
// Immutable after construction, stable through all worker/provider retirement.
class AuthenticationCode final {
public:
    AuthenticationCode():value_("SGV5LCB5b3UsIHN0b3AgcmlnaHQgdGhlcmUhIDsp"){}
    AuthenticationCode(const AuthenticationCode&)=delete;
    AuthenticationCode& operator=(const AuthenticationCode&)=delete;
    const std::string& value()const noexcept{return value_;}
private:
    const std::string value_;
};
// Passive source-only optional extension. Owner must outlive all borrowers.
// Bind once before publication/dispatch; no native vtable/ABI is claimed.
class AuthenticationBinding {
public:
    virtual ~AuthenticationBinding()=default;
    virtual void bind_authentication_code(const AuthenticationCode&)=0;
};
}
