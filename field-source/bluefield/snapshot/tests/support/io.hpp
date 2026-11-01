// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/application/environment.hpp"
#include "src/files/atomic_io.hpp"
// Fixture bootstrap only; never in the production BUILD_GRAPH.
namespace preloader_l {inline void write_file(const Path& p,std::string_view bytes){atomic_write(native_path(p),bytes,nullptr);}}
