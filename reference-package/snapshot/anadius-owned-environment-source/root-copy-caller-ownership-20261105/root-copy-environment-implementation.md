# Owning Environment actual Root copy caller

Status: environment.cpp source frozen for independent static review. No compiler, project tests, linker, reconstructed-DLL execution or ZIP update ran. Only environment.cpp is owned by this author; Root owns the actual caller/interface files and the SDK author owns component tests. Production Parent policy remains Serialized.

Frozen source: `outputs/anadius-owned-environment-source/environment.cpp` SHA256 `d3b83e35e3cb1237b25398c35f14a29f3eab7131f893e478b5e1d8e1e02ba9cf`.

## Native evidence and bounded problem

Fresh complete REA6.8.0/Ghidra12.1.4 `instructions-180028d30.json`, Evidence `ev_c1e35ff0c5c57064a1ddf6f2ba08016bd1eecedf05e9c21ef99d15c95c75e2c7`, binds to original anadius64.dll SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. The analyzed body contains656bytes in three inclusive ranges180028d30..180028faf,180028fb1..180028fbb,180028fbd..180028fc1 (658byte span; do not describe it as a contiguous656byte interval).

The first measured copy writes operations28e1f/schedule28e2a, reads cached slot08 at28e2e, invokes28e3a, THEN reloads live source executor28e3d and stores28e44. The second copy writes28e4c/28e54, invokes28e64, THEN reloads28e67/stores28e6b. The raw slot08 callback therefore owns payload/state only; its caller owns the final live executor load. Earlier stage correctly preserved that partial callback, but its per-callback Effect could end before the actual caller tail.

## Implemented owning-provider hook

OwnedTable now overrides the HOST-only `try_copy_environment_028d30` interface. It enters ONE existing registered Effect with exact full source/read and fresh move-style destination/write address claims BEFORE invoking the actual caller body. Destination's old payload/state/executor remain undefined inputs and are not inspected to authorize the copy.

A private synchronous stack IoEnvironmentCopySlot bridge binds that exact Effect and the exact source/destination identities. It validates the cached function against Control::copy_root only AFTER the actual body writes its native header prefix and performs its real binding lookup. No early copy_at/function lookup, raw callback recursion, second Effect, owner re-resolution, current-copy global, TLS/actor selector or shadow provider root is introduced.

The bridge invokes c.copy(...,true) for payload/state only. It records the actual destination capability and performs the existing callback post-admission while copy_output still marks the destination's legitimate partial executor prefix. Thus source fields are fully validated, payload/reference/work effects are real, and the raw callback does not fabricate an early executor write. The actual caller body then performs its original LIVE source executor load/store after the bridge returns.

The owning hook runs that body once, requires one normally returned exact slot/result, checks actual destination capability and complete source/destination fields, and clears copy_output ONLY AFTER the actual body tail completed. Full claim validation and ordinary Effect final admission follow while the same whole-call claims remain published. Private slot/adapters retire on the stack before Effect unlink; metadata node/allocator retirement now occurs after the full destination tail, rather than in the legitimate raw partial window.

Missing/ambiguous copy binding and foreign cached functions retain their preceding native header prefix plus registered unknown Effect. No rollback, speculative callback, duplicate native retain, cleanup or second lookup is attempted. All callbacks, native/body operations, exception construction and allocation/deletion remain outside metadata gates; protected scalar checks and state publication stay under metadata. No caller Body/Slot/stack reference is retained in durable records after return/unknown unwind.

## Compatibility and remaining scope

Raw legacy Control::copy_root remains the same partial slot08 callback for trusted direct callers. Unowned default/native providers still execute the Root-owned actual helper's native header -> lookup -> callback -> live executor order and its existing failure prefixes. Scoped whole-copy/constructor, sentinel result ownership, Policy work-release records, actual work counters/families and handler/response scheduling authority are unchanged.

Root owns the helper wiring for BOTH actual dispatch_prepared_response copies. SDK component checks separately observe pre-prefix whole admission, post-payload placeholder executor, post-caller full executor, exact capabilities/reference counts, default-provider live reload and missing/ambiguous/foreign callback prefixes. These checks are authored/static-reviewed, NOT run, and do not substitute for native runtime observations.

Actual Prepared two-scope tracked-family physical progress, a naturally established child-old1 Runtime stop, child scheduling/response/021770 tail order, general shared lower mutation, native EH/ABI equivalence and full runtime equivalence remain OPEN. This measured caller ownership fix earns no new native-function completion percentage. Original binaries and historical build/link/ZIP proofs remain unchanged.
