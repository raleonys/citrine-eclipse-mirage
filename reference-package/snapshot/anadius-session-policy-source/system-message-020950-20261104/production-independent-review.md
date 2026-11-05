# Independent020950 production source review

Result: STATIC PASS for the bounded system-category message value/SDK adapter and actual Policy message-slot selection. No blocking finding in the four frozen candidate diffs. No compiler, project tests, linker, target execution, packaging, or new REA session was used for this source review. The reviewer performed the fresh six-record native audit separately, whose full replies remain in this stage.

## Exact frozen sources

| File | SHA256 |
|---|---|
| outputs/anadius-socket-source/native.hpp |054f0a03884610c94ec0683f67735ce41fb2025130cbbd757f6c818da31d8e5c |
| outputs/anadius-socket-source/native.cpp |718bfac524e2b8d445e522b433abba165dee811e1301cc50a0ecc9fdc9a41a39 |
| outputs/anadius-session-policy-source/policy.hpp |aec906fed7543fcafeb9862cfef347b3391ccdf13c845d5b2529432cff8e388b |
| outputs/anadius-session-policy-source/policy.cpp |5c42ccbff002069dc4b1bdb111f6ed59efcb634ecc01efaef7890fd55b06307f |

## Recovered baselines, not contemporaneous captures

The production before files were missed before editing. The author later reconstructed `before/{native.hpp,native.cpp,policy.hpp,policy.cpp}.before.txt`; they must not be described as contemporaneous pre-edit captures.

Native files were recovered read-only from the historical ZIP and matched the source-workspace source-snapshot authority. Policy files were reconstructed by reversing only this stage's hunks and matched the exact prior independently reviewed tracked-stage hashes. This reviewer hashed those recovered files and compared their full source diffs with the four frozen candidates:

| Recovered baseline | SHA256 |
|---|---|
| native.hpp |b6b562b73e3fd6fed14a8c52f7749a33cf6d4171e4af09ab19748dfebc39b7b1 |
| native.cpp |7b0131c0f8ee6fc6bc7a3c8e4b5692295cd743ea39aac61437c8dd65a8d25919 |
| policy.hpp |77b073c58846de51e57a035462add6db66c6b94e8849dbf60703fd59285318cd |
| policy.cpp |b00747912def3dd0c14dd5be59aa8b42fa1459d8981330f96e56f51378e7fefe |

This establishes exact known-byte baseline comparison; it does not retroactively establish when a snapshot was taken. Prior native.cpp/hpp behavior and Policy tracked-work bridge hunks remain unchanged by the candidate.

## Normal message behavior

The injected imports use exact `decltype(&::FormatMessageA)` and `decltype(&::LocalFree)` signatures and default to real SDK functions. The helper begins only with a fresh null buffer and empty phase flags. It marks entry before any checkpoint/SDK effect and rejects reuse of a retained prefix.

The actual FormatMessage call uses flags0x1300, sourceNULL, the code converted to DWORD, language0x400, pointer-to-local allocated output buffer, size0 and ArgumentsNULL. These match native020989 and the local header signatures documented by native-findings.md.

The return DWORD only governs the last-byte LF and then CR tests. Each byte is removed at most once; a removal making the count zero chooses fallback. With a nonempty counted remainder, `std::string(buffer)` performs a C NUL-terminated copy, matching the separate native0209f0 strlen-like scan rather than copying the returned count. A leading/embedded NUL can shorten a successful result without itself selecting fallback. Count0 never dereferences the message buffer.

Fallback is exactly `asio.system error`. Both successful copy and fallback construct their HOST result before calling LocalFree, including LocalFree(NULL) after a failed FormatMessage that left the initial pointer null. A failed FormatMessage that wrote a nonnull pointer also reaches that same free. LocalFree's return is ignored, as in020a4a. No GetLastError-based alternate fallback, whitespace loop, or decimal-code suffix is introduced.

Metadata checkpoints save/restore LastError locally. The helper does not preserve LastError around the imported FormatMessage/LocalFree themselves, so their actual SDK error effects remain visible. It does not replace the native APIs with compiler std::system_category().message for this message path.

## Retained SDK prefix and final checkpoint

FormatMessage receives the address of caller-retained state.buffer. A throwing SDK, output-string allocation or metadata postcheck therefore leaves the entered state/pointer retained; it does not guess that the allocation was absent or already freed. Before LocalFree, release_entered becomes true. A throwing free or final postcheck keeps that release-entered state, forbidding automatic retry or a second free.

The final checkpoint is BEFORE `state={}`. During that checkpoint the state still rejects a nested invocation on the same state, and a thrown final check cannot clear the retained release prefix. Only successful free plus successful final check clears state for a later distinct call. The metadata checkpoint's small LastError guard is not an SDK-buffer RAII deleter.

Policy owns this state in Impl and passes its actual admission probe into the helper. Existing Policy failure handling poisons/retains the log/source prefix; it cannot subsequently close or begin another normal message as though the SDK prefix were known clean. There is no newly fabricated native exception cleanup claim.

## Actual selection and compatibility

Policy's Catalog returns category index1, and the category descriptor's message_slot_10 calls Impl::message with that index. The new text selection chooses020950 only for index1 when no explicit formatter override exists. Index0 still uses compiler generic-category text; index2 still uses the existing recovered020a80 misc category. The explicit format_override remains the first branch and its prior output-bound validation remains in place.

Options.system_message is appended AFTER format_override at the end of Options. Existing positional fields are not shifted by a middle insertion; all native imports have nonnull defaults and Policy validates their completeness. The HOST Options layout changes and requires a future authorized rebuild; no old binary ABI compatibility is implied.

Fresh binding proof is028620→objectA1110→vtable84248→slot+0x10=020950, from `ev_6ef7ee9e0cc7a42ee4977f336dc25f81c0f6d4180250698785767507f5185aa7`, `ev_eb486a9416f6cbfeed05d5815675c4a981ffb8d4bb337a32016db6ebf9b5f4aa` and `ev_c1a2d4b167be4a0479021d9a9f8526b20e0227eae9471a9457ad44bef111188a`. Exact helper normal behavior is checked against020950 pseudo `ev_0274fa3cd88e934ca61666cd5c1171f80edce8cbc7cd1a7dbe6e851dfdf2f0aa` and complete instructions `ev_0c36cbad633abd179eac63d16b40d5c52eabd28764feead934ec5968b478eefc`.

## Explicit limits

This is a HOST value/SDK adapter. It does not reproduce the native string representation, exact fallback04b0bc(0x20)/capacity31 allocation ABI, original SSO behavior, or MSVC unwind funclets. Native normal copy/fallback-before-LocalFree ordering is supported; native exceptional cleanup is unknown. Retaining a HOST unknown prefix does not prove either a native leak or native free during unwinding.

Compiler std::system_error category/what formatting in existing terminal throw projections remains outside this Policy message-slot correction. This review does not establish that every compiler exception message uses020950 or that the whole native error/CRT ABI is reconstructed. Tests are separate source work and were not executed here.
