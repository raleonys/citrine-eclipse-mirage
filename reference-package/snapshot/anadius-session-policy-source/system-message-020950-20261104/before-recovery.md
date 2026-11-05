# Reconstructed production baselines

The four production before files were not captured before editing. This was a recording error. With the root agent's explicit authorization, the exact known baseline bytes were recovered after editing and verified against pre-existing SHA256 authorities. These are reconstructed baselines, not contemporaneous captures. The two test before files are separate original captures and were not altered here.

| File in before/ | SHA256 | Recovery and independent authority |
|---|---|---|
| native.cpp.before.txt | 7b0131c0f8ee6fc6bc7a3c8e4b5692295cd743ea39aac61437c8dd65a8d25919 | Extracted only anadius-socket-source/native.cpp from the historical delivered ZIP; exact match to outputs/source-workspace/source-snapshot.json. |
| native.hpp.before.txt | b6b562b73e3fd6fed14a8c52f7749a33cf6d4171e4af09ab19748dfebc39b7b1 | Extracted only anadius-socket-source/native.hpp from that ZIP; exact match to the same snapshot. |
| policy.cpp.before.txt | b00747912def3dd0c14dd5be59aa8b42fa1459d8981330f96e56f51378e7fefe | Reversed only this stage's SystemMessageState field, concrete020950 formatter selection, SDK-pointer validation and introduced line wrapping; exact match to the prior tracked-executor stage's independently reviewed final current-file hash. |
| policy.hpp.before.txt | 77b073c58846de51e57a035462add6db66c6b94e8849dbf60703fd59285318cd | Reversed only this stage's trailing SystemMessageImports field/comment and category comment change; exact match to the prior tracked-executor stage's independently reviewed final current-file hash. |

Authorities were read without modifying them:

- outputs/source-workspace/source-snapshot.json SHA256 b7362cf518727f6fc997bfc1377bc4d83e2751903ead78cf376251cb0bf87e6d.
- outputs/dll-reverse-engineering.zip SHA256 790b71bbf92d02ff05a391d2985fdab4eaad3ab72f34de29ea51e79be13b267c.
- outputs/anadius-owned-environment-source/tracked-executor-implementation-20261104/evidence.json SHA256 edf6bdc1842f3eed8009b64fbf870b3a4ef9e3a1bf7d795a0216e7f06c1302ad. Its Policy final hashes also appear in that stage's independent-bridge-review.md.

Only the two named source entries were extracted from the ZIP into this stage. No executable, DLL, compiled output or historical evidence was changed. The successful byte-hash equality establishes the baseline contents; it does not retroactively establish a pre-edit capture event.
