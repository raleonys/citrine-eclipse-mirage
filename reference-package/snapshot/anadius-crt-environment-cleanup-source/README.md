# Dar ve geniş environment temizliği

Bu modül, değişmeyen `anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` içindeki **4 yeni gövdenin** bağımsız C++20 yeniden yazımıdır. Özgün DLL çalıştırılmadı veya değiştirilmedi. Kullanıcının gerçek environment verisine erişilmez.

| Giriş | Yeniden yazılan işlev | Normal akış |
|---|---|---|
| `0x180063f60` | `uninitialize_narrow_063f60` | Seçili pointer kimliği narrow-initial kimliğinden farklıysa serbest bırak |
| `0x180063f7c` | `uninitialize_wide_063f7c` | Aynı işlem wide-initial kimliğiyle |
| `0x180064010` | `uninitialize_environment_064010` | Narrow-current, wide-current, narrow-initial, wide-initial sırasını koru |
| `0x1800644a0` | `uninitialize_callback_0644a0` | Önce `064010`, ardından `AL=1` başarı değeri |

`0644a0` yalnız narrow temizliği değildir; hem narrow hem wide pointer vektörlerini temizler. Gerçek bounded `free_environment_063d80` gövdesi mevcut `../anadius-crt-environment-source/environment.cpp` kaynağından değiştirilmeden çağrılır; bu gövde yeni kurtarılan dört gövde sayısına eklenmez.

`State`, mevcut narrow `environment::State` nesnesine ve iki ek wide temizleme görünümüne **referans** tutar. Current ve initial alanları aynı pointer vektörüne gidebilir. Helper'lar `.data` kimliğini karşılaştırır; bounds metaverisi kimlik karşılaştırmasına katılmaz. `064010` sonraki alanları ancak önceki temizleme etkileri tamamlandıktan sonra okur. Örneğin narrow-current free etkisi wide-current veya initial alanlarını değiştirdiğinde sonraki işlemler güncel alanları görür. Ortak serbest bırakma gövdesi de pointer dizisinin sonraki öğesini her free etkisinden sonra tekrar okur.

Wide alanlar **temizlemeye özel**, sınırları belirli, canlı `char*` adres nesnelerinden oluşan vektörlerdir. Payload byte'ları UTF-16 içerik taşıyabilir; bu modül içeriği çözmez veya dönüştürmez. Arabirim native `char16_t**` ABI'sini temsil etmez ve böyle bir diziyi `char**` olarak yeniden yorumlamaz. Bu sınır mevcut ortak serbest bırakma kaynağını C++ nesne ömrü ve tür kurallarını bozmadan kullanmak içindir.

Özgün gövdeler global pointer'ları sıfırlamaz; kaynak da sıfırlamaz. Tamamlandıktan sonra alanlar serbest bırakılmış nesnelerin kimliğini taşımaya devam edebilir. Aynı temizliğin yeniden çağrılması için yeni sahiplik/ömür hazırlığı çağıranın sorumluluğudur. Yeni idempotence davranışı, kilitleme veya SEH/finally politikası eklenmemiştir. Borrowed state, provider ve pointer vektörleri işlem süresince canlı kalmalı; paylaşılan kullanımlar çağıran tarafından senkronize edilmelidir.

## Kanıt ve başlangıç tabloları

`collect-evidence.cjs`, özgün DLL ve iki saklanan REA çıktısının hash'lerini doğrular; seçilen tam pseudocode/assembly gövdeleri ile EvidenceID değerlerini `evidence.json` içine yazar. Yeni REA oturumu açmaz ve yeni EvidenceID üretmez. İlgili `A5288/A5290/A5298/A52A0` alanları ve normal/quick onexit tabloları PE `.data` sanal sıfır doldurma alanındadır; JSON bunların dosyada saklanan baytlar olmadığını açıkça gösterir.

Başlangıç çift tablosunda `0x18008ab48` uninitializer slotu `0644a0`, `0x18008ab50` initializer slotu `06447c` değerindedir. İki pointer için gerçek PE DIR64 giriş sözcüğü ve dosya ofseti kaydedilmiştir. `06447c`, normal `A52A8` ve quick `A52C0` tablolarını aynı cookie ile başlatır; bu ayrı modülün kapsamı dışındadır. Ortak durum kurarken gerçek normal tablo `termination::State.onexit.crt_0a52a8`, quick tablo `termination::State.quick_table_a52c0` olmalıdır; `onexit::State` kopyası üretmek registration/termination kimliğini ayırır.

## Doğrulama

```powershell
node outputs/anadius-crt-environment-cleanup-source/collect-evidence.cjs
& outputs/anadius-crt-environment-cleanup-source/build.ps1
```

Mevcut LLVM-MinGW ile üretim nesnesi ve bağımsız test EXE'si `C++20 -O2 -Wall -Wextra -Werror` bayraklarıyla derlenir; ortak gövde mevcut kısmi arşivden bağlanır. **447 kontrol**; 25 narrow/wide NULL/alias/farklı kimlik kombinasyonu, bütün free sırası, alanların sıfırlanmaması, işlem arası initial/current değişimleri, dizi öğelerinin sonraki free öncesi değişmesi ve bounded unterminated girişin açık reddini denetler. Test pointer vektörleri gerçek canlı `char*` nesneleridir; test provider'ı sahip olduğu depolamayı teardown'a kadar tutar ve çift serbest bırakmayı reddeder. Sonuç ve kaynak/arşiv hash'leri `validation.json` içindedir. Özgün ABI veya bütün DLL davranışı eşitliği kanıtlanmış değildir.
