# Session registry growth — fresh native findings, 2026-11-04

REA6.8.0 server identity and original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` were verified before Ghidra12.1.4 queries. Six complete fresh Evidence records (pseudocode plus instructions for each function below) are retained individually and in `rea-evidence-bundle.json`. The native session was closed after export. No DLL/process execution or mutation.

## Examined native path

- Accepted completion025720 calls023000 at0x180025766 on a successful accept. It initializes per-session crypto state, constructs greeting02d540 at0x180025810, sends that session's greeting and starts its first read0280d0 at0x180025919. The accept path then resubmits0270e0. No16-slot registration table is consulted in this examined body.
- Owner constructor023000 allocates0x1c8 bytes at0x180023020 for that newly accepted owner/control block, initializes strong/weak counts and the weak-self identity, moves socket state, constructs per-owner input/output/cipher state and returns the owner/control pair. Each successful accept reaches its own allocation path rather than a fixed16-entry source pool.
- Greeting02d540 builds the per-session XML/version/release/challenge string. Its examined body contains no fixed16-session admission check.

These static observations establish the examined allocation/greeting route. They do not guarantee arbitrary successful concurrent connections under Windows socket/thread/allocator limits, or full native ABI/EH behavior. Ghidra terminal-flow/no-return warnings remain in each complete record.

## Actual selected source restriction

Default ProfileSettings/Dependencies sessions=16 created a fixed Slot vector and eight fixed per-session binding arrays. Application::greeting rejected a valid new owner when no empty Slot existed, after native factory ownership had already been constructed. Graph separately rejected a valid SessionToken index beyond its initial fixed live-Logic row count. Thus changing only the Profile counter would leave an independent Graph restriction.

The correction must keep old Slot-backed receive/send/environment/queued/message objects stable and republish all affected frame/read/write/finish/queued/message/environment table views. Existing queued-owner stream_10 pointers into the old write-binding vector also require index-preserving translation. SessionToken slot/generation, old native owner/control identities and header-ledger span-field identities must survive growth. Source resource limits for jobs/closures/IO/environment remain separate.
