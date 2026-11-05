# Independent review addendum: missing input is known preflight rejection

Status: no blocking finding remains in this bounded correction. This addendum identifies the current exact Environment core/test bytes. It supersedes the v1 review's overly broad known-preflight statement for the three missing-address paths below; the v1 review remains an immutable historical record. Root's final source check found this omission after my v1 review. No compiler, project test, link, DLL execution or ZIP operation was performed.

## Current reviewed files

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-owned-environment-source/environment.hpp | a8d070d3b8dd5f0ed500fdc70984778136403b33748c21cb21dc3d0bec58416b |
| anadius-owned-environment-source/environment.cpp | 0c8e4c43442fe1471720e8bfb48cc78b7b19f7e20ba912e2d9bfac212f3539e5 |
| anadius-owned-environment-source/environment_tests.cpp | 9ded4933d9eb592638113835228d0ee1257e9e3906f4243dd33ff15a3036f934 |

Source report `scoped-environment-implementation-v2.md`: SHA-256 `7362df8242650c556b28fa60e77dbaa2229c60d8a261d75279460a524380b421`.

Test report `scoped-environment-test-implementation-v3.md`: SHA-256 `711267bb23996f534815889408cc60909fdf770d2574859fd7c147cb2b7fb71a`.

Prior core review `scoped-environment-core-independent-review.md`: SHA-256 `538fdd57e885c6f4715c948716c93d116d44e27be896844ffa217686148c38f6`. Its former cpp/test hashes are not the current source freeze.

The seven socket/Profile seams and their independent review `root-seam-independent-review.md`, SHA-256 `0ae105402d66d0ce93b43dd928957580c773a567199e07ddc0146be738f4513c`, are unchanged.

## Production correction

Three prepublication checks previously combined an entirely missing input row with corrupted fields of an existing owned row:

1. `shape()` for Full/Partial source input.
2. `inspect_source()` before a public scoped constructor's context inspection.
3. The first `enter_job()` protected-source capture.

The final source splits those outcomes. No registered row at the supplied input address calls the nonpoisoning UnsupportedInput rejection path. Existing registered row with mismatched defined fields still calls sticky invalid. The second entry capture already rejected a disappeared/replaced generation and continues to do so. The rejection helper releases metadata before constructing the exception.

In the first path, the run's stack admission/claim array has not been linked when shape rejects. In the second, no run has started. In the third, the scope admission has not been linked. Consequently, the refused operation publishes no Effect/Scope record, invokes no checkpoint or raw native callback, performs no native prefix, and does not change shared health merely because its supplied address is unregistered. This permits a healthy sibling to continue. The final source still checks address ownership/conflicts before raw full-field reads.

This does not relax failure after admission. A missing row discovered by `row()` or `query_row()` inside an already registered effect remains a sticky unknown. A previously owned protected source missing from `scope_metadata()` remains sticky. Genuine registered executor/header/state/capability corruption remains sticky. Same-actor checkpoint/admission reentry remains sticky. No native field ordering, work retain/drop, scope lifecycle, callback adapter or raw copy/move tail changed in this correction.

## Component assertion update

The earlier Root forged-address test expected Poisoned. That would contradict the general Full/Partial missing-row correction, so the test now explicitly requires a known Bound/inactive refusal. It copies all projection fields to an unregistered address, records the genuine source capability and source/forged fields plus counters/admission count, then calls the actual query route. It checks that the rejected query publishes no effect/checkpoint and preserves one genuine reference, all measured create/copy/move/destroy counts, both sets of fields and Bound health. Only after measuring that prefix does it perform a separate normal query of the genuine registered source, which must retain the same capability and healthy inactive state.

The registered-source foreign-executor branch remains Poisoned/inactive with untouched move headers. Thus the test source distinguishes missing caller input from corruption of an owned record instead of weakening all source checks. The raw slot08 executor-tail regression, full Root move admission, unknown-prefix cases, 4100-projection growth and family/source fixtures are unchanged by this test update. No assertion was executed.

## Unchanged evidence and limits

The stage's two saved native bodies remain original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`, Ghidra 12.1.4: move `180023cb0..180023d19` Evidence `ev_cd85a47c1ecc95b2c78a406e8478fe137fdc65a60d22d94b04b2f4dfa8d0ec65`, and constructor `180023dd0..180023f2f` Evidence `ev_a796bdbc87106dd39b97f8933a91e478c3113ea2ac400041f0e295bcba20e562`. This small HOST preflight correction does not require reinterpreting those native algorithms or claiming another recovered function.

The prior core review's explicit boundaries remain: trusted raw Empty and raw Root copy caller windows are not memory-sandbox/whole-copy proofs; positive actual Scope coverage, two-Scope tracked-family Runtime progress with independent B native/FLS/Parent retirement while A is held, naturally established child old-count-one after original return, child scheduling/prepare/response authority, full original response/021770 child tail, compilation, native ABI/EH and runtime equivalence remain OPEN. Production remains Serialized. No completion percentage is asserted.
