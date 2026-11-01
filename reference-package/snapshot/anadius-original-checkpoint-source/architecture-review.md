# B24 bağımsız mimari inceleme

Sonuç: **PASS**. İnceleyen `/root/b22_provider_audit`, üretim ve test yazarlarından bağımsızdır.

Seçili original çağrısı üç gerçek PTD kontrolü kullanır: resolver/slot erişimi öncesi, resolver döndükten sonra SDK öncesi ve SDK normal/exception dönüşü sonrası. Resolver exception atarsa da postcondition kontrol edilir. Bu sıra, FLS’yi değiştirip normal dönen resolver’ın SDK’ya ulaşmasını önler. Middle precheck hatası SDK çağırmaz ve gereksiz SDK postcheck çalıştırmaz.

On SDK-typed original yolunun tamamı ortak helper’a bağlıdır. Argümanlar, 32-bit dönüş/bitcast, LastError, output prefix ve tail korunur; retry, rollback veya FLS onarımı yapılmaz. Geçerli postcondition aynı exception’ı yeniden atar; geçersiz postcondition öncelik kazanır. Kontrol sırasında metadata kilidi SDK’ya taşınmaz.

PolicyStorage, callback_scope varsa B23 code owner seçili olsun veya olmasın Boundary oluşturur. Böylece foreign factory’de sahipli ve raw original yolları korunur. Scope bulunmayan direct legacy yol değişmez. B18–B23 admission, actual-FLS, slot seal/code lifetime, same-Execution, shared stream ve compiler-provider sözleşmeleri sürer. Sınıf düzeni değiştiğinden tüm tüketiciler yeniden derlenmelidir.

Root’un **56 core**, `/root/b22_integration` ajanının **28 integration** PASS kaydı; sekiz kaynak ve iki EXE hash’iyle doğrulandı, testler yeniden çalıştırılmadı. B23 arşiv kimliği ve 818 eski kaynak doğrulandı: **814 aynı**, **4 açık değişiklik**, **4 yeni kaynak**, toplam **822**. Ortak build sürüyordu; planlanan 242 nesne / 238 CPP / 279 grup + 19 terminal senaryosu bu incelemede PASS sayılmadı.

Core her on SDK stub’ının kesin argümanını, sonucunu ve üç kontrolünü doğrular. Integration actual-FLS olaylarını sahipli yol için G,R,G,S,G; raw yol için G,G,S,G olarak ayırır. Bir native çocuk, geçerli PTD altında SDK exception’ının aynı kimlikle PTD/native lease drain sonrası error policy’ye ulaştığını kanıtlar. Beş manual-admitted çocuk, null FLS/snapshot/resolver hatalarında prefix’i ve poisoned aktif sahipliği koruyup sonraki resolver/SDK çağrısını engellediğini doğrular. Bu poisoned testler full native wrapper’ın bilinmeyen PTD’yi başarıyla temizlediği iddiası değildir. Başarı kodları 201–206, genel abort 239 ve beklenmeyen dönüş 230’dan ayrıdır.

Shared token kod kökeni, module pin, hook başarısı, native ABI veya explicit engine unload koruması sağlamaz. Checkpoint’ler sürekli PTD dondurmaz; raw free/ABA, yanlış getter, kontrol aralarında değişip geri gelen durum ve arbitrary reentry desteklenmez. Dış quiescence ayrı kanıt olarak kalır.

**Yeni native gövde/kapanış: 0; DLL bütünü hazır değildir.** Generic CRT, native ABI/PE/hooks/unload, yedi online foreign call, configuration/license/legacy document ve B10 borcu açıktır. Orijinal/rebuilt hedef DLL veya oyun çalıştırılmadı; kullanıcı içeriği okunmadı. Ayrıntılı kanıt ve hash’ler `architecture-review.json` içindedir.
