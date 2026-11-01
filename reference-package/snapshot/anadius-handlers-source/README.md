# LSX yapılandırma, profil ve bilgi yanıtları

`handlers.cpp`, yedi LSX yanıt işleyicisinin kararlarını, XML çağrı sırasını ve veri okuma/ekleme davranışını C++20 olarak yeniden kurar. **Bağımsız DLL değildir.** XML reader/printer ve günlük/sistem zamanı yordamları zorunlu arayüzlerdir; gerçek serialization/escaping bu modülde uydurulmaz. Diğer mesajlar, dispatcher, encryption ve socket/session katmanları ayrı işlerdir.

İncelenen anadius64.dll SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Adresler bu DLL'nin tercih edilen tabanına aittir. `evidence.json`, her işlevin tutulan REA pseudocode/assembly Evidence kimliğini, sabit metinlerin VA/ham ofset/baytlarını ve bağımsız GetConfig tablo kontrolünü içerir. Yeni C++ özgün kaynak değildir.

| İşleyici | Özgün VA | Kaynaktaki davranış |
|---|---|---|
| GetConfig | `0x18002e5f0` | 34 Service kaydı, özgün ad/facility/sıra; 35 open/close çifti |
| GetProfile | `0x18002f530` | index zorunlu; dolu/boş/nümerik olmayan değer kabul edilir; 14 profil attribute'u |
| GetInternetConnectedState | `0x18002f2d0` | online veya PretendConnected sıfır değilse connected="1", aksi halde "0" |
| GetSetting | `0x18002f7c0` | SettingId alanı ve mevcut anahtar gerekli; bilinmeyende false/no output |
| GetSettings | `0x18002fa60` | Sabit sırayla altı ayar okunur; eksik map anahtarları boş değerle eklenir |
| GetGameInfo | `0x18002efd0` | GameInfoId alanı ve mevcut anahtar gerekli; GetSetting ile aynı ikinci lookup sırası |
| GetAllGameInfo | `0x18002da90` | 13 oyun alanı, HasExpiration="false", ardından sistem zamanı |

GetConfig servis tablosu manuel listeyle yetinilmeden tutulan pseudocode'daki sabit pointer/literal çiftlerine karşı ayrı script ile kontrol edildi: **34 çiftin metni ve sırası eşleşti**. `.rdata` sabit okumaları bağımsız salt okunur dosya gözlemidir; yeni REA Evidence olarak sunulmaz. Önceki `.rdata` kaydıyla kaynak kimliği ilişkilendirilir.

GetProfile, eksik index için log veya XML yazmadan false döner. Boş index varsa yanıt üretilir; sayı aralığı veya profil seçimi eklenmez. Country/GeoCountry/CommerceCountry aynı country alanını kullanır; AvatarId boş string, IsTrialSubscriber sabit false olur. Diğer değerler çağıranın profil durumundan gelir. Kullanıcının gerçek CFG/kimlik değerleri kaynak/testlere konulmadı. Günlük kullanıcı adını uzunlukla işlerken XML attribute'ları NUL sonlandırmalı C-string alır; test bu farkı kontrol eder.

GetSetting/GetGameInfo önce anahtarı arar; bulamazsa map'e kayıt eklemez. Başarılı aramadan sonra response açılır, ardından özgün operator[] yolu tekrar kullanılır. GetSettings/GetAllGameInfo ise doğrudan operator[] yapar ve eksik anahtarları ekler. XML hatası sırasında daha sonra okunacak alanlar önceden eklenmez; map değişiklikleri ve yarım XML, gerçekleşen çağrı noktasına kadar kalır. Kaynak yeni bir rollback eklemez.

`StateReferences` iki haritayı (`0x1800a3d88`, `0x1800a3d78`), profil verilerini ve online/pretend bayraklarını çağırandan alır. Gerçek global başlangıç değerleri tahmin edilmez. `Profile` ve std::map, MSVC belleği üzerine cast edilecek ABI yapıları değildir. XML ve profile state için eşzamanlı/yeniden giriş yapan mutasyonların tüm register-pointer ayrıntıları, allocator/SSO/EH/SEH davranışı ve gerçek global bağlar tamamlanmamıştır.

`./build.ps1`, mevcut LLVM-MinGW ile C++20 -O2 -Wall -Wextra -Werror derlemesi yapar. **628 sentetik kontrol geçti**: profil index yok/boş/diğer değerler, bağlı bayrak kombinasyonları, C-string/NUL, servis şekli/sırası, negatif lookup ve map ekleme, başarısız XML yazımında kısmi durum ve zaman çağrısının sırası. Gerçek CFG, XML parser, ağ, UI veya özgün binary çalıştırılmadı. Bu testler özgün kod ile diferansiyel davranış eşdeğerliği kanıtı değildir.
