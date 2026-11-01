# B4 bağımsız kaynak incelemesi

Bağımsız inceleyen: `/root/b4_architecture`. Dört üretim dosyası için iletilen sonuç: belirtilen yaşam döngüsü koşulları altında engel bulunmadı. İnceleyen raporu dosyaya yazmadan kullanım sınırına ulaştığı için bu kayıt, tamamlanmış inceleme mesajlarından ana ajan tarafından derlendi. Dosyalar PASS mesajından sonra değiştirilmedi. Hash değerleri JSON kaydında.

- **construction:** Registry stays unpublished through prepare, bind_state and B2 Constructed close; constructor failures publish nothing.
- **ownership:** Runtime registry precedes RuntimeOwner; reverse destruction releases backend/RegisteredServices before registry. Resources backing state stays alive until dependent providers die.
- **admission:** Permanent image thunk acquires shared State and active lease atomically with pause. No provider code executes under Hub mutex.
- **registration:** Every successful exit callback has a real CRT registration position; foreign LIFO interleaving is retained. Failed registrations remove their pending callback.
- **drain:** Exit callbacks are consumed before invocation; direct synthetic exceptions retain unconsumed suffix for retry. Native exit_entry retains its exception boundary.
- **shutdown:** External source admission stop precedes callback pause and duplicate-handle joins. Application cleanup runs after all joins while B2 Binding/providers remain alive.
- **cleanup-order:** B3 worker route removal precedes drain, but existing exit callback uses direct library/online/hook cleanup rather than worker routes. Original cleanup order remains intact.
- **retry:** Callback drain/finish are idempotent. If later B2 Binding close reports Busy, runtime remains retained and completed callback teardown is not repeated.
- **handler:** Prior terminate handler restored only when current handler is own dispatcher; sequential foreign replacement preserved. Concurrent foreign mutation explicitly excluded.
- **errors:** NativeExecution uses exact SDK pointer signatures and SRW storage address. Thread-error index is delegated; QPC API failure system_error is a new adapter policy.
- **metrics:** B4 supplies two registration policies; B3 supplies six existing source routes. Eight of 17 Services routes bound; nine lower policies remain. No native body/closure increase.
- **partial-factory:** Five default categories select concrete existing/new source adapters. Other providers and external truthful stop_sources remain mandatory; full default-production factory and PE entry remain blocked.

## Açık sınırlar

- The permanent thunks are safe only while their containing image remains mapped; DLL unload and loader-lock/CRT module teardown are unproven.
- Natural CRT exit is not a worker-quiescence mechanism. Controlled Runtime.close outside loader lock is the tested shutdown path.
- External stop_sources must also exclude future calls via cached application-entry pointers until B2 Binding is closed.
- The original initialize source ignores exit registration failure. No cleanup callback is invented after that failure; full resource release is not thereby guaranteed.
- Recovered execution.cpp prelock owner accesses and shared QPC frequency cache are not proven data-race-safe under general concurrent C++ execution. Adapter correctness is not a concurrency proof.
- Caller-provided RuntimeSeed must contain valid initialized SRW storage and must not copy a live SRW lock. Initial original globals are not guessed.
- Host registration/handler mutation and lifecycle use one serialized non-reentrant control thread; natural process exit must not race registration mutation.
- Actual config/console/threads/server/online/library-effects/host/hook providers, nine Services policies and foreign CRT/EH/object ABI remain explicit gates.
- Source tests do not install/load a DLL or prove byte-for-byte/runtime equivalence.
