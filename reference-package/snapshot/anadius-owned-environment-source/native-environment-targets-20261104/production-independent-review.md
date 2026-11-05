# Original-default classifier: independent production review

Result: STATIC PASS for the bounded selected initial-source classifier correction. No blocking finding in the five production diffs below. This is source review only; no compiler, project tests, linker, DLL execution, or packaging was run.

Reviewed against the five `*.before.txt` copies in this stage and fresh native evidence summarized by `native-findings.md` and `default-environment-review.md`.

| Exact reviewed source | SHA256 |
|---|---|
| `outputs/anadius-owned-environment-source/environment.cpp` | `7306d849d5ca78c59dd7aaa3b92825773bd236145e68d095df130e31ee7ea7d3` |
| `outputs/anadius-owned-environment-source/environment.hpp` | `626b2fae3ca7ea6f7920c09197487642d9de7c26c5091da30db26974ff6e10bd` |
| `outputs/anadius-session-profile-source/profile.cpp` | `d20cbb18eb74f9119a29c1058516d9aa36d48ad82898ac1fc4f04cf3fb6b8d18` |
| `outputs/anadius-session-profile-source/profile.hpp` | `5c8f0a3e432b1f9c8ae0ab1738173acc496d098345c24cb5ed5554d72cf76808` |
| `outputs/anadius-application-graph-source/graph.cpp` | `fc90cf952eb5bb9230421d8dc97981ed6c7b60cf94d5b18f0cbd695ea0fdaf84` |

## Checked behavior

- `ClassifierProfile` is immutable in `Control`. The constructor defaults to `Host`, rejects unknown enum values, and preserves prior capacity validation. `configuration()` changes only the nonempty schedule token's type-query function when `OriginalDefault` is selected. The token remains a capability, never a native code/table address cast.
- Default HOST callers keep the active classifier. Profile's new boolean defaults false and is consulted only when the entire environment configuration was empty and Profile supplies its own provider. Explicit custom providers retain their existing descriptors and validation; the boolean does not silently replace them.
- Graph captures the selected OriginalApplication lifecycle before moving policy options, then forwards the boolean to Profile. This is root's chosen existing lifecycle selector, rather than the additional Graph/resources-factory setting suggested in the earlier design review. It deliberately applies the original default classifier to Graph's automatically owned OriginalApplication provider; cooperative Graph configurations retain HOST behavior. No synchronous attach/detach source is changed.
- Checked broader existing fixtures: `graph_tests.cpp`'s OriginalApplication case closes unstarted; `worker_tests.cpp`'s original-dispatch case observes the first physical dispatch before a completed session. Policy-only OriginalApplication fixtures construct Profile directly, retaining the false default. Selected accepted sockets still originate `OwnedServerBackend::initialize_pending_socket_0211b0` through the fixed initial tables in `lifecycle.cpp:23-34`, then session construction and `023d20`. No current custom-family root counterexample to the Graph lifecycle selector was found.
- The real Slot root environment, owner+0x78 source mapping, context, origin, protected job scope, copy descriptors, response descriptors, and reference-ledger implementation are unchanged. The generic `023dd0` constructor remains unchanged and now selects its existing equality branch for those initial root sources.
- Constructor equality output is the actual selected empty operations sentinel with state zero. `JobHandoff::clone_environment()` now skips only the unsupported query on that exact empty sentinel. It still destroys the clone and closes the same protected EnvironmentJobScope, followed by the existing checkpoint/call check. HOST clones still query and destroy normally. The selected sentinel destroy is nonthrowing in its body and does not affect a real root row.
- Existing partial-configuration rejection and default receive/send provider checks remain in place. Repository source uses field assignment for the selected Profile Dependencies constructions; the added field does not shift an observed positional production initializer.

## Boundaries

Fresh native `0211b0`/`023890` supports initial schedule084908 returningA25C0, so type+8 equalsA25C8 in023dd0. This fix corrects that selected IO-constructor branch and removes the extra HOST owned-handler preparation path for its resulting state-zero IO environments. It does not zero native or HOST root contexts, add a scheduler-work retain, or reconstruct the distinct tracked family.

The query function is associated with the provider's schedule token. Its bounded equivalence relies on the actual Profile constructor-source registry resolving only initial Slot roots. Derived HOST response/factory rows are not inserted into that source registry. Treating arbitrary derived rows as newly valid constructor sources would require a fresh family/classifier review.

Response executor slots20/88 and their complete native scheduling semantics remain outside this correction. Their HOST adapters are unchanged. This review does not establish business payload equivalence, native CRT/RTTI/virtual ABI equivalence, or a runtime test pass.

The planned source regressions should still verify legacy active behavior, original-default empty construction with unchanged real root, direct receive/send completions, empty-clone scope closure, and Graph's OriginalApplication selection. They are not claimed to have run here.
