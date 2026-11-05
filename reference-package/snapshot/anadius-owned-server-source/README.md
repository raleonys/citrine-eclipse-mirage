<!-- PENDING-SOURCE-CHANGES -->
2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı. 296 nesne, 356 test grubu ve 19 framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.
<!-- END-PENDING-SOURCE-CHANGES -->

# B35 — Sahipli server backend ve dispatcher admission

`OwnedServerBackend`, mevcut `ServerBackend` sözleşmesini somutlaştırır ve
`Resources.server` alanına doğrudan uyabilir. Core dependency/state ömrünü,
selected host IOCP/context policy'sini, native gate'leri ve actual024990 scheduler,
0228e0 service,0211b0/024db0 acceptor+pending,026fb0/026e50 close yollarını sahiplenir.
SessionMemoryDomain/OwnedIoMemory ve actual023000 accepted factory yeniden kullanılır.
**Native0241b0,021b00/0271a0 veya class ABI recovered sayılmaz.**

`Runtime` üç mandatory lower taşır: distinct accept operation/endpoint/kernel
borrower sahibi submit_accept, explicit source run policy ve truthful external
quiescence attestation. Accept, receive operation'a alias edilmez. Backend
SourceBackend Resources.server bağlantısına uygundur; full Resources factory,
Transport ve Scheduling sınıfları tamamlanmış değildir.

## Whole-worker admission

`AdmittedDispatcherApi` parent Operation'ı bütün canonical source worker ve
dispatch zinciri için tutar. Yalnız fiziksel get sırasında bırakır; SDK error'u
hemen yakalar ve packet lookup/queued handshake/completion/cache erişiminden
önce tekrar girer. Her SDK girişinde parent PTD/phase doğrulanır; bu API'nin
metadata/PTD admission kontrolü SDK LastError'ını korur. Broad B33 mutex/Operation
LastError koruması iddia edilmez. B34'ün yalnız
completion etrafında scope açan kontrolsüz dispatcher yolu kullanılmaz.

Kapsam tek active worker ile açıkça sağlanan foreign callback PTD/API/context
ömür politikasıdır. Arbitrary lower, native/global concurrency veya foreign ABI
kanıtı değildir. OwnedIoMemory extra SDK strong/range loan hâlâ explicit
compatibility politikasıdır; exact native refcount trace'i veya cancel=drain değildir.

## Stop ve cleanup

Backend admission durur; Runtime.quiescent ROOT dışında doğru drain bilgisi
vermelidir. Sonra source allocation/pin/work/queue sayaçları ve service'in yalnız
acceptor+pending node'ları doğrulanır. Sayaçlar attestation'ın yerine geçmez.
False quiescence yeni admission'ı durdurur, bilinen cleanup retry mümkün olabilir.
Nested source hatası yakalanıp lower true dönse bile stickyPoison attested
olamaz;024db0'dan ve attested flag'den önce kontrol edilir. Attested olduktan
sonra callback admission sealed kalır. Unknown native/kernel prefix'te tahmini
rollback/destructor cleanup yapılmaz. Closed destructors zorunludur.

## Doğrulama

**24 core +122 integration =146 kontrol PASS.** Core iki ownchild
unknown physicalGet/Post prefix'ini retained state ile doğrular ve gerçek GQCS
wait aralığında supplied foreign callback PTD allocation/free yolunu sınar.
Integration gerçek source029ea0+Execution port/readiness yayını, IPv4 fallback
ve IPv6 seçimi, own IOCP/gates/WSA/heap/FLS/TLS ile actual023000 factory,
022e40 posted operation→0298b0/027c10→OwnedIoDispatcherEffects→027120 ve actual
session disposal/cache free yolunu çalıştırır. Stop relay packet'i admitted get
ile gerçekten drain edilir. Synthetic accept obligation ayrıca bitirilir;
quiescence true kaynak callback/cache/session/accept protokolü sonrasında verilir.

Bir own child, quiescence lower'ın nested with_operation hatasını yakalayıp
true dönmesini sınar: backend poisoned/attestedfalse kalır, socket/list/executor
ve sonraki teardown/accept etkileri oluşmaz. Failed prefix child process'te
bırakılır; native EH rollback uydurulmaz. Gerçek socket bağlantısı kurulmaz.

860 önceki kaynak byte-for-byte aynı;4yeniyle864kaynak,252nesne(248C++/4C),
299normalgrup+19framework terminal. Composition79kaynak/39nesne ve65compiler
observation. Native494mapped/498eligible/131normal/52historical değişmez; yeni
native body/ABI closure0. Son native body paketi B32'dir. Tek güncel ortak log
`work/b35-common-final-build.log`; ortak finalizer sonucu aşağıda kaydedilir.
Quick archive SHA'sı gerçek kullanıldığı haliyle tutulur; canonical varsayılmaz.

B36 priority1 actual021b00/0271a0 accept source, typed operation/kernel endpoint
buffer/pending socket drain ve existing accepted factory025720/response bağlantısıdır.
0228c0 tekrar audit/uydurma ve generic CRT yok; B31 attach/detach hooks bilinçli
undefined, loader/quiescence/foreign ABI açık. DLL BLOCKED; toplam yüzde uydurulmaz.
Orijinal/rebuiltDLL/oyun çalıştırılmadı; yeni REA byte sorgusu veya gerçek CFG/token
okuması yok. Dört input hash doğrulaması/bir unavailable ve generated version DLL
hariç tutma politikası değişmez; AV engeli tekrar denenmez.

## B35 ortak doğrulama

**24 çekirdek + 122 entegrasyon = 146 kontrol PASS.** Ortak build 252 nesne (248 C++ / 4 C), 299 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 860 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 864 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 494 adres, 498 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
