# Early registry publication integration source — 2026-11-04

Status: TEST SOURCE ONLY; not compiled, linked or executed. No production source change.

Added `original_registry_during_enable()` to existing `outputs/anadius-resources-factory-source/factory_tests.cpp`. It reuses the existing initialized Domain/State/HookEngine fixtures and actual `install_host` OriginalCrt selection. The existing b15 Driver supplies stable Backend state and an Execution bound to the actual assembly Milestones; this is not full Runtime/DllMain execution.

The actual matched callback attachment is published and setup begun. Actual `initialize_host_00e310` calls the assembled HostPlatform and owned engine. During the second lower enable (RegOpenKeyExA), the fixture sequentially invokes the typed reconstructed `reg_open_a_entry` on the controller and one joined foreign thread. A nonnull, mismatching `SOFTWARE\\Own\\Unrelated` subkey prevents the argument-only shortcut and reaches the ordinary registry comparison body. The synthetic typed original verifies Full callback permission, original permission, one native Context lease, exact arguments and incoming error state. Calls must return the distinct original outputs and release their native/admission leases.

The controller has the Domain's actual existing PTD before invocation and retains it afterward. The foreign thread starts with an empty FLS slot; the typed original must see a distinct valid private PTD, and callback return must clear that slot. The two calls are sequential; no general callback concurrency, nested registry call or Windows SDK reentry is asserted.

At enable, only connect and openA original slots are populated; all later original cells remain zero, callbacks->ready() is false, and no Milestone is published. Successful typed original dispatch necessarily passes actual per-hook publication. This observes the actual slot prefix and dispatch behavior; it does not directly read the private Publication mask or snapshot.

Normal setup completion remains explicit and later. Existing pause, actual hook cleanup and attachment close retire the fixture. The synthetic engine only records hook operations and returns owned test function addresses; it does not patch Windows code. No original DLL, reconstructed production DLL, ZIP, build proof or canonical build output was modified.

Test fixture-only HookEngine now has an originals array initialized to the same b15 function addresses it used before. Only this scenario overrides openA with its own typed observer. All other scenarios retain the existing default fake originals.

Pre-edit test bytes are retained in `factory_tests.cpp.before.txt`. Root provides fresh native Evidence and performs final static review. Source compilation/runtime results are unknown until the user explicitly authorizes a rebuild.
