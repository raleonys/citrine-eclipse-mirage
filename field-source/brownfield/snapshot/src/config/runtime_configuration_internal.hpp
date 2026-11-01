#pragma once

#include "preloader_l/config/runtime_configuration.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace config { namespace runtime_configuration_detail {

struct Setting {
    size_t name_off;
    size_t name_len;
    size_t value_off;
    size_t value_len;
    bool used;
};

struct State {
    bool loaded;
    Setting* settings;
    size_t setting_count;
    size_t setting_capacity;
    uint32_t present;
    char* arena;
    size_t arena_used;
    size_t arena_capacity;
};

enum class ParseCode : uint8_t { ok=0, invalid, resource };

void* transient_alloc(size_t bytes) noexcept;
void transient_free(void* ptr,size_t bytes) noexcept;
void reset_state(State& st) noexcept;
size_t cstr_len(const char* s) noexcept;
bool bytes_equal(const char* a,size_t an,const char* b,size_t bn) noexcept;
bool put_setting(State& st,const char* key,size_t key_len,const char* value,size_t value_len) noexcept;

ParseCode parse_config2(State& st,const char* data,size_t length) noexcept;
RuntimeConfigurationStatus map_code(ParseCode c) noexcept;

} } }
