#if !defined(_WIN32)
#error crypt_observer_diagnostics.cpp is Windows-only.
#endif

#if !PRELOADER_L_TRACE_DIAGNOSTICS
#error crypt_observer_diagnostics.cpp is debug-diagnostics-only.
#endif

#include "preloader_l/platform/crypt_observer.hpp"
#include "preloader_l/platform/pe_import.hpp"
#include "preloader_l/platform/crypt_guard.hpp"
#include "preloader_l/platform/identity_state.hpp"
#include "preloader_l/platform/architecture_provenance.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include "preloader_l/runtime/trace_macros.hpp"
#include "crypt_observer_diagnostics_internal.hpp"
#include <stddef.h>
#include <stdint.h>

extern "C" void* _ReturnAddress(void);
#if defined(_MSC_VER)
extern "C" long _InterlockedIncrement(long volatile*);
extern "C" long _InterlockedCompareExchange(long volatile*, long, long);
#pragma intrinsic(_ReturnAddress)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedCompareExchange)
#endif

namespace {
using BOOL = int;
using DWORD = unsigned long;
using HANDLE = void*;
using HMODULE = void*;
using FARPROC = void*;
using HCRYPTMSG = void*;
using HCERTSTORE = void*;
using PCCERT_CONTEXT = const void*;
using CryptMsgGetParamFn = BOOL (__stdcall *)(HCRYPTMSG, DWORD, DWORD, void*, DWORD*);
using CertFindCertificateInStoreFn = PCCERT_CONTEXT (__stdcall *)(
    HCERTSTORE, DWORD, DWORD, DWORD, const void*, PCCERT_CONTEXT);

constexpr DWORD kPageReadWrite = 0x04u;
constexpr DWORD kGetModuleHandleExFlagUnchangedRefcount = 0x00000002u;
constexpr DWORD kGetModuleHandleExFlagFromAddress = 0x00000004u;

extern "C" {
__declspec(dllimport) HMODULE __stdcall GetModuleHandleW(const wchar_t* name);
__declspec(dllimport) BOOL __stdcall GetModuleHandleExW(DWORD flags, const wchar_t* name, HMODULE* module);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE module, const char* name);
__declspec(dllimport) BOOL __stdcall VirtualProtect(void* address, size_t size,
                                                     DWORD new_protect, DWORD* old_protect);
__declspec(dllimport) DWORD __stdcall GetLastError();
__declspec(dllimport) void __stdcall SetLastError(DWORD value);
__declspec(dllimport) HANDLE __stdcall GetCurrentProcess();
__declspec(dllimport) BOOL __stdcall K32EnumProcessModules(HANDLE process, HMODULE* modules,
                                                           DWORD bytes, DWORD* bytes_needed);
__declspec(dllimport) BOOL __stdcall ReadProcessMemory(HANDLE process, const void* base,
                                                       void* buffer, size_t size, size_t* read);
__declspec(dllimport) void __stdcall OutputDebugStringA(const char* text);
}

enum class ObserverKind : uint8_t { msg_get_param, cert_find };

constexpr size_t kMaxObserverSlotPatches = 1024u;
struct ObserverSlotPatch {
    void** slot;
    void* original;
    void* replacement;
    HMODULE image;
    ObserverKind kind;
    bool active;
};

CryptMsgGetParamFn g_msg_original = nullptr;
CertFindCertificateInStoreFn g_cert_find_original = nullptr;
volatile long g_query_calls = 0;
volatile long g_msg_calls = 0;
volatile long g_cert_find_calls = 0;
ObserverSlotPatch g_observer_slot_patches[kMaxObserverSlotPatches]{};
size_t g_observer_slot_patch_count = 0u;
struct ObserverProtectionObligation {
    void* address;
    size_t size;
    DWORD original_protect;
    bool active;
};
ObserverProtectionObligation g_observer_protection_obligation{};

bool retry_observer_protection_restore() noexcept {
    if (!g_observer_protection_obligation.active) return true;
    DWORD ignored = 0;
    if (!VirtualProtect(g_observer_protection_obligation.address,
                        g_observer_protection_obligation.size,
                        g_observer_protection_obligation.original_protect,
                        &ignored))
        return false;
    g_observer_protection_obligation = ObserverProtectionObligation{};
    return true;
}
uint32_t atomic_increment(volatile long* value) noexcept {
#if defined(_MSC_VER)
    return static_cast<uint32_t>(_InterlockedIncrement(value));
#else
    return static_cast<uint32_t>(__atomic_add_fetch(value, 1L, __ATOMIC_RELAXED));
#endif
}

uint32_t atomic_load(const volatile long* value) noexcept {
#if defined(_MSC_VER)
    return static_cast<uint32_t>(_InterlockedCompareExchange(
        const_cast<volatile long*>(value), 0L, 0L));
#else
    return static_cast<uint32_t>(__atomic_load_n(value, __ATOMIC_RELAXED));
#endif
}



void set_msg_original_from_export() noexcept {
    if (g_msg_original) return;
    HMODULE crypt = GetModuleHandleW(L"CRYPT32.dll");
    if (!crypt) return;
    FARPROC p = GetProcAddress(crypt, "CryptMsgGetParam");
    if (p) g_msg_original = reinterpret_cast<CryptMsgGetParamFn>(p);
}

void set_cert_find_original_from_export() noexcept {
    if (g_cert_find_original) return;
    HMODULE crypt = GetModuleHandleW(L"CRYPT32.dll");
    if (!crypt) return;
    FARPROC p = GetProcAddress(crypt, "CertFindCertificateInStore");
    if (p) g_cert_find_original = reinterpret_cast<CertFindCertificateInStoreFn>(p);
}
uint32_t caller_rva(void* caller, bool* is_main) noexcept {
    if (is_main) *is_main = false;
    if (!caller) return 0;
    HMODULE owner = nullptr;
    const DWORD flags = kGetModuleHandleExFlagFromAddress |
                        kGetModuleHandleExFlagUnchangedRefcount;
    if (!GetModuleHandleExW(flags, reinterpret_cast<const wchar_t*>(caller), &owner) || !owner)
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(caller));
    HMODULE main = GetModuleHandleW(nullptr);
    if (is_main) *is_main = (owner == main);
    const uintptr_t pc = reinterpret_cast<uintptr_t>(caller);
    const uintptr_t base = reinterpret_cast<uintptr_t>(owner);
    if (pc < base || pc - base > UINT32_MAX)
        return static_cast<uint32_t>(pc);
    return static_cast<uint32_t>(pc - base);
}

bool safe_read_bytes(const void* address, void* output, size_t bytes) noexcept {
    if (!address || !output || bytes == 0) return false;
    size_t read = 0;
    return ReadProcessMemory(GetCurrentProcess(), address, output, bytes, &read) && read == bytes;
}
bool safe_read_u32(const DWORD* address, DWORD* value) noexcept {
    return safe_read_bytes(address, value, sizeof(*value));
}

bool safe_read_ptr(void* const* address, void** value) noexcept {
    return safe_read_bytes(address, value, sizeof(*value));
}



bool observer_patch_owned(void** slot, ObserverKind kind) noexcept {
    if (!slot) return false;
    for (size_t i = 0; i < g_observer_slot_patch_count; ++i) {
        const ObserverSlotPatch& patch = g_observer_slot_patches[i];
        if (patch.active && patch.slot == slot && patch.kind == kind)
            return true;
    }
    return false;
}

bool module_still_owns_slot(const ObserverSlotPatch& patch) noexcept {
    if (!patch.active || !patch.slot || !patch.image) return false;
    HMODULE owner = nullptr;
    const DWORD flags = kGetModuleHandleExFlagFromAddress |
                        kGetModuleHandleExFlagUnchangedRefcount;
    if (!GetModuleHandleExW(flags, reinterpret_cast<const wchar_t*>(patch.slot), &owner))
        return false;
    return owner == patch.image;
}

bool write_observer_slot(void** slot, void* expected, void* value) noexcept {
    if (!slot || !expected || !value) return false;
    // Audit3 S0206: never start a second mutation while a prior page
    // protection restoration is still owned by this diagnostic subsystem.
    if (!retry_observer_protection_restore()) return false;
    void* current = nullptr;
    if (!safe_read_ptr(slot, &current) || current != expected) return false;

    DWORD old_protect = 0;
    if (!VirtualProtect(slot, sizeof(void*), kPageReadWrite, &old_protect))
        return false;
    // Publish restoration ownership before mutating the slot. If restoration
    // fails, teardown/retry keeps this exact obligation until it succeeds.
    g_observer_protection_obligation =
        ObserverProtectionObligation{slot, sizeof(void*), old_protect, true};
    *slot = value;
    if (!retry_observer_protection_restore()) {
        // The page remains writable, so restore logical pointer ownership now.
        // A second protection attempt may succeed; if not, the obligation
        // remains published for uninstall/retry rather than being discarded.
        *slot = expected;
        (void)retry_observer_protection_restore();
        return false;
    }
    current = nullptr;
    return safe_read_ptr(slot, &current) && current == value;
}

preloader_l::platform::CryptObserverInstallResult add_observer_patch(
    HMODULE image, void** slot, void* original, void* replacement,
    ObserverKind kind) noexcept {
    using preloader_l::platform::CryptObserverInstallResult;
    if (!image || !slot || !original || !replacement)
        return CryptObserverInstallResult::invalid_image;
    if (observer_patch_owned(slot, kind))
        return CryptObserverInstallResult::already_installed;
    if (g_observer_slot_patch_count >= kMaxObserverSlotPatches)
        return CryptObserverInstallResult::tracking_capacity_exhausted;
    if (!write_observer_slot(slot, original, replacement))
        return CryptObserverInstallResult::protect_failed;
    g_observer_slot_patches[g_observer_slot_patch_count++] =
        ObserverSlotPatch{slot, original, replacement, image, kind, true};
    return CryptObserverInstallResult::installed;
}

bool restore_observer_slot_patches() noexcept {
    bool ok = true;
    for (size_t i = g_observer_slot_patch_count; i != 0; --i) {
        ObserverSlotPatch& patch = g_observer_slot_patches[i - 1u];
        if (!patch.active || !patch.slot) continue;

        // A module may legitimately unload before preloader_l. If the slot no
        // longer belongs to the image that owned it at install time there is
        // no live IAT entry left for us to restore, and touching the recycled
        // address would be unsafe.
        if (!module_still_owns_slot(patch)) {
            patch.active = false;
            continue;
        }

        void* current = nullptr;
        if (!safe_read_ptr(patch.slot, &current)) {
            ok = false;
            continue;
        }
        if (current == patch.replacement) {
            if (!write_observer_slot(patch.slot, patch.replacement, patch.original)) {
                ok = false;
                continue;
            }
        }
        // If another component replaced our pointer after installation, that
        // component owns the slot now. Do not overwrite it during teardown.
        patch.active = false;
    }

    bool any_active = false;
    bool msg_active = false;
    bool cert_active = false;
    for (size_t i = 0; i < g_observer_slot_patch_count; ++i) {
        const ObserverSlotPatch& patch = g_observer_slot_patches[i];
        if (!patch.active) continue;
        any_active = true;
        if (patch.kind == ObserverKind::msg_get_param) msg_active = true;
        if (patch.kind == ObserverKind::cert_find) cert_active = true;
    }
    if (!any_active) g_observer_slot_patch_count = 0u;
    if (!msg_active) g_msg_original = nullptr;
    if (!cert_active) g_cert_find_original = nullptr;
    return ok;
}
char hex_digit(unsigned value) noexcept {
    value &= 0xFu;
    return static_cast<char>(value < 10u ? ('0' + value) : ('A' + value - 10u));
}

void debug_hex64(const char* name, uint64_t value) noexcept {
    char line[128]{};
    size_t p = 0;
    const char prefix[] = "[preloader_l] ";
    for (size_t i = 0; prefix[i] && p + 1 < sizeof(line); ++i) line[p++] = prefix[i];
    for (size_t i = 0; name && name[i] && p + 1 < sizeof(line); ++i) line[p++] = name[i];
    const char middle[] = " hex=0x";
    for (size_t i = 0; middle[i] && p + 1 < sizeof(line); ++i) line[p++] = middle[i];
    for (int shift = 60; shift >= 0 && p + 1 < sizeof(line); shift -= 4)
        line[p++] = hex_digit(static_cast<unsigned>(value >> shift));
    if (p + 2 < sizeof(line)) line[p++] = '\n';
    line[p] = 0;
    OutputDebugStringA(line);
}
void debug_bytes_line(const char* name, const void* address, const uint8_t* bytes,
                      size_t count) noexcept {
    char line[256]{};
    size_t p = 0;
    const char prefix[] = "[preloader_l] ";
    for (size_t i = 0; prefix[i] && p + 1 < sizeof(line); ++i) line[p++] = prefix[i];
    for (size_t i = 0; name && name[i] && p + 1 < sizeof(line); ++i) line[p++] = name[i];
    const char middle[] = " addr=0x";
    for (size_t i = 0; middle[i] && p + 1 < sizeof(line); ++i) line[p++] = middle[i];
    const uint64_t a = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(address));
    for (int shift = 60; shift >= 0 && p + 1 < sizeof(line); shift -= 4)
        line[p++] = hex_digit(static_cast<unsigned>(a >> shift));
    const char bytes_prefix[] = " bytes=";
    for (size_t i = 0; bytes_prefix[i] && p + 1 < sizeof(line); ++i) line[p++] = bytes_prefix[i];
    for (size_t i = 0; i < count && p + 3 < sizeof(line); ++i) {
        line[p++] = hex_digit(bytes[i] >> 4);
        line[p++] = hex_digit(bytes[i]);
    }
    if (p + 2 < sizeof(line)) line[p++] = '\n';
    line[p] = 0;
    OutputDebugStringA(line);
}
size_t detect_call_length_before_return(const uint8_t* bytes, size_t size) noexcept {
    if (!bytes || size < 2) return 0;
    if (size >= 6 && bytes[size-6] == 0xFF && bytes[size-5] == 0x15) return 6; // call [rip+disp32]
    if (size >= 5 && bytes[size-5] == 0xE8) return 5; // call rel32
    if (size >= 2 && bytes[size-2] == 0xFF && (bytes[size-1] & 0xF8u) == 0xD0u) return 2; // call r64
    if (size >= 3 && (bytes[size-3] & 0xF0u) == 0x40u && bytes[size-2] == 0xFF &&
        (bytes[size-1] & 0xF8u) == 0xD0u) return 3;
    return 0;
}

void trace_query_caller_provenance(void* caller, bool main_caller) noexcept {
    if (!caller || !main_caller) return;
    const uintptr_t ret = reinterpret_cast<uintptr_t>(caller);
    HMODULE main = GetModuleHandleW(nullptr);
    debug_hex64("crypt_query_caller_abs", static_cast<uint64_t>(ret));
    debug_hex64("crypt_query_main_base", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(main)));

    // Capture a bounded code window around the return address. This is diagnostic
    // evidence only; none of these bytes are modified or executed by this code.
    constexpr size_t kBefore = 96;
    constexpr size_t kAfter = 32;
    uint8_t window[kBefore + kAfter]{};
    const void* window_start = reinterpret_cast<const void*>(ret - kBefore);
    if (!safe_read_bytes(window_start, window, sizeof(window))) {
        PRELOADER_L_WINDOWS_TRACE(
            preloader_l::runtime::TraceEvent::crypt_query_code_window_read, 0u);
        return;
    }
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::crypt_query_code_window_read, 1u);
    for (size_t off = 0; off < sizeof(window); off += 16) {
        char label[40]{};
        const char base[] = "crypt_query_code_chunk_";
        size_t q = 0;
        for (size_t i = 0; base[i] && q + 1 < sizeof(label); ++i) label[q++] = base[i];
        const unsigned index = static_cast<unsigned>(off / 16);
        label[q++] = static_cast<char>('0' + (index / 10));
        label[q++] = static_cast<char>('0' + (index % 10));
        label[q] = 0;
        debug_bytes_line(label,
                         reinterpret_cast<const void*>(ret - kBefore + off),
                         window + off, 16);
    }

    const size_t call_len = detect_call_length_before_return(window, kBefore);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::crypt_query_call_length,
        static_cast<uint32_t>(call_len));
    if (call_len == 0 || kBefore < call_len) return;

    const uintptr_t call_start = ret - call_len;
    debug_hex64("crypt_query_call_site", static_cast<uint64_t>(call_start));

    constexpr size_t kScan = 48;
    const size_t pre_call = kBefore - call_len;
    const size_t scan_start = pre_call > kScan ? pre_call - kScan : 0;
    const auto producer = preloader_l::platform::find_last_rdx_producer(
        window + scan_start, pre_call - scan_start);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::crypt_query_rdx_producer_kind,
        static_cast<uint32_t>(producer.kind));
    if (producer.kind == preloader_l::platform::RdxProducerKind::unknown) return;

    const uintptr_t instruction = ret - kBefore + scan_start + producer.offset;
    debug_hex64("crypt_query_rdx_producer_address", static_cast<uint64_t>(instruction));
    debug_hex64("crypt_query_rdx_producer_disp", static_cast<uint64_t>(producer.displacement));
    debug_hex64("crypt_query_rdx_producer_immediate", producer.immediate);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::crypt_query_rdx_source_register,
        producer.source_register);

    if (producer.kind == preloader_l::platform::RdxProducerKind::mov_rip_relative ||
        producer.kind == preloader_l::platform::RdxProducerKind::lea_rip_relative) {
        const uintptr_t source = instruction + producer.length + producer.displacement;
        debug_hex64("crypt_query_rdx_candidate_source", static_cast<uint64_t>(source));
        if (producer.kind == preloader_l::platform::RdxProducerKind::lea_rip_relative) {
            debug_hex64("crypt_query_rdx_candidate_value", static_cast<uint64_t>(source));
        } else {
            uint64_t value = 0;
            if (safe_read_bytes(reinterpret_cast<const void*>(source), &value, sizeof(value)))
                debug_hex64("crypt_query_rdx_candidate_value", value);
        }
    } else if (producer.kind == preloader_l::platform::RdxProducerKind::zero_xor) {
        debug_hex64("crypt_query_rdx_candidate_value", 0);
    } else if (producer.kind == preloader_l::platform::RdxProducerKind::mov_imm32 ||
               producer.kind == preloader_l::platform::RdxProducerKind::mov_imm64) {
        debug_hex64("crypt_query_rdx_candidate_value", producer.immediate);
    }
}
BOOL __stdcall observed_crypt_msg_get_param(HCRYPTMSG message, DWORD param_type,
                                             DWORD index, void* data,
                                             DWORD* data_size) noexcept {
    const uint32_t msg_call = atomic_increment(&g_msg_calls);
    const DWORD incoming_error = GetLastError();
    void* caller = _ReturnAddress();
    bool main_caller = false;
    const uint32_t rva = caller_rva(caller, &main_caller);

    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_call,
                                          msg_call);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_param_type,
                                          static_cast<uint32_t>(param_type));
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_index,
                                          static_cast<uint32_t>(index));
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_message_nonnull,
                                          message ? 1u : 0u);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_caller_rva,
                                          rva);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_caller_is_main,
                                          main_caller ? 1u : 0u);
    debug_hex64("crypt_msg_handle", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(message)));
    debug_hex64("crypt_msg_data_ptr", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data)));
    debug_hex64("crypt_msg_size_ptr", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data_size)));
    DWORD size_before = 0;
    if (safe_read_u32(data_size, &size_before))
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_size_before,
                                              static_cast<uint32_t>(size_before));

    if (preloader_l::platform::should_fail_closed_crypt_msg(message,
                                                          static_cast<uint32_t>(param_type),
                                                          main_caller)) {
        // The live host path calls CMSG_SIGNER_INFO_PARAM with a null message
        // after CryptQueryObject has already failed. Passing that null handle
        // into the observed crypt32 build raises an access violation. Keep the
        // trust result failed, but return a normal API failure to the caller so
        // the host can exercise its own error path instead of crashing inside
        // crypt32. No output buffer is modified.
        PRELOADER_L_WINDOWS_TRACE(
            preloader_l::runtime::TraceEvent::crypt_msg_null_guard,
            preloader_l::platform::crypt_msg_null_guard_error);
        PRELOADER_L_WINDOWS_TRACE(
            preloader_l::runtime::TraceEvent::crypt_msg_result, 0u);
        PRELOADER_L_WINDOWS_TRACE(
            preloader_l::runtime::TraceEvent::crypt_msg_error,
            preloader_l::platform::crypt_msg_null_guard_error);
        SetLastError(preloader_l::platform::crypt_msg_null_guard_error);
        return 0;
    }

    CryptMsgGetParamFn original = g_msg_original;
    if (!original) {
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::failure, 0xC002u);
        SetLastError(127u); // ERROR_PROC_NOT_FOUND
        return 0;
    }

    SetLastError(incoming_error);
    const BOOL result = original(message, param_type, index, data, data_size);
    const DWORD error = GetLastError();
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_result,
                                          result ? 1u : 0u);
    if (!result)
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_error,
                                              static_cast<uint32_t>(error));
    DWORD size_after = 0;
    if (safe_read_u32(data_size, &size_after))
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_msg_size_after,
                                              static_cast<uint32_t>(size_after));
    SetLastError(error);
    return result;
}

PCCERT_CONTEXT __stdcall observed_cert_find_certificate_in_store(
    HCERTSTORE store, DWORD encoding_type, DWORD find_flags, DWORD find_type,
    const void* find_para, PCCERT_CONTEXT previous_context) noexcept {
    const uint32_t cert_find_call = atomic_increment(&g_cert_find_calls);
    const DWORD incoming_error = GetLastError();
    void* caller = _ReturnAddress();
    bool main_caller = false;
    const uint32_t rva = caller_rva(caller, &main_caller);

    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_call,
                                          cert_find_call);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_encoding_type,
                                          static_cast<uint32_t>(encoding_type));
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_flags,
                                          static_cast<uint32_t>(find_flags));
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_type,
                                          static_cast<uint32_t>(find_type));
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_caller_rva,
                                          rva);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_caller_is_main,
                                          main_caller ? 1u : 0u);
    debug_hex64("cert_find_store", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(store)));
    debug_hex64("cert_find_para", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(find_para)));
    debug_hex64("cert_find_prev_context", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(previous_context)));

    CertFindCertificateInStoreFn original = g_cert_find_original;
    if (!original) {
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::failure, 0xC003u);
        SetLastError(127u); // ERROR_PROC_NOT_FOUND
        return nullptr;
    }

    SetLastError(incoming_error);
    PCCERT_CONTEXT result = original(store, encoding_type, find_flags, find_type,
                                     find_para, previous_context);
    const DWORD error = GetLastError();
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_result,
                                          result ? 1u : 0u);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_context_nonnull,
                                          result ? 1u : 0u);
    debug_hex64("cert_find_result_context", static_cast<uint64_t>(reinterpret_cast<uintptr_t>(result)));
    if (!result)
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::cert_find_error,
                                              static_cast<uint32_t>(error));
    SetLastError(error);
    return result;
}
preloader_l::platform::CryptObserverInstallResult install_for_image(void* image,
                                                                 ObserverKind kind) noexcept {
    if (!image) return preloader_l::platform::CryptObserverInstallResult::invalid_image;
    size_t image_size = 0;
    if (!preloader_l::platform::pe_image_size(image, &image_size))
        return preloader_l::platform::CryptObserverInstallResult::invalid_image;

    const char* function_name = nullptr;
    void* observer = nullptr;
    if (kind == ObserverKind::msg_get_param) {
        function_name = "CryptMsgGetParam";
        observer = reinterpret_cast<void*>(&observed_crypt_msg_get_param);
    } else {
        function_name = "CertFindCertificateInStore";
        observer = reinterpret_cast<void*>(&observed_cert_find_certificate_in_store);
    }

    preloader_l::platform::PeImportSlot found{};
    if (!preloader_l::platform::find_pe_import_slot(image, image_size, "CRYPT32.dll",
                                                function_name, &found))
        return preloader_l::platform::CryptObserverInstallResult::import_not_found;
    if (!found.slot) return preloader_l::platform::CryptObserverInstallResult::import_not_found;
    if (observer_patch_owned(found.slot, kind))
        return preloader_l::platform::CryptObserverInstallResult::already_installed;
    if (found.current == observer)
        return preloader_l::platform::CryptObserverInstallResult::already_installed;

    if (kind == ObserverKind::msg_get_param) {
        if (!g_msg_original)
            g_msg_original = reinterpret_cast<CryptMsgGetParamFn>(found.current);
        if (!g_msg_original)
            return preloader_l::platform::CryptObserverInstallResult::original_unavailable;
    } else {
        if (!g_cert_find_original)
            g_cert_find_original = reinterpret_cast<CertFindCertificateInStoreFn>(found.current);
        if (!g_cert_find_original)
            return preloader_l::platform::CryptObserverInstallResult::original_unavailable;
    }

    return add_observer_patch(static_cast<HMODULE>(image), found.slot,
                              found.current, observer, kind);
}

uint32_t install_loaded_modules(ObserverKind kind) noexcept {
    if (kind == ObserverKind::msg_get_param) set_msg_original_from_export();
    else set_cert_find_original_from_export();

    HMODULE stack_modules[64]{};
    HMODULE* modules = stack_modules;
    size_t capacity_bytes = sizeof(stack_modules);
    void* dynamic_storage = nullptr;
    size_t dynamic_size = 0u;
    DWORD needed = 0;

    for (unsigned attempt = 0; attempt < 8u; ++attempt) {
        if (!K32EnumProcessModules(GetCurrentProcess(), modules,
                                   static_cast<DWORD>(capacity_bytes), &needed)) {
            if (dynamic_storage)
                preloader_l::runtime::release_block(
                    dynamic_storage, dynamic_size,
                    preloader_l::runtime::windows_allocator_platform_ops());
            return 0;
        }
        if (static_cast<size_t>(needed) <= capacity_bytes) break;

        const size_t requested = static_cast<size_t>(needed);
        const auto allocation = preloader_l::runtime::allocate_block(
            requested, preloader_l::runtime::AllocationRetryPolicy::outer_retry_only,
            preloader_l::runtime::windows_allocator_platform_ops());
        if (allocation.terminal != preloader_l::runtime::AllocationTerminal::none ||
            !allocation.pointer) {
            if (dynamic_storage)
                preloader_l::runtime::release_block(
                    dynamic_storage, dynamic_size,
                    preloader_l::runtime::windows_allocator_platform_ops());
            return 0;
        }
        if (dynamic_storage)
            preloader_l::runtime::release_block(
                dynamic_storage, dynamic_size,
                preloader_l::runtime::windows_allocator_platform_ops());
        dynamic_storage = allocation.pointer;
        dynamic_size = requested;
        modules = static_cast<HMODULE*>(dynamic_storage);
        capacity_bytes = requested;
        needed = 0;
    }

    if (static_cast<size_t>(needed) > capacity_bytes) {
        if (dynamic_storage)
            preloader_l::runtime::release_block(
                dynamic_storage, dynamic_size,
                preloader_l::runtime::windows_allocator_platform_ops());
        return 0;
    }

    const size_t count = static_cast<size_t>(needed / sizeof(HMODULE));
    uint32_t patched = 0;
    for (size_t i = 0; i < count; ++i) {
        const auto result = install_for_image(modules[i], kind);
        if (result == preloader_l::platform::CryptObserverInstallResult::installed ||
            result == preloader_l::platform::CryptObserverInstallResult::already_installed)
            ++patched;
    }
    if (dynamic_storage)
        preloader_l::runtime::release_block(
            dynamic_storage, dynamic_size,
            preloader_l::runtime::windows_allocator_platform_ops());
    return patched;
}

} // namespace

namespace preloader_l { namespace platform {

void trace_crypt_query_diagnostics(uint32_t object_type, void* caller) noexcept {
    const uint32_t query_call = atomic_increment(&g_query_calls);
    bool main_caller = false;
    const uint32_t rva = caller_rva(caller, &main_caller);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_query_call, query_call);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_query_object_type, object_type);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_query_caller_rva, rva);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_query_caller_is_main,
                              main_caller ? 1u : 0u);
    if (query_call <= 4u) trace_query_caller_provenance(caller, main_caller);
}

void trace_crypt_query_companion_branch() noexcept {
    preloader_l::platform::note_reconstructed_identity_branch();
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::reconstructed_identity_branch,
        preloader_l::platform::reconstructed_identity_state().branch_hits);
}

bool uninstall_crypt_observer_diagnostics() noexcept {
    // A failed install can own page-protection restoration even when no slot
    // patch was admitted to g_observer_slot_patches. Drain that obligation
    // before/after normal slot rollback and report pending work truthfully.
    if (!retry_observer_protection_restore()) return false;
    if (!restore_observer_slot_patches()) return false;
    return retry_observer_protection_restore();
}

CryptObserverInstallResult install_crypt_msg_get_param_observer_main_module() noexcept {
    return install_for_image(GetModuleHandleW(nullptr), ObserverKind::msg_get_param);
}

CryptObserverInstallResult install_cert_find_certificate_observer_main_module() noexcept {
    return install_for_image(GetModuleHandleW(nullptr), ObserverKind::cert_find);
}

uint32_t install_crypt_msg_get_param_observer_loaded_modules() noexcept {
    return install_loaded_modules(ObserverKind::msg_get_param);
}

uint32_t install_cert_find_certificate_observer_loaded_modules() noexcept {
    return install_loaded_modules(ObserverKind::cert_find);
}

uint32_t crypt_query_observer_call_count() noexcept { return atomic_load(&g_query_calls); }
uint32_t crypt_msg_get_param_observer_call_count() noexcept { return atomic_load(&g_msg_calls); }
uint32_t cert_find_certificate_observer_call_count() noexcept { return atomic_load(&g_cert_find_calls); }

} }
