// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config/settings_values.hpp"
#include <stddef.h>
#include <stdint.h>

namespace redfield { namespace platform { namespace host_policy_detail {
constexpr uint32_t k_client_active=1u;
constexpr uint32_t k_client_secondary=2u;
constexpr uint32_t k_language=4u;
constexpr uint32_t k_pseudo=8u;

struct LanguagePolicy {
    bool enabled;
    redfield::config::ValueView key;
    redfield::config::ValueView value;
};

void* heap_alloc(size_t n) noexcept;
void heap_free(void* p) noexcept;
size_t cstr_len(const char* s) noexcept;
bool eq_ci(const char* a,size_t an,const char* b,size_t bn) noexcept;
bool wide_eq_utf8_ci(const uint16_t* a,const char* b,size_t bn) noexcept;
redfield::config::ValueView cfg(const char* key) noexcept;
redfield::config::ValueView configured_client_value_name() noexcept;
redfield::config::ValueView active_client_value_name() noexcept;
LanguagePolicy language_policy() noexcept;
uint32_t classify_a(const char* subkey) noexcept;
uint32_t classify_w(const uint16_t* subkey) noexcept;
bool track_handle(uintptr_t key,uint32_t flags) noexcept;
uint32_t handle_flags(uintptr_t key) noexcept;
bool erase_handle(uintptr_t key,uint32_t* flags_out) noexcept;
uintptr_t allocate_pseudo(uint32_t cls) noexcept;
bool begin_teardown_impl() noexcept;
void clear_state_impl() noexcept;
int32_t write_a(redfield::config::ValueView value,const char* fallback,size_t fallback_n,
                uint32_t input_capacity,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept;
int32_t write_w(redfield::config::ValueView value,const uint16_t* fallback,size_t fallback_n,
                uint32_t input_capacity,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept;
bool value_name_a(const char* name,redfield::config::ValueView policy) noexcept;
bool value_name_w(const uint16_t* name,redfield::config::ValueView policy) noexcept;
int32_t after_open(uint32_t cls,int32_t original_status,uintptr_t original_result,uintptr_t* result) noexcept;

} } }
