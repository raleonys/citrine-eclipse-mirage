# Relocated historical source evidence

This directory contains selected immutable historical evidence migrated byte-for-byte
from `docs/evidence/` to keep active source packages focused on current contracts.
These snapshots are NOT current runtime, release, Windows acceptance or gameplay
certifications. Current authoritative policies remain in `metadata/*.json`.

The original historical source tree at commit
`aba8ee3b21d59da6262a3126df85a2d373de8485` retains all 611 evidence files
(including logs, XML output, historical contract copies and the former index).
To inspect evidence intentionally removed from the active tree:

```sh
git show aba8ee3b21d59da6262a3126df85a2d373de8485:docs/evidence/PATH
```

The remaining records in this directory preserve their original bytes and historical
relative cross-references. Where a report mentions an old `docs/evidence/` path or
the old `INDEX.md` maps to a retired location, consult the original commit for
the exact source; active docs and metadata references to retained files have been
rebased to `metadata/evidence/`. This is an archival location, not a destination
for new generated validation dumps.

Retained records: 119. Retired-from-HEAD records: 492.
Git history is preserved, with no force push or release claim.
