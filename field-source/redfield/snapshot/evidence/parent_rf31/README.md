# REDFIELD RF31

LOCAL_BUILD / NOT_RELEASE. M20.3 Golden/resource/link/local dist entegrasyonu.
262 uygulama dosyası RF30 ile byte aynı;96 temiz Linux regresyon PASS.
61 Golden ve62 profil/resource/dist QA beklenen sonucu verdi;PE sentetik,
Windows süreçleri taklit.Gerçek Windows/MSVC/oyun kabulü NOT_RUN.

Windows developer DLL:scripts\build_orchestration\build_verify.cmd.
Çıktı:build\verify\msvc_x64\preloader_l.dll;surge.cfg yanında olmalıdır.
Production:aynı dizinde build_production.cmd;aday/resource/ABI/link denetimi
ve gerçek beş Golden gate geçerse dist\preloader_l.dll yazılır.
Debug/native/CRT girişleri ayrı profil,objeler veCRT kullanır.

294 maintained-code+4 vendor qualification PENDING;16 modül source review açık.
--require-refactor-complete beklenen pending reddi verir;tüm refactor bitmedi.
Profil oyun bildirimi RFCL-005 açık;legacy GetConfig exit14 ayrı diagnostic.
Ayrıntı:docs/REPORT_TR.md,docs/GOLDEN_PIPELINE_TR.md vemetadata/REFACTOR_REVIEW.json.
