# Derleme ve kontrol

Windows operator komutları WINDOWS_FIX12.md içindedir. CMake profile adları PRODUCTION, DEBUG, NATIVE, CRT, DIAGNOSTIC. NATIVE/CRT synthetic child süreçleri gerçek hesap/oyun kullanmaz. Üretim output preloader_l.dll; diğer profiller ayrı ad/dizin kullanır.

Linux: `python3 scripts/qa/test_portable.py --profile debug --jobs 4`; Production için `--profile production`; sanitizer için `--sanitize`. `--build-root /tmp/preloader_l-build` yalnız ara object/bin/log yerini seçer, uygulama runtime path politikasını değiştirmez. Kaynak build sırasında değişirse kabul raporu verilmez.

`python3 scripts/qa/check_source.py` sekiz kaynak invariant alanını denetler; bu BROWNFIELD bütün test shardlarının geçtiği anlamına gelmez. `python3 tests/tools/test_source_policy.py` negatif mutasyonları çalıştırır. `python3 scripts/qa/package_source.py` seal üretir; `python3 tests/tools/test_package_policy.py dist/preloader_l_BF02_FIX12_APPLICATION_SOURCE_NOT_RELEASE.zip` altı bozuk paketi reddetmeyi sınar.

Kümülatif source ZIP preloader_l/ kökü altında tüm owned src/include/build scripts/tests/vendor pinlerini içerir. Binari, object, build cache ve önceki release paketi source ZIP'e konmaz. SOURCE_MANIFEST ve PACKAGE_MANIFEST ayrı kapsamlıdır; exact ZIP SHA dış FINAL_QA raporuna bağlanır.
