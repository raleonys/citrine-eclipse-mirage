## Windows derleme girişleri

Tüm girişler scripts/build_orchestration altındadır. CMD dosyasını çalıştırın;
Python asıl motoru kullanır ve child exit kodu aynen korunur.

| Giriş | Çıktı ve kapsam |
|---|---|
| build_verify.cmd | build/verify/msvc_x64/preloader_l.dll; developer aggregate,96 regresyon ve ayrı legacy diagnostic;dist'e yazmaz |
| build_production.cmd | build/production/preloader.unsigned.dll; /O2 /MT /GS,RC/public ABI smoke/link audit ve beş Golden aşaması;hepsi geçerse dist/preloader_l.dll |
| build_debug.cmd | Ayrı /MTd,symbol,objeler;dist/debug/preloader_l_debug.dll;mevcut host kapsamı |
| build_native_acceptance.cmd | Ayrı /MT/native entry;dist/native_acceptance/preloader_l_native_test.dll;mevcut native host |
| build_crt_acceptance.cmd | Ayrı /MT /GS public/native DLL ve mevcut CRT host kapsamı;dist/crt_acceptance |

Production artık sırf eski GOLDEN_PENDING engeli nedeniyle durmaz. Gerçek gate
başarısızlığı derlemeyi durdurur ve o denemenin production dist çıktısını temizler.
Bu ortamda Windows DLL derlenmedi;yerel MSVC'nin layout/resource/.data koşullarını
sağladığı henüz kanıtlanmadı. Önceki build'in gerçek DLL PASS kaydı yeni DLL'in
kabulü sayılmaz. surge.cfg çalıştırılan DLL'in yanında bulunmalıdır.

