# Grow session registries and Graph logic rows — 2026-11-04

The selected Profile and Graph now grow their session/logic registries instead of rejecting a valid owner when the initial16 rows are full. Existing session objects and token generations stay stable; every selected internal span and queued-write alias is updated coherently. Initial session settings remain the starting allocation choice, while other job/IO/environment limits remain separate.

## Fresh native basis

Six fresh REA6.8.0/Ghidra12.1.4 records and exact addresses are retained in `native-findings.md` and the complete Evidence bundle.025720 constructs each accepted owner via023000 at0x180025766, builds greeting02d540 at0x180025810 and starts0280d0 reads at0x180025919.023000 allocates0x1c8 per accepted owner/control at0x180023020. The examined allocation/greeting path has no source-style16-session pool check. Identity/hash verified; session closed after export; no binary execution.

## Coherent production change

`profile.cpp` keeps Slot records in a deque, preserving addresses of old receive/send/environment/queued/message subobjects. On exhaustion it allocates replacements for all eight per-session registry vectors, checks existing admission/shape, appends only empty unpublished Slots, checks again and copies current binding records. A rollback guard removes unpublished appended rows if the precommit path throws.

Commit uses only noexcept swaps, direct span publication and passive setters. It updates frame owners, read string/stream bindings, write streams, finish/queued/message owners and both captured environment-source spans. Any queued stream_10 pointer into the old write table is translated by its exact old row index, including valid aliases. No old table allocation is destroyed until these views and aliases point at coherent replacements. Greeting rechecks admission after growth returns, covering old metadata destruction before adopting the new control block.

`graph.cpp` independently grows token/Logic rows before a valid new session index is used. It copies current old rows after allocation/admission checks, publishes under existing metadata scope and reacquires rows by index after Effects/Logic construction. A private registered flag prevents an unpublished Logic's rollback destructor from retiring a row it never owned. Registered live Logic destruction retains its previous checks and count updates.

Two small inline bind_environment_sources setters in environment_constructor.hpp and coherent_io.hpp update existing private captured spans without provider callbacks. Profile/Graph public comments now describe initial session sizing. Public RegistryView values are borrowed snapshots: callers must reacquire them after greeting can grow the registry; internal retained contexts are explicitly republished. Existing header-ledger registered fields keep their stable context addresses and are not rebound.

This uses existing selected worker/Parent serialization. It does not enable arbitrary concurrent metadata mutation or add global readiness, safe-unload or CRT frameworks. Natural allocation limits and existing failure-prefix retention remain; allocation failure is not a promise of a reusable unpoisoned runtime or exact native EH/allocator ABI.

## Source verification boundary

The independent production review is retained separately in independent-review.md, including six final production/header hashes and the RegistryView lifetime clarification. Test-source review is a separate addendum so the completed production review is not rewritten to imply tests were already checked.

The completed regression sources distinguish20 factory-backed Profile owners in a core scenario from real controlled-SDK two-peer pipelines starting with one registry row. The two-peer Profile path exercises old/new I/O after replacement; the two-peer Graph path exercises the independent Logic row growth. These are not20 real network clients or observed original DLL runs. Final test-source notes/addendum record their actual completed scope and hashes.

No compile, project tests, link, reconstructed DLL execution or ZIP update is authorized or performed. Before copies, implementation notes and final source hashes are retained here. Existing DLL/ZIP and frozen build/link/package proofs keep their historical bytes. File-difference counts measure pending source changes, not a numerical DLL-completion percentage. Continuous REA/Ghidra/subagent work remains active until the user stops or the objective is achieved.
