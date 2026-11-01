#include "runtime.hpp"
#include "../anadius-xml-source/memory_printer.hpp"
#include <stdexcept>

namespace anadius::achievement_runtime {
namespace {
class WriteOnlyReader final:public xml_recovered::AttributeReader {
    const char* attribute(handlers_recovered::Request,const char*) override {
        throw std::logic_error("Output-only achievement document cannot read requests");
    }
};
class Document final:public content_handlers_recovered::DocumentWriter {
public:
    Document():writer_(printer_,reader_){}
    content_handlers_recovered::Xml& xml() override{return writer_;}
    std::string c_string_without_nul() const override {
        const auto bytes=printer_.bytes_without_terminator();
        return std::string(bytes.substr(0,bytes.find('\0')));
    }
private:
    xml_recovered::MemoryPrinter printer_;
    WriteOnlyReader reader_;
    xml_recovered::HandlerWriter writer_;
};
}
std::string PersistenceEffects::utc_time_041b60(std::uint64_t& epoch) {
    return achievement_runtime::utc_time_041b60(clock_,formatter_,&epoch,0);
}
std::unique_ptr<content_handlers_recovered::DocumentWriter> PersistenceEffects::printer_0019b0() {
    return std::make_unique<Document>();
}
void PersistenceEffects::write_xml_03c1d0(const configuration::Path& path,std::string_view bytes) {
    (void)file_recovered::write_binary_03c1d0(path,bytes,files_);
}
void PersistenceEffects::save_steam_01f9a0(const content_handlers_recovered::AchievementMap& map) {
    // Native01f9a0 reads the one global map. Do not silently serialize a
    // different State if a caller supplies incompatible source identities.
    if(&map!=&state_.achievements)throw std::invalid_argument("Achievement map must be shared State map");
    achievement_writer::write_ini_01f9a0(paths_.ini_path,state_,directories_,files_);
}
content_handlers_recovered::StateReferences bind_state(configuration::State& state,
    const achievement_storage::Paths& paths,const std::uint8_t& online,
    std::int32_t& saved_avatar_size,content_handlers_recovered::Mutex& mutex,
    content_handlers_recovered::EventQueue& events) {
    return {state.content,state.entitlements,state.game_attributes,state.achievements,
        state.achievements_set,state.content_id,state.user_id,paths.xml_path,state.avatar,
        state.default_avatar,state.user_directory,state.last_avatar_path,online,
        saved_avatar_size,mutex,events};
}
} // namespace anadius::achievement_runtime
