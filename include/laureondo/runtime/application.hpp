// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/config/configuration.hpp"
#include <memory>

namespace laureondo::application {
// Application fields projected from RE constructor036d20; standard compiler CRT.
configuration::State initial_state();
struct Options {
    configuration::Path executable_path; // empty: actual process executable
    configuration::Path data_root;       // empty: RE LocalAppData environment/SURGE
    bool install_hooks=true;
    configuration::Path program_data; // empty: RE ProgramData environment value
};
class Application final {
public:
    explicit Application(Options={});
    ~Application();
    Application(const Application&)=delete;
    Application& operator=(const Application&)=delete;
    void start();
    void stop();
    // Startup fields are immutable after start. Inspect mutable achievement
    // state only after stop has joined all requests; this is not a live snapshot.
    const configuration::State& configuration() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
