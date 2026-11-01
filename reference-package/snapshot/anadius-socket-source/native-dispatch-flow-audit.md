# N13 — 032d00 dispatch normal akışı ve final kaynak bağlantısı

25 Ekim 2026. `native_dispatch.hpp/cpp`,032d00 per-call normal projection ve `RecoveredMessageDispatchApi::dispatch_032d00` final bağlantısını ekler. Native033d70 artık bu normal gövdeyi doğrudan seçebilir; önceki semantic dispatcher korunur ve yeni entegrasyonda çağrılmaz. Bu değişiklik native DOM/registry/printer/string/handler uygulamalarının veya ABI/EH eşdeğerliğinin tamamlanması değildir.

## Kimlik ve kanıt

REA6.5.0 server/catalog hizası ve kullanılabilir Ghidra sağlayıcısı `native-dispatch-rea-session.json` ile doğrulandı. Hedef açılmadı/yürütülmedi. Ghidra12.1.4 retained listing hashleri kontrol edildi. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

032d00 assembly Evidence:`ev_94c6661b424caa94ba28f70ebfbf7ee51eb890c653d2f2f1f6384efa2d1b7224`; pseudocode Evidence:`ev_4657037962313465cce62f6579fa74838943f7f547b0d739a8b152611c24c36b`. Tam032d00 ve on context yordamının bölümleri evidence/context sidecar'larındadır. Context işlevleri yeni kurtarılmış sayılmaz. ID ve SHA kayıtları `native-dispatch-evidence.json` içindedir. Aşağıdaki tüm adresler0x180 image prefix'ini kullanır.

| Adres / çağrı | Gözlem ve kaynakta korunan sıra |
|---|---|
|032d74→033114–033d25|TLS epoch/guard ve39 kayıtlı global RB-tree lazy initialization. Yeni kaynak önce `prepare_registry` çağırır; native statik-init/TLS/tree alt gövdesi açık zorunlu sağlayıcıdır. Eski39 kayıt tablosu aynen kullanılır.|
|032d93→003300;032da7→006260|İlk element adı LSX C-string olarak karşılaştırılır. Root kardeşi ayrıca denetlenmez.|
|032ddd/032df1;032e20→003fe0|İlk Request elementi ve adı; Request kardeşi varsa hata.|
|032e3b/032e59→002cc0;032e6f|Önce id, ardından recipient attribute varlığı; boş değer kabul. Sonra ilk command; command kardeşleri ayrıca denetlenmez.|
|032e88–032eb1→006b30;032ed3→0071d0;032efa→00b2b0|Sıfır prefix üzerinde command-name construction; exact lookup alt sağlayıcı. Sentinel eşleşirse yalnız ChallengeResponse fallback. String/lookup yardımcılarının native iç gövdeleri yeni sayılmaz.|
|032f34–032f6b→04b0f8;032f70|Geçici komut adı heap'teyse cap+1;4096 eşiğinde raw header/+39 ve delta≤31 kontrolü. Hata dalından önce serbest bırakılır; ölü prefix metadata resetlenmez.|
|032f88→0019b0;032f9f/032fb5→004080;032fcb/032fe1→005560|Printer(0,0,0), LSX/Response açma ve cached id/sender.|
|032ff0;032ff9/032ffd;033006→02d7e0|Printer callback'lerinden sonra live sentinel yeniden karşılaştırılır. Kayıtlı handler token'ı çağrı anında seçilen entry'den okunur; alternatif ChallengeResponse. AL false normal cleanup yapılmadan invalid terminal'e gider.|
|03301c/03302a→002f20;03302f–033045|İki close; command-name prefix aynı scratch olarak sıfırlanır(size0/cap15).|
|03304a/033051/03305e→02b300|Printer signed32 count sign-extend edilir, uint64 wrapping ile2*count+32 reserve çağrısı.|
|033063/03306d/033079→018b30|Count ve buffer pointer tekrar okunur. Count-1 bayt gereksiz scratch kopyasına append edilir; count0/negatif aritmetiği gizlenmez.|
|03307e/033088→009fd0;03308e/0330a2→00b520;0330af→001f60|Output constructor için printer data tekrar okunur; C-string uzunluğu kullanılır. Output önce kurulur, sonra heap scratch ve printer temizlenir. Native normal dönüş özgün output referansıdır.|

## Doğrulama ve kapsam

850 çekirdek +8.978 entegrasyon =9.828 yeni kontrol PASS. Çekirdek testler39 komut/alias, case-sensitive unknown, şekil/attribute hataları, boş attribute, command kardeşleri,16/4096 uzun unknown-name cleanup, live handler/sentinel callback değişikliği, embedded NUL, reserve/append/output buffer reload, signed count-1/0 wrapping ve dokuz aşamada injected failure prefix'lerini denetler. Otomatik scratch/output alias'ı bounded projection sınırı olarak açıkça reddedilir.

Entegrasyon gerçek source032d00→033d70→028d30/021420→copy/queue→send/finish zincirinde13 yanıt,150KB payload,64KiB parça, deque büyümeleri, borrowed/owned/deferred ömür, XML retry/autodetect, ECB/CBC, ChallengeResponse version/reset/rekey ve event sırasını bağımsız sentetik byte oracle'larıyla sınar. Orijinal DLL/oyun/network/IOCP çalıştırılmaz. Eski semantic032d00 çağrı sayısı sıfırdır. Testte gerçek upstream XML compatibility backend ve printer kullanılır; original sürüm/layout eşdeğerliği kanıtlanmaz. Registered handler etkisi kontrollü provider'dır; handler gövdeleri bu pakette yeniden kurtarılmaz.

Registry initialization ve exact lookup, document accessor/DOM, printer helpers,006b30/02b300/018b30/009fd0/00b520 ve02bd30/04d228 native exception ABI zorunlu alt sağlayıcılardır. Test helper'larının resize/copy davranışı native yardımcı gövdesi sayılmaz. ChallengeResponse mevcut semantic02d7e0 ve N12 SessionCipher/Factory ile bileşir. Native struct/table/vtable layout, reentrancy/concurrency/global lifecycle, unwind/SEH, allocator/heap ABI ve son replacement DLL linki açıktır. Kaynak gövdesi hata sonrası catch/rollback/destructor uydurmaz; C++ test fixture yaşam sonu native unwind kanıtı değildir.

Yeni unique mapped adres artışı0:032d00 önceden semantic kaynakla eşlenmişti; şimdi ek normal gövde ve final bridge vardır.1 normal-body claim ve1 typed source-provider closure eklenir;24 closure native ABI kapanışı anlamına gelmez. Önceki606 C++/header kaynağı ve CRT byte olarak korunur. Ortak build194 nesne(190 C++/4 C),201 normal +19 terminal grup/74 socket grubu; eski36 yerel validation kaydı tarihsel kalır. Bağımsız reviewer PASS yoktur; denetim aynı ajan tarafından yapılmıştır. Full DLL tamamlanmadı; toplam yüzde ölçülmedi.

Sıradaki N14:001830/004280/001e00 document ctor/parse/destroy kaynak sözleşmesi. Registry TLS/RB-tree ile string/printer alt backend açık kayıt olarak korunur. Öncelik1→2→3→5→4; CRT aşama5'e kadar bekler.
