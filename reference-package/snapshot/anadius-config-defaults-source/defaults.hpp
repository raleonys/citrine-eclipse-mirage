#pragma once
#include "../anadius-config-source/configuration.hpp"

namespace anadius::configuration_defaults {
// Explicit opt-in: post-construction values for the 41 modeled State fields.
// Evidence:036d20(&a39e0), invoked by initializer01390. Does not reconstruct the
// enclosing MSVC aggregate, its mutexes/storage, image entry or teardown.
// Returns a fresh independent value; no existing caller state is mutated.
configuration::State make_configuration_defaults_036d20();
} // namespace anadius::configuration_defaults
