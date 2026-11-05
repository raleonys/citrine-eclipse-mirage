# Complete normal84530 diagnostic design

Read-only design, 2026-11-04. No production edits, new REA activity, compilation, tests, linking, DLL execution or packaging. This supersedes the earlier uncertainty about the selected compiler system-message provider, using the newly retained actual archive-member evidence. It does not authorize implementation by itself.

## Recommendation

Implement an imports-owned, shared native system-category provider and a small projected std::system_error subtype that retains that provider. Use it in both actual **nonzero system-error84530 stop raises**. The standard constructor then performs its already-proved prefix/separator composition once while resolving the supplied category's message to the existing concrete020950 adapter.

This intentionally migrates the thrown system error's code().category() from compiler system_category to a stable per-selected-provider category named asio.system. Keep std::system_error catch compatibility and the signed code value. Update the existing HOST category-address assertions explicitly; they are implementation contracts being migrated with native provenance, not evidence that compiler system_category equals native asio.system.

Do not globally replace compiler_category(1), modify write84720 or generic terminal routes, or create a category per error. The category identity is created once per selected Policy/provider and is shared across its original/child stop errors and copies.

## Evidence now settles the normal value route

The current selected libc++.a has SHA7951a18341870b622e67bea17bed9cd953d827cc7fbd0b521f01944f1ac7d22c; its system_error.cpp.obj SHA31dfd7fc526dc8106710319b9c66cc4bf89bfac99c1614e254a774a9559452d7. `compiler-provider-review/findings.md` retains the full local primary-artifact interpretation and its historical-path/current-byte distinction.

The compiler default message is now demonstrably not native020950-equivalent: language0 versus400, repeated LF/CR/space trim versus oneLF then oneCR, remaining-count copy versus first-NUL copy, Unknown error versus asio.system error. This is a static value-domain mismatch, not an executed current OS result.

For the selected **nonzero** error, that same actual compiler member's make_error_str composes nonempty context, `: `, and category.message exactly once without numeric/name suffix. Its runtime_error::what returns the stored pointer. Native0248b0/00a270 constructs the same value sequence with literal pqcs and supplied message. Therefore supplying the concrete020950 category message addresses the remaining normal visible diagnostic mismatch without a custom what() override or duplicate preformatted string.

The compiler make_error_str skips message for code0, while native00a270's observed body invokes the supplied message slot without that value test. Actual02a190 calls028240 only for a nonzero captured status, so the proposed composition matches that selected caller. This does not complete arbitrary028240/code0 calls; retain that boundary explicitly.

## One provider, immutable imports, retained state

Suggested concrete object in socket native.hpp/.cpp:

```
class NativeSystemCategory final : public std::error_category {
    const SystemMessageImports imports_;
    mutable SystemMessageState state_;
    // Small metadata-only overlap guard; no gate held through SDK/callbacks.
public:
    const char* name() const noexcept override; // "asio.system"
    std::string message(int code) const override;
    std::string message_checked(int code,
        const std::function<void()>& borrowed_checkpoint) const;
};
```

Create one shared owner passively when Policy's immutable options are adopted. Validate both imports; constructing it must call no SDK. `message()` delegates to message_checked with an empty checkpoint, and message_checked calls existing system_message020950 with the provider's retained state/imports. Ordinary Policy logging can use message_checked with its current admission checkpoint, replacing its standalone SystemMessageState. This keeps the same selected injected FormatMessage/LocalFree provider and retained prefix for logs and terminal construction.

The callback is synchronous/borrowed and never stored. Do not capture Policy, Parent, JobPermit, TLS, a current constructor, or a global actor inside the category. Such captures either form lifetime cycles, become stale after escape, or make code().message unexpectedly require an already-poisoned Policy.

Actual production original/child constructors are serialized by the same Parent Operation and source admission. The provider still needs C++-defined overlap handling because an escaped exception can expose the category on another actor: a metadata-only atomic/mutex guard can reject overlap/reentry before touching state, releasing the guard on exit while leaving failed state intact. Never hold a nonrecursive mutex through FormatMessage, allocation, LocalFree or a supplied checkpoint. This is a HOST overlap bound, not proof of native concurrent category semantics, a new unload condition or a reason to close the reconstruction goal.

Native default_error_condition/equivalent virtual semantics were not recovered. Inheriting the standard base behavior is a named HOST projection for now; do not claim native error-condition comparisons merely because message/name match. Concurrent post-escape category.message semantics also remain open beyond the bounded guard; actual normal construction does not need a new scheduler/worker framework.

## Retained SDK failure prefixes

Use the accepted020950 helper directly; do not reimplement its language, trim or fallback in error_category. Its state records the allocated pointer and entry/release phase in the shared provider, not an ephemeral stack variable inside message(). A throwing SDK, string allocation or checked postcondition therefore leaves the prefix reachable through the provider owner.

Do not add an SDK-buffer RAII deleter, automatic retry, phase reset after failure, or guessed native EH cleanup. A failed Policy terminal continues to retain its poisoned actual graph/provider; no new Policy close/unload precondition is needed. Normal completion clears the state only after the helper's final check as already reviewed.

The lifetime statement is bounded: shared ownership preserves state while an owner exists. It does not prove what native EH does, nor guarantee an unknown failed SDK allocation is reclaimed if an independent escaped caller later destroys the last provider owner. That exceptional cleanup remains explicit open work; it must not be hidden by a destructor free or a permanent per-error registry.

## Projected exception and correct base-construction lifetime

Use a small final HOST exception, e.g. NativeSystemError, deriving std::system_error and retaining `shared_ptr<const NativeSystemCategory>`:

```
NativeSystemError(int code,
                  std::shared_ptr<const NativeSystemCategory> owner,
                  const char* context)
  : std::system_error(code, require_nonnull(owner), context),
    owner_(std::move(owner)) {}
```

The category parameter must be passed **by value**. It pins the category throughout base construction, before the owner_ member exists. If category.message throws, that parameter and Policy's own owner keep retained SDK state alive while unwinding. A null owner is rejected before dereference by require_nonnull. Do not construct a reference to a temporary category, acquire shared_from_this during construction, or assume a derived member is initialized before its base.

For nonzero code and context pqcs, actual compiler construction calls this category.message once, stores `pqcs: ` plus020950's text, and stores code/category. Inherited what() already returns that diagnostic; no override or second FormatMessage call is needed. No per-error category identity/cache or TLS is involved.

Explicit defaulted projected copy construction copies std::system_error's stored text/code and the shared owner; it must not call message again. The local compiler header declares the base copy constructor noexcept, and shared_ptr copy is noexcept. Expose a const shared-owner accessor if callers need to retain category lifetime alongside a deliberately extracted error_code. Assignment/move behavior should be stated/tested rather than accidentally dropping the owner of a still-referenced category.

### Base slicing and extracted error_code are borrowed

A copied std::error_code contains only a category reference. Copying/slicing just the std::system_error base also does not copy the derived shared owner. The projected type cannot retroactively make those standard values owning without an immortal registry or forbidden identity hacks.

Therefore guarantee lifetime for the actual projected exception, its projected copies and retained provider; document that independently extracted/sliced values need the provider owner to outlive them, as for any dynamic error_category. Actual selected terminal paths poison/retain their Policy and catch by reference; no inspected production base-value escape was found. This is an explicit C++ borrowing boundary, not a native unload proof. Do not claim arbitrary std::error_code/base-slice persistence is automatically owned.

## Policy integration and intentional poison

Add the shared native category/provider to Policy::Impl using Options.system_message once. Keep the same logical Policy category tokens used by NativeApi and logging; the new thrown error_code category is the typed HOST projection of native asio.system.

For original84530 raise: preserve its existing source/terminal admission and poison sequence, validate context84530 and error category index. For actual system index1, construct/throw NativeSystemError with the retained provider and pqcs. Generic/misc explicit HOST inputs retain their existing compiler-category path and pqcs context until their native message providers are separately selected. Leave84720/write unchanged.

For tracked finish raise: preserve call.admit, actual context/category validation, already-consumed work and stop/wake flags, and poison sequence. The system branch throws the same projected error/provider. No deferred receipt or second work decrement is added.

Category.message is a self-contained FormatMessage/LocalFree/string value leaf; it does not need Parent storage or ordinary Policy access. It must not call the stored ordinary admission checkpoint after intentional poison, and must not reopen Bound state to obtain formatting permission. The caller already retains its exact terminal Parent/actor/JobPermit authority and provider while making this final leaf call. A construction failure propagates through existing poisoned-prefix handling. The leaf's independence also permits later sequential code().message use without borrowing a dead Policy checkpoint.

This is a bounded diagnostic-value integration, not a new permission to call other Policy effects after poison. If SDK reentry corrupts broader source authority, existing outer terminal/error retention applies; full arbitrary native callback concurrency/exception cleanup remains unclaimed.

## Production/test scope

Likely source files: socket-source/native.hpp/.cpp for the provider/projected error, session-policy-source/policy.hpp/.cpp for ownership/logging/system84530 selection. Keep interfaces narrow and passively construct the owner. No loader, Runtime work-stop, receipt, Parent allocation, CRT/EH framework or unload-gate change is required.

Source cases should prove original and tracked system errors catchable as const std::system_error&, signed value preserved, category name asio.system and stable shared identity per provider, exact injected020950 message once, pqcs separator once/no compiler fallback suffix, copy lifetime after external provider references are released, no SDK replay on exception copies, shared log/terminal imports, fallback/trimming/NUL behavior and retained failure phases. Existing generic/write expectations remain compiler-backed. Category identity assertions change deliberately for selected system stop errors with this native provenance.

No test was executed. Normal84530 diagnostic value could be closed after coherent implementation, independent review and separately authorized validation; native EH/ABI, code0 generic constructor behavior, default-condition/equivalence, arbitrary slicing lifetime and post-escape concurrency remain explicitly separate open boundaries. Full reconstruction objective remains active.
