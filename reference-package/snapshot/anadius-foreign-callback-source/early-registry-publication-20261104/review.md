# Independent static review

Root reviewed the subagent's production-source audit and test-source patch against the saved pre-edit test source, current production assembly/publication/admission/PTD sources and seven fresh Ghidra Evidence records. Static review: PASS within the limits below. Compilation and execution remain unverified.

The test-only HookEngine override preserves its former default originals array for existing scenarios. The added function uses actual install_host OriginalCrt, actual matched attachment and HostPlatform/OwnedEngine calls. A synthetic engine enable callback at the second hook invokes typed RegOpenKeyExA with a nonnull unrelated subkey, selecting the general body rather than pure forwarding. It then invokes the same entry on one joined foreign thread. The source checks exact arguments, incoming original-call LastError, Full/body permission, active native lease, controller borrowed PTD and foreign distinct acquired PTD followed by retirement. Sequential calls and join avoid races on the probe fields.

Before enable returns, the test inspects the actual original-slot prefix (connect plus openA, remaining slots zero), callback ready=false and milestone bits=0. This is not a raw private Publication mask observation; successful typed original dispatch establishes that the selected published receipt is usable. The later setup-complete and existing explicit cleanup calls keep their former meaning.

Root removed one post-body exact LastError assertion after review. The general nonnull body calls the ambient encoding/CRT providers after the original; this integration scenario does not control every provider's outgoing LastError. Incoming error at the synthetic original remains checked. This keeps publication/PTD coverage independent of an unstated ambient SDK assumption.

Final test source SHA-256: `247c7838c29b3b1790b327653bb8ffbc2d3f3a89ccebddbac118d11ff7b913c1`. Pre-edit hash matches the last real full build: `8125d09a886ad080e79f13fb31ee85f9260bfad311b980dc2202d06a8179446d`. The author's notes describe their earlier pre-review bytes and remain historical.

No new production readiness change is required by the examined early single-actor path. No native runtime reentry observation, general shared-set concurrency, full ABI/CRT equivalence or numerical DLL-completion increase is claimed. The fixture uses controlled SDK/engine boundaries, not real system hook installation. No compiler, project test, link, DLL execution or packaging was run.
