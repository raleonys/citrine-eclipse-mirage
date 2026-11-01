# B29 independent accepted-session prerequisite audit

The refined B29 scope is justified: implement two genuinely missing normal typed algorithms, `023d20` and `028950`, plus the existing `025720` greeting lifetime correction. Keep the full `023000` shared-owner constructor for B30. Exactly two source function mappings are supported; no new native ABI/provider closure follows.

`023d20` copies the service, initializes cancellation and protocol defaults, copies executor tables78/88, invokes the selected table78 slot8 on actual destination/source payloads, then reads table90 live and calls `028950`. Saved rdata maps initialized table `084928` slot8 to token `0228c0`; that target has no retained body/code bytes, so its implementation remains a precise mandatory lower dependency. Never execute image VAs or substitute an invented payload move.

`028950` transfers socket/state, increments source control before destination ownership assignment, releases old destination control, reloads and clears source control, then releases that residual. It inserts destination at the live service head under the gate. **It does not unlink source.** Only after leaving the gate does it copy/reset protocol/options. Use the cached service argument through callbacks. Distinct SocketObject references must also have distinct AcceptorState backing references; destination is fresh and unlinked.

The future constructor performs two moves: pending to temporary, then temporary to owned socket. It initializes input and deque/proxy, resets the same handler, destroys the temporary with existing `024db0`, publishes caller output, then installs weak-self and drops temporary strong. An output-reference API preserves publication before possible weak-release callback failure. Original allocator and EH/unwind semantics remain required; the apparent allocator-null path is not a valid empty owner result.

The B28 baseline `Session::start_025720` retained greeting through `do_read`; native heap greeting free is at `025911`, before read start `025919`. Current B29 source now scopes greeting and send loop in an inner block, correcting this lifetime without adding a new source mapping.

The actual detached frame closure holds a raw owner, not a strong reference. Weak-self plus a pending read does not prove that owner survives detached work if I/O ends. Keep caller-owned lifetime/quiescence requirements explicit; this stage must not hide that gap behind a host shared_ptr factory.

Evidence IDs, detailed address phases, reused source bodies, required effects, failure prefixes and source hashes are in the JSON. Only these independent work reports were written. No original execution, new query, original input read, production/test change or generic CRT reopening occurred.

Current `move.hpp`/`move.cpp` and corrected `server.cpp` received an early read-only production review with no blocker found. Fresh backing validation, callback-sensitive control reload, live post-callback table90, cached gate and source-list retention match the supported typed domain. If source is the old head, inserting destination naturally updates its previous link; “no source unlink” does not mean that previous link stays unchanged. Current hashes are in JSON. Tests and final freeze/common-build evidence have not yet been reviewed.
