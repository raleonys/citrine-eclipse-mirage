# Original nonredirecting web calls — 2026-11-03

SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

Native00fd00 reads the requested port after length/family recognition.80/443
skip036880 and go directly to the original trampoline when different from the
current server port (00fd41..00fd73..00fde4). The prior concurrent untouched
lane did not include this recognized-family native fast path.

Original Host assembly now retains its real Milestones::server_port snapshot in
the existing port-snapshot slot. This value comes from the exact bound Execution
publication, uses release/acquire synchronization and is never replaced. Missing
or invalid publication returns nullopt. Online/Libraries readiness is not used.

The native bridge may classify an Original web candidate from the caller's
four-byte family/port prefix under its Context lease. To enter the concurrent
SDK-only lane, the same successful-create hook publication must still hold and
the actual port snapshot must exist and differ from80/443. Missing/equal port
releases the temporary reservation and returns to the existing ordinary body
route.3216 and other ports cannot select this web lane: their native wait and
possible redirect remain required. Uncontended calls keep their existing route.

The snapshot field remains the Cooperative redirect snapshot for existing staged
callers. Under Original it supplies read-only comparison only; staged cooperative
ready/original/web/other proofs still cannot be mixed with original hook receipts.
No new general redirect, CRT/PTD, log or body permission is granted.

Source core scenarios cover absent actual port, matching80, excluded3216,
retry443 on the same thread, original-only permission and reservation drain.
The previous two-thread native integration source now includes a third concurrent
recognized IPv4 HTTPS call using actual port publication while only the Server
milestone is present. Arguments, LastError and no-private-PTD observations remain
separate for all three spies. Both test sources are NOT RUN.

Five production files and two test files changed; the total remains65 pending
source/header differences. The Admission virtual signature changed again and
requires consistent future rebuild. Retained native00fd00 Evidence is linked by
the previous concurrency stage; no new original binary/provider observation was
made. No compilation, project tests, link, DLL execution or ZIP update occurred.
This is a bounded source behavior correction, not full callback concurrency or
native ABI/runtime equivalence. Nested callbacks, non-web waits and shared
redirect/body concurrency remain limited.
