# REDFIELD RF24 SNAPSHOT03 — yapılandırma okuma refaktörü

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

RF23'ten devam eden bu paket, yapılandırma okuma kurallarının kopyalarını kaldırır ve achievement hazırlığındaki gereksiz sayaçları temizler. Yeni oyun özelliği eklenmedi. Tüm codebase'in ayrıntılı refactor ve anlamsal denetiminin tamamlandığı iddia edilmez.

## Tamamlanan değişiklik

Settings, Profile, GameInfo, Image, Achievement yanıtları ve hook yapılandırmasında yedi ayrı leaf arama döngüsü bulunuyordu. Bunlar `include/redfield/config_snapshot_view.hpp` içindeki ConfigSnapshotView üzerinden tek kurala bağlandı:

- Yalnız ilgili bölümün doğrudan alanları okunur; iç içe alanlar eşleşmez.
- Bölüm ve anahtar adları büyük/küçük harfe duyarlıdır.
- Yinelenen anahtarlar ve alias'lar arasında kaynak sırasındaki son kayıt kazanır. Alias listesinin sırası öncelik oluşturmaz.
- Bulunan boş değer, bulunamayan değerden ayrılır; fallback yalnız bulunamayan değer içindir.
- Raw yapılandırma byte'ları korunur. Online gibi normalize edilmiş snapshot alanlarının politikası değiştirilmez.

View belleğe sahip değildir ve snapshot'ın ömrünü uzatmaz. Çağıran immutable snapshot'ı bütün okuma/format işlemi boyunca tutar; dönen pointer aynı ömre bağlıdır. Geçici ConfigSnapshot rvalue'ından view kurmak derleme aşamasında engellenir; bu bütün dangling-reference ihtimallerini otomatik önleyen bir mekanizma değildir. Hook generation mevcut gate-drain sözleşmesiyle tutulur. Yeni global state, parser veya indeks eklenmedi; asimptotik hızlanma iddiası yoktur.

Achievement ParseStage içindeki nn/tn sayaçları kaldırıldı; tek boyut bilgisi vector::size() oldu. İki ayrı boyut kaydını senkron tutma zorunluluğu kalktı. Totals-first sırası, ilk isim eşleşmesi ve yinelenen kayıt politikası korunur. Bu bir bellek sızıntısı düzeltmesi olarak sunulmaz; vector'ler zaten RAII kullanıyordu.

## Doğrulama

| Kontrol | Sonuç |
|---|---|
| Temiz GCC derlemesi |58 production core +2 test-support TU;156 obje,96 linked çıktı PASS|
| Regresyonlar |95/95 PASS; önceki94 testin stdout ve exit sonuçları RF23 ile aynı|
| Yeni snapshot testi |Eksik/boş değer,alias sırası,case/depth,raw değer,500 ara durum,held generation ve başarısız replace atomikliği PASS|
| Snapshot/5 yanıt/achievement projection/hook config |ASan ve UBSan altında ayrı ayrı8/8 PASS|
| Hook kaynak modeli |Normal/ASan/UBSan altında27'şer,toplam81 PASS|
| Mimari geriye dönüş kontrolleri |9 beklenen ret; hash/recipe yeniden bağlansa da mimari guard çalışır|
| Paket bozulması |2 beklenen ret|
| Encoding/environment/POSIX runner |16/13/7 kontrol PASS veya beklenen ret|
| Kaynak manifesti/798 ancestry recipe/ZIP CRC/deterministik repack/taze extract |PASS|
| GetConfig legacy tanısı |FAIL_OBSERVED,exit14;95 pozitif testten ayrı|
| RF24 Windows MSVC/native/game/CMake |NOT_RUN|

Sanitizer'lar bu ortamda GCC/Linux ile çalıştı; ASan leak detection kapalıydı. LeakSanitizer başarı iddiası yoktur. Hook modeli gerçek hook C++ kaynaklarını Win32/MinHook shim ile derler; Windows ABI veya çalışan oyunda executable patching kabulü değildir. RF23'ün heap/kapanış kanıtları parent altında tarihsel kayıt olarak saklanır; RF24'te yeniden çalışmış sayılmaz.

Üç public export,35 route binding,7 hook backend,vendor pinleri ve SAFE-FREEZE patch byte'ları korunur. Hook configuration tüketicisi değiştiği için model yeniden çalıştırıldı. CMake dosyası RF23 ile byte-identical; CMake bu ortamda bulunmadığı için configure/build çalıştırılmadı. Windows tam build planı176 compile eylemidir.

## Çalıştırma

Paketi yeni bir klasöre çıkarın ve `REDFIELD_RF24\scripts\build_orchestration\build_verify.cmd` çalıştırın. Başlangıç RF24 SNAPSHOT REFACTOR 03,58 core ve95 regresyon göstermelidir. DLL yolu `REDFIELD_RF24\build\msvc_x64\preloader_l.dll`; ayrıntılar aynı klasörde build.log ve receipt.json içine yazılır. Native kabulün tamamlanması ayrıca build'in son completion marker'ıyla doğrulanır. DLL'in oluşması tek başına tüm testlerin geçtiği anlamına gelmez.

## Açık kalan refactor alanları

Bu turda ortak snapshot okuma ve achievement staging kapsamı tamamlandı. Content/Entitlement projection dosyalarındaki tekrar eden token-view,string-assignment ve record-find yardımcıları sonraki inceleme adaylarıdır; farklı boş değer politikaları önce açıkça karşılaştırılmalıdır. Diğer modüllerin ayrıntılı anlamsal denetimi ve yeni Windows/game kabulü açık kalır. Kökeni alınmış veya sonradan yazılmış olması hiçbir uygulama dosyasına bakım muafiyeti vermez.

260 uygulama dosyası aynı REDFIELD bakım alanındadır.1327 exact adoption yalnız vendor/referans alanındadır;798 sıralı recipe,arşivlenmiş kaynak ile mevcut uygulama bağını doğrular. Kaynak izi kalite veya ideal algoritma kanıtı değildir. Canonical HF19/CURRENT,lisanslar ve immutable parent kayıtları korunur.

Parent RF23 ZIP SHA-256: `3c0fc8371abbbf62198cab0402b6ff338d6253b0535bacb7af57a9c183110d6f`.
