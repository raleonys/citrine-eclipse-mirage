<!-- PENDING-SOURCE-CHANGES -->
2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı. 296 nesne, 356 test grubu ve 19 framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.
<!-- END-PENDING-SOURCE-CHANGES -->

Güncel özgün loader profili — 2026-11-03, DERLENMEDİ: detach helper kaynakta tanımlı; özgün uygulama reason0 gibi ek iş yapmaz. Gerçek exit cleanup compiler CRT callback sırasındadır. Ek zorunlu pre-cleanup proof kaldırıldı. Attach gerçek image Resources tanımı eksik kalıyor. Önceki iki-symbol link kayıtları tarihseldir. Ayrıntı: `original-loader-profile.md`.

Güncel kaynak değişikliği — 2026-11-02, henüz derlenmedi/test edilmedi:
`require_loader_attach_runtime` artık bütün senkron başlangıcın `BOOL` sonucunu
döndürmekle yükümlüdür. `DllMain` ayrıca `ApplicationEntry` çağırmaz. Eski
`Runtime*`/Bound-runtime açıklamaları aşağıdaki tarihsel kayda aittir. Aynı
Runtime+Domain sahibinden gerçek kaynak başlangıcı için
`pre_unload::Controller::start_application` eklendi; bu yolun loader lock dışında
çalışma sözleşmesi devam eder. İki gerekli loader hook tanımı hâlâ eksiktir.
Kaynak kaydı: `../anadius-pre-unload-source/owned-attach.evidence.json`.
Son teslimin build/link/ZIP kayıtları bu kaynak değişikliklerini kapsamaz.

# B54 update

Current common build:946 sources,274 objects(270CPP/4C),337 groups+19 terminals.
Owned lower-free online component supplies24required+3owned-copy Effects routes
through existing10core/5boundary/7typedforeign/2Execution phase algorithms.
ExactCallbackStateView is bound once before source publication; SourceBackend
routes actual online worker/cleanup through it. Legacy path stays supported.
CodeLease and pre-cleanup foreign-user safety remain caller prerequisites.
After real joins/source exit, explicit lateforeignquiescence plus exactempty
runtime+Binding permits controller close. Lateproof is NOTretroactive object
safety; FreeFALSE logicalclear is NOTmodule unload success.
Closed owners stay throughResources retirement; conservative thrown-call poison
does NOTpreserve every retry offered by old Binding alone.
B52 configuration andB53 services explicitobject proofs remain; online_factory.o
is additionallyselected forSTATICFAILED-LINKDIAGNOSTIC. All3 object/source/
common-command/argv/verbose proofs required. CMake configured,NOTexecuted.
This is NOTloaderfactory reachability/nativeABI/EH/fullResources/readyDLL.
B53 history preserved. [B54 report](../anadius-online-factory-source/README.md).
Current common log:work/b54-common-final-build.log.

---

# B53 update

Current common build:942 sources,273 objects(269CPP/4C),335 groups+19 terminals.
Concrete owning remaining Services policies select existing application memory,
bootstrap logger, exit-wait cleanup and thread-error/RET/fatal source policies.
SourceBackend keeps its6 real worker/host/exit routes; Runtime registry supplies
the2 registrar routes. No giant Services lower or fake unused methods required.
Legacy Services remains supported. Narrow controller close follows real joins,
source exit cleanup, callback attachment close and registry finish.
The new make_runtime(Resources&) path uses direct new and consuming transfer;
the preserved public by-value constructor does not guarantee arbitrary
make_unique/forwarder constructor-failure safety for opaque SBO captures.
B52 explicit configuration factory selection is preserved; B53 additionally
selects the frozen services_factory.o for the STATIC FAILED-LINK DIAGNOSTIC.
Both object/source/common-command/argv/verbose proofs are required. CMake direct
sources are configured only, not executed. No loader factory reachability,
wholeResources/nativeABI/EH or successful PE output is claimed. DLL stays BLOCKED.
B52 history preserved. [B53 report](../anadius-services-factory-source/README.md).
Current common log:work/b53-common-final-build.log.

---

# B52 update

Current common build:938 sources,272 objects(268CPP/4C),333 groups+19 terminals.
Concrete owning configuration::Dependencies uses19 existing lower-free source
routes and2 mandatory narrow license/legacy-document effects; no giant lower.
Streaming visitor, source FilePass/hash/parser and seeded achievement paths stay.
The actual frozen configuration_factory.o is EXPLICITLY SELECTED by the failed
static-link diagnostic; its SHA/source/common-command/argv/verbose-consumption
are verified. Opt-in CMake directly configures the same CPP but is NOT executed.
This does NOT implement require_loader_* or prove loader factory reachability,
wholeResources, nativeABI/EH or successful PE output. No DLL is loaded/executed.
B31 gate remains blocked. B51 history preserved.
[B52 report](../anadius-configuration-factory-source/README.md).
Current common log:work/b52-common-final-build.log.

---

# B51 update

Current common build:934 sources,271 objects(267CPP/4C),331 normal groups+19 terminals.
Existing native Binding adds a sticky APPLICATION-only admission seal. Owner.close
seals new cached Application attempts before backend stop/join; an admitted startup SOURCE invocation
still live returns false/retry without backend stop, allowing existing workers to
finish. A live sealed Application attempt returns FALSE without provider/Errors
callbacks. Worker/exit/terminate and unbound/closed error behavior stay unchanged.
Source leases end before permanent Errors callbacks. Those opaque callbacks and
all machine-entry/image execution remain caller quiescence/lifetime obligations.
This is HOST admission policy, not foreign/rawjob/whole-image unload/loader proof.
No new production unit/body/trait. B31 reruns on actual B51 archive; DLL BLOCKED.
[B51 report](../anadius-application-admission-source/README.md).
Current common log:work/b51-common-final-build.log. B50 history preserved.

---

# B50 update

Current common build:932 sources,271 objects(267CPP/4C),329 normal groups+19 terminals.
Existing Stop preserves default AcceptOnly and adds selected SessionIo worker-owned
receive/send cancellation and posted source retirement. Actual observed packet,
source generation/submission and successful owner0 cleanup precede ACK; cancel
return is never terminal proof. All completion routes lease the admission epoch.
Posted work is software/native flag0 cleanup, never CancelIoEx. Independent new
admission/foreign/rawjob proof, real WSA/AcceptEx interoperability, fullResources,
nativeABI/EH/concurrency and loader remain open. No production unit/body/trait added.
B31 gate reruns on actual B50 archive; DLL remains BLOCKED.
[B50 report](../anadius-session-stop-source/README.md).
Current common log:work/b50-common-final-build.log. B49 gate history preserved.

---

# B49 update

Current common build:930 sources,271 objects(267CPP/4C),327 normal groups+19 terminals.
ACCEPT-only Stop posts NULL-OV controller wakes; actual CancelIoEx runs on the
original admitted worker after SDK issue returns. Exact source generation plus
submission sequence and real physical packet receipt precede normal native
cleanup acknowledgement. Cancel return/ERROR_NOT_FOUND is never terminal proof.
No fabricated accept completion or scheduler work decrement is introduced.
Software error packets are not kernel loans. Receive/send/rawjob/session drain,
real AcceptEx network interoperability, full Resources/native ABI/loader remain open.
B31 gate reruns on actual B49 archive; DLL remains BLOCKED.
[B49 report](../anadius-accept-stop-source/README.md).
Current common log:work/b49-common-v2-build.log. B48 gate history preserved.

---

# B48 update

Current common build:926 sources,270 objects(266CPP/4C),325 normal groups+19 terminals.
Concrete SourceBackend application-worker factory binds exact RuntimeOwner state.
Actual Domain worker constructs/runs/destroys Graph with prepared memory; dispatch
waits outside Parent until observed online+host completion. Started stop/failure/
timeout retains armed Graph/IO rather than inventing cancellation or drain.
Controller finalizes only after actual TrackingThreads join and source lease0.
Legacy server route remains. Full Resources, kernel/rawjob stop/drain, original
startup scheduling/EH/native ABI and loader hooks remain explicit/unproven.
B31 gate is rerun on actual B48 archive; DLL remains BLOCKED.
[B48 report](../anadius-application-worker-source/README.md).
Current common log:work/b48-common-final-build.log. B47 gate history preserved.

---

# B47 update

Current common build:922 sources,269 objects(265CPP/4C),323 normal groups+19 terminals.
Controller-owned PreparedMemory gives one distinct same-Domain NativeThreads worker
a move-only borrow; Graph/Policy/Profile/server construct and retire on worker.
Actual dedicated memory/provider release remains controller-only after observed
worker thread/FLS exit through duplicated native handle. Timeout retains ownership.
Legacy B46 same-thread assembly remains. No automatic full Resources/worker bridge,
stop policy, kernel/rawjob finality, native ABI/EH or loader proof is claimed.
B31 gate is rerun against actual B47 archive; DLL remains BLOCKED.
[B47 report](../anadius-worker-handoff-source/README.md).
Current common log:work/b47-common-final-build.log. B46 gate history is preserved.

---

# B46 güncellemesi

Ortak kaynak build:918 dosya,268 nesne(264C++/4C),321 normal grup ve19 terminal senaryo.
Somut SAME-THREAD Graph fabrikası Policy/Profile/NativeLogic/XML/Router/kripto ve
Parent-backed gerçek event ring'i kurar. StateView orijinal live referansları
tutar; Router span'leri her dispatch'te yenilenir. Opaque her çağrı guarded'dır.
Nativepop/free düzeni ve dış gerçek proof sonrası owned event retirement seçilir.
Nativebody0/ABIclosure0;501mapped/505eligible/139normal korunur.

Controller Domain/provider ile worker Policy yaşam döngüsünün iki aşamalı bootstrap
kurulumu, narrow domain effects, kernel/rawjob finality ve loader açık kalır.
B31 gate gerçek B46 arşivinde tekrar linklenir; DLL BLOCKED.
[B46 raporu](../anadius-application-graph-source/README.md).
Güncel ortak log: work/b46-common-final-build.log.
Önceki B45 gate kanıtları historical/ altında korunur.

---

# B45 güncellemesi

Ortak kaynak build:914 dosya,267 nesne(263C++/4C),319 normal grup ve19 terminal senaryo.
Somut SessionPolicy; NativeApi/category, Parent-owned hata metni, gerçek shared
ostream log, scoped guard, zero-only worker exception ve dedicated exit-wait
uygulamasını bağlar. Low32 wake same-worker serialized HOST seçimidir.
Gözlenen owner/after/worker-return/external-drain dört REQUIRED lifecycle kararı
olarak kalır. Native pending-exception/MSVC/GS TLS/atexit/ostream ABI kapanışı yok.
Nativebody0/ABIclosure0;501mapped/505eligible/139normal korunur.

Tam Resources/domain/config/event, gerçek rawjob/kernel finality ve loader
bağlantıları açık. B31 gate gerçek B45 arşivinde tekrar linklenir; DLL BLOCKED.
[B45 raporu](../anadius-session-policy-source/README.md).
Güncel ortak log: work/b45-common-final-build.log.
Önceki B44 gate kanıtları historical/ altında korunur.

---

# B44 güncellemesi

Ortak kaynak build:910 dosya,266 nesne(262C++/4C),317 normal grup ve19 terminal senaryo.
Boş Jobs/receive/send için Profile somut InlineJobs ve mevcut NativeAsyncReceive/
NativeAsyncSend adaptörlerini seçer. Typed SDK importları constructor'da çağrılmaz.
Jobs exact-once aynı worker inline, source-only ticket ve strict callback/drain
policy kullanır; native CRT detached thread/OS HANDLE eşdeğerliği değildir.
Jobs/Env/Deque lease'leri gerçek source drain ve dış attestation sonrası bırakılır.
Nativebody0/ABIclosure0;501mapped/505eligible/139normal korunur.

Tam Resources, hata/log/worker ve domain/config/event politikaları, gerçek rawjob/
kernel finality ve loader bağlantıları açık. B31 gate gerçek B44 arşivinde tekrar
linklenir; DLL BLOCKED. [B44 raporu](../anadius-owned-jobs-source/README.md).
Güncel ortak log: work/b44-common-final-build.log.
Önceki B43 gate kanıtları historical/ altında korunur.

---

# B43 güncellemesi

Ortak kaynak build:906 dosya,265 nesne(261C++/4C),315 normal grup ve19 terminal senaryo.
Boş Environment varsayılanında Profile kendi tablo/type/executor/scheduler/copy
bağlarını kurar. Context-free thunks capability resolver kullanır; env refs ve
source graph drain doğrulanmadan backing/Parent lease bırakılmaz.
Owned handler inline, response022e40forcepost1 seçilmiş HOST politikasıdır;
native0228c0/RTTI/vtable/EH/concurrency eşdeğerliği değildir.
Nativebody0/ABIclosure0;501mapped/505eligible/139normal korunur.

Tam Resources, kalan üretim sağlayıcıları, gerçek rawjob/kernel finality ve loader
bağlantıları açık. B31 gate gerçek B43 arşivinde tekrar linklenir; DLL BLOCKED.
[B43 raporu](../anadius-owned-environment-source/README.md).
Güncel ortak log: work/b43-common-final-build.log.
Önceki B42 gate kanıtları historical/ altında korunur.

---

# B42 güncellemesi

Ortak kaynak build:902 dosya,264 nesne(260C++/4C),313 normal grup ve19 terminal senaryo.
Profile yanıt kuyruğu büyümesi mevcut026670 üzerinden Parent-owned typed map
sağlayıcısına bağlandı. Parent LIVE map defteri, doğrudan026880 disposal dahil,
kaydı backing free callback'inden önce kaldırır. Provider lease harici kapanış
kanıtı ve session/job retirement sonrasında Runtime drain kontrolünden önce kapanır.
Native body0/ABIclosure0;501mapped/505eligible/139normal korunur.

Tam üretim sağlayıcı/Resources, gerçek job/kernel finality ve loader bağlantıları
hâlâ açık. B31 gate gerçek B42 arşivinde yeniden linklenir; DLL BLOCKED.
[B42 raporu](../anadius-owned-deque-source/README.md).
Güncel ortak log: work/b42-common-final-build.log.
Önceki B41 gate kanıtları historical/ altında korunur.

---

# B41 güncellemesi

Ortak kaynak build:898 dosya,263 nesne(259C++/4C),311 normal grup ve19 terminal senaryo.
Somut native_profile::Logic, gerçek typed033d70 gövdesini canlı request/output ve
aynı session cipher/flag durumuna bağlar. Greeting ve nested XML/printer/node/
cipher/random/effects sağlayıcı sınırları kontrol edilir; unknown prefix tutulur.
Seçilen XML/DOM/printer/registry/032d00 C++ semantik sağlayıcıları ve normal sonuna
ertelenen opaque destructor ömrü açık host politikasıdır; native ABI/EH kapanışı değildir.
Yeni native gövde0/ABIclosure0;501mapped/505eligible/139normal korunur.

Tam üretim sağlayıcı/Resources seçimi, job/kernel finality ve loader bağlantıları
hâlâ açıktır. B31 gate gerçek B41 arşivinde tekrar linklenir; DLL BLOCKED.
[B41 raporu](../anadius-native-profile-source/README.md).
Tek güncel ortak log: work/b41-common-final-build.log.
Önceki B40 gate kanıtları historical/ altında korunur.

---

# B40 güncellemesi

Ortak kaynak build:894 dosya,262 nesne(258C++/4C),309 normal grup ve19 terminal senaryo.
Concrete Profile/Application frame/read/write/response/message bağlamlarını ve
sabit kapasiteli session kayıtlarını kurar. Registry/observer ve ayrı frame-job
strong pin ömrü yönetilir; geç runner kopyaları tombstone üzerinden reddedilir.
Yeni native gövde0/ABIproviderclosure0;501mapped/505eligible/139normal korunur.

Mesaj alt sağlayıcısı normal-return compatibility'dir; native033d70 ara-prefix/EH,
gerçek detached concurrency, native scheduler/OS/job/kernel finality ve tam bootstrap
Resources/loader bağlantıları hâlâ açıktır. Özgün dosya veya DLL çalıştırılmadı.
B31 gate actual B40 arşivinde tekrar linklenir; DLL BLOCKED.
[B40 raporu](../anadius-session-profile-source/README.md).
Tek güncel ortak log: work/b40-common-final-build.log.
Önceki B39 gate kanıtları historical/ altında korunur.

---

# B39 güncellemesi

Ortak kaynak build:888 dosya,260 nesne(256C++/4C),307 normal grup ve19 terminal senaryo.
Somut server Runtime; actual0298b0/027c10 worker, full session/coherent IO ve
socket yardımcılarını birleştirir. make_server/install_server yalnız boş
Resources.server bileşenini kurar. Yeni native gövde0, ABI/provider kapanışı0.
501mapped/505eligible/139normal korunur.

Kalıcı graph, kopyalanmış ServerView, run-local admitted proxy yaşam süresi ve
harici sonlanma + kaynak koşullarıyla kontrollü retirement vardır. Application/Profile
mesaj/read/write bağlamları, N5/native033d70 farkı, raw-job/kernel finality,
tam bootstrap Resources ve loader attach/detach halen açıktır.
B31 gate gerçek B39 arşivinde yeniden linklenir; DLL BLOCKED.
[B39 raporu](../anadius-server-runtime-source/README.md).
Tek güncel ortak log: work/b39-common-final-build.log.
Önceki B38 gate kanıtları historical/ altında korunur.

---

# B38 güncellemesi

Ortak kaynak build:882 dosya,258 nesne(254C++/4C),305 normal grup ve19 terminal senaryo.
029df0 socket fabrikası,029590 holder reset ve029660 retry normal gövdeleri eklendi.
501mapped/505eligible/139normal;3 yeni gövde,0 native ABI/provider kapanışı.

Sahipli retry primary/null-cancel profilinde checked prearmed SDK sınırını kullanır.
Generic helper erken995 bildiriminden sonra devam eder; sahipli çoklu yayın ve
alternate packet finality profilleri açık olduğundan reddedilir.
N5 normal-return message bridge ile native033d70 prefix/EH kapsamı ayrıdır.

B31 attach/detach hooks undefined kalır; gerçek gate B38 arşivinde yeniden
linklenir. DLL BLOCKED. FullRuntime/Resources/Transport/Scheduling,
message/context/raw-job/kernel drain ve loader/ABI/EH halen açık.
[B38 raporu](../anadius-accept-helpers-source/README.md).
Tek güncel ortak log: work/b38-common-final-build.log.
Önceki B37 gate kanıtları historical/ altında korunur.

---

# B37 güncellemesi

Ortak source build:876kaynak,256nesne(252C++/4C),303normal/19terminal.
Full025720 normal body,021a10 private-error shim ve0270e0 rearm eklendi.
025720 eski coarse2 tanımı korur; yeni body üçüncü varyanttır. Coverage
498mapped/502eligible/136normal; yeni normal3, yeni nativeABIclosure0.

Coherent memory/router B35/B36 graph üzerinde cache retirement ve observed
SDK pins'i korur. N5 message bridge normal-return compatibility'dir; N9
native-prefix callback/EH eşdeğerliği değildir. Raw jobs ve terminalaccept
owner0 shutdown kontrollü host politikalarıdır; native quiescence kanıtı yoktur.
Native200-byte cipher transferinin198 tanımlı baytı kurulur; padding2 ve ABI açık.

B31 attach/detach hooks undefined kalır. Actual gate B37 archive üzerinde
yeniden linklenir; CMake target executed sayılmaz. DLL BLOCKED.
Socketfactory/reset/retry, fullRuntime/Resources/Transport/Scheduling,
native-prefix message/context/loader/EH/ABI/concurrency açık kalır.
[B37 raporu](../anadius-session-flow-source/README.md).
Tek güncel ortak log `work/b37-common-final-build.log`.
Önceki B36 sonuçları historical/ altında korunur.

---

# B36 güncellemesi

Güncel ortak build:870kaynak,254nesne(250C++/4C),301normalgrup/19terminal.
Actual021b00/0271a0 iki normal native kaynak olarak eklendi; mapped496,
eligible500, normal133. Scoped parent lease ve distinct typed accept/kernel
tampon ömrü B35 owning server/admitted worker grafiğine bağlandı.
Sadece normal kaynak iddiasıdır; yeni native ABI/provider kapanışı0.

Runtime gerçek packet finality ve helper/executor/context drain bağlarını
sağlamalıdır. 025720 initial/response composition, fullTransport/Scheduling/
Resources factory, native concurrency/foreignABI/EH ve loader açık kalır.
B31'in attach/detach hook'ları undefined; gate güncel B36 archive üzerinde
gerçekten yeniden linklenir. CMake target executed sayılmaz. DLL BLOCKED.

[B36 raporu](../anadius-accept-source/README.md).
Tek güncel ortak log: `work/b36-common-final-build.log`.
Önceki B35 sonuçları historical/ altında korunur.

---

# B35 güncellemesi



Son ortak source build B35:252nesne(248C++/4C)/299normalgrup/19terminal.

B34'ün860 kaynağı aynı;0priorchange+4yeni=864trackedsource. OwnedServerBackend

selected host context/IOCP/service/acceptor/pending/session/IO grafiğini sahiplenir.

AdmittedDispatcherApi whole-worker ROOT scope taşır; yalnız physical get dışında

SDK error'u yakalayıp lookup/completion/cache öncesi tekrar admission sağlar.



Native0241b0 ve021b00/0271a0 recovered değildir. Runtime accept/run/quiescence

lower'ları mandatory; trusted drain+sourcechecks gerekir. Full Transport/

Scheduling/Resources factory, kernel/rawjob/nativeconcurrency/foreignABI açık.

B31 attach/detach hooks bilerek undefined; değişmeyen gate B35 artifacts üzerinde

tekrar linklenir, CMake target executed sayılmaz. DLL hâlâBLOCKED.

[B35 raporu](../anadius-owned-server-source/README.md).

Tek güncel ortak log `work/b35-common-final-build.log`.

Önceki B34 ve daha eski kayıtlar historical/ altında korunur.



---



# B34 güncellemesi

Son ortak kaynak build'i B34:251nesne(247C++/4C)/297normalgrup/19terminal.
B33'ün856 kaynağından854 aynı;2reviewed parentediti+4yeni=860trackedsource.
OwnedIoMemory gerçek session-domain IO storage/projection ve observed-completion
loan router sağlar. SDK extra strong/buffer pin açık compatibility politikasıdır;
cancel request completion değildir. Kernel/raw-job drain, borrowedcontextömrü,
productionserver/accept/executor/loader/foreignABI grafiği açık kalır.

Bu son failed-link kaydı değişmeyenB31gate'in B34source/archive üzerindeki
tekrarıdır; CMake hedefi yürütülmüş sayılmaz. İki production attach/detach
factory hook'u bilerek undefined kalır. DLL hâlâBLOCKED.
[B34 kaynak/kanıt raporu](../anadius-io-memory-source/README.md).
Tek güncel ortak log: `work/b34-common-final-build.log`.
Önceki B33 ve daha eski kayıtlar historical/ altında korunur.

---

# B33 güncellemesi

Son ortak kaynak build'i B33:250nesne/295normalgrup/19terminal senaryo. 850eski
kaynak aynı;2readonlybacking-gözlem editi ve4yeni dosyayla856trackedsource.
SessionMemoryDomain gerçekheap/PTD session/control/proxy/queue/buffer/context
sahibidir; IOoperation/handlerstorage,kernelborrower/rawjobdrain ve tamserver
grafiği hâlâaçıktır. Bu klasördeki son failed-link kaydı değişmeyenB31gate'in
B33source/archiveüzerindeyenilenmesidir; CMake hedefi yürütülmüşsayılmaz.
[B33 kaynak/kanıt raporu](../anadius-session-memory-source/README.md).
Önceki B32/B31 kayıtları historical/ altında korunur. DLL hâlâBLOCKED.

---

# B32 güncellemesi

Son ortak kaynak build'i B32:249nesne/293normalgrup/19terminal senaryo. 846 önceki
kaynak ve iki B31 gate kaynağı korunur; dört B32 kaynak dosyasıyla852trackedfile.
B32 actual026880/026570/017420/026290 ve somutRecoveredSessionControl'ü ekler;
üretim allocator/free/contextdomain, executor/accept/server/raw-job/loader bağları açıktır.
Bu klasördeki son validation/source-verification, B32 common archive/snapshot
üzerinde tekrar edilen failed-link tanısıdır. Gate kodu B31'dekiyle aynıdır;
CMake hedefi hâlâ yürütülmemiştir. ToplamDLL yüzdesi bilinmiyor ve DLL BLOCKED.
Eski B31 kayıtları historical/ altında, asıl kaynak gelişimi
[B32 raporunda](../anadius-session-dispose-source/README.md) bulunur.

---

# B31 — DLL bağlama hedefi ve gerçek üretim engelleri

30 Ekim 2026. **DLL üretimi BLOCKED.** Bu paket çalışan bir anadius64.dll değildir.
Önceki B30 kaynak/test paketi korunur. Yeni teslimat, DLL hedefini gerçek kaynak
köklerine bağlayan ve eksik üretim bileşimini görünür kılan bir entegrasyon kapısıdır.

## Ne yapıldı?

- Ana CMake projesine varsayılanı kapalı `RECOVERED_ENABLE_DLL_LINK_GATE` seçeneği
  ve `anadius64_reconstruction` SHARED hedefi eklendi.
- Compiler CRT'nin `DllMainCRTStartup` girişi seçildi. Recovered CRT entry ikinci
  kez çağrılmıyor. Bu seçim özgün MSVC CRT/ABI eşdeğerliği iddiası değildir.
- Aday `DllMain`, zorunlu üretim attach factory'sinden gelecek gerçek Runtime
  üzerinden uygulama girişini çağırıyor. Thread reason'larında uygulama işi yok.
- `CDX @1`, `RUNE @2`, `anadius @3` aynı assembly RET leaf'ine alias olarak
  tanımlandı. Leaf'in nesne kodu tek `C3` talimatıdır; dönüş değeri uydurulmadı.
- Gerçek LLVM-MinGW compile/link denendi. Linker tam iki zorunlu entegrasyon
  sembolünün tanımı olmadığı için durdu. **DLL ve son link map oluşmadı.**
- Linker kaydı `bootstrap_factory`, `bootstrap_owner`, `bootstrap_native`,
  `bootstrap` ve `bootstrap_backend` dahil 25 core arşiv üyesinin seçildiğini
  gösteriyor. Bu, arşivi yalnız komut satırına eklemekten daha somut bir kontroldür;
  bütün üretim sağlayıcılarının kurulmuş olduğunu kanıtlamaz.

## Sonuç nasıl okunmalı?

`validation.json` içindeki `PASS_EXPECTED_BLOCKER`, tanılamanın beklenen eksikliği
doğruladığı anlamına gelir. Normal build **exit 1** verir. DLL derlemesi başarılı
sayılmaz; toplam DLL hazırlık yüzdesi ölçülmüş değildir. İki undefined sembol,
iki native fonksiyon kaldığı anlamına gelmez: ikisi de bu pakette tanımlanan yeni
entegrasyon sınırlarıdır ve çok sayıda zorunlu sağlayıcıyı temsil eder.

1. `require_loader_attach_runtime`: tek sahibi olan somut Resources grafiği,
   başlangıç değerleri, kalıcı Errors/Binding, worker/callback ömrü ve attach
   rollback politikası gerekir. Mevcut Runtime kontrol sözleşmesi loader lock
   dışındadır; onu doğrudan burada kurup publish etmek sözleşmeyi ihlal eder.
   Null yalnız tam rollback sonrası DLL yükleme başarısızlığıdır.
2. `require_loader_detach_quiescence`: normal FreeLibrary, süreç sonlanması,
   başarısız attach ve CRT yıkım sırası ayrılmalıdır. Loader altında worker join
   veya kanıtlanmamış Runtime::close çağrısı eklenemez.

`DllMain` noexcept'tir; yanlış phase sözleşmesinin exception'ı DLL ABI'sinden
çıkmaz, terminal olur. Bu, native EH davranışının çözüldüğü anlamına gelmez.
Nihai PE oluşmadığı için EAT alias RVA'ları, importlar, TLS ve unwind doğrulanmadı.

## Tekrar çalıştırma

Önce B30 `dll-reverse-engineering.zip` paketini açın; bu ek paketi aynı outputs
ağacına uygulayın. Kaynaklar, kayıtlı arşivler ve LLVM-MinGW toolchain mevcut olmalı.
PowerShell betiği 846 dondurulmuş kaynak hash'ini, common build kaydını, compiler'ı
ve iki arşivi doğrular; uyumsuzlukta link denemesine geçmez.

```powershell
# Normal build: üretim factory tanımları eksikken exit 1 beklenir.
& ./outputs/dll-link-gate-source/build-link-gate.ps1
# Tanılama: beklenen blocker doğrulanınca exit 0; DLL başarısı anlamına gelmez.
& ./outputs/dll-link-gate-source/build-link-gate.ps1 -Diagnose
python ./outputs/dll-link-gate-source/verify-link-gate.py
```

CMake kurulu ve LLVM-MinGW ile yapılandırılmış bir Windows x64 build'de:

```text
cmake -S outputs/source-workspace -B work/cmake-dll -G Ninja -DCMAKE_C_COMPILER=<toolchain>/bin/clang.exe -DCMAKE_CXX_COMPILER=<toolchain>/bin/clang++.exe -DRECOVERED_ENABLE_DLL_LINK_GATE=ON
cmake --build work/cmake-dll --target anadius64_reconstruction
```

Bu makinede CMake/Ninja bulunmadığı için CMake hedefi çalıştırılmadı. Eşdeğer
PowerShell compiler/link komutları çalıştırıldı ve tam argümanları kaydedildi.
291 eski test grubu bu aşamada yeniden çalıştırılmadı; son ortak build B30'dur.
846 eski CPP/HPP kaynağı ve canonical native envanteri değişmedi. Yeni native
ABI/provider kapanışı 0; generic CRT kuyruğu duraklatılmıştır.

Öncelikler ve kabul ölçütleri `roadmap.json`, bağımsız inceleme
`production-root-review.md`, kaynak/link denetimi `source-verification.json`
içindedir. REA 6.5.0 ve kullanılabilir Ghidra 12.1.4 oturum kimliği kontrol edildi;
bu aşamada orijinal hedef açılmadı, okunmadı, çalıştırılmadı veya değiştirilmedi.
Yeniden oluşturulan DLL/oyun da çalıştırılmadı; gerçek kullanıcı verileri okunmadı.
