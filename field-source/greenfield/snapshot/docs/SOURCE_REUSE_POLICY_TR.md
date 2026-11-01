# Kaynak yeniden kullanımı ve yetki kaydı
Kullanıcı, yüklenmiş beş kaynak arşivi ve Library kaynakları için ihtiyaç halinde kod alınmasını açıkça onayladı. Yetki, provenance/lisans-bildirimlerini atma veya orijinal authorship'i sahiplenme izni olarak yorumlanmaz.

GF00'da farklı projelerden **hiçbir kaynak dosyası kopyalanmadı**. Tüm C++ / CMake / QA iskeleti yeni yazıldı. `exports/preloader_l.def` mevcut kamuya açık arayüz sözleşmesini yeniden ifade eden küçük bir metindir; canonical release'den kaynak implementasyonu taşınmamıştır.

Kod aktarılacağı zaman: kaynak arşiv SHA-256, path, hash, lisans bildirimi, üçüncü taraf dependency ve adaptation diff `metadata/CODE_REUSE_LEDGER.json` içine zorunlu kaydedilir. Portlanan modül için derleyici/platform uyumu ve negatif testler bağımsız çalıştırılır.

Arşiv lisansları: Official HF19 root `LICENSE` GPL-3.0-only; EAappEmulater `LICENSE.txt` MIT; Maxima-Draconis `LICENSE` GPL-3.0; OriginSDK arşivinde kök LICENSE bulunamadı; DLL reverse-engineering bundle'da tedarikçi MinHook/TinyXML2 bildirimleri var, bütün rekonstrüksiyon için tek kök lisans yok. Kullanıcının sunduğu ayrı kullanım yetkisi kaydedildi; downstream dağıtımın atıf/notice yükümlülüğü paket başına korunacaktır.
