> **Historical checkpoint.** Use [DEVELOPMENT.md](DEVELOPMENT.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# MSVC Native test name collision

Owner-provided logs dated October 29, 2026 confirm earlier corrections:

| Profile | Actual Windows/MSVC result reported by the owner |
|---|---|
| Compile | PASS; three load/socket/profile/shutdown/unload cycles using the real DLL |
| Debug | PASS; same public DLL runtime verification |
| CRT | PASS; 6/6 selected CRT/public/source-DLL tests |
| Native | Compilation stopped at C2668 in the asynchronous receive integration test; full CTest did not run |
| Golden | Compilation and export/resource checks completed; product targets missing, BLOCKED/exit 3 |

The remaining Native compilation error arose when the global
`unexpected(void)` declaration from MSVC `eh.h` and the test helper
`receive_test::unexpected()` were visible simultaneously.
`using namespace receive_test` made twelve unqualified calls ambiguous.
Those calls were explicitly qualified as `receive_test::unexpected()`.
Fixture exception behavior and all test scenarios were retained.

The corresponding test adds the same global declaration for non-MSVC
compilers; it has no definition or call there. This allows a future
unqualified call to be rejected during compilation outside MSVC as well.
MSVC uses the real CRT declaration. This test-only declaration is not
added to the product DLL or its exports.

Eighteen test files containing similar unqualified calls/declarations
were compilation-checked with the same global CRT declaration forced
into scope. No collision was observed in other files that define helpers
inside their own class or namespace. Reproduction of the original error,
the correction, and the scan are separate evidence, not a substitute for
an actual MSVC rerun. Raw results are in
[MSVC_NATIVE_LOOKUP_FIX.json](../metadata/evidence/MSVC_NATIVE_LOOKUP_FIX.json).

Only the Native profile needs to be rerun for the revised source:
`scripts/build_orchestration/build_native_acceptance.cmd`.
Application, vendor, resource bytes, and all five exports remained unchanged.
The earlier Compile/Debug/CRT successes belong to the supplied logs;
a Windows result for the corrected complete Native suite is not yet available.
Golden product targets are not addressed by this task.
