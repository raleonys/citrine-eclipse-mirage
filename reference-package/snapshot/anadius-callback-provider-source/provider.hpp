#pragma once
#include "../anadius-callback-policy-source/policy.hpp"

namespace anadius::callback_provider {
enum class Operation {CompareA,CompareW,CopyA,CopyW};
enum class InvalidReason {NullInput,Destination,Range};
struct InvalidCall {Operation operation;InvalidReason reason;int error;};
using InvalidHandler=std::function<void(InvalidCall)>;
// Required explicit handler may return or throw. Fully readable NUL-terminated
// strings, nonoverlapping output and capacity <= INT_MAX are the supported domain.
// Valid output is cleared before invalid-copy notification. No CRT debug fill,
// global/thread handler mutation or original CRT ABI equivalence is claimed.
std::shared_ptr<callback_policy::CrtProvider> make_crt(std::string locale,InvalidHandler);
// Retains lower/context and permission; no mutex spans provider calls. Invalid
// postcondition wins over lower exception; output/error prefixes are not undone.
// ByteWide staging/invalid-output preclear precedes this LOWER CRT checkpoint;
// this is not an effect-free guard at the entire wide bridge entry.
std::shared_ptr<callback_policy::CrtProvider> check_crt(callback_policy::CrtApi,
    std::shared_ptr<callback_policy::CrtProvider>,std::function<bool()>);
enum class Filesystem {Utf8,Ansi,Oem};
struct EncodingSelection {
    Filesystem filesystem;
    std::uint32_t ansi_codepage;
    std::uint32_t oem_codepage;
};
// Freeze synthetic selector and explicit numerical alias resolutions. Recovered
// helpers still select cp65001/0/1 and flags BEFORE alias resolution. No implicit
// UTF8 flag upgrade: an alias resolved to65001 can fail with retained flags.
// Compiler locale and recovered PTD initial locale are separate from this policy.
std::unique_ptr<encoding_recovered::Calls> make_encoding(EncodingSelection,
    std::unique_ptr<encoding_recovered::Calls> lower=std::make_unique<encoding_recovered::NativeCalls>());
// Call before foreign make_factory/publication. Strong construction guarantee;
// preserves memory/stream/errors/permission, factory adds selected checkpoints.
void select(callback_policy::Resources&,std::string locale,InvalidHandler,EncodingSelection);
} // namespace anadius::callback_provider
