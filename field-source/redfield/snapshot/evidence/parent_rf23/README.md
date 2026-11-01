# REDFIELD RF23 OWNERSHIP02

LOCAL_BUILD / NOT_RELEASE

Bekleyen fiziksel release kuyruğu,worker clean işareti olsa da DLL unpin öncesinde finalize fence'inden geçer. Başarısız temizlikte kaynaklar korunur. XML geçici heap backend'i genel allocator/PTD'den ayrıdır; iki genel runtime TU'su yalnız regresyon desteği olarak derlenir. Üç kayıt serializer'ı ortak writer kullanır.

`scripts\build_orchestration\build_verify.cmd`:58 production core,2 test-support TU,94 regresyon; Windows planı175 compile eylemi.94 Linux regresyon,12 sanitizer execution ve3 Windows-branch heap model çalışması PASS. RF23 Windows/native/game/CMake NOT_RUN. Ayrıntılar docs/REPORT_TR.md içinde.
