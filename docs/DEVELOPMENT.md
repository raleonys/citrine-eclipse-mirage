# Yerel geliştirme ve operatör komutları

Windows: Visual Studio C++ x64 araçları/SDK, CMake >=3.20 ve Python 3.9+ gerekir.
Komutlar compiler standart CRT'sini kullanır: MSVC Release `/MT`, Debug `/MTd`.
Bu Linux oturumunda PowerShell/CMD ve MSVC çalıştırılması iddia edilmez.

`scripts/build_orchestration/` altındaki insan komutları:

| Komut | Çıktı / kontrol |
|---|---|
| `build_unstable.cmd` | Release uygulama DLL'i; export/resource ve gerçek yükleme/socket/kapatma/unload denetimi |
| `build_debug.cmd` | Debug uygulama DLL'i ve aynı public API denetimleri; debugger sembolleri korunur |
| `build_native_acceptance.cmd` | Windows üzerinde Yellowfield CTest kaynak/adapter/CRT testleri |
| `build_crt_acceptance.cmd` | Yalnız seçili compiler CRT/public DLL/source smoke hedeflerini derler ve çalıştırır |
| `build_production.cmd` | MSVC yerel Golden linki ve altı aşamalı kontrol; resmî yayın yapmaz |

İnsan CMD penceresi yalnız final sonucu renklendirir, çıkış kodunu gösterir ve
kapatılmayı bekler. Otomasyon için aynı isimli `.ps1` dosyasını doğrudan çağır;
bekleme yoktur. Argümanlar `-BuildDir build\...`, `-Config Release|RelWithDebInfo|Debug`.
Tek uygulayıcı `build_profile.ps1`; paylaşılan yardımcı `build_common.ps1`.
Her profil ayrı build dizini, kilit, toolchain hash kaydı ve yerel receipt kullanır.
MSVC ortam değişkenleri iş sonunda geri yüklenir; test hataları başarıya çevrilmez.
Golden ürün hedefleri eksikse operatör BLOCKED gösterir ve exit3 döndürür.
Receipt, build başarısı ile profil doğrulamasını ayrı tutar.
MSVC resource/test uyumluluk düzeltmesi: [MSVC_PROFILE_FIX.md](MSVC_PROFILE_FIX.md).

Kaynak QA (Windows gerektirmez):

```sh
python -B scripts/run_source_qa.py --report build/qa/SOURCE_QA.json
```

İstenerek yapılan kaynak değişikliğinden sonra `SOURCE_FILES.json`/köken kaydını
güncelle; ardından tüm kaynak kimliğini **açıkça** kaydet ve tekrar doğrula:

```sh
python -B scripts/check_manifest.py --refresh
python -B scripts/run_source_qa.py
python -B scripts/package_source.py --output ../yellowfield-startup-build-fix-source.zip
python -B scripts/check_falcion_golden_cleaning.py --package ../yellowfield-startup-build-fix-source.zip
```

Paketleme manifesti kendiliğinden yenilemez. Golden komutu eksik ürün hedefinde
nonzero döner; [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md) bu durumu açıklar.
Derleme ve testler Library, haricî yayın kimliği veya release onayı gerektirmez.

## Güncel somut runtime

Application, gerçek vendor IO ve DLL worker başlangıcı [RUNTIME_BINDING.md](RUNTIME_BINDING.md) içindedir.
Windows/MSVC normal derlemede Wine kullanılmaz. LLVM-MinGW/Wine çapraz testleri için
`-DPRELOADER_L_WINE_EXECUTABLE=/path/to/wine64` CMake seçeneği, gerçek programın
exit kodunu koruyan `scripts/run_wine_test.py` çalıştırıcısını seçer.
Wine servislerinin miras aldığı çıktı descriptor'ları test başarısını veya zamanını belirlemez.

Güncel uygulama C arayüzü ve kaynak DLL ayrımı: [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
Güncel doğrulama: [PUBLIC_ABI_STATUS.json](evidence/PUBLIC_ABI_STATUS.json).

Son kullanıcı native loglarında Compile/Debug/CRT PASS; kalan Native test
derlemesi düzeltmesi: [MSVC_NATIVE_LOOKUP_FIX.md](MSVC_NATIVE_LOOKUP_FIX.md).

Güncel beş başlık ve cache temizleme kaydı: [STARTUP_BUILD_FIX.md](STARTUP_BUILD_FIX.md).
Başarılı temizleme konsolda JSON basmaz; seçili build dizinindeki
`PYTHON_CACHE_CLEANUP.json` ayrıntıları saklar. Hatalar nonzero ile build’i durdurur.
