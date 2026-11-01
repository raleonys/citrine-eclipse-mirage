## Windows derleme girişleri

Tüm girişler scripts/build_orchestration altındadır;CMD ince invocation katmanıdır.
Mevcut derleme davranışı veprofil output kökleri korunur.

| Giriş | Çıktı ve kapsam |
|---|---|
| build_verify.cmd | build/verify/msvc_x64/preloader_l.dll;97 regresyon+ayrı legacy diagnostic;production/native DLL ve mevcut hostlar;dist'e yazmaz |
| build_production.cmd | /O2 /MT /GS,production entry veprofile-owned RC;public ABI smoke/linkaudit+beş Golden gate geçerse dist/preloader_l.dll |
| build_debug.cmd | Ayrı /MTd,debug symbols,objeler;dist/debug/preloader_l_debug.dll;current public ABI host |
| build_native_acceptance.cmd | Ayrı /MT,tests/native/dll_entry.cpp;dist/native_acceptance/preloader_l_native_test.dll;mevcut native host |
| build_crt_acceptance.cmd | Ayrı /MT /GS public/native entry vehostlar;dist/crt_acceptance;tam eski CRT black-box/parity açık |

surge.cfg DLL'in yanında olmalıdır. Production DllMain/SURGE/DllShutdown ve
ordinals1/2/3 aynı.Native yalnız RFTestStatus100/RFTestSetFaults101/RFTestTransport102
ek exportlarını taşır.Native entry ayrı source-role olarak kayıtlıdır;production
profile bu kaynağı reddeder.Nativetest artık production entry'ye macro vererek
oluşturulamaz.CMake developer aggregate de ayrı native entry'yi kullanır;bu
ortamda CMake/MSVC çalıştırılmadı.Production ve Debug include closure native test
başlığını almaz;shared core veMinHook her profilin kendi CRT/objeleriyle derlenir.

