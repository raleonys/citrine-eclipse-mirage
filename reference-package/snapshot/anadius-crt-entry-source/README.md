# Recovered outer CRT DLL entry: static-UCRT normal flow

`entry.cpp` recovers the outer entry/dispatch/process and thread lifecycle at
`18004bcfc`, `18004bbc8`, `18004b9e0`, `18004ba30`, `18004bb48`, and their
bounded helper functions. It compiles as an independent C++20 object. This is
reconstructed source, not the original source or a complete runnable DLL.

The caller owns `State`, `Effects` and the application onexit table. Construct
`State` with a reference to `crt_onexit_recovered::State::application_0a47a8`
from the onexit module (or the same actual `Table` owner). This preserves the
shared global object without pointer aliasing or snapshot copies. The owner
must outlive the entry state and every call. No hidden locks are introduced.

The target's static-UCRT flag and raw DllMain hook initially equal zero.
`static-image-facts.json` records the exact source ranges or PE virtual-tail
zeros under the original SHA-256. This module selects that static-UCRT mode:
startup-lock acquire returns false and release has no action; initial onexit
table setup stores six all-ones words. The original constructor/terminator
callback ranges at `1800959a0..1800959a0` and `1800959b0..1800959b0` are empty.
Those exact no-action branches do not claim general CRT support.

The recovered ordering includes CPU/CRT initialization, state `0 -> 1 -> 2`,
C then C++ table walking, TLS slot validation and callback, wrapping attach
count increment, application invocation, attach-failure rollback, and process
or thread detach cleanup. Signed-positive attach counts are tested through a
bit-preserving conversion. AL-only booleans remain separate from full EAX
results, including app result `0x100` and initializer error `0x100`.

Every remaining subsystem is a mandatory effect. Neither a platform API nor an
unknown operation gets a production success stub. The TLS callback is obtained
from a required live slot, its slot address is checked, and the callback is
reloaded after validation. No preferred-image VA becomes an executable pointer.

Only normal control flow is represented. C++ exceptions from effects propagate;
the reconstruction does not add rollback or finally blocks absent from that
normal flow. Native SEH filters, scope tables, unwind helpers, EH/XFG ABI,
loader registration and full CRT lifecycle equivalence remain open. This
module exposes no real `DllMain` and never loads the original or rebuilt DLL.

Run `build.ps1` to compile the object and execute synthetic `entry_tests.cpp`
fixtures. `validation.json` records compiler identity, arguments and artifact
hashes. Tests cover lifecycle ordering, AL/EAX distinctions, initializer
failures and partial state, application rollback, signed/wrapping counts,
terminal-policy codes, shared onexit state, TLS-slot reload and normal-flow
exception boundaries. No target, native loader or production CRT backend runs.

Run `collect-evidence.ps1` only to refresh read-only evidence against the exact
original input identity. It does not change that input.
