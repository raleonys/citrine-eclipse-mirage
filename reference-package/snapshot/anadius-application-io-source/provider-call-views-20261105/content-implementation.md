# Content invocation views — source implementation

Status: source-only candidate; no compilation, project tests, linking, DLL execution or packaging. Before copies are the stage's `before/anadius-content-effects-source__*` files. This stage integrates the retained parallel-message contract; it adds no new native audit and enables no Graph/NativeJobs concurrency.

`Content::make_call_view(const application_io::CallContext&)` returns a narrow `Effects` + `AvatarRequest` facade, without mutable `CheckedCalls` binding. Each view owns a separate existing persistence Engine, guarded file factory, immutable context and copied Options. The actual Runtime/configuration, Paths and Online remain the same borrowed roots. Their lifetimes and lower provider thread-safety remain caller obligations; this does not clone or serialize business roots.

An intrusive stack admission is registered before metadata allocation. A prepared list node and execution are allocated outside the owner's mutex, published before checkpoint/provider admission, and remain published through wrapper allocation. The mutex protects only health, registry and phase fields. No provider, checkpoint, native allocation or file operation runs under it. Distinct actors and ordinary original-worker helpers may progress independently. Same-actor active execution, preparation, close or legacy-child reentry is sticky; an already-created idle view cannot bypass it. Foreign/expired copied contexts are rejected before a checkpoint and do not poison healthy calls.

Normal `close()` runs under the exact live context, verifies empty/known guarded files, retires copied lower owners outside the mutex while the view remains published, then removes only its execution. Closed handles may subsequently be destroyed after the Parent Operation and original Content facade end. The wrapper retains owner metadata separately; registry-owned executions have only a nonowning owner pointer, avoiding an owner/execution strong-reference cycle. Unknown prefixes remain in the owner registry; wrapper destruction marks health unknown and never guesses file cleanup. Creator Content retirement still refuses outstanding executions/admissions and retains its original worker/fiber rule.

Core source coverage adds actual Prepared/NativeThreads normal view writes through existing XML printers and counted files, UTC, foreign-context denial, stale-context denial before stale checkpoint access, closed-handle rejection and sibling reuse. New isolated expected exits 175/176 retain actual adopted raw files after initial-state/open-return admission loss; 177 checks swallowed cross-idle-view reentry stops UTC before gmtime. Existing normal cases and isolated exits 171–174 are retained. The separate SDK handoff fixture covers two actual simultaneous actors; these checks are authored, NOT RUN. Before/current `git diff --no-index --check` reported no whitespace errors (line-ending notices only).

Final candidate SHA-256:

| Source | SHA-256 |
|---|---|
| `outputs/anadius-content-effects-source/effects.hpp` | `5904d4c70e8910b63f10be9215076680300ab0089990e8cccf1f028690f24a91` |
| `outputs/anadius-content-effects-source/effects.cpp` | `58b482063f9fa7d4263de0f6f21aa33cc5d5983b38ac2f96dc4b287cbe4b1812` |
| `outputs/anadius-content-effects-source/effects_tests.cpp` | `a7e1a97e13540d85de7798e97bbfbb102c88899e2169fc8b1633d3e018da521a` |
