# Next Licensing Handler/Core call-view plan

Read-only design after the provider stage froze. No existing source/report/evidence was changed; no compiler, project test, linker, executable, DLL, ZIP or REA operation ran.

## Selected source obstacles

`Core::Impl` owns one job/checkpoint/busy/checking tuple, one GuardedFiles and all encoding/clock/hash/token adapters (effects.cpp18–93). `Handlers::Impl` owns another such tuple, one semantic `license_recovered::Handlers`, and reciprocal raw Core/Handler check targets (152–201). Its current with_job reserves both root objects for one synchronous child (214–232). New call views must not install either root's job/checkpoint or change those shared raw reciprocal targets to the current child.

The semantic executor writes its own `used_` in all three request methods (license.cpp39,57,124). That is HOST authentication-binding bookkeeping, not a native global. Sharing that executor concurrently introduces a C++ race independently of the true business cells. The executor therefore belongs to the call, while its StateReferences remain references to the actual same roots.

## Concrete additive API

In existing Licensing effects.hpp/.cpp, add:

```cpp
class Core::CallView final : public license_recovered::Effects,
    public license_recovered::OwnedAuthEffects,
    public messages_recovered::UnknownMessageEffects {
  // All existing Effects/OwnedAuth/UnknownMessage virtual methods.
  void close();
};
std::shared_ptr<Core::CallView>
Core::make_call_view(const application_io::CallContext&);

class Handlers::CallView final : public dispatch_recovered::LicenseHandlers,
    public messages_recovered::UnknownMessageEffects {
  bool auth_code_02e3b0(const XmlNode&, Xml&) override;
  bool request_license_0320c0(const XmlNode&, Xml&) override;
  bool invalidate_license_030c50(const XmlNode&, Xml&) override;
  void unknown_message_07bd70(const UnknownMessageInput&) override;
  void close();
};
std::shared_ptr<Handlers::CallView>
Handlers::make_call_view(const application_io::CallContext&);
```

No CheckedCalls binding is needed on a view: its context is immutable. Existing default/with_job/HOST methods and public root binding stay intact. A public Handler-to-Core accessor is unnecessary: actual semantic license requests and diagnostics already exercise the private Core, while the standalone Core factory covers the existing direct Core API. Avoid another general provider interface/module.

## One exact context and bounded owner state

Reuse final `application_io::CallContext` SHA256 `2d2fd2f4bfc1b4abaad932a88b93dd1ad9f03240ce3cc5561b252d733ab096d4`. The factory first validates the exact live actor/permit without caller callback, then publishes a nonallocating stack Admission before any metadata allocation or caller checkpoint. Preparing rows protect later context/state/wrapper allocations from same-actor new-handler reentry.

Create **one stable immutable Context object per factory call**, e.g. `shared_ptr<const CallContext>` copied once after admission. A Handler view and its private Core execution share that same object by identity, not two independent context copies/checkpoint bindings. Context owns only the copied permit and borrowed callback, never a strong Graph MessageCall; caller retains raw checkpoint backing through close. A standalone Core view similarly has one context.

Use the completed Information/Content pattern: stable root-owned registry rows, view wrapper pins the creator backing, rows link back nonowningly, short gates only, shared sticky failed-owner health, explicit close and closed metadata-only destruction. A small private `OwnerControl` holding a metadata mutex/health/retirement state can be shared by the root Handler and its root Core; this is the same owner, not a global pool or business lock. Keep execution busy/checking flags per root or per view. Root/Core/Handler metadata methods must release that gate before invoking another check, target or provider.

A Handler row owns its private Core execution, a lightweight semantic executor and Node/Xml guarded adapters. The private Core owns its own GuardedFiles, encoding/clock/formatter/text/hash/stats/token adapters and shared lower Option captures. They reference the same actual Runtime/configuration/Online/AttemptState and pre-existing lower targets. Do not construct/load another Online, copy configuration, reset attempts or synthesize auth/token values. Root creator ownership remains live until all rows normally close; child retirement must not last-destroy creator-affine root backing. No strong registry-to-owner cycle.

Reciprocal validation remains deliberately asymmetric: Handler `admit` may invoke the **same row's** Core check; Core's reciprocal target performs Handler metadata/actor/health validation only, never Handler `admit`/Core again. A known parent Handler busy/checking state is legitimate during that nested Core check. Another row/idle facade must not exploit that allowance: same-actor cross-row reentry, factory allocation reentry and legacy with_job overlap reject sticky; different actor healthy calls do not wait on a provider-wide body guard.

## Authentication and binding history

Store the root's actual `const AuthenticationCode*` when its existing one-time binding successfully installs the persistent root semantic executor. Preserve that root executor/capture across every view close. The bound source cell and its value are borrowed immutable Runtime identities, not recreated equivalent bytes.

On view creation capture the root auth identity under the metadata gate. If already bound, construct the view's semantic executor with the SAME StateReferences and bind it once to that SAME source AuthenticationCode. Its local used_ then cannot race another executor. Root rebind still rejects because the persistent root executor/capture remains bound; no shared invented native used-bit is needed.

Preserve the existing unbound-diagnostic contract: effects_tests.cpp90–104 deliberately reports an unknown message without auth, then performs its initial bind. An unbound diagnostic view may exist, but contains no semantic executor; its three semantic methods reject just as unbound root methods do. Creating/using only that diagnostic must not fabricate successful semantic use or permanently consume first-binding permission. Bind excludes open/preparing view snapshots without waiting, and remains possible after an unbound diagnostic normally closes. This is creator lifecycle admission, not a new message strand. A published unbound view must not silently become semantically authorized by a later root binding.

## Effects, failure prefixes and normal close

Source SDK/file bodies remain outside all metadata gates and admission-error restorers. Final CallContext preserves only checkpoint metadata LastError, as in the completed provider stage. Keep owned Online auth's actual object/code reservation through its C-string copy. Preserve unknown diagnostic counted logging, separate resampling and owned dialog text; log locks end before the dialog so B can progress while A is held.

Known NoResponse is accepted only around the actual selected semantic/control source body. A marker from checkpoint/Node/Xml/file/SDK/postcheck remains unknown and poisons shared owner health. Do not catch a final-admission marker as a fresh business result, repeat completion or run cleanup twice. Handler and nested Core see the same failure state; a failed private Core must not leave the Handler/root healthy.

Close seals the entire row, validates the shared context, rejects pending raw-file owners, retires per-call semantic/adapters/Option captures outside the gate while still published, then performs final context/health checks and unlinks without allocation. If any target outcome becomes unknown, retain that exact partial row and raw owners; wrapper destruction neither calls stale raw checkpoint data nor guesses close/destruction. Creator root destruction remains on its existing actor and rejects active/pending/failed rows. Closed wrappers can later destroy empty metadata only.

## True shared mutation obligation

Per-view executors must NOT copy/reset:

- AttemptState attempted_files/blocked: request0320c0 reads/updates them across actual file callbacks (license.cpp124–155).
- configuration.denuvo_token: request load/store and invalidate030c50 clear the SAME string (57–60,145–152).
- game_attributes: ticket_text uses operator[] for keys at083960/083978, which may insert missing keys (79,81).

These are shared business roots. A facade registry does not make arbitrary concurrent mutations of C++ strings/maps/scalars defined, and retained native lack of a handler-level lock is not a proof that all such races are safe. Audit and project individual native-consistent shared accesses before full Original enablement; do not hide the issue with independent copies, a whole-license-request mutex or a new readiness barrier. The concrete UnknownMessage versus bound-auth/Core-UTC/GetConfig overlap avoids those mutating branches; a controlled single B token request while A only reports a diagnostic can additionally exercise file/AES ownership without falsely claiming two simultaneous token writers are solved.

## Minimal coherent authored group and meaningful fixture

Production: **Licensing effects.hpp and effects.cpp only**; include the existing shared CallContext header, no new translation unit or semantic business-algorithm rewrite. Tests: existing effects_tests.cpp plus worker-handoff-source/handoff_tests.cpp. Root snapshots/GO must precede edits.

Reuse existing Licensing real Domain/NativeThreads/PTD/FLS/PreparedMemory harness and existing own-EXE Online binding fixture. Cover bound cell identity and root rebind denial after normal view use; unbound diagnostic then initial bind after close; actual auth fallback/owned Online result; normal source NoResponse versus same-type callback failure; root/shared poison; wrong/stale context; same-actor cross-idle view/legacy reentry; raw input/output prefix retention and later metadata-only destruction. Preserve existing isolated181–184 cases and HOST/root tests.

In a real parallel fixture A's actual Licensing UnknownMessage dialog/owned provider effect stays held after log release. B on another actual child uses another view of the SAME Handler/Core owner, real bound auth and an actual Core leaf; optionally use a single controlled actual token read/decoding through separate per-view GuardedFiles with shared AttemptState, not a synthetic handler answer. B must close/return/join/reap outside the original Operation before A release. Original may use nonmutating root metadata/Core helpers while both views remain healthy. An isolated unknown-file variant must retain raw prefix and shared owner poison after real child exit, with no fabricated cleanup. This remains authored-source validation until a user explicitly authorizes execution.

## Exact inspected authority

| Source | SHA256 |
| --- | --- |
| Licensing effects.hpp | `7a84578546cecb74312818a094cdccdabbb641ab67f0c63787e7bfdf56684d07` |
| Licensing effects.cpp | `c66cf34ed9b2e23772c48e834810e5afca0200083dd14b30e0eb508547e7e5d9` |
| Licensing effects_tests.cpp | `2ff568a77527f25a9630e310dd31fd9eb91aa7416dac2c854fc7dad3e471b9e3` |
| Semantic license.hpp | `21da0faefca86f6c3ea18a49fc0624be8f7f636bce72b528e8577036d31684ec` |
| Semantic license.cpp | `87ea1eebab23fa2bbaff52689e4a076428e5cd8f292e028fbba985d4ce4376c0` |
| auth_state.hpp | `713f25d7dc4af5e59d05364bec8a9573a90cdb3e7976df39906e773d373f56b5` |
