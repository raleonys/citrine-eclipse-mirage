# REDFIELD RF28

LOCAL_BUILD / NOT_RELEASE

Tek maintained codebase;preloader_l kod/DLL kimliği ve DLL yanındaki surge.cfg korunur.
B01 configuration/hex/build incelemesi kapandı:20 modülün5'i tamam,15'i açık.
Uygulama implementation değişmedi;hex header buffer sözleşmesi ve güncel mimari
belgeleri düzeltildi.96/96 temiz Linux regresyon PASS;legacy exit14 ayrıdır.

Windows: scripts\build_orchestration\build_verify.cmd. Sabit /O2 /MT x64;
ayrı Debug seçeneği yoktur. CMake Debug /MTd tanımlıdır fakat burada çalıştırılmadı.
Native test DLL'i aynı core'u kullanır ve yalnız test girişlerini ekler.
Windows/MSVC,Debug/CRT,native,oyun,CMake NOT_RUN. Ayrıntılar docs/ARCHITECTURE.md,
docs/REFACTOR_STATUS_TR.md ve docs/REPORT_TR.md içindedir.
