# B36 bağımsız mimari inceleme

**PASS — desteklenen normal kaynak algoritmaları ve sınırlandırılmış sahiplik bileşimi. Üretim DLL durumu BLOCKED.** Reviewer `/root/b22_provider_audit`, production/core/integration yazarlarından bağımsızdır. Production ve test kaynaklarına dokunulmadı; reviewer hiçbir EXE/DLL/oyun çalıştırmadı. Full common build bu inceleme anında **PENDING**; aşağıdaki quick PASS bunu tamamlanmış saymaz.

## Kaynak ve yürütme kanıtı

Sekiz frozen kaynak hash'i `work/b36-code-freeze.json` ile birebir doğrulandı. B35 ZIP SHA256 `fdfb32d264ebbafb1093594c17c0946470feb4ce0f2719bdd2d19722e57b2da6`: 864 eski kaynaktan **862 aynı, 2 açık parent değişikliği**, 6 yeni kaynakla **870**. Parent diff yalnız `acquire_provider_lease_from_operation` deklarasyonu/gövdesidir; idle factory, heap alloc/free ve CRT davranışı değişmedi. Kaynak yolları ve sekiz tam SHA JSON'dadır.

Current core **2045 PASS**, `/root/acceptor_tests` tarafından gerçekten çalıştırılan immutable kayıt; root quick JSON bunu normalize eder, yeni root çalıştırması değildir. Current integration **177 PASS**, `/root/b22_integration` tarafından yazılan final testi root'un taze çalıştırmasıdır. Toplam **2222**, compile/run 0 ve stderr boş. İki current quick JSON, gerçek core execution kaydı, EXE hash'leri ve kaynak before/after bilgileri bağımsız doğrulandı; reviewer yeniden çalıştırmadı. Eski integration-agent 173 kaydı tarihsel önceki adaydır; final177 yerine kullanılmaz ve toplama eklenmez.

Her iki current quick, B35 ZIP içindeki `source-workspace/build/anadius_core_partial.a` ile eşleşen immutable `work/b36-baseline-core.a` kullanır: `a6075742edd1fb2d80744cbc5476f01abd3e6a51827f920bc91b6628c583a3ad`. Integration taze `accept.cpp`, `owned.cpp`, parent `memory.cpp` ile link edilmiştir. Tam quick/EXE SHA'ları JSON'dadır.

## Native normal gövdeler

**021b00 ve 0271a0 için iki yeni normal typed gövde sınıflandırması uygundur.** Her iki adres B35 inventory'de `no_definition_mapping` idi. Retained pseudo ve seçilmiş ASM karşılaştırıldı: 021b00 129, 0271a0 336 instruction satırı. Evidence ID'leri JSON'dadır. Yeni native ABI/provider closure **0**, tarihsel52 değişmez. Beklenen mapped **496**, eligible **500**, normal-body claim **133**; full common/registration henüz bitmiş değildir.

021b00: 0x1d8 istek/473 physical byte, environment öncesi capture ve sonrası canlı protocol/pending yüklemeleri; work-count sırası; invalid listener 2719/system ve occupied pending **1/system**; fresh/reset/listener sırası; BOOL true dahil hemen WSA error okuma; true/997 ve failure/copied-error publication ayrımları korunur.

0271a0: error girişte kopyalanır; owner0 yalnız cleanup yapar. Signed remote length MOVSXD sonrası unsigned<=28 koşulu korunur; oversize2726 başarılı setsockopt ile sıfırlanabilir. Cached listener/accepted, failure category→WSA error ve koşulsuz system-category comparison doğru sıradadır. Retry flag native **+188**; main/alternate gerçek OVERLAPPED ve logical metadata/queued resetlenir, **+20 next korunur**, alternate canlı yeniden okunur; retry erken dönüşünde cleanup/output/callback yoktur.

Success transferde pending/accepted canlı okunur; scheduler yalnız destination socket INVALID ise gerekir; occupied+null scheduler misc1 yoludur. NULL IOCP +GetLastError0 da başarıdır. State0x10→237f0→canlı protocol→endpoint flag/28 byte→accepted INVALID sırası ve hatalarda da optional endpoint copy korunur. Environment move ardından raw server-this snapshot, source destruction, canlı accepted close, storage release **callback/query/schedule öncesindedir**. Serbest bırakılan operation/projection daha sonra kullanılmaz.

Nested schedule: executor factory query öncesi cache edilir; nested move ve conditional temporary destructor; borrowed slot18 önceliği; aksi halde publish slot10 TLS/allocation öncesi cache edilir, local handler da önceden kopyalanır. Mutable returned task ve canlı nested/outer normal destructor sıraları korunur. Host22520 uygulaması ve copied-error028ab0 sınırı yeni native gövde sayılmamıştır. Orijinal Native EH rollback uydurulmaz.

## Sahiplik ve kapanış

Owned model gerçek473-byte ledger ve ayrı C++ sidecar/OVERLAPPED kullanır. Checked inert Posted base reservation, pending receive/send üzerine bind edilmesini reddeder; önceki packet binding sidecar destruction öncesi temizlenir. Submit, retry-arm ve completion, public mutable span yerine **kendi kayıtlı raw+68/288** bölgesini doğrular; completion bunu loan/consumed değişmeden yapar. Foreign ve başka canlı row buffer'ı SDK/handler/pin/poison öncesinde reddedilir.

Bütün473-byte SDK loan observed completion'a kadar tutulur; cancel intent bırakmaz. Submission guard inline completion'ı reddeder. Task callback ve lower forward yolları active/poison guard kapsamındadır; task free öncesi bütün gerekli alanlarını yerellere alır. Native cleanup eski row'u cache/free/reproject edebildiğinden dönüşte eski projection okunmaz.

Close, Base snapshot'ını Accept gate dışında ve Retiring seal **öncesinde** alır; sonra kendi sayaçlarını tekrar kontrol eder. Böylece Base→Accept callback sırası ters kilitle karşılaşmaz ve devam eden callback gereksiz poison olmaz. Lease gate dışında bırakılır. Parent scoped factory mevcut Operation/thread/fiber/PTD admission ve metadata lease mekanizmasını kullanır.

## Açık sınırlar

029df0/029590/29660/021a10 helper seçimi ve gerçek kernel accept/retry davranışı mandatory lower sınırlarıdır. **237f0 zaten mapped normal gövdedir**; integration mevcut `replace_owner_0237f0` gövdesini kullanır, production ControlFactory/domain seçimi hâlâ açıktır. 22520 original gövde kurtarılmış sayılmaz. Error +4 padding, gerçek native layout/prototype/ABI, original Native EH ve güvenli unknown-outcome recovery kanıtlanmamıştır.

Integration kendi process heap/FLS/IOCP yolunu kullanır; AcceptEx/address/setsockopt importları injected olup gerçek kernel AcceptEx/network request yoktur. Parent Operation, tek admitted worker, controller PTD hazırlığı, borrowed context/provider ömrü, composite accept-first router, bütün raw/cache erişimlerinin adapter üzerinden geçmesi ve dış drain/quiescence attestation zorunludur. Generation kernel ABA kanıtı değildir. 0241b0 context, 0228c0 executor, genel concurrency/unload ve B31 loader hook'ları açık kalır; üretim DLL teslimi hazır değildir.

B35 composition baseline **79 dosya/39 object**, yeni owned.hpp/cpp ile beklenen **81/40**. `accept.hpp/cpp` native eligible gruptadır. Planned common: **254 object (250CPP+4C), 301 ordinary group +19 terminal group**; bu rapor bunların koşulduğunu iddia etmez. Kaynak/test/pipeline değişikliği yapılmadan yalnız bu iki inceleme dosyası yazıldı.
