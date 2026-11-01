# N9 — 033d70 typed normal mesaj akışı

24 Ekim 2026. Yeni `native_message.hpp/cpp` bir normal akış gövdesi ve `RecoveredNativeMessageApi::process_message_033d70` çağırıcı kaynak köprüsü ekler. `028d30` artık N5'in normal dönüşte request writeback yapan semantic adapter'i yerine bu gövdeye bağlanabilir. Eski semantic033d70 ve N5 kaynakları korunur; unique adres sayısı artmaz. Tam DLL/native ABI veya tüm alt sağlayıcılar tamamlanmış değildir.

## Kanıt kimliği

REA6.5.0 sunucu/catalog hizası ve kullanılabilir Ghidra sağlayıcısı `native-message-rea-session.json` ile doğrulandı; yeni hedef açılmadı/çalıştırılmadı. Önceden alınmış Ghidra12.1.4 dökümleri anadius64 SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` kimliğine bağlıdır. Tam listing hashleri ve on yeniden kurtarılmış sayılmayan bağlam işlevi evidence JSON'undadır.

033d70 assembly Evidence: `ev_971ebfa9e879b1b543e464bfc367047eed787a968cfaef065ce973d0b4f34bd6`.
Pseudocode Evidence: `ev_7a38e9dc5496630753954e74801f942d9111225ca95c1c603a4ae22b88f504df`.
Assembly, decompiler değişken alias/reuse ayrıntılarında belirleyicidir. `native-message-evidence.asm.txt` ve `.pseudo.c.txt` tam hedef bölümünü korur.

| Adres / çağrı | Gözlem ve kaynak karşılığı |
|---|---|
| 033db5→001830 | Tek document constructor(1,0); retry döngüsü aynı document'i kullanır |
| 033de5–033e00 | Live encrypted+c4 kontrolü; gerçek03ce30 yerinde decode, ardından042480(handler,request,0) |
| 033e26→004280 /033e2b | Live input data ve count=-1 parser'a gider; success parser return'den değil document error+6c alanından seçilir |
| 033e39–033f03 | Error8, plain flag ve tüm size alanında `<LSX>` bulunmaması birlikte flag=1 ve loop-head retry sağlar |
| 033f08–033fc4 | Error7 yalnız bir kez retry; tüm size alanında ilk `version=""` eşleşmesi034730(offset,10,080097,0) ile silinir; yoksa da bir kez retry |
| 034090/0340cf | Received log'dan sonra032d00(temp-sret,handler,same-document) |
| 0340df–0341ab | Dönen string identity response'dan farklıysa eski response free/reset,32byte move, producer reset; ardından temporary destructor |
| 034207–034292 | Live flag yeniden okunur. Cipher false output16'yı sıfırlar, size0/cap15 yazar, local response'u bırakır ve document destructor'a gider |
| 03429a /0343d7 | Response ve event gerçek03d190 typed codec ile kodlanır |
| 0342b4–0342ef /0345a6–0345d0 | Spare kapasitede size++ sonra eski sonda iki sıfır bayt; aksi012350(value,1,0,0) mandatory provider |
| 0342f4–034337 | Queue getter02c400 cached; count kontrolü, mutex getter02c620, lock04a6ac, max-counter terminal ve ikinci count kontrolü |
| 034358→009eb0 | Front event gerçek kaynak COPY constructor'ıyla temporary'ye alınır |
| 034431–0344ba | Event kendi NUL'u ile response'a eklenir. Spare path size güncellemesini COPY'den önce yapar, terminator sonra; growth007e30(value,count,0,source,count) |
| 0344bf–034570 | Front slot callback'lardan sonra yeniden okunur; free/reset ardından count--, son event ise offset0, değilse offset++; temp destructor sonra unlock |
| 034005–034020 | Output32 move önce; local response size0/cap15/byte0 reset sonra; document destructor en son |
| 034637–03464c | Lock error→thread_error5; counter INT_MAX ise decrement→thread_error6; normal gövdede fazladan unlock eklenmez |

N9, native033d70'nin doğrudan03ce30/034730/03d190/009eb0 kaynaklarını kullanır.034730 growth kaynağı N7'de kapalıdır fakat bu gövdedeki version erase growth gerektirmez.079850 arama sözleşmesi bounded bayt karşılaştırmasıyla modellenir; genel CRT memchr gövdesi kurtarılmış sayılmaz.012350/007e30 bu pakette **zorunlu soyut alt sağlayıcılardır**; testte008350/02b940 kaynaklarıyla kontrollü başarılı davranış verilir, o adreslerin gövdeleri kurtarıldı diye sayılmaz.

## N5 sınırındaki somut değişim

Yeni seçilen gövdede decrypt callback'i gerçek decoded request prefix'ini; handler/parser callback'i gerçek version-erase prefix'ini görür. Callback exception'ı bunu geri almaz. N5 `CompatibleMessageCoreApi` değişmeden tarihsel alternatif olarak kalır; yeni native zincirde seçilmez. Eski adapter'in testleri kendi eski sözleşmesini sınamaya devam eder. Kaynak kapanışı sayısı14→15 olur; seçili compatibility provider0, tarihsel N5 alternatif1. Native033d70 tüm ABI/alt sağlayıcı/EH eşdeğerliği ayrıca **açık** tutulur.

## Testler ve kapsam

**1.713 çekirdek +2.013 entegrasyon =3.726 yeni kontrol PASS.** Ortak build:190 üretim nesnesi (186 C++/4 C),193 normal +19 terminal grup,66 socket grubu. Önceki586 kaynak dosyası ve CRT korunur; beş yeni kaynak/test dosyasıyla591 kaynak snapshot girdisi vardır. Son kayıt `source-workspace/build-record.json`; yerel/common mirror aynı build hashine bağlıdır. Socket standalone scripti ve CMake bu aşamada ayrıca çalıştırılmış sayılmaz.

Çekirdek bağımsız payload oracle'ı inline/heap, embedded NUL,0–34 uzunlukları,128/2047/2048/4095/4096/8192/150000 boyutları; plain/encoded ve log açık/kapalı yollarında tam baytları karşılaştırır. Parser error hücresi ile return8 ayrımı, code8 autodetection, code7 retry/tek erase, NUL sonrasındaki pattern araması, encrypted retry'nin tekrar decode/decrypt başına dönmesi sınanır. Tek reusable document ve output move/reset/destroy sırası kontrol edilir.

Event kontrolleri bir event limitini, event cipher false olduğunda da pop yapılmasını, empty-after-lock yeniden kontrolünü, live front-slot değişimini ve mutex max sayacının terminal6 öncesi azaltılmasını içerir. Callback failure'larında pop/unlock/document cleanup eklenmez.012350 failure'da output yayımlanmaz; cipher false→bozuk aligned-header terminalinde output önce sıfırlanır. Spare append COPY failure'ında yeni size görünür, son NUL/pop/unlock görünmez. Document destructor exception'ında output zaten taşınmıştır.

Entegrasyon yeni final kaynak köprüsüyle gerçek028d30→021420→borrowed/owned response→009eb0 COPY→026670 map growth→028270/02a500→021fd0/0261d0/025a70 zincirini kullanır.13 yanıt, deque0→8→16,150000bayt payload, event'in ayrı NUL'u ve64KiB gönderim parçaları bağımsız oracle ile karşılaştırılır. Plain/explicit encrypted/version-retry/autodetected encrypted × borrowed/owned yolları vardır. Ertelenmiş task native response heap'ini yaşatır; invoke sırasında queue COPY sonrası source bırakılır. Outer028d30 callback failure'ında decoded input kalır ve normal dönüş sonrası cleanup icat edilmez.

## Sınırlar

Document parse/dispatch032d00, cipher042480/042610, stream logging grupları, queue/mutex getter ve lock API'leri native sağlayıcılardır. Testte gerçek upstream XML compatibility reader ve mevcut semantic032d00 alt gövdesi kontrollü provider üzerinden kullanılır; native XML object/vtable/stream ABI ve cipher gövdeleri kapanmaz. Log çağrıları006880/0088e0/007990 grupları olarak soyutlanır; bu grubun içindeki callback-intermediate native durum eşdeğerliği iddia edilmez.

Locals açık caller-owned projection'dır: nonaliasing, coherent string/mutation memory ve her çağrıya özgü document/iki string storage gerekir. Gerçek MSVC automatic stack layout, arbitrary reentrancy/concurrency ve global provenance kurulmadı. Bounded missing/ambiguous memory explicit UnsupportedInput verir; native fault/SEH eşdeğerliği değildir. Normal akışta catch/rollback/unwind destructor veya otomatik mutex guard yoktur; native EH cleanup kapsamı ayrıca açık kalır. Allocator/free/invalid/thread terminal ve generic CRT COPY/fill sağlayıcıları tamamlanmadı. Gerçek network/thread/IOCP/027c10 worker veya orijinal DLL/oyun yürütmesi yoktur. Aynı ajan incelemesi yapıldı; bağımsız final reviewer PASS yoktur.

Orijinaller değiştirilmedi; gerçek CFG/token/achievement içeriği okunmadı. Antivirüsün önceden engellediği orijinal version.dll okuması tekrar denenmez; o dosyanın güncel hash'i doğrulanmış sayılmaz. Eski üretilmiş version.dll paket dışında tutulur. Diğer dört girişin güncel hash'i paketleme kaydında ayrı doğrulanır.

**N10:**012350/007e30 append growth/reallocation normal gövdeleri ve bu yeni orkestrasyonun lower provider kaynak bağlantıları. CRT aşama5'e kadar bekler. Tam DLL tamamlanmadı; toplam yüzde veya güvenilir bitiş tarihi ölçülmüş değildir.
