# REDFIELD RF27 — surge.cfg sözleşmesi ve yapılandırma refaktörü

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

Kullanıcının düzeltmesi uygulandı: yapılandırma dosyasının doğal adı surge.cfg'dir. RF26'da preloader_l.cfg yapılması yanlıştı. Loader,örnek ve test/native-host fixture'ları surge.cfg adına döndü. preloader_l kod/DLL kimliği,REDFIELD proje adı,surge.cfg ise bağımsız uyumluluk sözleşmesidir.

## Dosya adı düzeltmesi

include/preloader_l/config_file.hpp içindeki module_config_basename sabiti yalnız surge.cfg tanımlar. Loader bu sabiti kullanarak dosyayı mutlak modül yolunun bulunduğu klasörde arar. DLL adından CFG türetilmez; CWD araması veya başka ada fallback eklenmez. examples/surge.cfg günceldir.

Mevcut surge.cfg içeriğini bu düzeltme için değiştirmeniz gerekmez. RF26 için dosya adını değiştirdiyseniz tekrar surge.cfg yapın ve preloader_l.dll'in yanında tutun. Önceki paket/raporlardaki yanlış yönlendirme tarihsel kayıt olarak saklanır; güncel talimat budur.

ServiceSession testine aynı modül klasöründe yanlış DLL-stem CFG dosyası eklendi. surge.cfg yokken bu dosyanın yüklenmediği,surge.cfg varken doğru dosyanın yüklendiği,sonrasında surge.cfg kaldırılınca yanlış dosyaya fallback olmadığı doğrulanır. Çalışma klasöründeki surge.cfg de okunmaz. Invalid/too_large/NUL/directory/missing durumları eski snapshot'ı korumaya devam eder.

## Devam eden refactor

ConfigStore'un callback Context'i,her replace çağrısında zorunlu olan staged snapshot ve Achievement stage bağlarını nullable pointer yerine referanslarla ifade eder. Sahiplik ConfigStore'un yerel shared/vector/string nesnelerindedir; referanslar ömür uzatmaz.

Ham leaf path/key/value kopyalama,üç typed projection ile aynı parser-validated SourceView üzerinden yapılır. Borrowed token byte'ları owned string alanlarına kopyalanır; raw escape/decode,Online exact-literal politikası,leaf limiti ve atomik publish sırası değişmez. Snapshot/source ömrü mevcut sözleşmededir. Bu tur gözlenmiş bir crash veya bellek sızıntısı düzeltmesi olarak sunulmaz.

## Doğrulama

| Kontrol | Sonuç |
|---|---|
| Temiz GCC derlemesi |58 production core +2 test-support TU;157 obje,97 linked çıktı PASS|
| Regresyon |96/96 PASS;stdout/exit RF26 ile96/96 aynı;ServiceSession senaryosu genişletildi|
| CFG/session,ConfigStore,snapshot,transaction,Achievement,TCP settings |ASan ve UBSan altında ayrı ayrı6/6 PASS|
| Allocation transaction |115 sıradan C++ tahsis hatasında önceki snapshot ve üç projection korunur|
| Mimari fixture'lar |13 beklenen ret|
| Paket bozma |2 beklenen ret|
| Encoding/environment/POSIX runner |16/13/7 kontrol PASS veya beklenen ret|
|1584 ancestry recipe/manifest/ZIP CRC/deterministik repack/taze extract |PASS|
| GetConfig legacy tanısı |FAIL_OBSERVED,exit14;96 pozitif testten ayrı|
| RF27 Windows MSVC/native/game/CMake |NOT_RUN|

ASan leak detection kapalıdır; leak tespiti PASS iddiası yoktur. RF26'nın81 hook-model kontrolü tarihsel kayıt olarak tutulur; RF27'de yeniden çalışmış sayılmaz. Hook backend/config kaynakları değişmedi. Üç production C export,35 route binding,7 hook hedefi,vendor pinleri,LICENSE,CMake ve SAFE-FREEZE byte'ları korunur. Windows tam build planı177 compile eylemidir. CMake bu ortamda bulunmadığı için configure/build çalıştırılmadı.

262 aktif uygulama dosyasında proje adı için0 eşleşme vardır;namespace/include/makro kimliği preloader_l kalır. SPDX GPL-3.0-only satırları korunur.262 RF26 uygulama dosyasının özgün byte'ları provenance/rf26 içinde doğrulanır.258 güncel dosya yalnız CFG token düzeltmesiyle eşleşir veya byte-identical'dır;üç üretim dosyası ve genişletilen ServiceSession testi ayrı incelenmiştir.1327 exact adoption ve1584 sıralı recipe kaynak bağını taşır.

## Çalıştırma ve kapsam

`REDFIELD_RF27\scripts\build_orchestration\build_verify.cmd` çalıştırın. Başlangıç RF27 CONFIGURATION REFACTOR 06,58 core,96 regresyon göstermelidir. DLL build\msvc_x64\preloader_l.dll,log ve receipt aynı klasördedir. CFG surge.cfg adıyla DLL'in yanındadır. Son completion marker PRELOADER_L_RF27_SCOPED_PASS_WITH_RECORDED_DIAGNOSTIC olmalıdır;DLL'in oluşması tek başına bütün native kabul değildir.

Bu tur dosya adı sözleşmesini ve belirtilen callback/raw-capture düzenlemesini tamamlar. Bütün codebase'in ayrıntılı anlamsal refactor'ı tamamlandı denmez. Canonical HF19/CURRENT ve immutable parent değişmedi. Bu kaynaklarla oyunun yeniden açıldığı iddia edilmez.

Parent RF26 ZIP SHA-256:0e04ca9de5573efdd9a98f32dbfe8ff74c1e406fc7c159e6b536d9ec7a91bdb8.
