# Original fixed-service teardown correction — 2026-11-05

Changed only `outputs/anadius-owned-server-source/server.cpp`. Root captured actual `before/server.cpp`, integration-test source and pending64 before authorization. No test source, other production file, status, historical build proof, original binary or ZIP was edited by this author. No compiler, tests, linker or binary execution was run.

After the existing socket/source/IO retirement checks and Original shared Winsock release, the Original branch now projects the measured two-service first pass: actual service gate enter/leave while its socket list is already empty, then scheduler shutdown high32 of word_38 replaced with **exactly1**, retaining its low32 wake bits. The value update runs under existing exclusive Retiring admission and does not alias-cast uint64 storage as a dword atomic, widen it to a whole-word atomic or claim native atomic/layout ABI equivalence.

The deleting pass is now service-gate deletion→scheduler-gate deletion→nonnull IOCP CloseHandle→registry-gate deletion last. Original ignores CloseHandle's ordinary BOOL return and never queries GetLastError for it, matching026420. Its retained handle fields are consumed after a returned call, including false; subsequent source Phase::Closed describes completed source control flow, **not certification that the OS handle closed successfully**. An injected C++ exception interrupts before the following store/steps and uses the existing poisoned-prefix/no-replay policy; no guessed retry, rollback or native EH cleanup was introduced.

Host retains its prior port-first checked CloseHandle/GetLastError/exception path, service then scheduler gate deletion, and checked per-context cleanup. No Host test contract was migrated into Original by changing defaults.

The pre-existing `Constructed→destroy_context` path can retire an unpublished backend with no initialized native resources. To preserve that path, the new gate round-trip is conditioned on service_gate_live and the new shutdown-word assignment on scheduler_gate_live. It does not enter an uninitialized CRITICAL_SECTION or fabricate an initialized scheduler shutdown for the passive Core. Successful Quiescing has both flags and executes the measured sequence. No additional unload/drain/admission condition was added.

Scope remains the already-selected own_thread0, no timer, zero-work/empty-queue, fixed socket-service→scheduler composition. Native hidden-worker/timer/custom-service shutdown branches and native heap/base-table/free/EH layout remain separate work. The existing source invariants, not a new generic registry framework, select this bounded sequence. Fresh evidence is the unchanged10-record audit in this stage (`native-findings.md`, index and exported bundle); this implementation made no additional REA query and did not rewrite those pre-change observations.

| File | Before SHA256 | Final SHA256 |
|---|---|---|
|server.cpp|8f46ebb2adfc054fd2a8710c4d7f4da2e2a6044b2e3ddeede7e6d973eba84c32|653c9671ef4eb818f3f0ba15124e135d4a5286629bb62e728c0beea682ecad57|

Source tests and independent review are assigned separately by root. Their presence does not establish execution or rebuilt-DLL validation.
