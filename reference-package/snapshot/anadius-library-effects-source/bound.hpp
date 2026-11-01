#pragma once
#include "effects.hpp"
#include "phase_binding.hpp"

namespace anadius::library_effects {
struct BoundOptions {
    std::shared_ptr<void> lifetime;
    std::shared_ptr<std::ostream> stream;
    std::shared_ptr<Dialog> dialog;
    std::shared_ptr<encoding_recovered::Errors> errors=std::make_shared<encoding_recovered::PortableErrors>();
};
// Concrete seven-method provider. No lower Effects object. Selected source
// compiler/SDK policy; SourceBackend still owns native wait/path-sync ordering.
// Bind/retire on constructor controller/fiber, after worker joins. Calls may
// run on admitted workers concurrently (a phase wait must not block finish).
class BoundLibrary final:public recovered::libraries::Effects,public PhaseBinding {
public:
    explicit BoundLibrary(BoundOptions);
    ~BoundLibrary()noexcept override;
    BoundLibrary(const BoundLibrary&)=delete;BoundLibrary& operator=(const BoundLibrary&)=delete;
    void bind_library_phase(execution_recovered::Execution&,std::uint8_t&)override;
    void begin_load_log_0863e0()override;
    void append_log_path_and_flush(std::string_view)override;
    void message_038bc0(std::string,std::string,std::uint32_t)override;
    [[noreturn]]void throw_runtime_00cd70(int)override;
    [[noreturn]]void throw_filesystem_00cdb0(std::uint32_t)override;
    void wait_steps_036880(int)override;
    void finish_flag_036820()override;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    Impl& attached()const;
    template<class F>void invoke(F&&);
};
}
