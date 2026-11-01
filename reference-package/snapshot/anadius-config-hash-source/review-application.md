# Config2 mutable uygulama çekirdeği: bağımsız kaynak incelemesi

Sonuç **PASS**, engelleyici bulgu yok. `application.hpp/.cpp`, `application_tests.cpp` ve mevcut consumer callsite denetimi okundu; üretim kaynaklarına dokunulmadı. Bu inceleme testleri tekrar çalıştırmadı. Hash'e bağlı kaynaklar ve gözlenen standalone95 kontrol kaydı JSON eşinde tutulur.

Dosya akışı gerçek03b3f0→fresh parser→selected Config2 copy kullanıyor; tek BOM parser içinde işleniyor. Parser/input owned storage, path log'undan önce yok edilmiş durumda; dönen hash Node tam sahibi. Read/parse hataları log/apply öncesinde yayılıyor; log hatası ayar uygulamasını başlatmıyor.

Mutable bölüm destination Game→Emulator→User→Content→Entitlements→Achievements sırasıyla yeniden kullanılıyor. Ayrı record destination Content ve Entitlements arasında korunuyor. Lookup'lar yardımcılardan önce topluca yapılmıyor; gerçek alan uygulaması ile sırayla yürütülüyor. Böylece normal source-order kapasite geçmişi ve helper exception öncesindeki kısmi state korunuyor. Language flag, path/registry conversion callback'inden sonra güncel state'ten okunuyor; bunu doğrulayan callback fixture'ı var.

Beş sıralı tüketim dalı doğru stage'de: Content/Entitlements outer list, AchievementNames/Totals/SteamIdOverrides contained child list. Recursive positional copy'nin eski destination child kapasitesi etkisi korunuyor. Root tests, aynı Content source için önceki kapasite64/128 değişiminin farklı sıra üretmesini ve önceki Entitlement payload512 kapasitesinin AchievementTotals kısmi hata sırasını değiştirmesini açıkça kontrol ediyor. Bunlar yalnız key sayısını veya lexical sıralamayı yeniden doğrulayan testler değildir.

Achievements lookup sırası IDs sonra Set'tir; iki mutable lookup belirsiz C++ argüman değerlendirme sırasına bırakılmamış. Override, User sonrasında döner ve populated koleksiyonları özgün normal akış gibi atlar. Mevcut state SteamAppId boş değilse Prefix lookup yapılır; CFG'deki boş değer eski değeri silmez. Base koleksiyon vektörleri append eder. Kaynaklardaki Name/Version/State ve Group/Version/Type/Tag lookup sıraları callsite audit ile uyumludur.

NativeFactory testi yalnız kendi `application-generated-fixture.cfg` dosyasında çalışır; kullanıcı CFG/token okunmaz. Özgün DLL/oyun çalıştırılmaz. Path/locale/license/persistence/OS sağlayıcıları açık bağımlılıklardır. İnceleme native allocation/fault, MSVC STL/allocator/SEH/ABI veya tam DLL davranış eşdeğerliği kanıtı değildir.
