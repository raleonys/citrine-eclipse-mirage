# MinHook vendored dependency

Current source packaging requires the pinned MinHook tree to be physically present at `third_party/minhook/upstream`.

- upstream repository: TsudaKageyu/minhook
- pinned commit: `8af6b4acae5a9388fd742b56fa79ece89d96f823`
- configure-time network access: **forbidden**
- FetchContent/package-manager fallback: **forbidden**
- upstream CMake execution: **forbidden**
- SAFE_FREEZE mutation: applied only to a build-tree copy by `scripts/build_orchestration/PatchMinHookSafeFreeze.cmake`

`UPSTREAM_COMMIT.txt` and `UPSTREAM_MANIFEST.sha256` are release provenance. The pristine vendored tree is covered by the project active manifest as well.
