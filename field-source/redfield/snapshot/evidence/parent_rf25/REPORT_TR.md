# REDFIELD RF25 PROJECTION04 — kayıt yapılandırma refaktörü

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

RF24 üzerinde Content ve Entitlement projection kodları düzenlendi. Ortak mekanizma tek yerde toplandı; iki modülün farklı davranışları açıkça korunur. Bu çalışma, bütün codebase'in ayrıntılı refactor ve anlamsal denetiminin tamamlandığı anlamına gelmez.

## Tamamlanan değişiklikler

`src/config/projection_support.hpp`, parser tarafından doğrulanmış token span'lerini ödünç alan SourceView,kayıt kimliği üzerinden arama ve başarısızlığı bildiren string atama işlemlerini birleştirir. Yeni parser,decode veya indeks eklenmez. Span yalnız leaf callback sırasında kullanılır; yayımlanan kayıtlar kendi string'lerine sahiptir.

Content/Entitlement başlıklarındaki Cur/tv/teq,string atama ve arama kopyaları kaldırıldı. Yalnız vector referansı taşıyan ParseStage ara katmanları çıkarıldı. Yeni kayıt doğrudan oluşturulur; başarılı push_back sonrasında ikinci kez aynı kimlikle aramak yerine records.back() kullanılır. Arama hâlâ doğrusaldır; hash indeks veya asimptotik hızlanma iddiası yoktur.

Bu iki projection artık runtime/record_views.hpp içindeki Input DTO'ları üzerinden dolaşmıyor. Kullanımı biten ContentRecordInput ve EntitlementRecordInput tanımları da kaldırıldı; yanıtların kullandığı ContentRecordView/EntitlementRecordView değişmedi. Achievement projection'ın mevcut runtime text-view bağımlılığı bu turda değiştirilmedi; tüm config/runtime bağımlılıklarının kaldırıldığı söylenemez.

| Sözleşme | Korunan davranış |
|---|---|
| Content boş alan |Önceki/default değer korunur|
| Entitlement boş alan |Önceki/default değer boşaltılır|
| Aynı kimlik |İlk oluşturulma sırası korunur; sonraki alanlar mevcut kaydı günceller|
| İç içe/unknown leaf |İlgili kimlik default alanlarla oluşturulur; nested alan doğrudan update olmaz|
| Boş nesne |Leaf olmadığı için kayıt oluşturulmaz|
| Kimlik/case/raw byte |Boş kimlik geçerli; case-sensitive; decode yapılmaz|
| Hata |Staged aday yayımlanmaz; önceki shared snapshot korunur|

## Doğrulama

Yeni test,gerçek ConfigStore::replace çağrısındaki sıradan C++ tahsislerini sırayla std::bad_alloc ile başarısız kılar. Normal çalışmada 76 ayrı tahsis noktası denendi: her birinde sonuç resource ve yayımlanan snapshot pointer'ı eski değerdi. Sonraki başarılı çağrıda iki modülün farklı boş değer politikaları,sıra/default/nested/boş kimlik davranışı ve kaynak buffer değiştikten sonra kayıtların owned olması doğrulandı. Enjeksiyon yalnız test executable'ındaki new/delete replacement'ıdır; production DLL'e yeni fault switch girmez. Aligned/platform heap veya her olası girdi için evrensel allocation-failure kanıtı değildir.

| Kontrol | Sonuç |
|---|---|
| Temiz GCC derlemesi |58 production core +2 test-support TU;157 obje,97 linked çıktı PASS|
| Regresyon |96/96 PASS; önceki95 test stdout/exit RF24 ile aynı|
| Content/Entitlement projection,transaction,iki yanıt,ConfigStore |ASan ve UBSan altında ayrı ayrı6/6 PASS|
| Mimari geriye dönüş |9 beklenen ret; metadata/hash yeniden bağlansa da guard çalışır|
| Paket bozma |2 beklenen ret|
| Encoding/environment/POSIX runner |16/13/7 kontrol PASS veya beklenen ret|
| Manifest/1060 ancestry recipe/ZIP CRC/deterministik repack/taze extract |PASS|
| GetConfig legacy tanısı |FAIL_OBSERVED,exit14;96 pozitif testten ayrı|
| RF25 Windows MSVC/native/game/CMake |NOT_RUN|

ASan leak detection kapalıdır; leak tespiti PASS iddiası yoktur. Son kaynak değişiklikleri tüm temiz derlemeye ve sanitizer profillerine dahil edildi. RF24'ün81 hook-model kontrolü tarihsel kayıt olarak saklandı; izlenen hook girdileri değişmedi ve RF25'te yeniden çalışmış sayılmıyor.7 hook backend,35 route binding,3 public export,vendor pinleri ve SAFE-FREEZE byte'ları korunur. Windows tam build planı177 compile eylemidir. CMake dosyası değişmedi; bu ortamda CMake bulunmadığı için configure/build çalıştırılmadı.

## Çalıştırma ve kalan kapsam

Yeni klasöre çıkarıp `REDFIELD_RF25\scripts\build_orchestration\build_verify.cmd` çalıştırın. Başlangıç RF25 PROJECTION REFACTOR 04,58 core,96 regresyon göstermelidir. DLL `REDFIELD_RF25\build\msvc_x64\preloader_l.dll`,log ve receipt aynı build klasöründedir. DLL'in oluşması tek başına tüm native kabulün geçtiği anlamına gelmez; build'in son completion marker'ını kontrol edin.

Bu tur iki projection'ın yardımcıları,sahiplik adaptörleri ve hata atomikliği kapsamını kapatır. Achievement projection'ın borrowed text/integer dönüşümü ve diğer modüllerin ayrıntılı anlamsal incelemesi açık kalır. Yeni özellik eklenmedi; mevcut oyun açılışının bu paketle yeniden doğrulandığı iddia edilmez.

262 uygulama dosyası aynı REDFIELD bakım alanındadır; köken bakım muafiyeti vermez.1327 exact adoption yalnız vendor/referans alanındadır.1060 sıralı recipe,önceki kaynaklarla güncel uygulama arasındaki byte bağını doğrular; bu ideal kod kalitesinin kanıtı değildir. Canonical HF19/CURRENT,lisanslar ve immutable parent kayıtları korunur.

Parent RF24 ZIP SHA-256: `3f9ede705eecbe0cd4a7295b1a0307f3ae9a9b5e0cc30670805470ce57000b55`.
