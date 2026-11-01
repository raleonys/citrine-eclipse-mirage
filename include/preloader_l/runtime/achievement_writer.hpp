// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/config/configuration.hpp"
#include "preloader_l/core/binary_io.hpp"

namespace preloader_l::achievement_writer {
class Effects {
public:
    virtual ~Effects()=default;
    // Must propagate the existing filesystem error contract. No default OS call.
    virtual void create_directories_018e00(const configuration::Path&)=0;
};
// Logical byte serialization used by01f9a0, not a separate original procedure.
// Ordered by the source achievement map key; section name is record.steam_id.
std::string serialize_ini(const configuration::State&);
// Empty ini_path skips all work. Byte format and parent policy are preserved;
// checked atomic replacement supersedes the recovered ignored write result.
void write_ini_01f9a0(const configuration::Path& ini_path,
    const configuration::State&,Effects&,file::Factory&);
} // namespace preloader_l::achievement_writer
