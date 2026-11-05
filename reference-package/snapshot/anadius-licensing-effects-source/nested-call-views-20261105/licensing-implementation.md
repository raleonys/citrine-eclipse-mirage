# Licensing nested call views — source implementation

Only `outputs/anadius-licensing-effects-source/effects.hpp` and `effects.cpp` were authored. No compiler, project test, linker, executable, DLL, ZIP or REA operation ran. Root owns fresh native evidence and test source authorship; production Graph/Profile admission remains unchanged.

| Source | Root-captured before SHA256 | Final candidate SHA256 |
| --- | --- | --- |
| effects.hpp | `7a84578546cecb74312818a094cdccdabbb641ab67f0c63787e7bfdf56684d07` | `0557bdf684023afdfd8113edab54a5c0547c2f8134ab0095be7cfa48c6d2b183` |
| effects.cpp | `c66cf34ed9b2e23772c48e834810e5afca0200083dd14b30e0eb508547e7e5d9` | `630608decc1362d15b4775e658f523acb1f5f1cf083f47865b6fdde9278c8a54` |

Before copies are in this stage's root-captured `before` snapshot, taken before GO. This report freezes the author's source candidate; any concrete review correction requires a successor/addendum rather than rewriting a recorded proof.

## Additive interfaces and actual roots

`Core::make_call_view(const application_io::CallContext&)` returns a narrow shared Core::CallView implementing the full existing Effects, OwnedAuthEffects and UnknownMessageEffects virtual surfaces. `Handlers::make_call_view` returns LicenseHandlers+UnknownMessageEffects. Both have explicit `void close()` and noncopyable handles. Existing root methods/with_job/checked-call and authentication signatures remain.

Each registry row owns a private execution Impl. It uses the SAME actual Runtime/configuration/Online/AttemptState and shared lower provider targets; per-call Options are shared ownership captures, not business-state copies. Core has a private GuardedFiles plus clock/format/encoding/hash/stats/token adapters. Handler owns a private Core execution and a lightweight semantic license::Handlers whose local used_ does not race another executor. Existing actual semantic/file/hash/AES/Online algorithms are reused through these private executions rather than replaced by synthetic success.

The factory copies one stable immutable `shared_ptr<const CallContext>` after stack admission. Nested Handler and private Core receive that **same object**. Core's reciprocal target calls only its own Handler row's metadata/permit check; Handler admission may call that private Core check, but there is no Core→Handler.admit→Core recursion. No root current-job/checkpoint/reciprocal target is redirected to a child view. No strong Graph MessageCall capture is introduced; raw checkpoint backing remains the caller's obligation through close.

## Health, lifetime and admission

Handler and its existing root Core share one sticky atomic OwnerHealth. Unknown private execution, swallowed same-actor reentry and denied postchecks poison both layers and their root/default routes. Healthy activity alone does not fail the owner or reserve a whole-handler mutex.

Owner registries strongly retain stable Impl rows; each row's owner link is nonowning. Public wrappers pin the creator backing and a small private execution facade. There is no registry-owner strong cycle. The private facade's destructor is metadata-only: an unclosed wrapper first marks shared failure, and the root registry still owns the actual Impl/file prefix. Thus wrapper/control-block allocation failure or a dropped unclosed handle cannot run speculative file cleanup. On successful close only empty metadata remains for later-actor wrapper destruction. Creator root destruction rejects pending/open/failed rows and retires real lower captures on its historical actor.

A nonallocating exact actor Admission is linked before metadata allocation or caller callbacks. Stable preparing rows cover subsequent wrapper allocation. Candidate nodes/context/executions allocate outside owner gates, then splice and recheck. View active/checking/closing reservations are per execution, excluding same-actor cross-row/idle-handle reentry while allowing disjoint actors and healthy original methods. The legacy Core and Handler with_job pre-action reservations also inspect same-actor view activity, so a caller action cannot bypass an active view before reaching another effect.

No new metadata gate spans SDK, file, stream/log lock, Online query/copy, allocator, checkpoint or target retirement. New errors are constructed after unlocking. Added metadata scopes preserve source LastError; the final shared CallContext preserves admission-checkpoint metadata, while actual SDK leaves remain outside that restorer.

## Authentication history and control flow

Successful root binding stores its exact pre-existing AuthenticationCode pointer alongside the persistent root semantic executor. A bound view constructs its private semantic executor using the same source cell and actual StateReferences. Closing views never resets the root optional executor/auth capture, so root rebind remains denied without inventing a shared native used-bit.

An unbound diagnostic view contains no semantic executor. Auth/request/invalidate preconditions reject without poisoning it or manufacturing a fallback, while the existing unknown diagnostic remains usable. After that view normally closes, the original creator may perform its FIRST root binding. Root binding excludes preparing/open view snapshots without waiting, and no published unbound view acquires later semantic authority implicitly.

Known NoResponse is handled only around the actual selected source body. Opaque Node/Xml/SDK/file/checkpoint same-type markers poison independently; source completion/postchecks are never retried or reclassified. Logging/request resampling/dialog ownership reuse the existing body, whose log leases end before the dialog. Nested Handler/Core use one exact admission throughout.

## Normal close and retained prefixes

Close first validates the still-live captured actor/context, seals its row and checks shared health. It rejects pending raw files. Core retires private Option captures while registered and then revalidates before marking closed/unlinking. Handler first retires its private semantic executor, seals/retires private Core captures and private AttemptState ownership, checks the same reciprocal context, then closes both nested metadata prefixes and unlinks its own row. Target retirement and list-node deallocation happen outside metadata gates.

First unknown failure retains the actual partial row; no guessed retry/close/free or admission replay is added. Registry ownership preserves private Core/Handler callback backlinks for that retained row. Root authentication, Runtime, business roots and provider lifetimes are not inferred from successful child exit or empty Parent allocations.

## Validation and remaining integration

Manually traced all full Core forwarding methods, per-call Handler semantic/Node/Xml routes, exact context pointer propagation, reciprocal recursion boundary, authentication history, shared-health legacy reads, metadata allocation/retirement and unknown-prefix ownership. Peer review identified the legacy with_job pre-action bypass, which the final candidate closes for both layers. Root authors existing prepared unit extensions and the SDK agent authors actual two-child fixtures; no runtime passing claim is made.

Actual attempted_files/blocked, denuvo_token string and ticket_text game_attributes operator[] stay shared and mutable. This facade foundation does NOT make arbitrary simultaneous token/string/map mutations defined; those specific accesses still require audit before full Original enablement. No copies, token reset, whole-license-request lock, readiness barrier, new unload condition or broad CRT/EH framework was added. This stage is a prerequisite, not completion of coupled Graph/Profile/NativeJobs concurrency or native C++ ABI fidelity.
