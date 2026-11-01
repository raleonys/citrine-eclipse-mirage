#pragma once
#include "src/achievements/utc.hpp"
#include "src/achievements/storage.hpp"
#include "src/handlers/content.hpp"

namespace preloader_l::achievement_runtime {
// New source composition, not a separately identified original function.
// One caller-owned State/Paths supplies the native shared-global identities.
// Other content/provider methods stay abstract; no successful no-op effects.
class PersistenceEffects:public content::Effects {
public:
    PersistenceEffects(configuration::State& state,achievement_storage::Paths& paths,
        Clock& clock,Formatter& formatter,achievement_writer::Effects& directories,
        files::Factory& files)
        :state_(state),paths_(paths),clock_(clock),formatter_(formatter),
         directories_(directories),files_(files){}
    std::string utc_time_041b60(std::uint64_t&) final;
    std::unique_ptr<content::DocumentWriter> printer_0019b0() final;
    void write_xml_03c1d0(const configuration::Path&,std::string_view) final;
    void save_steam_01f9a0(const content::AchievementMap&) final;
protected:
    configuration::State& state_;
    achievement_storage::Paths& paths_;
    Clock& clock_;
    Formatter& formatter_;
    achievement_writer::Effects& directories_;
    files::Factory& files_;
};
// Vectors are borrowed as spans: do not reallocate them while using this view.
// Mutex/event/global lifetimes and initialization remain caller obligations.
content::StateReferences bind_state(configuration::State&,
    const achievement_storage::Paths&,const std::uint8_t& online,
    std::int32_t& saved_avatar_size,content::Mutex&,
    content::EventQueue&);
} // namespace preloader_l::achievement_runtime
