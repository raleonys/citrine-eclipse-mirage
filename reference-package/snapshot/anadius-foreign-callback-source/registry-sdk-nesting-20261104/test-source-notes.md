# SDK-phase registry nesting test sources

Status: source additions only, author statically reviewed; **not compiled or executed**. No project tests, link, DLL execution or packaging occurred. Root owns fresh REA evidence and independent integration review.

## Core source scenario

`foreign_tests.cpp` adds `registry_sdk_frames()` and calls it from the normal main. It selects the actual Original-profile Domain and Admission with a controlled hook proof, enters one real root CallbackThreads scope, and checks:

- No nested admission outside a typed SDK interval; nonregistry child/SDK selection declines.
- Same-slot child body retains exact SDK parent and root PTD; its publication proof runs under Checking with body/original permission masked.
- An ancestor SDK marker cannot admit another body while the exact child body is active, even when the callback enum equals its parent's.
- A child SDK marker binds that child identity and allows a distinct third-level query body; exact leaves restore the child and then root.
- Another thread still observes ordinary Overlap; known missing selected hook publication unwinds the child reservation cleanly.
- Begin/end metadata preserves WinLastError. Pure-forward and connect SDK parents do not select this general-registry nesting route.

The core scenario uses public automatic tokens as a controlled source fixture. It does not claim actual Windows nested calls or arbitrary concurrent set access.

## Typed integration source scenario

`foreign_integration_tests.cpp` adds `registry_sdk_nesting()` and calls it from the normal main. It uses the existing observed-policy fixture pattern: concrete callback Policy, actual native_context Owner and auto-bound Admission, exact root port binding, managed stream, actual per-slot Publication plus an observing forwarding proxy. Four Domain scope closures are explicitly bound, as in the pre-existing concurrent-redirect fixture. The outer `fc::make_factory` convenience wrapper is not itself called in this observed branch.

A foreign thread runs nonnull unrelated openA -> recursive same-slot nonnull openA -> queryA -> queryW -> real close through the actual typed native entries and OriginalEffects. Controlled SDK stubs inspect raw arguments/outputs, incoming WinLastError, actual native Context lease counts1..5, Full/original permission and one identical non-sentinel root PTD. Source checks validate return-bit/output propagation and exact parent permissions after every child. Root alone retires the acquired PTD; tracked raw allocation membership returns to its pre-call count.

The original-provider proxy probes that resolution cannot inherit an ancestor SDK interval. A controlled encoding provider probes the same denial while the child general body executes comparisons after its SDK has returned. These denied probes use Admission directly, avoiding accidental terminal calls through noexcept native wrappers. A joined second actor checks ordinary Overlap during the SDK chain. A direct call to the captured concrete OriginalEffects under an explicit root admission throws a named synthetic lower exception; the source requires that same exception and ordinary parent permission restoration with nesting denied afterward.

Outgoing WinLastError after a complete nonnull general body is deliberately not asserted against a guessed constant: later comparison/encoding/CRT effects own that state. Exact incoming SDK error and frame-metadata preservation are checked in their respective controlled source scopes.

No full prepared real Windows host/loader invocation, real SDK reentry, arbitrary general concurrency, native EH ABI or new-frame tamper-terminal validation is claimed. Test source additions are not a whole-DLL completion increase.

## Final author hashes

- `foreign_tests.cpp`: `4e637b28b56f3fa012728a46376e7b769c76b038fd9484433af2e21153708f26`
- `foreign_integration_tests.cpp`: `dbde68ae4021e68881b2601b0d5234a3c49f1f3a362715cb4f416e513192e761`

Pre-edit copies are retained as `foreign_tests.cpp.before.txt` and `foreign_integration_tests.cpp.before.txt`. Any subsequent root-review edits need their own final hash record rather than rewriting this author note.
