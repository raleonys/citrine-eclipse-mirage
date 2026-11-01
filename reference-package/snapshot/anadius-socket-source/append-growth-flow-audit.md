# N10 — 012350 /007e30 append growth kaynak ve alan denetimi

24 Ekim 2026. `append_growth.hpp/cpp` iki normal gövde ve `RecoveredMessageAppendApi` ile iki final lower-source bağlantısı ekler. N9 `NativeMessageApi::grow_nul_012350` /`grow_append_007e30` artık gerçek kaynak gövdelerine bağlanabilir; kontrollü008350/02b940 growth substitutes bu yeni zincirde seçilmez. Genel CRT üretimi değişmedi; heap/byte effects ve native ABI/EH tamamlanmadı.

## Kimlik ve kanıt

REA6.5.0 server/catalog hizası ve kullanılabilir Ghidra sağlayıcısı `append-growth-rea-session.json` ile doğrulandı. Yeni hedef açılmadı/yürütülmedi; Ghidra12.1.4 ile önceden alınmış listing yeniden kullanıldı. Bağlı anadius64 SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Tam listing ve kaynak hashleri `append-growth-evidence.json` içindedir. On bağlam yordamı yeni gövde sayılmaz.

| Gövde | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
| 0x180012350 | `ev_00bf42becc4f92a8d4b78d914a9566385470f50393b6531e88756e3dd467972a` | `ev_3fbac447ca491496cb5cd50e118f7d13550dafe9739d4478cec4459874537f70` |
| 0x180007e30 | `ev_2f324be7d0140daa9db7786faa691d87237a55cdc7aa18602f7d2fa5a7c84549` | `ev_2bb832e12c4c27a1cb7ea8eeacaddf030c000eb99f2042529c2826614681d5f3` |

## Adresli işlem sırası

| Native adresler | Gözlem ve yeni kaynak karşılığı |
|---|---|
| 01235e–01237c /007e3b–007e58 | Old size cached; maximum-old<extra length terminali eski kapasite/allocation öncesinde |
| 012387–0123c4 /007e63–007ea5 | Old capacity cached; newsize=old+extra, capacity=max(newsize\|15,oldcap+oldcap/2); taşma/fallback maximum |
| 0123ed/012411 /007ece/007ef2 | Her çağrı allocation yapar; spare/extra0 için erken dönüş yok.4096 ve üzeri byte sayısına+39,32byte alignment, data-8 raw header |
| 01241d/012424 /007efe/007f0e | Size ve capacity COPY'den önce yazılır |
| 012431 /007f1b | Old heap pointer allocation ve metadata stores sonrasında live olarak okunur; inline/heap seçimi cached oldcap ile |
| 012437/012478 | Old size kadar bir COPY, ardından oldsize konumuna yalnız bir char, oldsize+1 konumuna NUL |
| 012440/012444 /01247d/012481 | Char ve NUL ayrı bayt store'ları; extra kadar fill yapılmaz |
| 007f21/007f2f /007f6b/007f79 | Önce old payload, sonra source/count COPY. Size artışı extra, gerçek ikinci COPY uzunluğu count; NUL oldsize+count konumunda |
| 012453–01246e /007f46–007f61 | Tail/NUL sonrasında cached oldcap sized-free; büyük eski heap header sınırı olddata-raw-8≤31 |
| 012487 /007f83 | Yeni data pointer yalnız ilk8 storage baytına en son yayımlanır; free callback metadata değişimleri geri yazılmaz |
| 0124ab/0124b1/0124b7 /007fa8/007fae/007fb4 | Invalid/overflow/length terminalleri zorunlu alt sağlayıcılardır |

`012350(value,extra,hint,char)` yeni size'ı old+extra yapar ama yalnız **bir char ve bir NUL** yazar. Extra4 olduğunda kalan yeni logical baytlar doldurulmaz. `007e30(value,extra,hint,source,count)` extra/count eşitliğini varsaymaz; count daha küçük veya bounded allocation izin verirse daha büyük olabilir. Hint native kodda kullanılmaz. Bu gözlemler generic resize/replace ile sessizce normalleştirilmedi.

Yeni private allocation hazırlığı her iki assembly'nin ortak normal mantığını paylaşır; native fonksiyonlar birbirini veya008350'yi çağırıyor diye bir CALL iddiası yoktur. İlk old-payload COPY'den önce old pointer cached olur; sonraki COPY callback'i pointer metadata'sını değiştirse de free hedefi cached pointer'dır. Self-source yeni buffer'a COPY yapılana kadar eski allocation'da yaşar. Native078d30 COPY ve04b0bc/04b0f8 heap/terminal gövdeleri bu pakette yeniden kurtarılmaz.

## Doğrulama

**3.461 çekirdek +2.039 entegrasyon =5.500 yeni kontrol PASS.** Ortak build191 üretim nesnesi (187 C++/4 C),195 normal +19 terminal grup ve68 socket grubudur. Beş yeni kaynak/test dosyası; önceki591 kaynak/CRT byte olarak korunur, toplam596 kaynak snapshot girdisi vardır. Ortak build son otoritedir; standalone socket scripti ve CMake bu aşamada ayrıca çalıştırılmış sayılmaz. Yerel/common kayıtlar aynı build hashine bağlıdır.

Çekirdek oracle tüm allocation payload'ını bağımsız bir sentinel byte dizisiyle karşılaştırır: tüm256 karakter değeri, embedded NUL, inline/heap,0/1/7/15/16/31/63/4094/4095/8191 eski boyutları, spare0/8/100, extra0/1/7/16/4096, count/extra ayrımı, self-source ve eski terminatorun kopyalanması. Fixture'ın0xcc sentinel baytları yalnız yazılmayan alanları gözler; özgün native heap'in ilk içeriği olarak sunulmaz. Allocation rounding, her COPY sayısı/uzunluğu,4096/+39 header/sized-free ve ilk8 pointer yayını ayrı kontrol edilir.

Allocation failure metadata öncesinde; COPY failure yeni metadata/eski pointer durumunda; old-free failure tail/NUL sonrasında pointer yayını öncesinde sınanır. Allocator callback pointer/cap/size değiştirirse cached oldsize/cap ve live oldpointer ayrımı korunur. Free callback metadata'sı en sonda geri alınmaz. Küçük NULL allocation explicit terminale çevrilmez: metadata yazıldıktan sonra bounded COPY reddeder. Büyük NULL invalid terminali metadata öncesindedir. Char store tamamlanıp ayrı NUL store bounded failure verdiğinde char kalır. Büyük bozuk eski header COPY/tail sonrasında free öncesinde reddedilir. Symbolic maximum fallback exact allocation argümanı gözlenir, dev allocation yapılmaz;00cd30 normal-valid-state ulaşılabilirliği iddia edilmez.

Entegrasyon gerçek033d70'yi iki final lower-source override ile N1–N9 kuyruk/gönderim zincirine bağlar. Plain/encoded/version-retry/autodetect × borrowed/owned,13 yanıt, iki deque map growth,150000bayt payload, event'in ayrı NUL'u ve64KiB parçalar tam bayt oracle ile sınanır. N9 controlled growth sayaçları sıfır kalır.15/31/4095 tam kapasite plain yanıtlar gerçek012350'ye ulaşır;5000bayt event gerçek007e30'un31 ve10001bayt iki COPY'sini tetikler. Ertelenmiş task native source heap'ini yaşatır; queue COPY sonrası kaynak bırakılır. Gerçek012350 küçük NULL failure'ında033d70 local metadata görünür, output move/document cleanup yapılmaz.

## İlerleme ve açık sınırlar

İki adres önceki envanterde `no_definition_mapping` idi; şimdi ikisi de normal gövde +final provider tanımına ve compiled symbol kanıtına sahiptir. Anadius canonical2076 içinde kaynak eşlemeleri448→450, compiled address correlations449→451; normal-body claims78→80 olur. Bunlar tamamlanma yüzdesi değildir. Source closure15→17; selected compatibility0 ve tarihsel N5 alternatif1 korunur. Yeni sınıf aynı ReceiveBufferContext'i NativeMessageContext ile paylaşmalı ve ömür/bellek eşlemeleri tutarlı olmalıdır.

Allocator/free/COPY/terminal, XML/032d00,042480/042610 cipher, queue/mutex getter ve stream log sağlayıcıları native tarafta açıktır. Typed normal gövdeler MSVC STL/global/vtable/ABI/EH/SEH/concurrency veya native fault eşdeğerliği kurmaz. Exception durumuna otomatik rollback, free veya unlock eklenmez. Entegrasyon XML/cipher/log/mutex/completion/send sınırlarında kontrollü sağlayıcılar kullanır; gerçek DLL/oyun/network/thread yürütmesi yoktur. Aynı ajan denetimi yapılmıştır; bağımsız final reviewer PASS yoktur.

Orijinaller değiştirilmez, gerçek CFG/token/achievement içerikleri okunmaz. Önceden antivirüs tarafından engellenen orijinal version.dll okuması tekrar denenmez; güncel hash'i doğrulanmış sayılmaz. Eski üretilmiş version.dll ZIP dışında kalır; diğer dört girdinin güncel hash'i paketleme kaydında doğrulanır.

**N11:**042480/042610 live-prefix cipher normal akış ve kaynak bağlantıları; mevcut AES block kaynaklarını aynı mutable cipher/IV durumuyla birleştirmek. Genel CRT aşama5'e kadar bekler. Tam DLL henüz tamamlanmadı; ölçülmüş toplam yüzde veya güvenilir bitiş tarihi yoktur.
