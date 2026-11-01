# B10 kaynak incelemesi — bağımsızlık sınırlaması

Sonuç **PASS_WITH_SELF_REVIEW_LIMITATION**. Delegated workers returned usage-limit errors. Root completed registry and tests, reviewed the production, and captured executable evidence. No independent full-production review was completed for B10.

- **registry_seed:** Registry is explicitly newly empty and source-owned. It does not adopt the compiler CRT private list or claim original global defaults.
- **storage:** Each20slot block retains shared targets before source entry publication. Block/vector allocation precedes publication. Source entries hold known logical MutexState/condition/ready identities only.
- **typed_lock:** Real typed SRWLOCK is separate from the logical uintptr cell. Private ReleasePlatform maps cell identity and runs existing unlock_04a6b4 on count1 without overlaying an SDK object on integer storage.
- **source_order:** Existing cleanup_exit_waits obtains thread id before registry CS, then ready/unlock/wake/clear/count order. Other-thread entries remain untouched; consumed references released before registry unlock.
- **lifetime:** Known live entries make close return false and stop new registration; existing cleanup remains allowed. Final close requires controller and external worker stop/join. CS destruction occurs at quiescent registry destruction. Open registry/locked targets and unknown SDK outcomes terminate.
- **logger:** Known bootstrap stream/literal identities map to an owned compiler ostream and stable token; no original address dereference. Formatted insertion and endl preserve selected width/fill/stream-state/flush behavior.
- **terminal:** Fatal route logs retained Unhandled exception and newline/flush then compiler abort. Stream exceptions may propagate before abort. Unconditional unexpected-return abort deliberately differs from original INT3.
- **core:** 73 checks cover full BOOL0x100 CS initialization, failures, ownership, multi-block registration, two actual thread ids, source cleanup order, exact logger bytes and five own terminal children.
- **integration:** 31 checks run full B4/B6/B7 source graph, three actual recovered CRT workers, explicit test registration, cleanup before same-domain payload deletion, FLS cleanup and join timeout/retry. External foreign/network/config effects are controlled. Workers resumed serially.
- **limits:** 17/17 counts selected Services composition routes. Three config policies plus native CRT/Cnd/stream/EH/foreign/general concurrency/image unload/loader-lock/PE entry remain open. No native body/closure increase.
