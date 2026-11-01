# REDFIELD RF33

LOCAL_BUILD / NOT_RELEASE. Registry REG_SZ çıktısı, Windows dönüşüm/bellek
sahipliğinden ayrılan tek bir byte yazıcısına taşındı. Hizasız uint16_t yazımı
kaldırıldı. 98 temiz GCC regresyonu geçti; RF32'nin 97 testinin stdout/exit'i aynı.
Eski GetConfig exit14 ayrı FAIL_OBSERVED tanısıdır; normal PASS sayılmaz.

Windows developer DLL: scripts\build_orchestration\build_verify.cmd.
Çıktı build\verify\msvc_x64\preloader_l.dll; surge.cfg yanında bulunmalıdır.
Production: aynı dizinde build_production.cmd. Debug/native/CRT ayrı profillerdir.
Bu paket kaynak teslimidir; burada gerçek Windows/MSVC DLL üretilmedi.

268 uygulama dosyası: 265 parent dosyası aynı, bir mevcut dosya değişti, iki yeni.
İki yeni portable dosya source/test kapsamıyla qualified; 298 code + dört vendor
integration qualification ve 16 tüm-modül incelemesi açık. Debug/native/CRT
taşımasının kalan kısmı ve oyun profilinin kök nedeni tamamlanmış sayılmıyor.
Ayrıntı: docs/REPORT_TR.md ve metadata/REGISTRY_VALUE_OUTPUT_CONTRACT.json.
