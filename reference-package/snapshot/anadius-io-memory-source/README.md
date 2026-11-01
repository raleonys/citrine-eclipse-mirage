# B34 — Somut IO storage/projection ve completion sahibi

`OwnedIoMemory`, B33 `SessionMemoryDomain` üzerinde gerçek heap/PTD destekli IO
allocation'larını, typed operation/task/response projeksiyonlarını ve completion
ömrünü sahiplenir. Yeni kaynak bileşimidir; **yeni native gövde veya ABI kapanışı
sayılmaz**. Native mapped494/eligible498/normal131 ve52tarihsel kapanış aynı kalır.
Composition77kaynak/38nesne; ortak build251nesne(247C++/4C),297normalgrup+19terminal.

İki sabit binding tablosu storage ve dispatcher içindir. Ayrı host variant
projeksiyonları native ASIO/MSVC ABI overlay'i değildir. Raw allocation ve
sidecar OVERLAPPED kimlikleri ayrılır. Generation handle'ları ve cross-kind
cache kullanımında orijinal raw adres/extent korunur; değiştirilmiş public
descriptor kimliği veya span native free öncesinde reddedilir. Generation
controller handle içindir; kernel OVERLAPPED ABA/finality kanıtı değildir.

Pinned SDK wrapper her gerçek SDK buffer aralığı için parent byte loan ve **ek
source-control strong** tutar. Bu açık compatibility pin politikasıdır; exact
native counter trace'i veya native concurrency kanıtı değildir. Cancel isteği
completion değildir ve borrower bırakmaz. Yalnız gözlenen final completion
loan'ı bırakabilir; eski IoDispatcherEffects router'ı bunu bypass etmemelidir.

Completion Error'u binding/loan değişmeden kopyalar ve actual native cleanup'a
yönelir. Cleanup cache'e bırakabilir veya callback reentrant read başlatabilir;
eski record sonradan okunmaz. Bilinmeyen SDK/callback/native-free sonucu
poison+retention gerektirir. Parent allocation/free çağrısına girildikten
sonraki **her** exception,
bilinen parent bad_alloc/HeapFreeFALSE dâhil fail-stop kalır; IO metadata/lease
için reset veya otomatik recovery yolu yoktur. Context'ler hâlâ
borrowed'dır; dispatcher/handler/environment ömürleri son kullanıma kadar sürer.

Parent shared root/provider lease IO ömrünü korur. Operation bütün source-chain,
buffer view ve ref release'lerini kapsar. Kernel completion, worker stop/cache
terminal sırası ve detached raw-job drain ayrıca kanıtlanmalıdır. IO close
callback/cache drain sonrasında provider lease'i own lock dışında bırakır;
sonra parent close mümkündür. Her iki destructor için Closed zorunludur.
Bütün blocking GQCS/run_worker döngüsünü tek global parent Operation altında
tutmak, detached iş aynı domaine girmek istediğinde deadlock yaratabilir.
Üretim worker için bekleme, packet admission, completion ve terminal cache
scope sınırları B35+ assembly içinde ayrıca çözülmelidir.

## Doğrulama kapsamı

Reviewed runtime kayıtları: **77 core +57 integration =134 PASS**.
Core sayısı hazırlık aşamasında tahmin edilmez. Integration gerçek process
heap/FLS ve actual023000→0280d0/025c40/021d10/observed027710EOF,
frame025480→021770→028d30, GetConfig/XML ve plaintext/AES yollarını kullanır.
Eski receive fixture allocation sahibi kaldırıldı: concrete OwnedIoMemory
storage/projection ve pinned SDK wrapper kullanılır. Exact pending input free
reddedilir; cancel extra strong/pin'i korur. Reentrant read ve worker terminal
cache kimlikleri gözlenen callback drain'inden sonra kontrollü bırakılır.

Son weak actual017420→026290 yoluyla control'ü bırakır; ardından IO ve parent
heap/FLS kapanır. SDK transport/scheduling, FrameClosure ve response-job
adaptörü kontrollü test sağlayıcılarıdır; üretim server veya kernel/raw-job
quiescence kanıtı değildir. Selected synchronous serialized source graph için
IO storage bağlıdır; bu kapsam bütün server/acceptor/foreign ABI kapanışı değildir.

B33'ün856 kaynağından854 aynı; iki reviewed parent dosyası providerlease,
bufferpin ve control-ownership için değişti;4yeniyle860kaynak. Tam fresh build
zorunludur. **Tek güncel ortak log `work/b34-common-final-build.log`**; kesin
common sonuç finalizer bölümüne kaydedilir. Quick archive SHA çalıştırıldığı
haliyle tutulur; B33 canonical/native eşdeğerliği olduğu varsayılmaz.

B35 priority1 production ServerBackend/Transport/Scheduling+acceptor kaynak
grafiğidir. B31 attach/detach factory hook'ları bilinçli undefined kalır.
Loader/quiescence, selected0228c0, licensing/foreign ABI açık; generic CRT
paused. Toplam DLL yüzdesi bu sayaçlardan türetilmez. Yeni REA/orijinal byte
sorgusu, DLL/oyun çalıştırma veya gerçek CFG/token içeriği okuma yoktur.
Generated version DLL hariç tutma politikası değişmez.

## B34 ortak doğrulama

**77 çekirdek + 57 entegrasyon = 134 kontrol PASS.** Ortak build 251 nesne (247 C++ / 4 C), 297 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 854 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 860 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 494 adres, 498 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
