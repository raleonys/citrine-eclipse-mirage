#pragma once
#include "../anadius-native-source/native_callbacks.hpp"
#include "../anadius-bootstrap-native-source/native_entries.hpp"

namespace anadius::permanent_errors {
struct Policies {
    native_recovered::Errors& host;
    bootstrap_native_recovered::Errors& bootstrap;
};
// Explicit HOST policy selection, not a recovered original exception policy.
// Select before publishing either binding, outside loader lock. Both references
// have stable non-owning identities within this mapped provider image. Storage
// has no heap ownership or automatic object destruction and outlives Runtime,
// Context and all their provider owners. Do not delete either reference.
//
// Every error unconditionally calls compiler std::abort; no success return,
// exception rethrow, logging, provider access, cleanup or retry is attempted.
// This storage lifetime DOES NOT retain code/vtables/CRT across image unload,
// establish safe process/loader teardown, or drain future external entries.
// Caller must retain mapped image/code and exclude entries before unloading.
Policies compiler_abort_policy() noexcept;
}
