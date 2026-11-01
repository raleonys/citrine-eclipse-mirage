# B23 bağımsız mimari inceleme

Sonuç: **PASS**. İnceleyen: `/root/b22_provider_audit`; üretim ve iki test grubunun yazarlarından bağımsızdır.

Seçili original sağlayıcısı, taşınmış RuntimeSeed’in exact on-slot dizisine bağlanır; açık code-lifetime sahibi factory, Storage ve final OriginalEffects boyunca tutulur. Context ilk yayınlandığında slotların dolu olması varsayılmaz: controller Setup sırasında güncel nonzero slotu kullanabilir; host setup tamamlanınca on slot mühürlenir ve ardından foreign readiness kanıtı çağrılır.

Ready aşamasında seçili slot değişirse, sıfıra değişme dâhil, sağlayıcı **Stopped** olur. Eski değerin geri yazılması çağrıları yeniden açmaz. Değişmemiş boş slot çağrılmaz; bu durum diğer değişmemiş slotları durdurmaz. Gerçek SDK çağrısı boyunca metadata kilidi tutulmaz ve LastError korunur.

Başarısız pause/close, in-flight çağrı ve false quiescence durumunda code owner bırakılmaz. Başarılı binding close sonrasında da owner, Context yok edilene kadar yaşar. Kaynak sınıf düzeni değiştiği için bütün tüketicilerin yeniden derlenmesi zorunludur; explicit out-of-line virtual destructor incelendi. Eski lifetime testi varsayımı için kanıtlanmamış ODR/weak-symbol nedeni iddia edilmez; test moved-from std::function capture’ını açıkça bırakır.

Root’un **41 core**, `/root/b22_integration` ajanının **24 integration** PASS kaydı; dokuz kaynak ve iki EXE hash’iyle doğrulandı. Testler yeniden çalıştırılmadı. B22 arşiv kimliği ve 814 önceki kaynak doğrulandı: **809 değişmeden**, **5 açık değişiklik**, **4 yeni kaynak**, toplam **818**. Ortak build sürüyordu; planlanan 241 nesne / 237 CPP / 277 grup + 19 terminal senaryosu bu incelemede PASS sayılmadı.

Integration kendi source native callback ve SDK stub’ını, sentetik hook engine, heap ve FLS alanlarını kullanır. Üç terminal çocuk 231/233/234 kodlarını yalnız warming/değişmiş slot/sıfırlanmış slot reddi, stub’a ulaşmama ve lease/PTD drain koşullarından sonra üretir; genel abort 239 ve beklenmeyen dönüş 230 başarı sayılmaz. Başarısız hook-create nonzero output prefix’i korunur; bu metadata hook başarısı veya çalıştırılabilir adres kanıtı sayılmaz.

Shared token gerçek modülü pinlemez, kod kökenini/ABI’yi doğrulamaz ve açık engine uninitialize/unload işlemini engellemez. Caller gerçek code owner’ı ve setup slot-write serialization’ını sağlamalıdır. Yeni original yolu alt SDK çağrısına PTD pre/post kontrolü eklemez. Provider Stopped, ForeignDomain poison ile aynı durum değildir. Setup tek kullanımlıdır; sealing sonrası remaining-proof hatası için mevcut activate_foreign retry yolu kullanılır. Dış quiescence ayrı kalır.

**Yeni native gövde/kapanış: 0; DLL bütünü hazır değildir.** Generic CRT, native ABI/PE/hooks/unload, yedi online foreign call, configuration/license/legacy document ve B10 borcu açıktır. Orijinal/rebuilt DLL veya oyun çalıştırılmadı, kullanıcı içeriği okunmadı. Ayrıntılı kanıt ve hash’ler `architecture-review.json` içindedir.
