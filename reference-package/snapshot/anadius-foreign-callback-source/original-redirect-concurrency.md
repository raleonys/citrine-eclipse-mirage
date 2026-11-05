# Original connect redirect integration — 4 November 2026

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

Fresh REA/Ghidra observations from the parent and three independent auditors
were used for this stage. Each child has its own MCP target session: initial
target-unavailable replies were retained, then each opened the same original
anadius64.dll and verified SHA2569e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.
All three child sessions closed after exporting complete Evidence. These are
fresh native observations, not relabeled retained decompilation.

Native00fd00 facts controlling the change:

| Address | Observed behavior |
| --- | --- |
|00fd42|Read and cache caller port|
|00fd64|Non-web calls036880(-1), including3216|
|00fd6e..00fd7a|Special3216 / server-port redirect decision after wait|
|00fd8a..00fdbb|Literal, requested port, literal, server port, newline/flush|
|00fdc0 /00fdc6 /00fdc8|Separate counter load, increment, store; no locked RMW|
|00fdce|Compare cached EAX with15, not another shared-cell read|
|00fdd3..00fde0|Reread server port, htons, write only caller port|
|00fe02|Tail dispatch original connect|
|00fe48 /00fe52|Terminal15 dialog then exit(-1), before caller port write/SDK|

Primary fresh instruction Evidence is
ev_839077f99c02612bc8d409358a72a00b4078cf9a201c931361fd8e1c4381c349.
The independent exact19-byte counter read is
ev_2c123225d73c1d6658e7511d29381e3e29841c9cbc05314f94b00a96a0b2d5f4.
Typed direct counter references identify READfdc0 and WRITEfdc8; computed aliases
and external writers are not excluded by that negative search.

The Original connect lane now has explicit Checking, Waiting, Redirect and
Original phases. Untouched calls dispatch through the existing exact trampoline.
Web calls with an actual bound snapshot choose redirect/original immediately.
Every non-web port, including3216, can enter wait-only before Server publication;
no bound-port provider is read until the actual recovered wait returns. That
checkpoint then binds the actual port and chooses redirect-body or original-only
permission. Native entry calls the actual recovered redirect helper before body
completion and SDK dispatch. General registry/allocator/private recovered PTD
permission remains separate.

Independent review found and corrected two intermediate issues: a premature
non-web port-provider read, and restoring wait/body authority after a failed
checkpoint. Failed checkpoints now remain permissionless Checking until leave.
Counted lifetime, thread/fiber identity, LastError and pause retention cover all
phases. No mutex spans actual wait/body/SDK effects and no external unload proof
was added. Other general callbacks and nested callbacks remain bounded.

All active recovered counter updates now share transient lock-free atomic_ref
relaxed load/store operations. The uint32 increment is cached for terminal15.
This avoids C++ conflicting plain counter accesses while retaining possible lost
updates from the native unlocked load/increment/store. fetch_add, CAS and a
serialized counter would strengthen that behavior and were not substituted.
Alignment/always-lock-free assertions express requirements for a future build;
they are not evidence that this source has compiled or emitted exact machine code.
External plain counter access requires quiescence. The atomic_ref lifetime ends
before dialog/exit callbacks.

The concrete Policy effects use the same managed Stream with per-insertion
leases and immutable Dialog API/local buffers; they require no general registry,
source allocator or private recovered PTD access. Whole lines may interleave.
The existing locale/buffer/tie/reentry/custom-provider limits remain. The two
Original SDK wait/body routes also apply to uncontended eligible connect calls.

Unexecuted source scenarios cover the permission table and rejected checkpoint
prefixes, two concurrent actual redirect helpers plus one passthrough, preserved
caller tails, one-or-two retained unlocked increments, flushed per-thread LastError,
cold IPv6 wait before Server publication, post-wait original/redirect decisions,
same-port/web redirects and Original terminal15 child expectation193. Source core
tests inspect phase contracts; integration sources invoke actual native entry,
concrete Policy, recovered wait and redirect helper. None were run.

Seven production source/header files and two test files changed.68 source/header
files differ from the frozen1017-source full build. A final independent static
review recorded exact current source hashes and found no further concrete defect;
this is not compiler or runtime validation. Admission vtable and internal source
contracts require consistent rebuilding of affected consumers when authorized.
No build, project test, link, DLL execution, original modification or ZIP update
occurred. No new normal native body/ABI closure or overall percentage is claimed.

The next native-backed audit already identifies a remaining fidelity boundary:
cold80/443 still falls back to an ordinary body without a bound snapshot, and this
stage's Original redirect helper uses a snapshot local. Native reads a live word
independently for branch/log/final rewrite. The separate server-port audit records
the coherent live uint16 model and actual configuration/bound writers needed to
remove that boundary. General nested original-only callback candidates are also
audited separately; Windows internal reentry was not observed or asserted.
