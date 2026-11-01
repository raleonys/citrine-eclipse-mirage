# REDFIELD RF20 — BUILD FIX04

**LOCAL_BUILD / NOT_RELEASE**

Windows XML semantic kodunun eksik allocator/PTD link bağımlılıkları orijinal HF19 kaynaklarıyla tamamlandı. 56 çekirdek TU,86 regresyon. Temiz Linux normal86/86 ve odaklı allocator/PTD/Windows-dalı48 çalıştırma geçti. FIX04 yerel Windows/MSVC/DLL/oyun NOT_RUN_HERE.

Yeni paketi ayrı klasöre çıkarıp `scripts\build_orchestration\build_verify.cmd` çalıştırın. Başarılı Windows akışında DLL: `build\msvc_x64\preloader_l.dll`; log: `build\msvc_x64\build.log`. Testlerden sonra DLL link/smoke/native aşamaları çalışır. Ayrıntılar `docs/REPORT_TR.md` içinde.
