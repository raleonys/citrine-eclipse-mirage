# GF11 — gerçek sahiplik sembol kapanışı (NOT_RELEASE)

GF10 dispatcher arşivi 58 proje sembolünü undefined olarak rapor eder. Diğer GF10 kütüphanelerinde 53 gerçek tanım zaten mevcuttur. HF19 içinde kalan beşinin sahipleri `socket_runtime.cpp` (2), `runtime_state.cpp` (2) ve `socket_exchange.cpp` (1) dosyalarıdır. GF11 bu üç dosyayı hiçbir değişiklik yapmadan derleyip iki özgün HF19 testini çalıştırır.

Bu **tam executable link değildir**. `socket_exchange.cpp` yeni transitive semboller içerebilir. `gf_preloader` shipping hedefi GF00 no-op durumundadır. `GetConfig` özgün testindeki exit=14 ayrı açık bulgudur.

Doğrulama: `python3 scripts/verify_gf11.py --parent-zip <GF10.zip> --hf19-zip <HF19.zip>`; kaynak derlemesi `cmake -S . -B _build -G Ninja -DGREENFIELD_PORTABLE_QA_ONLY=ON -DCMAKE_BUILD_TYPE=Debug && cmake --build _build --target gf11_original_socket_runtime gf11_original_runtime_state gf11_exchange_units && ctest --test-dir _build -R ^gf11_ --output-on-failure`. Windows native testleri bu pakette yapılmadı.

GF10 kaynağından miras kalan `scripts/__pycache__/gf10_pe_export_gate.cpython-313.pyc` GF11 kaynak paketinden gereksiz debug cache olarak çıkarıldı; GF10 immutable parent arşivi değişmedi.
