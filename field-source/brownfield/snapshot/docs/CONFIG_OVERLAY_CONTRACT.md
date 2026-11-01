# Configuration-first base + override contract (Candidate02 / NOT_RELEASE)

Release runtime loads `surge.cfg` followed by optional `surge_override.cfg` from the **DLL module's own directory** with UTF-16 Win32 paths. Both are parsed as complete Config2 documents; if both exist their *outer object bodies* are combined in base-first, override-second order, and exactly one authoritative transactional `set_runtime_configuration_cfg()` publication occurs before hook selection. The existing Config2 parser and behavioral service-state parser consume the same combined document. Later direct scalar leaves therefore replace earlier normalized values in the model; base-only leaves survive. Nested collection and exceptional Anadius-donor parity is **NOT PROVEN** and remains an acceptance boundary.

- Both missing: `not_found` and default 7-hook surface; base-only or override-only: parse and publish one document once.
- Any present malformed document, read/close failure, allocation failure, or commit failure: fail closed without publishing an intermediate base. Existing startup error path stops before MinHook staging.
- `BlockReruns` / `Documents`: selected only from committed policy. No live CFG publication when hook ownership is frozen.
- Separate local byte buffer for each file, no shared mutable snapshot and no config override from process CWD.
- **Not a release or donor behavioral parity claim.** Changes affect production Config2 loading; Windows MSVC, per-mask physical 7/9/9/11, native fault injection, CRT/Golden and CollegeFB27/Madden27 must run on Candidate02 exact source identity. Previous Candidate01 receipts cannot be inherited.
- Universal governance G0 remains fail-closed. Candidate02 carries inherited historical release-state metadata until separately authorized provenance alignment.
