#pragma once
#include "../anadius-config-storage-source/filesystem.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include "../anadius-file-source/binary_io.hpp"

namespace anadius::license_recovered {
// Borrow the live cells used at the configuration call point. In particular,
// User/UserId from the current document is assigned AFTER KeyForLicense.
// These references must not be replaced by a snapshot or independent defaults.
struct KeyState {
    const configuration::Path& license_base;
    const std::string& content_id;
    const std::string& user_id;
};
struct KeyDependencies {
    encoding_recovered::Calls& encoding;
    encoding_recovered::Errors& errors;
    status_recovered::Stats& stats;
    config_storage::Filesystem& filesystem;
    file_recovered::Factory& files;
};

// Recovered normal body at 0x18002a830. Reuses the existing path/status/AES/
// directory/file implementations; no additional Effects lower or factory.
// The owned string models the consumed COPY supplied by the native caller.
// This is a compiler-C++ boundary, not the original MSVC string or EH ABI.
// Borrowed dependencies and state must remain alive and externally serialized.
// No Runtime/Configuration factory binding or real file operation is implicit.
void apply_license_key_02a830(std::string owned_key,KeyState,KeyDependencies);
} // namespace anadius::license_recovered
