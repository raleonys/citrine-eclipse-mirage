# Owning Environment synchronous prepare40 implementation

Status: Environment2 frozen for independent static review. No compiler, project tests, linker, reconstructed-DLL execution or ZIP update ran. Only environment.hpp/.cpp are owned by this author; Root owns actual helper/caller interfaces and SDK owns component tests. Production Parent policy remains Serialized.

## Frozen source

- `outputs/anadius-owned-environment-source/environment.hpp` SHA256 `f45705884f794a56e11d511601d187d2111dab1dbc3ead78caf3210f3c4747fa`.
- `outputs/anadius-owned-environment-source/environment.cpp` SHA256 `332027af5206b7ba6aa4bd72e59ffa087a539d4d341608bba6b4cf1481de2968`.

## Native constraints

Fresh complete REA6.8.0/Ghidra12.1.4 replies bind to original anadius64.dll SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`:

- `instructions-1800229d0.json`: send scheduling body1800229d0..180022c3a,619bytes, Evidence `ev_c3016d57cf23af5fa783396940f99c700701bbb46e18247c1af6ebb1aae50d11`. Executor40 caches at180022a02 BEFORE source query180022a0e; the cached function executes180022a1d. The following native move transfers header/payload/state and only then loads the live executor180022a5e. The moved-from temporary's live destroy target is compared with017a30 at180022a82, and only the non-guard target calls180022a8c.
- `instructions-180027710.json`: receive completion body180027710..180027a15,774bytes, Evidence `ev_6ff5df9f920a7b81c7935c726699b51d98c9cb814b0887bf86d738158d86a902`. The nested prepare path caches40 at1800278f0, queries180027900, then invokes the cached function18002790f before move/guard cleanup.

Static field/callback order is distinct from actual runtime overlap, native C++ ABI/EH equivalence or new function-completion percentage. Root retains the complete provider replies and their limitations.

## Implemented bounded contract

OwnedTable's optional HOST try_prepare_environment_40 hook and JobScope's stable JobState-owned Preparation facade route to ONE Control::prepare_whole implementation. JobScope::prepare_40 invokes the SAME actual prepare_io_environment_40 helper with that explicit synchronous facade. No provider pointer is added to IoCompletionEnvironment, HandlerTaskContext, posted task, or other deferred context, and no scheduling authority is granted.

The explicit scope path validates actor/membership and canonical sentinels before even the helper temporary accessor. Root/Scope whole preparation reserves FULL source read plus complete destination and actual helper temporary write addresses in one registered Effect BEFORE the first checkpoint, executor cache, query or native prefix. The actual body temporary already exists before admission and its lifetime extends through owning-hook final admission. Destination/temporary old payload/state/executor are not consulted as native initialization evidence.

A private stack Calls adapter binds the exact outer Effect and exact source/output/temporary identities. It requires a single source query, passes that exact returned source capability to one canonical cached Control::prepare function, constructs ONLY the actual body temporary with the shared Transform::Prepare algorithm, moves ONLY that temporary into the exact output through the existing native023cb0 helper, and allows the conditional guard cleanup ONLY on that moved-from temporary. It performs no raw Root callback recursion, second effect, source owner re-resolution, ambient current-operation lookup or temporary context swap.

Prepare preserves the actual shared family and clears flag MASK0x1 (bit0). A tracked result receives one genuine family work retain at the existing publication point; the native move transfers that existing pin/capability and introduces no extra retain/decrement. No fake environment row, source root, work counter or readiness/unload barrier is created.

After the actual helper body finishes, the owning hook requires the exact output reference and completed query/construct/move sequence. It first verifies temporary OWNERSHIP metadata is empty, then checks exact source/output capabilities, full registered fields and the live moved-from canonical empty temporary. Normal Effect final admission still holds all three address claims; its registered source/output fields are checked before retirement. The nonmutating checkpoint contract remains necessary: no extra authority is claimed over arbitrary raw writes to an unowned canonical temporary during the final callback.

All helper/body/SDK/checkpoint effects, allocation, exception construction and private adapter/record retirement occur outside metadata gates. Scalar checks/splices remain under metadata. No Body/Calls/scratch stack references or function closure are retained in a durable Effect. On unknown unwind the registered failed Effect retains only exact address/prefix/source metadata; phase/failed guards prevent later dereference/replay of an ended helper stack temporary. Existing per-row source, lineage, provider lease, known-caller rejection and shared sticky-unknown policies remain.

Raw exported prepare40 stays original-worker-only. Default unowned providers keep their actual cache-before-query/cached-call/move/live-guard ordering and failure prefixes. Existing scoped constructor/copy, actual Root copy caller ownership, empty result ownership, Runtime/Policy stop semantics and handler/response scheduling remain unchanged.

## Evidence limits and next work

Source review covered explicit scope precedence before source reads, pre-checkpoint three-address reservation, exact cached function/query/output/temp sequencing, true family/work transfer, actual helper temporary lifetime and unknown replay denial. Component tests are separately authored and statically reviewed, NOT run.

Actual Prepared two-scope tracked-family progress, B physical exit/reap while A is held after a real decrement, and a naturally established child-old1 Runtime release remain OPEN. A separate read-only plan identifies the missing typed selected-provider ownership injection into Profile; it is not implemented in this stage. No shadow Environment, fake JobPermit, replaced Runtime Ports, manual scheduler reset or test getter substitutes for those fixtures. Child handler/response publication, original028d30 response/021770 tail, general shared lower mutation and native runtime/EH/ABI equivalence remain separate open work. Original binaries and historical build/link/ZIP proofs remain unchanged.
