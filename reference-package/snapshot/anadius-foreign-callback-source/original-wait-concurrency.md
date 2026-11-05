# Original non-web wait-to-SDK calls — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

Native00fd00 reads the caller's port at00fd42, calls036880(-1) at00fd64
for non-web ports, then checks special3216 and the server port before dispatch.
A nonredirecting port therefore must still wait for all three native steps.
The concurrent source SDK lane previously covered only untouched and80/443
calls. A recognized non-web call overlapping another callback still failed at
the general admission gate.

The lane now has a separate wait-only phase. Selection requires actual connect
publication, an existing immutable actual server-port snapshot, a different
requested port and requested port neither80/443 nor3216. While waiting, only
connect_wait_scope is granted: original SDK, general Policy and redirect body
permissions are false. The native bridge calls the existing Policy/Execution
036880(-1) routine. On its normal return, publication and the same bound-port
value are rechecked before permission changes to original-only and the exact
trampoline is called.

The Execution source mutex covers each native predicate/flag critical section
and clock/cache access. It is released before Sleep; no new wait implementation,
timeout, cancellation or simulated ready result was added. The selected execution
providers use SDK thread/SRW/QPC/Sleep functions on their normal path. A separate
private callback PTD is not created for this restricted route. Error/ABI behavior
outside the selected source contract remains unvalidated.

Pause seals new calls but preserves the actual already-admitted wait. It cannot
cause the SDK call before the real steps complete. Unknown publication-provider
exceptions retain the existing poison/fail-stop treatment. Missing/equal server
port and3216 keep their existing ordinary wait/redirect body route. Cold startup
before any actual port snapshot, nested callbacks and shared redirect-body
concurrency remain limited.

Core test source now distinguishes wait-only and original-only permission,
excludes3216/matching port and invokes the actual recovered Execution wait before
the source completion checkpoint. The existing controlled-wait integration source
has an additional OriginalCrt mode: a primary ordinary callback remains admitted,
an actual IPv6 connect_entry waits on incomplete Online/Libraries flags, host
setup/pause proceeds, the primary admission leaves, actual finish calls release
the waiter and the SDK spy observes no private PTD and one unchanged sockaddr.
These are written expectations only; no test process was launched.

Four production files and two test files changed;65 source/header differences
remain pending. Admission's virtual signature/slots require consistent future
rebuild. No compilation, project test, link, DLL execution or ZIP update occurred.
No new normal native-body count, native ABI closure or total percentage is claimed.
