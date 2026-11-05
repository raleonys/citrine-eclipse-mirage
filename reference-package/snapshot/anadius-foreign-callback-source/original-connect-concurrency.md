# Concurrent untouched connect calls — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

Native00fd00 first compares the supplied length and family with the two global
lengths. Unknown length, or mismatched family at a recognized length, branches
directly to the original connect trampoline at LAB_18000fde4 / call0x18000fe02.
That route does not read a port, wait on036880, log, change the redirect counter,
write sockaddr bytes or call this DLL's CRT/PTD helpers. Retained pseudocode
Evidence: ev_56829116b2c9c143c2c0fe7d3fa88a54fc6499474948b76b08e8cd2740d33c68.

The selected OriginalCrt source profile previously rejected every overlapping
callback in foreign_callback::Domain::enter before reaching this direct route.
Original overlap handling is therefore a source behavior limitation, independently
of the still-unverified native callback/runtime ABI.

The source bridge now allows concurrent untouched connects while another body or
SDK-only connect is in flight. Selection uses only the Context's immutable length
snapshot and the caller's two-byte family under an actual native Context lease.
An unknown length reads no caller bytes, including null pointer/-1 forwarding.
A recognized IP family still follows the existing ordinary body path.

Only the original per-hook publication profile advertises this route. It obtains
the same actual successful-create publication for connect before SDK permission.
The existing exact OriginalEffects/Publication trampoline and original-checkpoint
pre/post checks remain in use. A trivial thread-local record identifies the
admitted thread/fiber; a mutex-protected count retains all active calls. No
private CRT/PTD allocation, shared Policy body, wait, redirect or log permission
is granted. Locks do not span publication callbacks or the original SDK call.

Uncontended calls keep their existing ordinary PTD/body route. Cooperative
profiles and other callback entries keep their existing admission behavior.
Nested same-thread/fiber reentry, recognized IPv4/IPv6 bodies and concurrent
registry/shared-state operations are not generalized by this change. This is not
full callback concurrency or native ABI equivalence.

Pause seals new admissions and reports false while either ordinary bodies or
SDK-only lanes remain active. An already admitted original call can finish and
pass its original-only metadata check after pause. Binding/provider teardown
still uses the existing actual call counts; no new external safe-unload proof
or different DLL detach policy was introduced.

One unexecuted integration source scenario holds an actual ordinary admission
while two native connect_entry calls reach the exact SDK spy concurrently.
It covers null/unknown length and two-byte mismatched family, restricted
permissions, no source PTD creation, distinct incoming/outgoing LastError,
host setup completion, pause with retained lanes and final binding drain.
The spy serializes its own observations rather than racing the fixture's check
counter. This is written test source, not a PASS or observed execution.

Four production files and one test source changed.65 source/header files now
differ from the frozen1017-source build. The Admission vtable and Domain layout
changed; all affected consumers require a consistent future rebuild. Only
lexical delimiter consistency and source/hash bookkeeping were observed here.
No compile, project test, link, DLL execution or ZIP update occurred. No new
native normal-body count, ABI closure or overall completion percentage is claimed.

The preceding static application review is retained alongside this change:
02d540 greeting fields/order, SDK binding and NUL;034cf0 random-byte/hex projection;
030010/030d30 achievement decisions/persistence; and a broad direct-XML-literal
comparison for39 command registrations/34unique handlers.32 have canonical
retained function bodies;2LAB handlers retain separate existing disassembly.
The three apparent literal absences resolve to license/literals.hpp, not missing
handlers. This broad presence comparison is not a complete per-command behavior
proof. The full2076-entry current source map reports518 tagged/explicit entries,
46composition-only entries and1512 requiring review; those are heuristic source
presence labels, not completion or missing-function counts. Generic CRT remains
deferred.
