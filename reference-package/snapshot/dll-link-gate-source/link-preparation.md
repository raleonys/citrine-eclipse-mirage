# Current rebuild helpers — source preparation, 2026-11-03

TOOLING_SOURCE_ONLY_NOT_EXECUTED. No C++ or test source changed in this step.

The active workflow is verify-current-rebuild.py, link-current-rebuild.py and
package-current-rebuild.py. The older *fresh-full-build* scripts and
check-current-full-dll-link.py describe earlier toolchains/blocked-link workflows;
they are not the current entry point. Their historical results were not rewritten.

Source corrections in the active helpers:

- Link preflight validates the rebuild pointer against the verified run, common
  record/snapshot hashes, every source hash, selected compiler hash and both
  archives before rotating the prior output directory. Resolved move targets
  remain within the named project/run directories; existing history is preserved.
- Link success requires exit0, a nonempty actual DLL, and no undefined symbols.
  Failure writes its actual diagnostic and returns nonzero. Neither loader symbol
  is treated as an expected successful blocker. A candidate DLL gets a size/hash
  record, without claiming it was loaded or runtime-validated.
- The package helper requires the v3 link record for the same verified run,
  checks current entry/header hashes, consistent readiness/result fields and any
  actual candidate DLL's path, size and hash before changing delivery metadata.
- Verification/documentation history uses history/<run-name> instead of always
  comparing a new run against history/20261101-full. Existing history is untouched.
- The linker is under main(); importing or parsing its source starts no tools.

The future sequence remains: explicitly authorized fresh common build/tests in
a new rebuild run; verify that run; compile/link the entry against those verified
archives; package only if requested. The verifier still expects the established
run.json/historical-common/compiler-checkpoint inputs. This step did not create
a new run or claim the full workflow has executed successfully. Tool selection
continues through work/use-tools.ps1 and the configured ANADIUS_TOOLCHAIN_ROOT.

Only AST syntax parsing of the three Python helpers was performed. No helper was
invoked/imported, no compiler or linker started, no project test or DLL executed,
and no ZIP created. There are still55 pending C++/header files from earlier work.
Their build/runtime state has not changed. No new DLL completion percentage or
successful link is claimed. Frozen build/link/receipt and the delivered ZIP remain
unchanged; the last actual full-link result is still the historical failure.
