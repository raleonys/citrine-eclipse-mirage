#pragma once
#include "dialog.hpp"
#include "../anadius-bootstrap-backend-source/backend.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include <memory>
#include <ostream>

namespace anadius::library_effects {
// Binds five existing library Effects policies. The two phase operations stay
// in the owned lower provider; SourceBackend's LibraryExecution intercepts
// those operations to its shared Execution/ready state. No replacement state,
// guessed defaults, loading, callback registration or I/O at construction.
class LibraryEffects final:public recovered::libraries::Effects {
public:
    LibraryEffects(std::shared_ptr<std::ostream>,std::shared_ptr<Dialog>,
        std::unique_ptr<recovered::libraries::Effects>,
        std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>());
    void begin_load_log_0863e0() override;
    void append_log_path_and_flush(std::string_view) override;
    void message_038bc0(std::string,std::string,std::uint32_t) override;
    [[noreturn]] void throw_runtime_00cd70(int) override;
    [[noreturn]] void throw_filesystem_00cdb0(std::uint32_t) override;
    void wait_steps_036880(int) override;
    void finish_flag_036820() override;
private:
    // Keep stream AND its streambuf live; serialize all stream users, including
    // B10/B11, externally. These methods are separate source log operations,
    // not an invented atomic line transaction. Lower dies before its providers.
    std::shared_ptr<std::ostream> stream_;
    std::shared_ptr<Dialog> dialog_;
    std::unique_ptr<encoding_recovered::Errors> errors_;
    std::unique_ptr<recovered::libraries::Effects> lower_;
};
bootstrap_backend::Resources bind_library_effects(bootstrap_backend::Resources,
    std::shared_ptr<std::ostream>,std::shared_ptr<Dialog>,
    std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>());
} // namespace anadius::library_effects
