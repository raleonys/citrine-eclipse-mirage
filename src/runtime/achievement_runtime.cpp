// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/achievement_runtime.hpp"
#include "citrine_eclipse_mirage/xml/memory_printer.hpp"
#include "citrine_eclipse_mirage/core/paths.hpp"
#include <stdexcept>

namespace citrine_eclipse_mirage::achievement_runtime {
namespace {
class WriteOnlyReader final:public xml::AttributeReader {
    const char* attribute(handlers::Request,const char*) override {
        throw std::logic_error("Output-only achievement document cannot read requests");
    }
};
class Document final:public content_handlers::DocumentWriter {
public:
    Document():writer_(printer_,reader_){}
    content_handlers::Xml& xml() override{return writer_;}
    std::string c_string_without_nul() const override {
        const auto bytes=printer_.bytes_without_terminator();
        return std::string(bytes.substr(0,bytes.find('\0')));
    }
private:
    xml::MemoryPrinter printer_;
    WriteOnlyReader reader_;
    xml::HandlerWriter writer_;
};
}
std::string PersistenceEffects::utc_time_041b60(std::uint64_t& epoch) {
    return achievement_runtime::utc_time_041b60(clock_,formatter_,&epoch,0);
}
std::unique_ptr<content_handlers::DocumentWriter> PersistenceEffects::printer_0019b0() {
    return std::make_unique<Document>();
}
void PersistenceEffects::write_xml_03c1d0(const configuration::Path& path,std::string_view bytes) {
    const file::FileWrite write{path,bytes};files_.replace_atomic({&write,1});
}
void PersistenceEffects::save_steam_01f9a0(const content_handlers::AchievementMap& map) {
    // Native01f9a0 reads the one global map. Do not silently serialize a
    // different State if a caller supplies incompatible source identities.
    if(&map!=&state_.achievements)throw std::invalid_argument("Achievement map must be shared State map");
    achievement_writer::write_ini_01f9a0(paths_.ini_path,state_,directories_,files_);
}
void PersistenceEffects::persist_achievement(const configuration::Path& path,std::string_view xml,
    const content_handlers::AchievementMap& map) {
    if(&map!=&state_.achievements)throw std::invalid_argument("Achievement map must be shared State map");
    std::vector<file::FileWrite> writes;
    if(!path.empty())writes.push_back({path,xml});
    std::string ini;
    if(!paths_.ini_path.empty()) {
        // Validate and prepare the complete watcher INI before creating its
        // parent directory. A rejected section name must have no disk effects.
        ini=achievement_writer::serialize_ini(state_);
        directories_.create_directories_018e00(path::parent_path_019820(paths_.ini_path));
        writes.push_back({paths_.ini_path,ini});
    }
    if(!writes.empty())files_.replace_atomic(writes);
}
content_handlers::StateReferences bind_state(configuration::State& state,
    const achievement_storage::Paths& paths,const std::uint8_t& online,
    std::int32_t& saved_avatar_size,content_handlers::Mutex& mutex,
    content_handlers::EventQueue& events) {
    return {state.content,state.entitlements,state.game_attributes,state.achievements,
        state.achievements_set,state.content_id,state.user_id,paths.xml_path,state.avatar,
        state.default_avatar,state.user_directory,state.last_avatar_path,online,
        saved_avatar_size,mutex,events};
}
} // namespace citrine_eclipse_mirage::achievement_runtime
