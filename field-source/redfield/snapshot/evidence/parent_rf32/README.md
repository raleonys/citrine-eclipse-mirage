# REDFIELD RF32

LOCAL_BUILD / NOT_RELEASE. Native test kodu production DLL girişinden ayrıldı.
İki ayrı giriş tek WindowsWorker uygulamasını kullanır; production gözlemci
state taşımaz. Native test fault/counter/export kodu yalnız tests/native içindedir.
97 temiz Linux regresyon PASS; önceki 96 stdout/exit aynı. Eski/yeni worker için
16 karşılaştırma,32 kontrollü yürütüm geçti; Win32/hook sınırları modeldir.
Gerçek Windows/MSVC/oyun kabulü henüz NOT_RUN.

Windows developer DLL: scripts\build_orchestration\build_verify.cmd.
Çıktı build\verify\msvc_x64\preloader_l.dll; surge.cfg yanında bulunmalıdır.
Production girişi aynı dizinde build_production.cmd; beş gerçek Golden gate
geçerse dist\preloader_l.dll yazılır. Debug/native/CRT ayrı profile/objeye sahiptir.

29 eski native/CRT senaryosu ve4 eski Debug hostunun açık kapsamı
docs/ACCEPTANCE_COVERAGE_TR.md içinde kayıtlıdır; tam parity tamamlanmadı.
298 maintained-code+4 vendor qualification PENDING;16 modül source review açık.
Legacy GetConfig exit14 ayrı tanıdır; oyun profil bildirimi RFCL-005 açıktır.
Ayrıntı: docs/REPORT_TR.md ve metadata/REFACTOR_REVIEW.json.
