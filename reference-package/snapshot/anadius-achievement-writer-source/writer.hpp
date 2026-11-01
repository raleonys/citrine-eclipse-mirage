#pragma once
#include "../anadius-config-source/configuration.hpp"
#include "../anadius-file-source/binary_io.hpp"

namespace anadius::achievement_writer {
class Effects {
public:
    virtual ~Effects()=default;
    // Must propagate the existing filesystem error contract. No default OS call.
    virtual void create_directories_018e00(const configuration::Path&)=0;
};
// Logical byte serialization used by01f9a0, not a separate original procedure.
// Ordered by the source achievement map key; section name is record.steam_id.
std::string serialize_ini(const configuration::State&);
// Empty ini_path skips all work. Uses actual recovered path/file source bodies.
// The03c1d0 boolean is ignored, matching the original caller.
void write_ini_01f9a0(const configuration::Path& ini_path,
    const configuration::State&,Effects&,file_recovered::Factory&);
} // namespace anadius::achievement_writer
