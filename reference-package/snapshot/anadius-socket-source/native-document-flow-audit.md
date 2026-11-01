# N14 — Document oluşturma, parse ve temizleme kaynak bağlantıları

25 Ekim 2026. `native_document.hpp/cpp`,001830/004280/001e00 normal gövdelerini ve `RecoveredMessageDocumentApi` üç final document bağlantısını ekler. Bunlar N13 dispatch ve N12 cipher ile aynı033d70 zincirinde seçilir. Native clear/parser engine/nodes/mempool/vtable/allocator veya ABI/EH eşdeğerliği tamamlanmış sayılmaz.

## Kimlik ve adresli kanıt

REA6.5.0 server/catalog hizası ve kullanılabilir Ghidra sağlayıcısı `native-document-rea-session.json` ile doğrulanır. Orijinal hedef açılmaz/yürütülmez. Ghidra12.1.4 retained listing hashleri kontrol edilir. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Üç hedefin ve on iki context yordamının tam bölümleri evidence/context sidecar'larındadır. Context işlevleri yeni kurtarılmış sayılmaz. Tüm ID ve hashler `native-document-evidence.json` içindedir. Aşağıdaki tüm adresler0x180 image prefix'ini kullanır.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|001830|`ev_de11d55b943716ed390775f77e60b3f24b2c5f17431bc50f1f7fc7eca335c9c9`|`ev_abf6ead481323ed34418e13f343856b36f0183b4a90796a24f1a890f0dad1914`|
|004280|`ev_795f6b99b0b687dd3ceffca4901b4e92f2721869b41c336f72e0ba4c670d4a4f`|`ev_783e89582f054f61046e9c06b28ad2145076dbd6721a32cd6bfe2c615e85ff20`|
|001e00|`ev_b1ea754068b65d01028de2c76b93bcfaf0dca2608322419cc6418811342f3097`|`ev_310b19c5bb059cd1c7953ed5ff55dac04b85c180edb01c590a4176041e650755`|

001830 constructor49 tekil store yapar. Minimum dokunulan0x308 baytlık aralıkta354 bayt yazılır,422 bayta dokunulmaz. Bu minimum aralık tam nesne boyutunun bağımsız kanıtı değildir. Mode+69,flags+70,error+6c,document self+8,vector+a8→self+b0 ve dört pool+108/+188/+208/+288 initial identity/pointer/capacity alanları korunur. Vtable alanlarına yazılan değerler explicit provider identity'leridir; gerçek native vtable uygulaması veya ABI bağı kurulmaz. Bütün nesneyi sıfırlama kısayolu kullanılmaz.0019ab'de self+8 son yazımdır.

004298→002e60 clear, count/null/first-NUL doğrulamasından önce gelir. Count0,null veya ilkNUL için004358→005820(error13,line0,detail0). Size_t(-1) yolunda0042c0–0042c7 kaynak NUL'a kadar taranır.0042cd→04b100 allocation çağrısı assembly'de açıkça görülür; pseudocode thunk etiketinin04b0bc'ye çözülmesi çağrı adresini değiştirmez.0042d5 buffer'ı document+98'e publish eder.0042e2→078d30 copy sonrasında0042e7 live+98 tekrar okunur,0042f1 NUL yazılır ve0042f5→0041b0 parser engine çağrılır. Canlı error+6c sıfır değilse003110,002d70(+108),002d70(+188),002e10(+208),002dc0(+288) sırasıyla çağrılır. Dönüş004338/00435d'de yeniden okunan error alanıdır; callback öncesi cached hata değildir. Error hücresi existing NativeMessageDocument.error_6c ile doğrudan aynı bellek hücresine bağlıdır; kopyalanmış error senkronizasyonu kullanılmaz.

001e14 vtable identity reset,001e17→002e60 clear, sonra pool destructor sırası001c30(+288),001cb0(+208),001bb0(+188),001d30(+108).001e4c live vector+a8 self+b0 ile karşılaştırılır; farklıysa001e5f→04b0f8 free. Ardından001e64 live flags+78 bit0x200 test edilir; set ise001e6d live pointer+80 okunur ve001e74→04b0f8 free. Bu çağrılarda yalnız RCX tanımlıdır; sized-free ikinci parametresi uydurulmaz.001e7b/+7e/+85 flag/pointer/count sıfırlanır ve001e99→001ea0 base destructor tail-JMP normal kaynak çağrısı olarak temsil edilir. Failure prefix'lerinde normal cleanup/rollback uydurulmaz.

## Doğrulama ve kapsam

567 çekirdek +15.264 entegrasyon =15.831 yeni kontrol PASS. Final ortak build195 üretim nesnesi(191 C++/4 C),203 normal +19 terminal grup ve76 socket grubudur. Yeni testler bounded memory, aynı error hücresi, normal callback sırası/yeniden okuma/failure prefix'leri ve mevcut mesaj/cipher/dispatch/kuyruk/gönderim kaynak zinciriyle bütünleşmeyi doğrular. Sayılar coverage yüzdesi veya native çalışma gözlemi değildir; kesin komutlar ve sonuçlar common/local validation kayıtlarındadır. Orijinal DLL/oyun/network/IOCP çalıştırılmaz; gerçek CFG/token/achievement içerikleri okunmaz. Girdi bütünlük denetimleri yalnız hash içindir.

Çekirdek testler assembly kaynaklı49 alan anchor'ı ve49 ayrı eksik-mapping prefix'i,422 korunmuş bayt, raw modebyte/full flags, clear öncesi cached input/count, count0/null/firstNUL, explicit length/embeddedNUL, kopyadan sonra+98 pointer değişikliği, sembolik büyük count+1 ve null-allocation copy davranışını denetler. Error cleanup callback'inin canlı signed error'u değiştirmesi, destructor vector-free callback'inin sonraki flags/pointer'ı değiştirmesi, reset-before-base ve tüm üç bağlayıcı için missing/zero/duplicate/mismatched error storage kontrolleri vardır. Bunlar bounded bellek modeli kontrolleridir; özgün native fault veya EH gözlemi değildir.

Entegrasyon ECB/CBC, version retry/autodetect, ChallengeResponse version/reset/rekey,13 yanıt/150KB payload,64KiB parçalar, deque büyümesi, borrowed/owned/deferred ömür ve event sırasını gerçek kaynak zincirinde denetler. Eski document construct/parse/destroy ve semantic dispatch sağlayıcılarının çağrı sayısı sıfırdır. Kötü XML ve boş/C-string-NUL girdileri ayrıca doğrudan033d70 mesaj döngüsünde sınanır; boş yanıtın sonraki queue/send davranışı bu ek hata testlerinin kapsamı değildir. Parser wrapper'ın dönüşü ve error+6c aynı canlı hücreden gelir. Parser sırasında yükselen hata, allocation ve paylaşılan status etkilerini geri almaz.

Bağımsız ikinci ajan üç üretim gövdesini ve final bağlarını retained assembly ile karşılaştırdı; kapsamı sınırlı statik kaynak denetimi PASS. Tam cpp/hpp SHA-256 ve bulgular `native-document-source-review.json` içindedir. Bu inceleme test yürütmesi veya tam native eşdeğerlik incelemesi sayılmaz.

002e60 clear,0041b0 parser engine,003110 node cleanup,pool clear/destructor yardımcıları,001ea0 base destructor,04b100 allocation ve04b0f8 free zorunlu alt sağlayıcılardır. Testteki upstream XML compatibility backend özgün tinyxml sürüm/layout/ABI veya native mempool davranışı kanıtı değildir. Raw memory projection,vtable identity ve final source bridge gerçek native vtable/global/object lifecycle kurulumu sayılmaz. Native exception-unwind/SEH/concurrency/allocator/ABI ve son replacement DLL linki açıktır. Host fixture teardown native cleanup kanıtı değildir.

Üç adres önceden native kaynak tanımı ve compiled symbol ile eşlenmiyordu. N14 üç unique adres,üç normal-body claim veüç typed source-provider closure ekler. Toplam27 source closure native ABI kapanışı anlamına gelmez. Önceki611 C++/header kaynağı ve CRT byte olarak korunur. Önceki37 yerel validation kaydı tarihsel kalır. Tam native eşdeğerlik için bağımsız son reviewer PASS iddiası yoktur. Full DLL tamamlanmadı; toplam yüzde ölçülmedi.

Sıradaki N15:002e60 document clear ve003110 node cleanup normal kaynak sözleşmesi.0041b0 parser engine,mempool,registry/string/printer ve native backend açık kayıt olarak korunur. Öncelik1→2→3→5→4; genel CRT aşama5'e kadar bekler.
