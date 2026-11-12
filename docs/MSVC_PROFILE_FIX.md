> **Historical checkpoint.** Use [DEVELOPMENT.md](DEVELOPMENT.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# MSVC profile log corrections

Five owner-supplied logs show that the previous vendor validation blocker was
resolved. Release, Debug, and Golden application DLLs were compiled and linked
with MSVC 19.51. This observation does not establish full runtime, native-test,
or game acceptance.

| Profile | Result in the supplied log | Correction / outstanding work |
|---|---|---|
| Compile | DLL and runtime host compiled; all five exports were correct. The resource allowlist rejected a third manifest. | Apply `/MANIFEST:NO` to the product DLL under all MSVC profiles. PNG and VERSIONINFO validation remains strict. |
| Debug | DLL and runtime host compiled; resource validation stopped because of the same additional manifest. | Apply the same link rule to Debug. Preserve Debug symbols. |
| Native | Targets including the Source API DLL/host compiled, but MSVC rejected `__int128` in an execution test. | Replace the test oracle with four 32-bit multiplications and a signed high-half adjustment using standard 64-bit arithmetic. Application arithmetic is unchanged. |
| CRT | The general ALL target also compiled an unselected execution test and encountered the same error. | In addition to correcting the oracle, restrict CRT builds to their own CRT/DLL test targets. Native continues to build all tests. |
| Golden | MSVC DLL/host compilation, export validation, and two resource checks succeeded; required product targets were missing, so the chain stopped before it began with exit code 3. | Do not claim completion of missing targets. Report `BLOCKED`; preserve exit code 3 and the check that the original DLL remains unchanged. |

The third resource added by MSVC was `[24,2,1033]` (`RT_MANIFEST`). The two
owner-adopted resources remain `[10,101,1033]` (SURGE PNG) and `[16,1,1033]`
(VERSIONINFO). Resource files, vendor bytes, and all five exports remain
unchanged. The Source API development DLL receives the same manifest link rule;
EXE manifests are unaffected. The metadata checker is not relaxed to allow the
extra manifest.

`BUILD_RECEIPT.json` now records build success separately from validation
of the selected profile. If Golden targets are missing, validation reports
`BLOCKED` and exit code 3; a compilation PASS is not treated as a Golden PASS.

Compile, Debug, Native, and CRT profiles must be rerun on Windows for the new
source. GCC, LLVM-MinGW/Wine, and source QA evidence from that session is not
presented as an actual MSVC rerun of the corrected sources. Input log hashes
and current results are in
[MSVC_PROFILE_FIX.json](../metadata/evidence/MSVC_PROFILE_FIX.json).
