# REDFIELD RF30

LOCAL_BUILD / NOT_RELEASE. M20 ortak derleme/profil refactor'ı;uygulama gövdeleri
RF29 ile üretimde aynı;bir profil regresyonu genişletildi.Örnek surge.cfg eksik User alanlarıyla boş profil üretiyordu;örnek düzeltildi.96/96 temiz Linux regresyon PASS. Windows/Debug/native/CRT/Golden/
oyun/CMake bu ortamda NOT_RUN.281 maintained code+4 vendor qualification PENDING.

Windows'ta mevcut developer DLL için scripts\build_orchestration\build_verify.cmd.
Çıktı artık build\verify\msvc_x64\preloader_l.dll;surge.cfg DLL'in yanında.
Ayrı build_debug.cmd,build_native_acceptance.cmd,build_crt_acceptance.cmd aynı
dizinde;her biri kendi build/profil ve nonproduction dist dizinini kullanır.
build_production.cmd aday derlemesini yapar;henüz bağlanmayan Golden nedeniyle
ardından exit1 verir ve dist/preloader_l.dll yazmaz.Bu bilinçli açık kapsamdır.
Source integrity:scripts\verify_source.cmd;--require-refactor-complete pending
nedeniyle beklenen exit1 verir,normal build için gerekmez.Ayrıntılar
docs/BUILD_PROFILES_TR.md,docs/REPORT_TR.md ve metadata/REFACTOR_REVIEW.json.
