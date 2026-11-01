# B10 bootstrap logging and Services completion

CompletionServices selects five remaining methods of the existing Services
interface: log_insert, log_finish, cleanup_threads, terminate_log and
unexpected_terminate_return. Its twelve other methods still reach the mandatory
owned lower provider. Combined with B3/B4/SourceServices this supplies selected
source implementations for17/17 Services methods. This count is a composition
metric; it is not a full native-provider/DLL readiness claim.

## Explicit stream ownership and identities

BootstrapLogger requires an explicit shared_ptr<std::ostream>. The caller must
keep that stream's streambuf alive too; owning an ostream alone cannot adopt an
independently destroyed custom streambuf. Construction performs no stream I/O,
does not choose a global console, and does not register callbacks.

The original preferred-image stream value0x1800a4130 is an opaque observed
identity. It is never cast to a compiler ostream or dereferenced. A logger also
accepts its own stable byte-token address and returns that token after insert.
The immovable logger keeps token identity stable. Null/foreign stream identities
and unknown literal addresses are rejected before insertion; this adapter is
bounded to the actual bootstrap/fatal routes rather than an arbitrary native
ostream or address resolver.

| Literal VA | Exact retained C string |
|---|---|
| 0x180083cd8 | LSX emu |
| 0x180086720 | space + version + space |
| 0x180086718 | 2.0.1.0 |
| 0x180086700 | Unhandled exception |

The second literal's exact byte string is0x2076657273696f6e20. The four payloads,
terminators, hashes and retained data identity are in logging-evidence.json.
They came from the complete retained read_bytes record
ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082
at0x18007f000, not a fresh original-file read.

## Selected compiler stream policy

Retained006880 is formatted NUL-terminated C-string insertion. The selected
compiler implementation uses ostream operator<<(const char*) so width/fill and
stream-state handling remain with that compiler's stream implementation rather
than being replaced by unformatted write(). Retained0088e0 widens newline, puts
it and flushes; std::endl is the selected compiler counterpart.

No stream flags, exception mask, locale, width or fill are reset by the adapter.
Normal stream errors follow the selected ostream behavior: a masked failure may
set stream state while returning the token; enabled exceptions propagate. The
adapter does not invent a successful write check or swallow stream exceptions.

A per-logger recursive mutex serializes its calls and permits same-thread
streambuf callback reentry. Each insertion/finish is separately serialized; a
multi-part line is not made one atomic transaction. Other direct users of the
supplied ostream require their own synchronization. General native iostream
locking, cross-thread reentry/deadlock, concurrent locale mutation and lifetime
equivalence are not established.

These are rebuilt source compatibility choices. The original MSVC basic_ios
layout, vtable adjustment, streambuf/facet locking, locale/EH/fault semantics and
physical native stream pointer are not adopted or proved equivalent.

## Fatal and unexpected-return routes

terminate_log performs the retained fatal C-string insertion, widened
newline/flush, then compiler std::abort. Healthy logical output is
"Unhandled exception" followed by the selected stream's widened newline.
If insertion or finish throws, that exception propagates before the abort call:
the source sequence is preserved without inventing logging-success fallback.
What a native MSVC terminate callback does after such an exception remains an
EH/runtime boundary.

The original038890 calls038920 and has INT3 if it unexpectedly returns. The
selected unexpected_terminate_return route unconditionally calls compiler
abort, so it cannot depend on debugger continuation or pretend success.
This deliberately differs from original INT3/debugger/SEH behavior.

CompletionServices owns logger and shared exit-wait registry before its lower
Services member, so reverse destruction keeps those policy owners live through
lower-provider destruction. Worker/FLS stop and joins must precede destruction.
The registry retains its separate explicit close contract; this wrapper does
not silently close a shared registry or substitute an automatic worker join.

## Static anchors and limits

| Native address | Observation | Pseudocode Evidence ID |
|---|---|---|
| 0x180006880 | formatted C-string insertion | ev_fbcd6dd7e3f40d8a0949ddad6acfeb50aac3210ec0b8ffe4e7905d1397f3bb36 |
| 0x1800088e0 | widened newline/put/flush | ev_e32e8e2eddad4e3ec7d392028d39ca1acab03d86ac509c1bbffe7e88089f5f5a |
| 0x180038970 | three bootstrap insertions then finish | ev_e66705fdaa9bdd88fb50be13d22e354939026d931f472329e0f14e17c9fb4fbd |
| 0x180038920 | fatal insertion/finish then abort | ev_b35c015816e9d434a8d5dbb8a0296baf874922ffb138fb566a634535a3c178d3 |
| 0x180038890 | fatal call then INT3 fallback | ev_fa9e792ca1dc6003fa08b9f989066ed017af372bde3045f31207ef552c7e855b |

Target SHA256 is9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.
The retained bundle SHA is
c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495.
No original/user CFG/token/achievement input, blocked version file or historical
generated version DLL was read by this author; no original or rebuilt DLL was
executed or modified. Prior746 source files remain unchanged.

Native bodies/closures added0. Selected Services method coverage does not close
the remaining configuration, server/online/host/hook/foreign providers, full
mutable CRT/EH/ABI/concurrency/image-unload/loader-lock/PE-entry requirements.
