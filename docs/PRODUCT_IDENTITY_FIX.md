# SURGE ürün adlarının tutarlılığı

Kullanıcı talimatı: “bunlar tutarlı şekilde düzeltilmeli”.

`include/preloader_l/core/product_identity.hpp` config, override, veri klasörü,
avatar ve online DLL adlarını tanımlar. Config projeksiyonları ve somut runtime
aynı adları kullanır. Config arama konumu EXE dizini, öncelik base -> override,
varsayılan veri kökü %LOCALAPPDATA%/SURGE olarak kalır. Eksik config isteğe
bağlıdır; eski dosya adlarına otomatik geri dönüş veya dosya taşıma eklenmez.
Alt config projeksiyonundaki eski iki bileşenli veri kökü de mevcut runtime
SURGE köküyle eşitlenir. Online modülü doğrudan preloader_l_online.dll ister;
uygulama adaptöründe eski adı yeni ada çeviren özel koşul kaldırılmıştır.

Dört config hata mesajı surge.cfg der. Shutdown kaynak türü DllShutdownStatus,
sabitler DLL_SHUTDOWN_* olur; değerler0..4 ve DllMain@1/SURGE@2/DllShutdown@3
korunur. Hostlar ve mevcut config/provider/diagnostic testleri buna göre
güncellenir. Production'a yeni export eklenmez.

Orijinal kaynak dosyalarına işaret eden iki köken yorumu, geçmiş raporlar,
karşılaştırma tabloları, lisans/kaynak atıfları ve yellowfield.* JSON şema
kimlikleri korunur. Bunlar ürünün kullandığı dosya adı veya görünen kimlik
değildir. Test host'unda kaldırılmış YellowfieldShutdown export'unun yokluğu
denetlenmeye devam eder. Önceki ZIP'ler ve vendor payload'ları değiştirilmez.

Doğrulama sonuçları metadata/evidence/PRODUCT_IDENTITY_FIX.json içindedir. Cross-build
ve Wine sonuçları gerçek Windows/MSVC Golden veya oyun kabulü yerine geçmez.
