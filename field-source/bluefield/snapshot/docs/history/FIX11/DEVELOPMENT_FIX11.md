# Build ve geliştirme

Tek aktif kaynak ağacı `src/` altındadır. `cmake/sources.cmake` ve
`metadata/BUILD_GRAPH.json` açık source listelerini tutar. Aynı kaynaklar farklı
profile özel nesnelere derlenir; kopya üretim implementasyonu tutulmaz.

| Giriş | CRT/config | Çıktı |
|---|---|---|
| build_production.cmd | /MT, Release, /O2 /GS /guard:cf | dist/production/preloader_l.dll |
| build_debug.cmd | /MTd, Debug | dist/debug/preloader_l_debug.dll |
| test_native.cmd | /MT, Release, ayrı native fixture | dist/native/preloader_l_native_test.dll |
| test_crt.cmd | /MTd, Debug, ayrı CRT fixture | dist/crt/preloader_l_crt_test.dll |

Girişler `scripts/build_orchestration/` içindedir. Thin CMD, aynı-stem PowerShell
hedefine argümanları ve exit code'u geçirir, sonucu gösterir ve pencereyi açık
tutar. CI doğrudan PS1/Python hedefini çağırır. `build_driver.ps1` iç helper'dır.
VS/CMake yolu runtime'da yerel vswhere ve CMake capabilities ile seçilir;
araç indirme, PATH'e kalıcı yazma veya otomatik dependency refresh yoktur.

Portable QA için g++ ve Python 3 yeterlidir. test_portable.py gerçek app çekirdeğini
derler; kaynak/başlık/toolchain/flags hash'lerini ve atomik nesne çıktısını kaydeder.
Link adımları tüm object derlemeleri bittikten sonra başlar. Aynı profile eşzamanlı
build lock ile reddedilir. Source state build sırasında değişirse PASS verilmez.

```text
python3 scripts/qa/test_portable.py
python3 scripts/qa/test_portable.py --sanitize
python3 scripts/qa/check_source.py
python3 tests/tools/test_source_policy.py
python3 tests/tools/test_pe_verifier.py
python3 scripts/qa/package_source.py
python3 scripts/qa/check_package.py dist/BLUEFIELD_BF02_FIX11_APPLICATION_SOURCE_NOT_RELEASE.zip
```

ASan/UBSan ve LeakSanitizer ayrı kayıtlanır. Ortam LSan'ı bloke ederse yalnız
`ASAN_OPTIONS=detect_leaks=0` ile yapılan ayrı ASan/UBSan koşusu kabul edilir;
bu, bellek sızıntısı bulunmadığı sonucu değildir.

Compiler selector environment is cleared before CMake; one operator file lock owns
the profile output directory. PRODUCTION runs protocol tool selftests separately
from the DLL PE check. Full native Golden acceptance remains NOT_RUN.

PRODUCTION uses the selected VS x64 vcvars environment and NMake Makefiles in
build/production/msvc. Its real MSVC link rule is checked by CORVEXON. Only a
successful five-stage Golden receipt permits dist/production/preloader_l.dll.
See GOLDEN_PIPELINE.md for raw/staged/output paths and failure handling.
