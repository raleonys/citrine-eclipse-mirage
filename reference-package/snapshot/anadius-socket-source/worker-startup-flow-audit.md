# Worker startup and entry: independent retained static audit

Scope: the normal bodies at `0x18002a000` and `0x18002a4a0`, read in full from both retained pseudocode and assembly. No original binary, original thread, real operating-system state, user configuration, token or achievement file was executed/read. This report records static observations, not observed runtime behavior.

## Evidence identity

Target SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Retained pseudocode SHA-256: `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; retained assembly SHA-256: `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

`02a000` pseudocode Evidence `ev_5e78eef88f702baed2927d9748d4a109ac08003daafc401dcb7edab1bb2b2aab`; assembly Evidence `ev_9bb61c1aa2239aa4ceaa3b76b45ca58ddf61af660271bf12e342b874b7583917`.

`02a4a0` pseudocode Evidence `ev_326989fa8214581db6a42e9ea6032da072a9887b4b3666295c91af39e968d292`; assembly Evidence `ev_a156da50becb7358bbfc13525d003f388046ac1b32fde8dc694fec79d90d8a53`.

## 02a000 startup contract

- `02a012..027` caches worker/function/uint32 stack size and calls `CreateEventW(nullptr, TRUE, FALSE, nullptr)` before accessing function memory. `02a02d` writes function+08, then `02a031` caches that returned event locally. A null function has a native fault after the API call; explicit host rejection must occur at that point without invented cleanup.
- On zero entry-event return, `02a039..061` caches `GetLastError` before virtual delete slot00(function,1), then unconditionally resolves category `028620`. Nonzero cached error raises `028240` with opaque context `0x1800844c8`. Error zero falls through, despite the delete callback; continuing requires a live host projection and is not proof of native use-after-delete safety.
- `02a063..079` creates another identical manual-reset, initially nonsignaled event; stores worker+10 first and function+10 second. `02a07d` tests the LIVE worker+10 field. Its failure path caches error, deletes function with flag1, resolves category regardless of error and raises only nonzero error with context `0x1800844e0`.
- `02a0b0..0d1` zeroes the local thread ID and calls existing `05e8e0(nullptr, cached_stack_size, opaque 0x18002a4a0, function, 0, &id)`. `02a0d6` stores worker+08; `02a0da` tests the returned handle, not a later live worker field.
- Thread failure `02a0df..122` caches `GetLastError`, calls function delete slot00 flag1, closes the cached entry event if nonzero, then reads LIVE worker+10 and conditionally closes that value. This reload follows the first close callback. Category resolution occurs after cleanup even with zero error. Only nonzero cached error raises with context `0x1800844f4`.
- Normal/fallthrough tail `02a124..13a` uses the cached entry event: if nonzero, `WaitForSingleObject(entry, 0xffffffff)` then `CloseHandle(entry)`; both return values ignored. Thread failure with error zero can therefore close the same cached entry event twice. There is no extra `GetLastError`, fabricated retry, field clearing, rollback or automatic resource owner.

## 02a4a0 entry contract

- `02a4b2..4b6` reads function+08 and calls `SetEvent`; result ignored.
- `02a4bc..4c2` calls required virtual run slot08(function).
- `02a4c5` reads function+10 AFTER run; `02a4c9..4d4` calls delete slot00(function,1).
- `02a4d6..4d9` signals the CACHED exit event, without further function access; `02a4df..4e9` calls `SleepEx(0xffffffff, TRUE)`, ignores its result, then `02a4f0` returns zero. No loop, wait retry or inferred exception handler is present.

## Reconstruction boundaries

Two new original normal procedures are counted. Adapter helpers, typed projections, selected bridge composition and SDK wrappers add no original procedure. Existing `anadius-thread-source/threads.cpp` already implements `05e8e0`; reuse is composition, not new CRT recovery.

Allocation/deallocation, function run implementation and scalar deleting destructor, actual object lifetime, `028240` exception class/ABI, unwind cleanup, real thread scheduling/concurrency and function-vtable executable binding remain required providers or unresolved. Original-image entry/context addresses remain opaque tokens. Typed host fields are not native layout overlays. Controlled source tests establish source behavior under selected effects, not original DLL equivalence.
