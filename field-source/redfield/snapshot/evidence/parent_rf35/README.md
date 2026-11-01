# REDFIELD RF35

MSVC_NAMESPACE_FIX01 · LOCAL_BUILD / NOT_RELEASE.

RF33 kullanıcı logundaki C2386 hatasına neden olan gereksiz net namespace aliası
kaldırıldı. RF34 kaynak çalışması korunur; oyun/transport davranışı değiştirilmedi.
98 temiz GCC regresyon PASS; RF34'ün 98 stdout/exit sonucu aynı. Bu ortamda gerçek
Windows/MSVC derlemesi yapılmadı; tüm Windows zinciri için başarı iddiası yok.

ZIP'i yeni bir klasöre açıp scripts\build_orchestration\build_verify.cmd çalıştırın.
Windows DLL çıktısı: build\verify\msvc_x64\preloader_l.dll.
surge.cfg DLL'in yanında olmalıdır. Eski build klasörünü taşımayın.

Refactor: 6 modülün kaynak incelemesi tamam, 14 modül açık. 16 code QUALIFIED,
284 code ve 4 vendor integration PENDING. M09 TCP modülü bu dar düzeltmeyle kapanmadı.
