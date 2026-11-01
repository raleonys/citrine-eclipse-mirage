# MinHook Architecture — 3.2.5 SQUALERIS

3.2.5 SQUALERIS inherits the MinHook architecture introduced and accepted in the 3.2.3 MELOTRIX line.

`DLL_PROCESS_ATTACH -> schedule worker -> DllMain return -> create/queue 9 hooks -> one NtGetNextThread SAFE-FREEZE MH_ApplyQueued -> PTD/config/service/network`.

The transaction has three meaningful outcomes: committed, rolled back, or rollback incomplete. Rollback-incomplete is never treated as initialization success and never frees a possibly-live trampoline. The same transaction membership is retained until PTD succeeds or the hook surface is globally disabled. Normal shutdown globally disables first, drains call gates, then removes subsystem tickets and uninitializes MinHook.

The official published HF16 topology remains **7 detours**. HF17 PREPUBLICATION candidate adds four CreateProcess/SHGetFolderPath MinHook detours, physically staging **11 in one transaction**; Config2 gates their behavior. Windows/MSVC runtime and donor conditional-registration parity are not yet accepted.
