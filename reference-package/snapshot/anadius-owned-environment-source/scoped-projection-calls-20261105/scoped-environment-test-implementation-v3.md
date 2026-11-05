# Scoped Environment bounded root component tests v3

Status: authored source and static review only. No compiler, project test,
linker, reconstructed DLL execution or ZIP operation was run.

Only owned source changed: `outputs/anadius-owned-environment-source/environment_tests.cpp`.
Captured before SHA-256: `0777c3614d7f8cdb96bfe3c02546a737f29557ac1b809bc7061dfea59f84c734`.
Final SHA-256: `9ded4933d9eb592638113835228d0ee1257e9e3906f4243dd33ff15a3036f934`.

This report supersedes v2 for the final test source. The previous reports are
unchanged:

- `scoped-environment-test-implementation.md`: SHA-256
  `c77e4cdb7ca920475db929b55e3ba5f7df934bfe8436f7e700877a3bee51c0b6`.
- `scoped-environment-test-implementation-v2.md`: SHA-256
  `22d23fad0702ed7deb944bc6695de2a3b17c87a1eb2e63b678d7dad9e1df96b3`,
  describing preceding source
  `02b5350e6359e276391e76a895f52cac9fe47eaffcaa270a53c9452c92e3bc7f`.

## Known unregistered input versus corrupted registered row

The `address` isolated branch now distinguishes two different public failure
boundaries. It copies a valid source's defined fields to a wholly unregistered
address and attempts its raw root query. UnsupportedInput must occur BEFORE
effect publication or any checkpoint. The actual provider remains Bound with
active_calls0, exactly its one genuine reference, unchanged creation/copy/move/
destruction counters and unchanged source/forged fields. Admission count is
compared BEFORE another valid query can add legitimate callbacks.

The original registered source is then queried normally and must retain its
same capability, Bound phase, reference count and inactive state. A known bad
address therefore cannot silently poison an unrelated healthy owner or create
a copied authority merely by duplicating projection fields.

The existing `executor` branch remains separate: it corrupts the executor of
an ACTUALLY REGISTERED source. Whole-move full-source preflight must poison the
provider, retain the original reference, publish no effect (active_calls0), and
leave both source and destination native prefixes untouched. This is not
weakened into the healthy foreign-address result.

This revision changes only the `address` branch relative to the v2 source.
It does not manufacture scope permission to test initial enter_job or a scoped
constructor positively; those production paths retain their own explicit
preflight checks and require genuine Parent capabilities.

## Preserved bounded regressions and accounting

The raw slot08 test still detects payload/copy-counter publication inside a
nonmutating callback and requires the destination's defined placeholder
executor until the raw slot returns. It never queries the incomplete output.
Only the actual caller's tail installs the source executor; final capability,
reference/copy counters and normal cleanup remain exact.

The whole root move test still requires FIRST callback admission before any
source/destination native header write, then checks exact capability transfer,
LastError, moved-from sentinels and one move without reference creation. The
`move-probe` terminal312 still verifies swallowed same-actor reentry cannot
commit a native prefix or replay its retained admitted unknown effect.
Checkpoint/probe/unknown admission, tracked-before/after and handler-failure
branches still expect active_calls1 for an already-published failed effect,
keeping that retained membership distinct from known input preflight.

The existing 4100-projection growth/reuse, OriginalDefault classifier and
tracked executor family test bodies remain byte-for-byte equal to the captured
source, rechecked by read-only text comparison. Existing self-deleting handler
ownership handoff, no task/environment reread, consumed-prefix checks, eleven
declared isolated branches and natural successful cleanup remain unchanged.
No declared mode has been executed.

## Limits

These are sole-worker component source tests using EnvFixture's controlled FLS
and WorkStop. Its finish_job deliberately rejects because there is no genuine
prepared child authority. No JobPermit, counter reset, fake work pin, duplicate
Environment business root or production test getter was added. Positive
Scope.operations/Scope.copy child coverage, actual two-Scope tracked progress
with A held after a real decrement and B native/FLS/Parent reaping on the same
selected Environment/Parent/Runtime capability, and actual Runtime
oldcount1/Returned-child stop composition remain OPEN. No compiler/runtime,
native ABI/EH or full original child response-tail equivalence is claimed.
