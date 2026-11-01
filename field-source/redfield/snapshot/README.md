# REDFIELD RF37

GOLDEN_BINDING_FIX01 · LOCAL_BUILD / NOT_RELEASE.

RF36'nın gerçek Windows DLL'i ve failure.json incelendi. Tek reddedilen import,
Windows core synchronization API setiydi. Mevcut96KiB registry yol tamponlarını
hesaba katmayan6KiB FALCION bütçesi dehedefe uyarlandı. Yalnız tam API-set adı ve
tam üçişlev kabul edilir;dinamik CRT/bilinmeyen importlar veveri aşımı reddedilir.
HF19'un dörtpolicy dosyası veprovenance kopyaları değiştirilmedi.

Gönderilen gerçek RF36 DLL'i beşGolden aşamasının tamamını byte değişmeden geçti.
RF37'de268application dosyası RF36 ilebirebir aynıdır.98temizGCCregresyon PASS;
RF36'nın98stdout/exitsonucu aynı. Gerçek RF37MSVC yeniden derlemesi yapılmadı.

Yeni klasöre açıp scripts\build_orchestration\build_production.cmd çalıştırın.
Başarılı zincir sonrası production çıktı:dist\preloader_l.dll.
Geliştirici regresyonları:aynı klasörde build_verify.cmd. surge.cfg DLL yanında.
Kaynakreview6modül kapalı/14açık;16codequalified/284code+4vendorpending aynı.
