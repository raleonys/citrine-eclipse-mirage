# Universal G0 Stage02 (IDS QA ONLY / NOT_RELEASE)

This changes only QA/governance routing. No source release authority is conferred.

- `check_governance_first.py` calls an external-data authority boundary, not a hardcoded historical hotfix pin. Stage02 deliberately always returns BLOCKED.
- `test_release_qa.py` keeps the eight-shard partition and same step universe using `metadata/RELEASE_QA_CATALOG.json`. All release-identity-dependent historical tests are REVIEW_REQUIRED, so no release QA can be claimed until reviewed.
- Independent Library master SHA, release intent, current pointer and external catalog approval remain required. A self-generated matching hash is not authority.
- The original official release ZIPs and source C++/CMake are untouched.
- Use `scripts/test_universal_governance_contract.py` for QA-only structural and negative acceptance; it is not a G0 substitute.

- The inherited 64-step order is frozen by an external-parent-replayed ordered SHA-256 in the migration catalog. Silent removal or reordering of a GENERAL_CHECK is rejected; updating lineage data alone is not independent release authority.
