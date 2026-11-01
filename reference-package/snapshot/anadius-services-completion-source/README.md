# B10 — Sahip olunan exit-wait cleanup ve logging/termination

Kalan beş Services metodu somut seçilmiş kaynak politikalarına bağlandı: log_insert, log_finish, cleanup_threads, terminate_log ve unexpected_terminate_return. B3/B4/B7/B9 ile birlikte Services **17/17 bağlıdır**. Bu, tam native Services/CRT/ABI eşdeğerliği değildir. Configuration18/21, üç politika açık; tam DLL **BLOCKED**.

**Bağımsız inceleme sınırlaması:** alt ajanlar kullanım limiti hatası verdi. Root registry/testleri tamamladı ve kendi incelemesini çalıştırılabilir kanıtlarla kaydetti. Bu aşamada bağımsız tam üretim incelemesi yapılmış gibi gösterilmez. Sonuç PASS_WITH_SELF_REVIEW_LIMITATION; hash/build/envanter kontrolleri aynen zorunlu kalır.

## Sahiplik ve kullanım

Explicit empty_owned registry, count1 WaitTarget ve explicit shared ostream sahibi oluşturulur. CompletionServices, SourceServices ile birlikte B4 Runtime'a verilebilir. Registry'nin sahibi saklanmalı; Runtime admission stop/join/cleanup bittikten sonra registry.close() çağrılmalıdır. Canlı kayıtla close false döner; açık registry veya kilitli hedefin yıkımı terminaldir. shared_ptr worker join yerine geçmez.

Registry head'i özgün/compiler CRT globali değildir. Native CRT notify_all_at_thread_exit kayıtlarının otomatik devralındığı iddia edilmez. Hedefin ready hücresi protected veya post-join okunur. Typed SRWLOCK, recovered logical uintptr hücresi üzerine cast edilmez; özel release adapter'i kimliği eşler.

Logger yalnız gözlenen bootstrap stream kimliği ve dört literal VA'yı kabul eder. Bunlar pointer olarak dereference edilmez. Üç başlangıç parçası LSX emu version 2.0.1.0 satırını, fatal literal Unhandled exception satırını verir. Compiler ostream insertion/endl seçilmiştir; mevcut stream/streambuf sahibi açık girdidir. Stream exception'ı abort noktasından önce yayılabilir. Unexpected-return için compiler abort, özgün INT3/debugger davranışından farklı açık politikadır.

## Doğrulama

73 çekirdek +31 entegrasyon =104 kontrol PASS. Core gerçek native CS/SRW/CV, BOOL0x100 başarı, kurulum hatası,25kayıt/iki blok, iki gerçek thread id seçimi, ready→unlock→notify sırası ve beş terminal alt süreci sınar. Native condition wake çalıştırılır; tam condition wait ABI'si test edilmiş sayılmaz.

Entegrasyon B4 Runtime ve B6/B7 ortak domain'iyle üç gerçek recovered CRT worker kullanır. Test SDK wrapper'ı worker başlamadan önce explicit source kayıt oluşturur; bu bir compiler CRT kayıt yakalaması değildir. Launch nesnesi ExitThread yolundan önce silinir. Her worker cleanup'ının native payload delete'den önce tamamlandığı gözlenir. Join timeout/retry, application cleanup ve FLS/heap son kapatması geçer. Workers, diğer kontrollü provider'ların eşzamanlılığı iddia edilmeden join sırasında sırayla başlatılır. Network/foreign/config etkileri fixture'dır; gerçek kullanıcı CFG/token/achievement okunmaz.

Önceki746 kaynak byte düzeyinde aynı; yedi yeni dosyayla toplam753 kaynak. Yeni native gövde/kapanış0; mapped487/eligible491/normalclaims124/tarihsel kapanış52 korunur. Bu sayılardan toplam DLL tamamlanma yüzdesi çıkarılmaz. Orijinaller/rebuilt DLL çalıştırılmadı/değiştirilmedi; yeni orijinal byte okuması yok. Engellenmiş version girdileri tekrar denenmedi.

Sonraki B11: üç configuration politikası ve gerçek production Resources/foreign provider boşlukları; B10 bağımsız review fırsatı ayrıca açık. Öncelik1→2→3→5→4; generic CRT leaf/numeric0033c0 beklemededir.

---

# B10 owned exit-wait domain

ExitWaitRegistry requires explicit InitialRegistry::empty_owned. It creates a new logical head,20slot blocks and a real owned critical section. It never adopts an original VA, arbitrary native mutex/Cnd pointer or compiler-private registration list. WaitTarget requires explicit mutex type and ready seed; only nonrecursive count1 locking is supported. SDK SRWLOCK and CONDITION_VARIABLE are actual typed objects separate from the recovered logical MutexState.

Registration requires the current owner and retains a strong target before publishing pointers. Any allocation for an additional block occurs first. Existing cleanup_exit_waits and unlock_04a6b4 perform the source ready→unlock→notify→clear/count order. Private pointer lookups admit only owned live targets. GetCurrentThreadId precedes registry critical-section entry. Ready is plain32-bit memory and may only be read under its mutex or after join/quiescence.

Targets may not be reused while registered. The registry retains consumed targets through WakeAll completion, then clears source ownership. close must run on the constructor's controller after external B3/B4 admission stop and joins. False keeps entries and CS live and stops new registration; existing cleanup may finish. Even successful close retains the native CS until quiescent object destruction, preventing an immediate delete while a live object has late callers. Callers must retain owners during calls; destructor with open registry/live target terminates.

Native SDK callbacks obey nonthrowing outcome contracts. An injected unknown native outcome terminates rather than fabricating rollback. Source cleanup exceptions while holding the registry CS terminate. No hidden cancellation, complete condition-variable waiting implementation, compiler thread-exit registration interception, general concurrency or native ABI equality is claimed.

---

# B10 bootstrap logging and Services completion

CompletionServices selects five remaining methods of the existing Services
interface: log_insert, log_finish, cleanup_threads, terminate_log and
unexpected_terminate_return. Its twelve other methods still reach the mandatory
owned lower provider. Combined with B3/B4/SourceServices this supplies selected
source implementations for17/17 Services methods. This count is a composition
metric; it is not a full native-provider/DLL readiness claim.

## Explicit stream ownership and identities

BootstrapLogger requires an explicit shared_ptr<std::ostream>. The caller must
keep that stream's streambuf alive too; owning an ostream alone cannot adopt an
independently destroyed custom streambuf. Construction performs no stream I/O,
does not choose a global console, and does not register callbacks.

The original preferred-image stream value0x1800a4130 is an opaque observed
identity. It is never cast to a compiler ostream or dereferenced. A logger also
accepts its own stable byte-token address and returns that token after insert.
The immovable logger keeps token identity stable. Null/foreign stream identities
and unknown literal addresses are rejected before insertion; this adapter is
bounded to the actual bootstrap/fatal routes rather than an arbitrary native
ostream or address resolver.

| Literal VA | Exact retained C string |
|---|---|
| 0x180083cd8 | LSX emu |
| 0x180086720 | space + version + space |
| 0x180086718 | 2.0.1.0 |
| 0x180086700 | Unhandled exception |

The second literal's exact byte string is0x2076657273696f6e20. The four payloads,
terminators, hashes and retained data identity are in logging-evidence.json.
They came from the complete retained read_bytes record
ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082
at0x18007f000, not a fresh original-file read.

## Selected compiler stream policy

Retained006880 is formatted NUL-terminated C-string insertion. The selected
compiler implementation uses ostream operator<<(const char*) so width/fill and
stream-state handling remain with that compiler's stream implementation rather
than being replaced by unformatted write(). Retained0088e0 widens newline, puts
it and flushes; std::endl is the selected compiler counterpart.

No stream flags, exception mask, locale, width or fill are reset by the adapter.
Normal stream errors follow the selected ostream behavior: a masked failure may
set stream state while returning the token; enabled exceptions propagate. The
adapter does not invent a successful write check or swallow stream exceptions.

A per-logger recursive mutex serializes its calls and permits same-thread
streambuf callback reentry. Each insertion/finish is separately serialized; a
multi-part line is not made one atomic transaction. Other direct users of the
supplied ostream require their own synchronization. General native iostream
locking, cross-thread reentry/deadlock, concurrent locale mutation and lifetime
equivalence are not established.

These are rebuilt source compatibility choices. The original MSVC basic_ios
layout, vtable adjustment, streambuf/facet locking, locale/EH/fault semantics and
physical native stream pointer are not adopted or proved equivalent.

## Fatal and unexpected-return routes

terminate_log performs the retained fatal C-string insertion, widened
newline/flush, then compiler std::abort. Healthy logical output is
"Unhandled exception" followed by the selected stream's widened newline.
If insertion or finish throws, that exception propagates before the abort call:
the source sequence is preserved without inventing logging-success fallback.
What a native MSVC terminate callback does after such an exception remains an
EH/runtime boundary.

The original038890 calls038920 and has INT3 if it unexpectedly returns. The
selected unexpected_terminate_return route unconditionally calls compiler
abort, so it cannot depend on debugger continuation or pretend success.
This deliberately differs from original INT3/debugger/SEH behavior.

CompletionServices owns logger and shared exit-wait registry before its lower
Services member, so reverse destruction keeps those policy owners live through
lower-provider destruction. Worker/FLS stop and joins must precede destruction.
The registry retains its separate explicit close contract; this wrapper does
not silently close a shared registry or substitute an automatic worker join.

## Static anchors and limits

| Native address | Observation | Pseudocode Evidence ID |
|---|---|---|
| 0x180006880 | formatted C-string insertion | ev_fbcd6dd7e3f40d8a0949ddad6acfeb50aac3210ec0b8ffe4e7905d1397f3bb36 |
| 0x1800088e0 | widened newline/put/flush | ev_e32e8e2eddad4e3ec7d392028d39ca1acab03d86ac509c1bbffe7e88089f5f5a |
| 0x180038970 | three bootstrap insertions then finish | ev_e66705fdaa9bdd88fb50be13d22e354939026d931f472329e0f14e17c9fb4fbd |
| 0x180038920 | fatal insertion/finish then abort | ev_b35c015816e9d434a8d5dbb8a0296baf874922ffb138fb566a634535a3c178d3 |
| 0x180038890 | fatal call then INT3 fallback | ev_fa9e792ca1dc6003fa08b9f989066ed017af372bde3045f31207ef552c7e855b |

Target SHA256 is9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.
The retained bundle SHA is
c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495.
No original/user CFG/token/achievement input, blocked version file or historical
generated version DLL was read by this author; no original or rebuilt DLL was
executed or modified. Prior746 source files remain unchanged.

Native bodies/closures added0. Selected Services method coverage does not close
the remaining configuration, server/online/host/hook/foreign providers, full
mutable CRT/EH/ABI/concurrency/image-unload/loader-lock/PE-entry requirements.

## B10 ortak doğrulama

**73 çekirdek + 31 entegrasyon = 104 kontrol PASS.** Ortak build 227 nesne (223 C++ / 4 C), 251 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 746 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 753 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
