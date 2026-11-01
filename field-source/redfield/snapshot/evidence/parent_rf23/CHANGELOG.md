# REDFIELD source checkpoints

## RF20 BUILD FIX03 — 2026-10-20 — NON_RELEASE_WORK / NOT_RELEASE

- VsDevCmd/helper runs from stable build CWD; private temporary capture directory is never its process working directory.
- Internal helper is removed by ExitStack before private dump directory cleanup, including failure/timeout. No ignore-cleanup or success-on-error bypass.
- Exact FIX02/new CWD-pin model,13 current environment mocks and7 real process/compiler cases; Windows NOT_RUN. C++ unchanged; parent70/70 evidence historical.

## RF20 BUILD FIX02 — 2026-10-20 — NON_RELEASE_WORK / NOT_RELEASE

- Live process output from append-only logs, per-CXX/LINK/RUN progress,5-second silent-child elapsed notices, explicit timeout/exit status; Python -u launcher.
- Visual Studio discovery/setup now visibly uses the same process runner. Separate UTF-16 environment dump remains private; setup console output uses normal CMD encoding.
-13 simulated environment cases and7 real POSIX process/GCC compiler cases; fresh full normal regression run recorded separately. Windows NOT_RUN; C++/ABI/vendor unchanged.

## RF20 BUILD FIX01 — 2026-10-20 — NON_RELEASE_WORK / NOT_RELEASE

- Visual Studio environment bootstrap uses a generated internal CMD with fixed subprocess arguments; setup path no longer crosses CRT quoting as embedded command text.
- Preserve setup output/exit and UTF-16 environment capture; save msvc_environment.log, retain timeout/failure diagnostics, sanitize compiler/linker injection variables before/after.
- Update stale launcher labels and entrypoint descriptions. C++/ABI/export/vendor bytes unchanged.13 focused simulated Python cases; actual Windows NOT_RUN. Historical RF20 runtime receipts classified separately.

## RF20 — 2026-10-20 — NON_RELEASE_WORK / NOT_RELEASE

- LocalService-owned AchievementSession and Grant TCP binding; committed snapshot QueryAchievements.
- Bounded per-peer FrameDelivery retains response/event and ticket; full response callback commits, then event precedes next request.
- Cancellation before commit rolls back after pending callback drains; event failure after commit preserves state. No remote ACK/retry/fanout.
- Three new real loopback regression binaries;1114 exact +24 derived unchanged,35 LocalService bindings. Exact RF19 parent; fresh QA only.

## RF19 — 2026-10-20 — NON_RELEASE_WORK / NOT_RELEASE

- Owned Grant response/completion-event preparation; aynı candidate, tam measure/admission/render ve failure rollback.
- Exact original selector/wire; private size-query adapter. Frame/combined bütçeleri, move/RAII/close ownership.
- Üç yeni wire/selector/admission/lifetime test;34 production route unchanged, Grant TCP/event queue deferred.
-1114 exact +24 derived (14 TU,10 header); exact RF18 parent, eski PASS taşınmaz.
