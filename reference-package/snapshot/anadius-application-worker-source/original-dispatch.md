# Original Worker dispatch — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

The completed native Image entry selects OriginalApplication, but the Worker
Proxy still unconditionally called the additional HOST startup barrier before
IO context dispatch. That barrier waits for separate Host and Online milestones
and can time out after 10 seconds. It was introduced for the cooperative source
composition, rather than recovered from server_start at 0x180029ea0. Thus merely
selecting original lifecycle retirement did not preserve original dispatch order.

Pending now captures the selected OriginalApplication mode before Graph consumes
its options. At the same dispatch point, that profile retains actual thread/fiber/
PTD, poison and stop admission checks, marks admission, and proceeds directly to
the real backend. Explicit and SourceOwned retain the existing finite Host+Online
barrier. No original Execution wait or module's own lower wait is removed.
There is no fabricated milestone, timeout extension, polling or deferred entry.

Retained server evidence-map.json identifies assembly and pseudocode for
0x180029ea0 and 0x180024480: context/constructor, port publication/readiness,
accept, context run, normal destruction. Bootstrap source at 0x180038970 retains
the separate three-worker/host/synchronous-main-library order. The true library
phase has no worker-ready wait; the false library phase retains wait_steps(2).
This bounded source review does not establish every wait in the lower graph or
OS loader-lock safety. No fresh native analysis or target execution occurred.

A new unexecuted worker test source uses the existing real Domain/NativeThreads
fixture with controlled socket/IOCP boundaries. It sends neither Host nor Online
startup notification, expects actual accept995/relay dispatch and local original
retirement, then checks real native/FLS exit before controller memory retirement.
The existing two cooperative cases still assert that one milestone is insufficient.
No test result or assertion count is claimed: none was compiled or executed.

Both loader helpers remain source-defined. Current source differs from the last
full build in 54 source/header files. Last full build, historical failed link,
package receipt and delivered ZIP are preserved. This fixes one observed source
ordering mismatch; it is not a new native body recovery or an overall DLL completion
percentage. Compiler CRT/EH, general callback concurrency, native teardown and
live FreeLibrary behavior remain unvalidated.
