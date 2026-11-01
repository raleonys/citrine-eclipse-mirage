#pragma once
#include "../anadius-encoding-source/encoding.hpp"
#include <functional>
#include <memory>

namespace anadius::callback_identity {
// Source checkpoint wrapper, NOT a frozen compiler locale or handler. Every
// actual lower call is preceded and followed by the selected PTD permission.
// No mutex spans lower calls. Returned counts/errors and output prefixes remain
// untouched. A failed postcondition takes precedence over a lower exception;
// no output rollback, blind FLS restoration or provider retry is performed.
// Arbitrary lower-provider reentry, raw PTD free/ABA and compiler locale/handler
// mutation remain caller constraints; pointer checkpoints cannot freeze them.
class CheckedEncoding final:public encoding_recovered::Calls {
public:
    CheckedEncoding(std::unique_ptr<encoding_recovered::Calls>,std::function<bool()>);
    std::uint32_t locale_codepage_0629ec() override;
    bool are_file_apis_ansi() override;
    int multi_byte_to_wide(std::uint32_t,std::uint32_t,const char*,int,char16_t*,int) override;
    int wide_to_multi_byte(std::uint32_t,std::uint32_t,const char16_t*,int,char*,int,bool*) override;
    std::uint32_t last_error() override;
private:
    void check();
    template<class F>auto invoke(F&& call)->decltype(call());
    std::unique_ptr<encoding_recovered::Calls> lower_;
    std::function<bool()> permission_;
};
std::unique_ptr<encoding_recovered::Calls> check_encoding(std::unique_ptr<encoding_recovered::Calls>,std::function<bool()>);
} // namespace anadius::callback_identity
