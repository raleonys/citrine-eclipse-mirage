# REDFIELD RF20 BUILD FIX01 — Windows ortam başlatma

2026-10-20 · NON_RELEASE_WORK / NOT_RELEASE · semantic_version=null

Kullanıcının Windows çıktısı, RF20 scriptinin Visual Studio18 Community VsDevCmd ortamını hazırlayan komutta exit1 aldığını gösteriyor. Bu C++ compilation veya linker hatası değildir; kod derleme aşamasına geçilmemiştir. Eski çağrı stdout'u nul'a yönlendirdiği ve check=True exception ayrıntıyı bastırdığı için gerçek VsDevCmd teşhisi logda yoktur. Kullanıcının kurulumunun neden exit1 verdiği henüz NOT_ESTABLISHED'dır.

## Değişiklik

Python Windows subprocess bir argument list'ini CRT command-line kurallarıyla serileştirir; CMD script metni ayrı bir parser'dır. Eski list'teki gömülü quoted setup yolu bu sınırdan geçiyordu. FIX01, cmd.exe'ye yalnız sabit `/d /u /v:off /c capture.cmd` argümanlarını verir. Setup yolu environment üzerinden, geçici internal CMD içindeki quoted CALL'a gider. Bu değişiklik eski quoting kusurunu kaldırır; kullanıcının tek exit1 satırı quoting'in tek kök neden olduğunu kanıtlamaz.

VsDevCmd çıktısı nul'a gönderilmez. Setup exit değeri kontrol edilir ve msvc_environment.log'a yazılır; başarısızlık terminalde gerçek diagnostic ile görünür. Başarı halinde SET çıktısı ayrı UTF-16LE geçici dosyadan okunur. Ortam dump'ı loga kopyalanmaz; içinde token bulunabileceği için geçici dosya finally/TemporaryDirectory temizliğiyle bırakılır. CL/_CL_/LINK/_LINK_ hem setup öncesi hem sonrası temizlenir; Windows key casing normalize edilir. PATH/cl.exe kontrolü, eksik capture, invalid UTF-16 ve setup timeout fail-closed sonuç verir. Compiler object/PE x64 identity guards korunur.

Generated capture.cmd build altındaki temporary directory'de internal implementation'dır; yeni operator entrypoint değildir. Yalnız mevcut aynı-directory/stem CMD launcher'lar vardır; BAT launcher eklenmedi. Stale RF05/32, RF02 başlıkları ve ENTRYPOINTS açıklamaları güncellendi.

## Kullanım

1. Tam ZIP'i ayrı klasöre çıkarın.
2. REDFIELD_RF20 içinden `scripts\build_orchestration\build_verify.cmd` çalıştırın.
3. Setup başarısızsa `build\msvc_x64\msvc_environment.log` ayrıntıyı içerir. Compiler başlarsa `build\msvc_x64\build.log` oluşur.
4. Tam başarılı Windows akışında production DLL `build\msvc_x64\preloader_l.dll` konumundadır; native test DLL ayrıca üretilir. Başarısız bir aşama sonraki aşamayı çalıştırmaz.

## Doğrulama

13 focused Python behavior case: existing compiler sanitization; space/ampersand/parentheses path and UTF-16 capture; setup exit1/exit42 diagnostic; vswhere exit7; missing vswhere/setup/capture; empty PATH; invalid UTF-16; timeout output; percent-path rejection; empty/ASCII/BOM diagnostic decode. subprocess/cl discovery mock'tur; gerçek Windows CMD parser, VsDevCmd veya MSVC çalıştırıldığı iddia edilmez. Bu kontroller original root cause'u kapatmaz; yeni Windows denemesinin çıktısı gereklidir.

İki bağımsız integrity negative fixture, değiştirilmiş builder SHA ve manifest yeniden bağlansa bile eksik CMD launcher durumunu beklenen şekilde reddetti. Nihai paket fresh extraction verifier ve deterministik repackage byte eşitliğiyle kontrol edildi.

Tam parent ZIP SHA/CRC ve manifest/source/adoption/recipe verifier doğrulanır. C++ source/header/test/tool/vendor/provenance/exports ve CMake/BUILD_GRAPH/DERIVED byte'ları exact RF20 ZIP'le karşılaştırılır. Parent RF20'nin70×4 runtime evidence'i ayrı historical dizinde korunur, FIX01 için fresh PASS değildir. Windows/MSVC, yeni DLL, C++ regression rerun, sanitizer ve oyun kabulü bu düzeltme için NOT_RUN.

Parent SHA-256: `619123736b20ed2457fbd6d0e26e1395d7fcce2249a43be7cb37e72d8f656cd5`. Library governance2026-10-19.3/v5 current read yapıldı; parent pin'leri korunur. Canonical HF19/CURRENT, GPL-3.0-only, third-party pins, public ABI ve C++ implementation değişmez. semantic_version=null, NOT_RELEASE.
