# preloader_l — BF02 FIX12 NON_RELEASE

SURGE kimlik, dosya sahipliği, avatar, log ve provider geçişini içeren kümülatif C++20 kaynak paketi. Geliştirme soyunun adı BLUEFIELD; çalışan teknik kimlik preloader_l. Kaynak atası ve BROWNFIELD QA/upstream kanıtları metadata içinde korunur.

Windows x64 / MSVC gerekir. Üretim hedefi `preloader_l.dll`, export/ordinal sözleşmesi `DllMain@1`, `SURGE@2`, `DllShutdown@3` olarak korunur. Paket bir Windows DLL başarı veya oyun uyumluluğu garantisi değildir. Mevcut sürüm/pointer değiştirilmez.

Config zorunlu değildir. `surge.cfg.example` örnektir; DLL yanına `surge.cfg` olarak kopyalanması isteğe bağlıdır. `surge_override.cfg` aynı dizindedir. SDK, port ve numeric profil varsayılanları korunur; Config2/PEGTL diline yeni ayar veya gramer eklenmedi.

Bütün uygulama verileri `%LOCALAPPDATA%\SURGE` altında RuntimePaths tarafından türetilir. Varsayılan avatar resmi 208×208 `surge.png` dosyasıdır. DLL yanındaki sidecar yalnızca aynı resmi byte dizisi ise okunur; değiştirilmez. Sidecar yok/başka içerik ise gömülü resmi görsel ilk avatar isteğinde sahipli asset dizinine yazılır. Açık özel avatar yolu salt okunur. Eski cache ve loglar taşınmaz, silinmez ve fallback olarak kullanılmaz.

Production/Native/CRT disk diagnostics derlemede kapalıdır. Debug/Diagnostic yalnızca sınırlı sabit olayları `SURGE/surge.log` dosyasına yazar; 256 KiB sınırına gelince append durur. Hassas veri kabul eden log API yoktur.

Online provider yalnız DLL dizinindeki `preloader_l_online.dll`; export `version_check` sonucu 2, `create_online`, yedi slotlu C++ vtable sözleşmesi korunur. Dosya adı native ABI kanıtı değildir. Provider yokluğu, yanlış sürüm ve null factory çevrimdışı davranışa döner. Paket orijinal provider binary içermez.

Windows operator girişleri `scripts/build_orchestration` altındadır: `build_production.cmd`, `build_debug.cmd`, `build_diagnostic.cmd`, `test_native.cmd`, `test_crt.cmd`. Production gerçek MSVC → URSYNEX → ARYRA → MUSTAVEX → CORVEXON → FALCION zinciri başarılı olmadan staged DLL yayımlamaz. Bu yalnız LOCAL_BUILD yayınıdır.

Portable doğrulama: `python3 scripts/qa/test_portable.py --profile production`, Debug için `--profile debug`, ASan/UBSan için `--sanitize`. `scripts/qa/check_source.py`, `tests/tools/test_source_policy.py` ve `scripts/qa/package_source.py` kaynak/negatif/paket kontrolleridir.

Dosya sözleşmeleri, riskler ve Windows adımları: `docs/MIGRATION_FIX12.md`, `docs/FILESYSTEM_FIX12.md`, `docs/WINDOWS_FIX12.md`. Statik çağrı envanteri `metadata/FILESYSTEM_CALLS_FIX12.json`; tüm dosya şablonları `metadata/FILESYSTEM_TEMPLATES_FIX12.json`. Önceki genel belgeler docs/history/FIX11 altında tarihsel kayıttır; güncel path talimatı değildir.
