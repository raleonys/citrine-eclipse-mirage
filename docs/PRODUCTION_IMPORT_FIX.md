> Historical snapshot: the subsequent hook 11/lifecycle and metadata
> corrections are documented in
> [Bluefield alignment](HOOK_ALIGNMENT.md) and
> [Brownfield metadata audit](REFERENCE_METADATA_AUDIT.md).

# Production, CRYPT32, and final output corrections

Work item: YF-PRODUCTION-20261029. The parent source ZIP and
its SHA-256 sidecar were preserved.

In the owner's actual MSVC Production log, DLL compilation
and the first five Golden stages passed. OLE32 was absent;
VERSIONINFO was the only PE resource, and both PNGs resided
in read-only `.rdata`. The sixth, FALCION stage stopped
because of `scripts/__pycache__/golden_policy.cpython-313.pyc`.
Although the top-level Python process used `-B`, three
CORVEXON subprocess interpreters did not receive that flag.
A regression comparing old and new child-process behavior
reproduced and corrected this root cause. All three
subprocess invocations now receive `-B`; the pipeline
and scoped PowerShell process inherit
`PYTHONDONTWRITEBYTECODE=1`.

For caches produced by earlier work,
`clear_tool_python_cache.py` removes only standard cache-tag/
optimization bytecode files corresponding to manifest-owned
Python sources in scripts/tools/tests. It does not touch
vendor or actual source bytes, refresh the manifest,
delete unknown files, or make FALCION ignore them.
The owner can therefore rebuild the existing source.

The owner requested an actual CRYPT32 import for hook 11,
as in Brownfield/Bluefield. Both references contained
a direct `&CryptQueryObject` reference and a `crypt32`
link dependency. Yellowfield's earlier dynamic-only
lookup incorrectly applied the Anadius baseline import
list to this new hook. The core now links through
`crypt32.lib` and resolves the owned native target
using the SDK address `&::CryptQueryObject`.
The unnecessary explicit crypt32 LoadLibrary/FreeLibrary
lease was removed; the normal PE dependency graph
carries its lifetime. MinHook trampoline/callback/
cleanup and the RE selector behavior remain intact.

`CryptQueryObject` is not an export of this DLL:
the import table contains
`CRYPT32.dll -> CryptQueryObject`.
Golden and actual DLL inspection require that import;
the public export count remained five at this checkpoint,
and OLE32 was not reintroduced. Reference hashes
and the owner's native log are in
[PRODUCTION_IMPORT_FIX.json](../metadata/evidence/PRODUCTION_IMPORT_FIX.json).

The parent package had also failed to write the final
file to dist. After six Golden stages and three actual
runtime cycles with the finalized `preloader_l.dll`
copy, it now writes the DLL, SHA-256, and receipt.
The runtime host uses the supplied file's actual
basename during unload checks instead of hard-coding
the unsigned name. The filter remains
`preloader_l.dll`.
See [Golden and dist flow](GOLDEN_PROTOCOLS.md) and
[distribution contract](../metadata/DIST_CONTRACT.json).

Synthetic orchestration tests verify stop-on-first-error
at each Golden failure, withholding dist after runtime
failure or DLL mutation, retaining the prior output,
and rolling back a file-commit error. These are not
MSVC Golden PASS results. The entire pipeline and dist
output for the revised source had not been executed on
actual Windows/MSVC, and no game call trace was provided.

Local validation passed Release/Wine 173/173,
affected Debug 5/5, and 28 infrastructure regressions.
Three runtime cycles passed with an actual LLVM DLL
named `preloader_l.dll`. Both Release and Debug imports
included CRYPT32!CryptQueryObject, OLE32 was absent,
and the VERSIONINFO and PNG data placement remained
unchanged. Native MSVC acceptance of the current
source/dist and game acceptance are separate and
were not run at this checkpoint.
