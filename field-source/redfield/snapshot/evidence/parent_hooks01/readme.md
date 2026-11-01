# REDFIELD RF21 HOOKS01

LOCAL_BUILD / NOT_RELEASE

Windows worker'ına7 HF19 core MinHook detour'u eklendi:5 registry, CryptQueryObject ve hedefe duyarlı connect. SAFE-FREEZE, immutable config sahipliği ve retry edilebilir rollback/drain/teardown dahil. SURGE passive; process-wide htons detour yok.

Yeni klasöre çıkarıp `scripts\build_orchestration\build_verify.cmd` çalıştırın. Hedef DLL `build\msvc_x64\preloader_l.dll`; ana log aynı klasörde build.log. Windows toplam170 compile işlemi;90 regresyon +ayrı legacy tanı +production/native DLL testleri.

90 portable regresyon ve güncel Windows kaynaklarıyla81 kontrollü hook senaryosu geçti. Yeni Windows/MSVC/SAFE-FREEZE/game NOT_RUN. SDK handshake/session crypto henüz entegre değil. Ayrıntılar ve test kapsamı docs/REPORT_TR.md içinde.
