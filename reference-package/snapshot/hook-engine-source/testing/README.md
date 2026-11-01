# Synthetic validation of the selected upstream engine

`../build-fixture-tests.ps1` compiles the **actual unmodified** official MinHook
1.3.4 C units `hook.c`, `buffer.c`, `trampoline.c` and `hde64.c`, with one test-only
forced include: `fake_win32.h`. That header includes the Windows SDK headers
first, declares separate `ReaTest*` functions, then redirects every engine Win32
API call site to those functions. It does not redefine an imported function.
Production C objects are built separately without this header.

The script rejects any `__imp_` undefined symbol in the four test C objects.
The fake APIs do not call their Windows counterparts. Heap operations use
ordinary C heap memory; allocation/protection/query operations use owned aligned
`std::array` data; thread, context and module operations use synthetic records.
Executable protection constants are metadata in the fake API only. No array is
marked executable, called, emulated or used as a real function pointer target.
The ordinary runtime libraries used to start and finish the test EXE remain real.

Checks cover public lifecycle/status failures, normal five-byte and hotpatch
seven-byte patching/restoration, real trampoline/relay bytes, actual HDE64
instruction decoding and RIP/CALL/Jcc/JMP relocation, all/queued hooks, 80-hook
table and buffer growth, synthetic instruction pointer mapping, failure paths,
and synthetic module/symbol resolution. Fake APIs abort on an unexpected address
or heap owner. Real process threads, real module symbols and real page protection
are outside this validation.

The chosen upstream version's **individual disable** path calls
`Freeze(..., ACTION_ENABLE)`. The fixture records its resulting unchanged fake
trampoline instruction pointer; the disable-all path maps it back to the target.
This observation is about the selected upstream dependency only. It neither
proves that the target binary contains this exact version nor establishes
production runtime equivalence or safety of deleting active trampolines.

Integration: run the build script as a distinct fixture group. It writes
`testing/build/*.fake.obj`, `testing/build/hook-engine-fixture-tests.exe`, and
`../fixture-validation.json`. Link `fixture_tests.cpp`, `testing/fake_win32.cpp`
and **only the four `.fake.obj` objects** for this group. Never link production
native API engine objects into this test EXE. Never link the `.fake.obj` objects
into the production partial archives. All commands and hashes are recorded in
the validation JSON.
