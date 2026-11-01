# REDFIELD RF36

CRT_STARTUP_FIX01 · LOCAL_BUILD / NOT_RELEASE.

RF35 kullanıcı MSVC production logu 74 kaynak dosyasını derliyor; DLL girişinde
WindowsWorker constinit bildirimi C2127 veriyor. RF36, inert worker nesnesini
mevcut varsayılan CRT başlangıcıyla kurar. Production ve native girişleri aynı
kurala geçirildi. Constructor yalnız üye durumunu ve callback adreslerini kurar.

ZIP'i yeni klasöre açın. Production DLL için:
scripts\build_orchestration\build_production.cmd
Başarılı tüm pipeline sonrası çıktı: dist\preloader_l.dll.
Geliştirici regresyonları için build_verify.cmd aynı klasördedir.
surge.cfg DLL'in yanında olmalıdır. Eski build çıktısını taşımayın.

98 temiz GCC regresyon PASS; RF35'in98 stdout/exit sonucu aynı. 22 kontrollü
production/native karşılaştırması (44çalıştırma),7modül testi,15CRT-entry negatif
ve2constructor yan-etki mutasyonu geçti. Bunlar gerçek MSVC/SDK/DLL testi değildir.
Gerçek RF36 Windows/MSVC/native/CRT/Debug/oyun NOT_RUN.

RF35 namespace veRF34 lifetime çalışmaları korunur. Kaynak incelemesi6modül
kapalı/14açık;16code qualified/284code+4vendor pending. Yeni modül kapanışı yok.
