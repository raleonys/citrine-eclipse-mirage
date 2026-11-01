# REDFIELD RF26 — kod kimliği düzeltmesi ve Achievement refaktörü

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

Kullanıcının açık düzeltmesi uygulandı: REDFIELD yalnız projenin adıdır. Aktif uygulama kodunun kimliği preloader_l'dir. Önceki namespace/include adlandırması düzeltilirken mevcut RF25 davranışı ve ownership düzeni korundu; Achievement projection refactor'ına devam edildi.

## Kod kimliği

262 aktif src/include/tests/tools dosyasının içeriği ve yolları büyük/küçük harf duyarsız tarandı: proje adı için0 eşleşme. Namespace'ler,include referansları ve69 header'ın kökü preloader_l oldu. Native-test makrosu,build hedefleri,geçici environment alanları,test fixture isimleri ve operator completion marker'ları da aynı kimliği kullanır. CMake project(REDFIELD),ekran başlığı ve metadata project alanı proje adı olarak kalır.

Kaynak doğrulayıcısı artık preloader_l'yi eski mimari diye reddetmez. Aktif uygulamaya proje adı geri girerse reddeder; emekli global API,eski tests/adopted veya tests/regression ağacı,köken bazlı bakım muafiyeti ve production PTD coupling kontrolleri sürer.13 olumsuz mimari fixture,hash/recipe yeniden bağlandığında da ilgili kontrole ulaşarak beklenen ret sonucunu üretti.

**CFG dosya adı preloader_l.cfg oldu.** Eski isimde CFG kullanıyorsanız adını preloader_l.cfg yapıp preloader_l.dll'in yanına koyun. examples/preloader_l.cfg günceldir. Loader yalnız modülün bulunduğu klasörü kullanır; CWD araması eklenmez. Bu dosya adı değişikliği kullanıcının proje adını koddan çıkarma isteğinin bilinçli sonucudur.

Eski isimler immutable provenance snapshot'larında ve önceki sonuç kayıtlarında özgün byte'larıyla kalır. Bunlar güncel uygulama namespace'i veya derlenen translation unit değildir; tarihsel kanıtların içeriği yeniden yazılmaz.

## SPDX satırları

SPDX-License-Identifier: GPL-3.0-only satırları dosyanın mevcut proje lisansını kısa ve makine tarafından okunabilir biçimde ifade etmek için eklenmişti. Refactor için zorunlu bir C++ öğesi değiller; derlemeyi ve çalışma davranışını etkilemezler. Projede zaten kayıtlı GPL-3.0-only lisansını dosya düzeyinde görünür kıldıkları için korundu. SPDX etiketi copyright sahibi atamaz; özgün bildirimler değiştirilmez. LICENSE ve üçüncü taraf dosyaları RF25 ile byte-identical kaldı.

Resmî açıklama: https://spdx.dev/learn/handling-license-info/ . SPDX,kısa tanımlayıcıyı dosyaya uygulanan lisansı ifade etme yöntemi olarak açıklar; GNU lisanslarında -only ile -or-later farklı seçimlerdir. Bu tur lisans seçimini değiştirmiyor.

## Achievement projection refaktörü

Achievement artık Content/Entitlement ile ortak SourceView ve kayıt arama yardımcısını kullanır. Staging id/name alanları std::string_view taşır; parse kaynağı finalize tamamlanana kadar çağırana aittir. Yayımlanan AchievementRecord alanları owned string'lerdir. Üç projection başlığının runtime/record_views.hpp bağımlılığı kaldırılmıştır; yanıtların runtime view'ları kendi görevlerinde korunur.

Signed32 dönüşümü okunabilir adımlara ayrıldı. İşaret,leading zero,overflow,boş/whitespace token politikası değişmedi. Totals-first sıra,yinelenen total kayıtları,ilk eşleşen isim,boş kimliğin atlanması,nested leaf ve raw byte davranışı korundu. İndeks/hash yapısı eklenmedi.

Başarısız finalize için ikinci bir manuel stage-clearing yolu kaldırıldı. ConfigStore'un yayımlanmamış adayına ve yerel vector/string sahiplerine ait destructor'lar temizliği yapar; hata resource olarak döner ve önceki snapshot tutulur. Mevcut allocation transaction testi Achievement staging/finalize alanlarını da kapsayacak şekilde genişletildi: normal profilde 115 ayrı sıradan C++ tahsis hatası denendi; her birinde eski snapshot pointer'ı ve üç projection'ın eski verisi korundu. Enjeksiyon yalnız test executable'ındadır; DLL'e fault switch eklenmez.

## Doğrulama

| Kontrol | Sonuç |
|---|---|
| Temiz GCC derlemesi |58 production core +2 test-support TU;157 obje,97 linked çıktı PASS|
| Regresyon |96/96 PASS;95 test stdout/exit RF25 ile aynı;1 mevcut test bilinçli genişletildi|
| Projection/transaction/Achievement response-session/snapshot/shutdown |ASan ve UBSan altında ayrı ayrı8/8 PASS|
| Güncel hook kaynak modeli |Normal/ASan/UBSan altında27'şer,toplam81 PASS|
| Mimari geri dönüş fixture'ları |13 beklenen ret|
| Paket bozma |2 beklenen ret|
| Encoding/environment/POSIX runner |16/13/7 kontrol PASS veya beklenen ret|
|1322 ancestry recipe/manifest/ZIP CRC/deterministik repack/taze extract |PASS|
| GetConfig legacy tanısı |FAIL_OBSERVED,exit14;96 pozitif testten ayrı|
| RF26 MSVC/native Windows/game/CMake |NOT_RUN|

ASan leak detection kapalıydı; leak tespiti PASS iddiası yoktur. Hook modeli gerçek güncel hook kaynaklarını Linux Win32/MinHook shim ile derler; Windows ABI veya çalışan oyunda executable patching testi değildir. CMake hedef/makro adları güncellendi ve mekanik dönüşüm kontrol edildi; CMake bu ortamda bulunmadığı için configure/build çalıştırılmadı. Windows tam build planı177 compile eylemidir.

Üç production C export,35 route binding,7 hook hedefi,vendor pinleri ve SAFE-FREEZE byte'ları korunur.262 RF25 uygulama dosyasının özgün byte'ları provenance/rf25 içindedir. Güncel260 dosya yalnız açık adlandırma dönüşümüyle eşleşir; Achievement başlığı ve genişletilmiş transaction testi ayrı anlamsal değişikliklerdir.1327 exact adoption vendor/referans alanındadır;1322 sıralı recipe güncel kaynak bağını doğrular.

## Çalıştırma ve kalan kapsam

`REDFIELD_RF26\scripts\build_orchestration\build_verify.cmd` çalıştırın. Başlangıç RF26 IDENTITY AND PROJECTION REFACTOR 05,58 core,96 regresyon göstermelidir. DLL `REDFIELD_RF26\build\msvc_x64\preloader_l.dll`,log/receipt aynı build klasöründedir. CFG için yeni preloader_l.cfg adını kullanın. Son completion marker PRELOADER_L_RF26_SCOPED_PASS_WITH_RECORDED_DIAGNOSTIC olmalıdır; DLL'in oluşması tek başına native kabulün tamamı değildir.

Bu tur kod kimliği düzeltmesi ve belirtilen Achievement projection refaktörünü tamamlar. Diğer modüllerin ayrıntılı anlamsal denetimi sürer; bütün codebase refactor'ı tamamlandı denmez. Canonical HF19/CURRENT,immutable parent ve lisans kayıtları korunur. Yeni Windows/game kabulü devralınmaz.

Parent RF25 ZIP SHA-256: f07222956f70d10a7d89a95f03533958545578ca46d66ad0b8c9b7a88f36506b.
