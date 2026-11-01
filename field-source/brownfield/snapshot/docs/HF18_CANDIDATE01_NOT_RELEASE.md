# HF18 Candidate01 — Configuration-First / Core+Addon

STATUS: SOURCE_CANDIDATE_ONLY / NOT_RELEASED / MSVC_NATIVE_GAME_NOT_RUN.
Parent is exact unchanged H1 TEST_ONLY source sha256 b85e850d29f4eec675aca907d296d9d6da784e136a78691314a8dfdcb637fcbf.
Official HF17 is a different immutable artifact and was not modified.

HF18 Candidate01 stages 7 core detours and 0/2/4 addon detours in one MinHook transaction AFTER PTD, service acquire/bind and module-relative surge.cfg commit. A missing surge.cfg selects 0 addons. Non-missing invalid/io/resource Config2 errors fail startup before any hook commit. Config2 policy writes and clear are frozen during hook ownership; shutdown retries keep freeze until full teardown. There is NO surge_override.cfg adoption in Candidate01; base+override semantics require separate approval and donor/Consumer evidence.

Known gates before release: full Windows MSVC compile/link and source QA from THIS exact source, negative early PTD/service/config/freeze failure paths, concurrency/policy generation, actual 7/9/9/11 with source-chosen config (not compile mask), rollback/restore/reload, service lifetime, platform cleanup, CRT/Golden and both real games. Historical HF17 QA scripts and docs may still assert pre-HF18 ordering and must be revised in bounded fashion, not relabeled PASS.
