# PR #23 post-merge listener shutdown acceptance

**Status: test plan; execution evidence remains separately required.** The follow-up baseline is merged PR #23: `f121ecf99d275376fdc045e93a5873066c30fd11`, incorporated into `main` by merge commit `d9c860d17769738fd6f03a58a7f3c4b9f875b067` on 2026-11-01.

## Observed local Windows evidence (not source-commit-pinned)

The owner supplied five logs from a local extracted directory, identifying MSVC 19.51.36260 (VS 2026). No source commit or source manifest receipt was included. Therefore these outcomes cannot establish exact PR #23 or merge-head behavior, or named Consumer/game acceptance.

| Local profile | Observed result | Uploaded log SHA-256 |
| --- | --- | --- |
| Production / local six-stage Golden | PASS, final DLL SHA-256 `605c216142c6a14e28d3c4d1c7f78ed3691ded34fb620197ed465c03e42d71d0` | `0298660199c0639437bd5e0aa6715cac2f87785dba1464fefe11d4f107cb073a` |
| Debug build and host | PASS, three load/socket/shutdown/unload cycles | `bb53e6a29ebd5b5c0ef124fd5418179389a08ae904388408f2a02a3853224f58` |
| Compile Only / unsigned Release | **FAIL: stopped service still accepts connections** | `fb46b395e5da6c5955cdba711f0695c259dbf36242c2065abdae2b5aad7973b4` |
| Static CRT | PASS, 6/6 CTest | `0c006aaad07bf741b32b64d659a10c68afb3949b95520bf051d707211b75d821` |
| Native Acceptance | PASS, 209/209 CTest | `defb495ee7e9da16c638b9e59dd071c23ae92f2a7bb78d48dc38e1bea46f3d15` |

The first observed failure block is:

```text
zero-timeout shutdown status: 2
FAIL: stopped service still accepts connections
ERROR: Application public API and real load/socket/shutdown/unload cycles failed (exit 1).
COMPILE ONLY: FAIL
Exit code: 1
```

The existing `tools/preloader_l_runtime_host/main.cpp` obtains the published `EALsxPort`, calls `DllShutdown`, then attempts loopback `connect` to that port and expects an error. Other local profiles passed the same check. The evidence is compatible with an intermittent shutdown/listener race or a separate port owner but **does not establish either cause**, a deterministic runtime defect, or causality from PR #23. Raw logs remain owner-supplied attachments and are not committed because they contain local filesystem identity.

## Reproducible exact-checkout follow-up

The new `msvc-shutdown-lifecycle.yml` uses Windows Server 2022/MSVC 2022 and builds the actual production DLL with `preloader_l_runtime_host` in both Debug and Release. It executes the unchanged `dll_public_lifecycle` CTest in **five separate processes per configuration**, recording every exit code and a separate JUnit file. One failed attempt fails the job; no time-based grace or bypass masks the failure. Git checkout SHA, build log, CTest output and configure logs are preserved.

This is **CI detection only**: no production code, ABI, Winsock error contract, service lifecycle policy, vendor tree or Golden finalization is changed.

## Required disposition

- Prove or disprove recurrence on the exact GitHub checkout with full Debug/Release evidence. If reproduced, independently identify listener ownership, port reuse and close/accept ordering before proposing a production fix.
- Keep complete Native/Production/Golden and named Consumer/game acceptance as separate gates. The owner logs do not automatically satisfy them for this PR.
- Preserve PR #23 as merged. Do not rewrite or close historical findings without evidence.
- Keep this PR **DRAFT** and do not merge, publish or promote without owner review of exact-head Windows results and any unresolved failures.
