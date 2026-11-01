# B37 — Tam kabul-oturum akışı

`flow.cpp`, retained Ghidra C/ASM üzerinden üç normal gövde kurar:
`0x180021a10` private Error kopyası, `0x180025720` tam kabul/oturum başlangıcı
ve `0x1800270e0` yeniden accept başlatma. Önceki025720 iki coarse tanımı korunur;
yeni tam gövde üçüncü varyanttır. Bu adres zaten derlenmiş etiket taşıdığı için
mapped/eligible artışı yalnız iki helper'dan gelir. Kanıt/adres/hash listeleri
`evidence.json` ve retained function kayıtlarındadır.

`025720`, gerçek023000 factory ardından local reset durumunun198 baytlık tanımlı cipher
bölgesini kopyalar; destination kendi AES schedule referansını korur. Native200-byte
kopyanın tail2 padding'i ve C++ vptr/class ABI'si modellenmez. Gerçek polymorphic
nesneye memcpy uygulanmaz. Greeting returned-ref data/length bir kez yakalanır;
02a200 gönderimlerinde socket/state canlı okunur, parça üst sınırı65536'dır.
LOCAL output string send'lerden sonra canlı alanlarıyla free edilir; sonra actual
0280d0 başlar. Local control ancak read sonrası okunup strong/weak salınır;
son rearm için original handler.server tekrar canlı okunur. Error log/free ve
throw prefix sıraları korunur; unknown native EH rollback uydurulmaz.

`CoherentIoMemory`, accept/read/write/handler/response aynı worker cache'ini
kullandığında önce accept sidecar'ını sonlandırır. 41 baytlık task için 73 bayt
Posted projection zorlanmaz. Cache eviction/free ve completion tek adaptörden
geçer; Base generation/observed-pin/poison denetimleri korunur. Eski completion
API'leri eski Base-memory davranışıyla çalışmaya devam eder. Environment lower
öncesi/sonrası hem accept hem Base phase'i denetlenir; yakalanıp yutulan Base
poison yeni I/O yayınına geçemez. Own-child237 testi ve final guard'ı çıkarılmış
kopya kaynağın beklenen temiz başarısızlığı bu farkı doğrular.

Entegrasyon gerçek own IOCP/heap/FLS üzerinde actual factory, uzun greeting,
frame/read, message-thread, owned response/send ve disposal kaynaklarını çalıştırır.
Plaintext ve bağımsız çözülen şifreli yanıtlar sınanır. **Mesaj çekirdeği N5
normal-return compatibility writeback'tir; N9 prefix-message callback/EH eşdeğerliği
değildir.** Raw frame job kontrollü senkron yürürken source handler strong'ı canlıdır.
İkinci accept'in gözlenen terminal packet'i için owner0 shutdown yönlendirmesi
seçilmiş host politikasıdır; native kernel quiescence kanıtı sayılmaz. SDK network
çağrıları enjekte edilir; gerçek ağ/özgün DLL/oyun çalıştırılmaz.

Borrowed context/provider ömürleri, aynı domain ve FlowServer identity eşleşmesi
çağıranın sözleşmesidir. B35 whole-worker admission ve tek worker profili gereklidir.
Heap/header okumaları yalnız bounded Parent bölgelerindedir. Beş önceki kaynakta
nonvirtual/friend ekleri vardır; data/vptr layout değişikliği iddia edilmez.

Quick: 546 core + 179 integration = 725 kontrol PASS.
Kaynak876; nesne256(252C++/4C); normalgrup303+19framework terminal hedeflenir.
Native498mapped/502eligible/136normal;3normal gövde,2yeni adres,0ABIclosure.
Tam DLL BLOCKED: helper factory/reset/retry, native-prefix message binding,
fullRuntime/Resources/Transport/Scheduling, nativeABI/EH/concurrency ve loader açıktır.
Genel DLL yüzdesi bu sayılardan türetilmez; generic CRT beklemededir.
Sonraki B38,029df0/029590/029660 alt sınırlarıdır. Tek güncel ortak log
`work/b37-common-final-build.log`; ortak build sonucu aşağıya eklenir.
Özgün dosyalar değiştirilmedi; yeni target byte sorgusu/CFG-token içeriği okuma
yoktur. AV engelli version input/output tekrar denenmez.

## B37 ortak doğrulama

**546 çekirdek + 179 entegrasyon = 725 kontrol PASS.** Ortak build 256 nesne (252 C++ / 4 C), 303 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 865 kaynak aynı; 5 önceki dosya açık hash kaydıyla güncellendi; toplam 876 kaynak. Yeni typed normal gövde 3, yeni native ABI/provider kapanışı0; 498 adres, 502 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
