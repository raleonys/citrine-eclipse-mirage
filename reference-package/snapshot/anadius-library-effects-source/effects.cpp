#include "effects.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <exception>
#include <stdexcept>
#include <type_traits>

namespace anadius::library_effects {
LibraryEffects::LibraryEffects(std::shared_ptr<std::ostream> stream,std::shared_ptr<Dialog> dialog,
    std::unique_ptr<recovered::libraries::Effects> lower,std::unique_ptr<encoding_recovered::Errors> errors)
    :stream_(std::move(stream)),dialog_(std::move(dialog)),errors_(std::move(errors)),lower_(std::move(lower)){
    if(!stream_||!dialog_||!errors_||!lower_)
        throw std::invalid_argument("Owned stream, dialog, errors and lower phase Effects required");
}
void LibraryEffects::begin_load_log_0863e0(){const auto shared=shared_log::acquire(*stream_);*stream_<<"Loading ";}
void LibraryEffects::append_log_path_and_flush(std::string_view text){
    //035260:006860 counted formatted insertion then0088e0 newline/flush.
    const auto shared=shared_log::acquire(*stream_);*stream_<<text;*stream_<<std::endl;
}
void LibraryEffects::message_038bc0(std::string title,std::string text,std::uint32_t flags){dialog_->show(title,text,flags);}
[[noreturn]] void LibraryEffects::throw_runtime_00cd70(int code){errors_->runtime_00cd70(code);std::terminate();}
[[noreturn]] void LibraryEffects::throw_filesystem_00cdb0(std::uint32_t code){errors_->filesystem_00cdb0(code);std::terminate();}
void LibraryEffects::wait_steps_036880(int count){lower_->wait_steps_036880(count);}
void LibraryEffects::finish_flag_036820(){lower_->finish_flag_036820();}
bootstrap_backend::Resources bind_library_effects(bootstrap_backend::Resources r,
    std::shared_ptr<std::ostream> stream,std::shared_ptr<Dialog> dialog,std::unique_ptr<encoding_recovered::Errors> errors){
    r.library_effects=std::make_unique<LibraryEffects>(std::move(stream),std::move(dialog),std::move(r.library_effects),std::move(errors));
    return r;
}
static_assert(!std::is_abstract_v<LibraryEffects>);
} // namespace anadius::library_effects
