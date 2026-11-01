#include "preloader_core.hpp"
#include <cstring>
#include <limits>

namespace recovered_preloader {
void Once::call(Host& host, bool ignore_poison, void* context, Callback callback) {
    // All x64 locked operations are full barriers; seq_cst preserves that order.
    auto observed = state.load();
    for (;;) {
        const auto kind = observed & 3;
        const auto queued = observed & Waiters;
        if (kind == Complete) return;
        if (kind == Running) {
            if (!queued) {
                auto expected = std::uint32_t{Running};
                const auto desired = observed | Waiters;
                if (!state.compare_exchange_strong(expected, desired)) {
                    observed = expected;
                    continue;
                }
                observed = desired;
            }
            if (host.wait32(state, observed) != 1) (void)host.get_last_error();
            observed = state.load();
            continue;
        }
        if (kind == Poisoned && !ignore_poison)
            host.invalid_instruction(TrapSite::PoisonedOnce);
        auto expected = observed;
        if (!state.compare_exchange_strong(expected, queued | Running)) {
            observed = expected;
            continue;
        }
        OnceCall call;
        call.was_poisoned = kind == Poisoned;
        callback(context, call); // callback may deliberately set next_state.
        const auto old = state.exchange(call.next_state);
        if (old & Waiters) host.wake_all(state);
        return;
    }
}

void Core::ensure_engine() {
    // 0x1800016d3 -> table 0x180005318 -> 0x180001727.
    if (engine_once.state.load() == Once::Complete) return;
    struct Capture { Core* self; bool pending; } capture{this, true};
    engine_once.call(host_, false, &capture, [](void* raw, OnceCall&) {
        auto& c = *static_cast<Capture*>(raw);
        const bool pending = c.pending;
        c.pending = false;
        if (!pending) c.self->host_.invalid_instruction(TrapSite::ConsumedInitializer);
        const int status = c.self->engine_.initialize();
        // 0 is remapped to -2 before comparisons; a literal -2 is also accepted.
        if (status != 0 && status != 1 && status != -2)
            c.self->host_.invalid_instruction(TrapSite::EngineInitialization);
    });
}

void Core::create_one(void* target, void* detour, void** result) {
    ensure_engine();
    void* candidate = nullptr;
    if (engine_.create(target, detour, &candidate) == 0) *result = candidate;
}

BOOL Core::dll_main(HMODULE module, DWORD reason, LPVOID reserved) {
    (void)reserved;
    // Functional DllMain at 0x180001000; CRT entry is separate at 0x180003338.
    if (reason == DLL_PROCESS_ATTACH) {
        if (host_.disable_thread_library_calls(module) == 0) (void)host_.get_last_error();
        void* result = reinterpret_cast<void*>(original_process_next);
        create_one(reinterpret_cast<void*>(bindings_.process_next_target),
                   reinterpret_cast<void*>(bindings_.process_next_detour), &result);
        original_process_next = reinterpret_cast<ProcessNext>(result);
        result = reinterpret_cast<void*>(original_trylock);
        create_one(reinterpret_cast<void*>(bindings_.trylock_target),
                   reinterpret_cast<void*>(bindings_.trylock_detour), &result);
        original_trylock = reinterpret_cast<MutexRoutine>(result);
        result = reinterpret_cast<void*>(original_crypt);
        create_one(reinterpret_cast<void*>(bindings_.crypt_target),
                   reinterpret_cast<void*>(bindings_.crypt_detour), &result);
        original_crypt = reinterpret_cast<CryptQuery>(result);
        ensure_engine();
        (void)engine_.enable(nullptr);
    }
    return 1;
}

BOOL Core::process_next(HANDLE snapshot, LPPROCESSENTRY32W entry) {
    // 0x180001229: preserve the exact BOOL value returned by the original.
    if (!original_process_next) host_.invalid_instruction(TrapSite::MissingOriginal);
    const BOOL result = original_process_next(snapshot, entry);
    if (entry && result != 0) {
        const DWORD pid = host_.current_process_id();
        if (entry->th32ProcessID == pid) entry->th32ParentProcessID = pid;
    }
    return result;
}

int Core::mutex_trylock(void* mutex) {
    // 0x180001139. Range initialization callback has a two-register return in
    // the artifact. Explicit fields here avoid copying Ghidra's false void ABI.
    if (image_once.state.load() != Once::Complete) {
        struct Capture { Core* self; bool pending; } capture{this, true};
        image_once.call(host_, true, &capture, [](void* raw, OnceCall& once) {
            auto& c = *static_cast<Capture*>(raw);
            const bool pending = c.pending;
            c.pending = false;
            if (!pending) c.self->host_.invalid_instruction(TrapSite::ConsumedInitializer);
            if (once.was_poisoned)
                c.self->host_.invalid_instruction(TrapSite::PoisonedImageRange);
            const auto base = reinterpret_cast<std::uintptr_t>(c.self->host_.main_module());
            if (!base) {
                (void)c.self->host_.get_last_error();
                c.self->host_.invalid_instruction(TrapSite::NullMainImage);
            }
            // Same unchecked NT-header reads as 0x180001212/0x180001216.
            std::int32_t e_lfanew;
            std::memcpy(&e_lfanew, reinterpret_cast<const void*>(base + 0x3c), 4);
            std::uint32_t size;
            std::memcpy(&size, reinterpret_cast<const void*>(base + e_lfanew + 0x50), 4);
            c.self->image_begin_ = base;
            c.self->image_end_ = base + size;
        });
    }
    const auto address = reinterpret_cast<std::uintptr_t>(mutex);
    if (image_begin_ <= address && address < image_end_ &&
        mutex_once.state.load() != Once::Complete &&
        mutex_once.state.load() != Once::Complete) {
        struct Capture { Core* self; void* mutex; bool pending; } capture{this, mutex, true};
        mutex_once.call(host_, true, &capture, [](void* raw, OnceCall&) {
            auto& c = *static_cast<Capture*>(raw);
            const bool pending = c.pending;
            c.pending = false;
            if (!pending) c.self->host_.invalid_instruction(TrapSite::ConsumedInitializer);
            (void)c.self->host_.mutex_lock(c.mutex); // return value is ignored.
        });
    }
    if (!original_trylock) host_.invalid_instruction(TrapSite::MissingOriginal);
    return original_trylock(mutex);
}

BOOL Core::crypt_query(DWORD type, const void* object, DWORD expected_content,
                      DWORD expected_format, DWORD flags, DWORD* encoding,
                      DWORD* content, DWORD* format, HCERTSTORE* store,
                      HCRYPTMSG* message, const void** context) {
    // 0x180001264: full WinCrypt API, including all seven stack arguments.
    static constexpr wchar_t replacement[] = L"EAAntiCheat.GameServiceLauncher.dll";
    static constexpr char suffix[] = "preloader_l.dll";
    const void* selected = object;
    if (type == 1) {
        if (!object) {
            selected = replacement;
        } else {
            auto input = static_cast<const wchar_t*>(object);
            std::size_t units = 0;
            while (input[units] != 0) ++units;
            // Original allocates UTF-16-unit count bytes initially, and grows as
            // it emits WTF-8. The empty buffer uses dangling aligned pointer 1.
            std::size_t capacity = units, length = 0;
            auto bytes = units ? static_cast<unsigned char*>(host_.allocate(units))
                               : reinterpret_cast<unsigned char*>(std::uintptr_t{1});
            if (units && !bytes) host_.invalid_instruction(TrapSite::Allocation);
            bool contains_unpaired_surrogate = false;
            for (std::size_t i = 0; i < units; ++i) {
                std::uint32_t cp = static_cast<std::uint16_t>(input[i]);
                if (cp >= 0xd800 && cp <= 0xdbff && i + 1 < units &&
                    static_cast<std::uint16_t>(input[i + 1]) >= 0xdc00 &&
                    static_cast<std::uint16_t>(input[i + 1]) <= 0xdfff) {
                    cp = 0x10000 + ((cp & 0x3ff) << 10) +
                         (static_cast<std::uint16_t>(input[++i]) & 0x3ff);
                } else if (cp >= 0xd800 && cp <= 0xdfff) {
                    contains_unpaired_surrogate = true;
                }
                unsigned char encoded[4];
                std::size_t n;
                if (cp < 0x80) {
                    n = 1; encoded[0] = static_cast<unsigned char>(cp);
                } else if (cp < 0x800) {
                    n = 2; encoded[0] = static_cast<unsigned char>(0xc0 | (cp >> 6));
                    encoded[1] = static_cast<unsigned char>(0x80 | (cp & 0x3f));
                } else if (cp < 0x10000) {
                    n = 3; encoded[0] = static_cast<unsigned char>(0xe0 | (cp >> 12));
                    encoded[1] = static_cast<unsigned char>(0x80 | ((cp >> 6) & 0x3f));
                    encoded[2] = static_cast<unsigned char>(0x80 | (cp & 0x3f));
                } else {
                    n = 4; encoded[0] = static_cast<unsigned char>(0xf0 | (cp >> 18));
                    encoded[1] = static_cast<unsigned char>(0x80 | ((cp >> 12) & 0x3f));
                    encoded[2] = static_cast<unsigned char>(0x80 | ((cp >> 6) & 0x3f));
                    encoded[3] = static_cast<unsigned char>(0x80 | (cp & 0x3f));
                }
                if (capacity - length < n) {
                    if (length > std::numeric_limits<std::size_t>::max() - n)
                        host_.invalid_instruction(TrapSite::Allocation);
                    const auto wanted = length + n;
                    auto next = capacity * 2; // unsigned wrap agrees with LEA.
                    if (next < wanted) next = wanted;
                    if (next < 8) next = 8;
                    if (next > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()))
                        host_.invalid_instruction(TrapSite::Allocation);
                    auto grown = static_cast<unsigned char*>(capacity
                        ? host_.reallocate(bytes, next) : host_.allocate(next));
                    if (!grown) host_.invalid_instruction(TrapSite::Allocation);
                    bytes = grown;
                    capacity = next;
                }
                std::memcpy(bytes + length, encoded, n);
                length += n;
            }
            // Artifact first completes conversion, then scans ED A0..BF sequences
            // and UD2s. Do not normalize malformed UTF-16 or abort at first unit.
            if (contains_unpaired_surrogate)
                host_.invalid_instruction(TrapSite::InvalidUtf16);
            if (length >= sizeof(suffix) - 1 &&
                std::memcmp(suffix, bytes + length - (sizeof(suffix) - 1), sizeof(suffix) - 1) == 0)
                selected = replacement;
            if (capacity != 0) host_.release(bytes);
        }
    }
    if (!original_crypt) host_.invalid_instruction(TrapSite::MissingOriginal);
    return original_crypt(type, selected, expected_content, expected_format,
                          flags, encoding, content, format, store, message, context);
}
} // namespace recovered_preloader
