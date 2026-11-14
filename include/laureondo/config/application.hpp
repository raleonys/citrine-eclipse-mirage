// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/config/hash_table.hpp"
#include "laureondo/core/binary_io.hpp"

namespace laureondo::config_application {
struct CopyEvent {
    std::string section;
    std::uint64_t bucket_count;
    std::vector<std::string> traversal;
};
struct Trace { std::vector<CopyEvent> copies; };
// Owned first selected Config2 copy, with capacity retained at every depth.
config_hash::Node read_config2(const configuration::Path&,file::Factory&);
// Normal source behavior of 019ad0 after logging. Lookups mutate the local
// container; section and record destinations are reused in original sequence.
// Optional trace is source diagnostics only, never original binary behavior.
void apply_config2(configuration::State&,config_hash::Node& selected_config2,
                   bool override_pass,configuration::Dependencies&,Trace* =nullptr);
// Uses actual file/parser/hash code; Dependencies::load_document is bypassed.
// All other OS/path/locale/log/license boundaries remain caller supplied.
void load_and_apply(configuration::State&,bool override_pass,
                    configuration::Dependencies&,file::Factory&,Trace* =nullptr);
} // namespace laureondo::config_application
