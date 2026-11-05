# Actual SRW two-actor test — authored source only

Only owned source: `outputs/anadius-execution-source/execution_tests.cpp`. Before SHA256 `6dd51e5471d32f12d13c093f5627f7e8e7760fbef49b5495d4389c13568f2a25`; final authored SHA256 `0114ef64df93b92dfd3ddf8a6bd4c4a0181a3ade3c2b26f439ef36e12a10f89a`. Root retained the exact before copy in this stage. Production execution.cpp/.hpp belong to the root and were not edited by this author.

Existing Fake mutex/recursion/type/error/overflow/flag/timing cases are unchanged. They remain controlled single-actor tests. New `actual_srw_two_actors()` is registered by normal main under `_WIN32`, with a local SDK-forwarding Platform in the same file. It uses main as actual actor A and one typed CreateThread WINAPI child B; no Domain/PTD/Parent authority or separate native thread framework is needed by these low-level mutex helpers. Both existing standalone test definitions link only execution.cpp+execution_tests.cpp, so this avoids a new WindowsPlatform.o/build-unit dependency. Main's future PASS wording now truthfully permits the real Win32 case instead of describing everything as mock-only.

## Physical lock path and semantic ordering

The recovered initializer prepares one exact type2 MutexState/SRW cell, preserving selected opaque markers. A calls actual `lock_04a6ac` against the real supplied `srw_10`, verifies its owner/count while owning the native lock, and writes a plain protected payload 41. B's actual acquire callback forwards the SAME raw cell. Before normal blocking acquire, B performs a TEST-ONLY TryAcquireSRWLockExclusive which must fail while A owns SRW, then signals an attempted event and calls actual AcquireSRWLockExclusive. This is not a new production timed/try branch.

Main must observe B reaching that boundary while still owning SRW, and B's entered event must remain unsignaled. Thus a new whole-helper/body mutex that prevents B reaching its native-acquire boundary would fail the finite handshake, rather than pass after releasing A early. No sleep/scheduling delay guesses establish contention.

A's recovered unlock must clear owner/count BEFORE the Platform's actual release: the release callback checks those fields while still holding the actual SRW. B then acquires, verifies its own actual thread id/count1, reads payload41, writes42 and unlocks. The exact source-native acquire/release counts are2/2; the test-only TryAcquire probe is separately identified. After the actual thread handle signals, main checks B's returned status/private errors, idle owner/count, payload42 and unchanged type/storage/opaque fields. It does not assert undocumented idle SRW bit patterns.

The SDK-forwarding callbacks set distinct LastError stamps immediately after actual acquire/release. A/B assert those stamps survive the source helper's owner/count metadata operations. Test owner observations use temporary relaxed atomic_ref values; none remain live across SDK calls/waits/release. Plain count reads occur only by the actual SRW owner or after child exit/join. The live pre-acquire callback does not peek at raw count or assume an idle owner.

## Child safety and finite retirement

The existing plain global `checks` counter is incremented only by controller assertions. Child callbacks record private atomic errors/counters/events and never call global require. The WINAPI procedure catches any unexpected C++ exception before returning a failure status; nothing escapes across the callback boundary. Two owned manual-reset events and the actual thread handle remain live until the finite native join finishes. A failed wait/assertion after launch terminates the own fixture process, preserving borrowed stack/event/SRW storage instead of unwinding it under an active child. Normal handles close only after actual B exit.

## Validation limits

Static typed review checked actual WINAPI signatures, direct SRW-cell identity/alignment, count ownership, per-thread LastError, callback/global-counter separation, finite joins and default main registration. The before/current static whitespace diff emitted no whitespace diagnostic. The root's production candidate at review is execution.cpp `c9a30ba892a526b9b00068d7e8d15927a60e2ecf7eda8d23a02bbfa2576f6884` and execution.hpp `eb6c3dd0fd279ee9aecafef33c60ba184ab424b3e003143b5271f91ce818ab55`.

No compiler, project test, executable, reconstructed DLL, linker or ZIP operation ran. This is authored test source, not an observed SRW handoff result. A physical x64 handoff alone cannot demonstrate absence of the original C++ data race; complete atomic live-owner access is the separate static production obligation. No sanitizer run, original mutex/MSVC ABI/EH equivalence, actual Domain reconstruction, general business concurrency or Graph/Profile/NativeJobs policy enable is claimed. Independent reviews are separate artifacts; this note will remain unchanged after freeze.
