# Host başlangıcı — kısmi C++ kaynak

Bu modül, bootstrap içindeki iki worker arasında çağrılan `0x18000e310` işlevini ve `0x18000f120` / `0x180046970` yardımcılarını uygulama mantığı düzeyinde yeniden kurar. Kaynak `host_init.cpp`, arayüz `host_init.hpp` içindedir. Dayanak, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olan anadius64.dll için saklanan REA/Ghidra assembly ve pseudocode kayıtlarıdır. Orijinal dosya değiştirilmedi ve çalıştırılmadı.

`StateRefs`, mevcut `configuration::State`, registry ikame değerleri haritası, etkin hedefler vektörü ve on original-pointer slotuna bağlanır. Bootstrap bağımlılığının `between_workers_00e310()` metodu `initialize_host_00e310(refs,effects)` çağırabilir. Yol kuralları paths.cpp içindedir; filesystem kod sayfası için encoding modülüne bağlı bir Effects gerekir.

İşlem sırası:

1. İkame haritasını temizler; ClientKey, ClientKey2, ClientValue ve ClientPath eklenir. İlk üç değer filesystem kod sayfasından UTF-16'ya dönüştürülür. Dönüşümden önce boş map değeri eklenir; hata kısmi haritayı bırakır.
2. Dil registry yolu boşsa registry_spoof bayrağını sıfırlar. Yol varsa parent/filename ile LanguageKey/LanguageValue kurar. INSTALLED_LANGUAGE yoksa boş değer eklenir ve Language dönüşümü yapılır. Dolu yol diğer spoof bayrağı değerlerini değiştirmez.
3. Etkin hedefler vektörünün boyutunu sıfırlar; önceki hook'ları burada kaldırmaz ve original slotları sıfırlamaz.
4. `(type=2,condition=3)`, `(1,3)`, `(0x20,3)` koşul maskeleri oluşturulur. Major6, minor2, service-pack-major0 ve type mask0x23 ile sürüm sorgulanır. Bu statik istektir; gerçek OS sürümü gözlemi değildir.
5. Hook motoru başlatılır ve sonuç loglanır. Zaten başlatılmış anlamındaki1 dahil her sıfır olmayan sonuç bu çağrıda kurulumu durdurur.
6. Connect ve beş registry hook'u istenir. Modern dal registry için KernelBase.dll, diğer dal advapi32.dll seçer. block_reruns sıfır değilse iki CreateProcess hook'u; documents doluysa iki SHGetFolderPath hook'u eklenir. Altı ila on istek vardır. Dil spoof bayrağı sıfır olsa da registry hook istekleri atlanmaz.

046970 yalnız mevcut modül handle'ını arar; LoadLibrary yapmaz. Modül yoksa11, sembol yoksa12 döner. Hedefi patch motorundan önce dışarı yazar. 00f120, create başarılı olunca enable çağırır; iki sonuç sıfırsa hedefi vektöre ekler. Create/enable hatası loglanır ve sonraki hedefe geçilir. Backend'in original slotuna yaptığı yazı başarısız normal dönüşte de korunur. Rollback eklenmez; exception sonraki işlemleri durdurur.

## Bağlantılar ve sınırlar

Bu modül bağımsız DLL değildir. On callback gövdesi registry-source (5), host-callbacks-source (2 process/2 folder) ve network-hook-source (connect) klasörlerindedir. Son klasör 0102b0 exit cleanup'ını da içerir. `callback-bindings.json`, enum/adres/original-pointer-global/kaynak eşlemesini verir.

`../hook-engine-source`, seçilen resmî MinHook motorunu ve HostInitEffects/CleanupEffects bağlayıcılarını içerir. Bu, özgün motorun kesin sürüm eşitliği değildir. `../anadius-native-source`, on SDK girişini, derlenen adresleri döndüren Resolver'ı, process-global Context binding'i ve SDK tipli original trampoline çağrılarını sağlar. Binding hook kurulmadan önce yayınlanmalı; original slot dizisi bütün katmanlarda aynı olmalıdır. Tercih edilen özgün binary VA'ları kaynak callback adresi yerine kullanılamaz.

Gerçek app/CRT/log/locale/copy politikaları, tam loader/shutdown bağları ve özgün MSVC EH/SEH/XFG eşitliği açık sınırlar olarak durur. Başarı döndüren üretim stub'ı yoktur. std::map/vector/u16string, özgün MSVC ağaç düğümü, allocator ve SSO kapasite ABI'si değildir. Sentetik allocation/exception prefix testleri özgün allocator hata zamanlamasının eşitliğini kanıtlamaz. Gerçek durum paylaşımı ve senkronizasyon çağıranın sorumluluğudur.

## Doğrulama

`build.ps1`, C++20/O2/Wall/Wextra/Werror ile bir üretim nesnesi ve statik sentetik test EXE'si derler. paths.cpp bağımlılığı vardır. **6013 kontrol geçti:** konfigürasyon kombinasyonları, modül/callback sırası, init/create/enable/lookup hataları, output slotları, loglar, dış çağrı exception prefix'leri, embedded NUL ve dil alanı yokluğu. Gerçek registry, modül yükleme, hook veya oyun süreci çağrılmaz.

Native girişlerin host-init üzerinden ortak doğrulaması anadius-native-source testlerinde bulunur. Adres/Evidence kimlikleri `evidence.json`; tarihsel build kaydı `validation.json`, güncel ortak sonuç `common-validation.json` içindedir. Eski README hashleri önceki belge sürümüne aittir; güncel hashler source-workspace/documentation-snapshot.json ve paket manifestinde tutulur.


