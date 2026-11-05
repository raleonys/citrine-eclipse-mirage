# Hook başına original adresinin yayımlanması

Durum: kaynak uygulaması tamamlandı; derlenmedi veya çalıştırılmadı. İnceleme yalnız yazarın statik kaynak incelemesidir. Toplam DLL tamamlanma yüzdesine yeni bir oran eklenmedi.

Önceki REA/Ghidra kaydı `../anadius-host-platform-source/synchronous-hook-order.audit.json` içinde korunur. Bu aşamada yeni REA sorgusu yapılmadı. Bu kayıttaki `0x1800466e0` create işlemi original çıktısını yazar; `0x18000f120` içinde create çağrısı `0x18000f14a`, başarıdan sonraki enable çağrısı `0x18000f163`, başarılı enable sonrasındaki target ekleme ise `0x18000f1b8` çevresindedir. `0x180046a10` enable yönlendirmesidir. Bunlar sıralama kanıtıdır; yeni C++ kodunun çalıştığını kanıtlamaz.

## Kaynak değişikliği

`originals.hpp/.cpp` içine tek sefer bağlanan `Publication` eklendi. Gerçek Runtime original dizisinin kimliğini, callback slotunu, target adresini ve create çıktı hücresini eşleştirir. Başarılı normal create dönüşünde yalnız ilgili adresi mühürler; enable öncesinde aynı target için başarılı create kaydı aranır. Başka slotlar kurulurken yayımlanmış adres mutex üzerinden başka bir okuyucuya sunulabilir. Setup sırasında diğer ham hücreler okunmaz.

Normal create hatası, çıktı hücresi sıfırdan farklı olsa veya değiştirilmiş olsa da yayın oluşturmaz. Henüz yayımlanmamış slot açıkça yeniden denenebilir. Yayımlanan slotun yeniden create edilmesi alt çağrıdan önce reddedilir. Create/enable sırasında sonucu belirsiz bir exception kalıcı Poisoned durumu oluşturur; stop bunu temizlenmiş saymaz. Belirsiz sahipliğin yok edilmesi terminaldir. Bu, engine cleanup veya callback drain kanıtı değildir.

`HostPlatform` create/enable yolları bu protokole bağlandı. Seçilen `install_host` assembly, HostPlatform ve callback Policy için aynı paylaşılan Publication sahibini üretir. Mevcut `OriginalProvider` sanal arayüzü ve legacy bind/make_factory yolu korunur. `host_platform::Resources` alanı eklendiği için Resources/Domain ve bunları içeren AssemblyInputs yerleşimi değişmiştir; etkilenen tüketiciler sonraki açık rebuild talimatında yeniden derlenmelidir.

Kod sahibi, gerçek engine/trampoline kodunu yaşatmalıdır; rastgele bir nonnull token yeterli kanıt değildir. Controller işlemleri aynı thread/fiber üzerinde kalır. Protokol dışı ham slot yazımı desteklenmez. Metadata kilidi alt engine/original çağrıları boyunca tutulmaz. Publication tek başına bütün engine reentry veya eşzamanlı callback koşullarını çözmez.

## Çalıştırılmamış senaryolar

`originals_tests.cpp` içinde tek bağlama, yanlış hücre, seeded slotun reddi, create sonrası foreign metadata okuması, yanlış target, normal başarısız create ve retry, LastError, stop ve kod sahipliği senaryoları eklendi. Ayrı terminal senaryo belirsiz create için 188 çıkışını bekler; gözlenmiş bir sonuç değildir.

`platform_integration_tests.cpp`, gerçek HostPlatform adaptöründe sahte engine enable gövdesinden önce başka thread üzerinden yayımlanan adresi okumayı tarif eder. İlk create hata döndürüp çıktı yazdığında sonraki beş hook'un kurulmaya devam etmesini ve başarısız slotun yayımlanmamasını bekler. Sahte adresler çağrılmaz. Bu senaryo tam seçilmiş Assembly'nin çalışma doğrulaması değildir; assembly eşleştirmesi yalnız kaynakta incelendi.

## Açık kalan iş

Genel foreign-callback warming/readiness kapısı değişmedi. Dolayısıyla bu aşama erken callback gövdesini açmaz ve özgün senkron loader başlangıcının güvenliğini kanıtlamaz. Sonraki somut iş, connect passthrough ve bekleme yollarının bağımlılıklarını ayrı değerlendirerek hook başına yayın kaydını callback kabulüne bağlamaktır. Stop, PTD, state ve kod sahipliği koşulları korunmalıdır. Loader hook tanımları, rollback ve image/foreign/CRT finality açık kalır.

Bu aşama 3 üretim CPP, 3 üretim header ve 3 senaryo/destek dosyasını değiştirdi. Son tam build ile farklı toplam kaynak/header sayısı 80'dir; bu bir tamamlanma ölçüsü değildir. Derleme, proje testi, link, DLL çalıştırma, ZIP paketleme ve özgün dosya erişimi/değişikliği yapılmadı. Eski build kanıtları tarihsel olarak korundu. Kesin dosya hashleri ve kaynak satırları `per-hook-publication.evidence.json` içindedir.
