# Online kitaplığı yükleyicisi ve profil önbelleği

`online.cpp`, anadius64.dll içindeki dört uygulama gövdesini ve bir ayırma yardımcısının kullanılan uzmanlaşmasını okunabilir C++20 olarak kurar. `recovered_effects.cpp`, daha önce kurtarılan Windows yol/kod sayfası, kitaplık yükleme, dosya durumu ve binary dosya kodunu birleştiren **yeni** bağlayıcıdır. Kaynak bağımsız DLL üretmez; bu API özgün MSVC sınıf/vtable/CRT ABI'si değildir.

Özgün dosya SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Aşağıdaki sonuçlar statik gözlemdir. Özgün DLL, online kitaplığı ve oyun çalıştırılmadı. Gerçek kullanıcı profili, cookie veya yapılandırma kimliği okunmadı. Testlerin bütün kimlikleri ve yolları sentetiktir.

| Özgün VA | Kurtarılan davranış | Assembly Evidence |
|---|---|---|
| `0x180035b00` | status(flags=3), normal dosya türü ve hata kuralları | `ev_1938a4b0b7d093daeda62b052bd63a5823849aada8b911905b5381b65447c5d0` |
| `0x180035b60` | aşama1 bekle, online yükle, koşullu normal temizlik, ikinci bayrağı bitir | `ev_d120db083716acb73a3aab91edb94f7ae2910ca33f5cd2a7baa1d1e72b62a030` |
| `0x180035bd0` | kitaplık/sürüm/factory, sağlayıcı durumu, canlı veya önbellek profil | `ev_42d4b70085bbc786efef3d59635d848e062d1ecab79b8bd4e827ea88833cf586` |
| `0x1800366f0` | nesne virtual+0, ardından FreeLibrary; her birini normal dönüşten sonra sıfırla | `ev_1fcea4b34ce7b7c2ba51c2e17ba2693f4d7fdb691fc078637e4d3b78e8cf139f` |
| `0x18003cf00` | kullanılan sınırsız `;` ayırma yolu; boş/NUL alanlar korunur | `ev_19bcf15ba615e9b59425d363b0e25771f6d86dec8bea66181a3c28381748b4bb` |

Adresler ve tekil talimatlar `evidence.json` içinde bulunur. Asıl listing üst dizindeki `anadius-all-functions.asm.txt`, pseudocode ise `anadius-all-functions.pseudo.c` dosyasıdır. Literal baytları, hash kontrolünden sonra `.rdata` (`VA=0x18007f000`, raw=`0x7d800`, size=`0x21e00`) sınırları içinde doğrulandı. `.rdata` Evidence: `ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082`.

## Korunan akış

Error mode önce eski değer OR1 olarak ayarlanır. `anadius64online.dll` literalinin 19 baytı (`0x13`, sonlandırıcı dahil değil) dosya kod sayfasıyla wide yola dönüştürülür. Çalıştırılabilir dosya yolunun parent'ına eklenip `LoadLibraryW` sınırına verilir; normal dönüşte error mode geri yüklenir. Dönüşüm/yol/yükleme bir exception üretirse bu gövdede otomatik geri yükleme eklenmez. Yükleme başarısızsa sessiz dönüş vardır.

Başarılı modülde `version_check` aranır; beklenen sonuç 2'dir. Eksik export günlüğe yazılır. Yanlış sürüm `crack error` / `Online library version mismatch!` / flags=`0x10` ileti yolunu ve ardından günlüğü çağırır. Sürüm uygunsa `create_online` export'u aranır ve cookie yolunun ayrı wide kopyasıyla çağrılır. Nesnenin virtual+8 sonucu 0 ise online bayrağı **günlük ve profil callback'lerinden önce** 1 yapılır. Virtual+0x10 kullanıcı adı, +0x18 persona kimliği, +0x20 kullanıcı kimliği döndürür; NUL ile sonlandırılmış C-string olarak kopyalanır.

Sonuç 0 dışındaysa offline günlüğü vardır; yalnız sonuç 1 önce `authentication failed, ` ekler ve iki parça tek flush ile tamamlanır. Önceki profili kullanma bayrağı ve cookie dosyasının normal dosya olması gerekir. Son avatar dosyası varsa avatar yolu kopyalanır; ardından profil dosyası binary okunur. İçerik `;` ile ayrılır; **yalnız tam üç alan** persona, kullanıcı kimliği ve kullanıcı adı olarak atanır. Baştaki/sondaki boş alan ve metnin içindeki NUL korunur; fazla veya eksik alan mevcut kimlikleri değiştirmez. Bu davranış profil içeriğini doğruladığı veya kimlik doğrulama yaptığı anlamına gelmez.

Canlı profil yolunda cookie ve son avatarın dosya olması avatarı günceller. Profil kullanma bayrağı açıksa `persona;user;username` binary yazılır; yazma bool sonucu göz ardı edilir. Bu bayrak online kitaplığının aranmasını/yüklenmesini engellemez.

`exists_035b00` genel bir exists fonksiyonu değildir: yalnız status.kind==2 döndürür. Hata sıfır değilse tür 1 veya 9 dışındaki türlerde `filesystem_error_018130("status", error, path)` gerekir. Temizlikte destroy veya FreeLibrary çağrısı exception üretirse sonraki işlemler yapılmaz. FreeLibrary false sonucu yine handle'ın sıfırlanmasını engellemez. Worker yalnız online bayrağı sıfırsa normal temizlik yapar; normal sona ulaşırsa `a3c3c` bitiş bayrağını bildirir. Loader başlangıçta eski online/nesne/profil durumunu sıfırlamaz. Genel cleanup online bayrağını değiştirmez.

## Bağlama ve sınırlar

`StateRefs`, ilgili global değerleri çağıranın sağlamasını ister; varsayılan global başlangıç durumu icat edilmez. `Effects`, export araması, sağlayıcı callback'leri, günlük/UI, filesystem exception ve aşama bayrağı gibi gerçek sınırları zorunlu tutar. Sağlayıcı nesne/procedure değerleri opaque token'lardır; kaynak bunları gerçek adreslere cast edip çağırmaz. Provider çağrıları için sahte başarı gövdesi yoktur.

`RecoveredEffects(remaining, librarySystemCalls, stats, encodingCalls, encodingErrors, fileFactory)` seçilmiş bağımlılıkları birleştirir. Yol kuralları için `paths.cpp`, kod sayfası için `encoding.cpp`, statü için `status.cpp`, dosyalar için `binary_io.cpp`, suppress/load kopyası için `libraries.cpp` kullanır. UTF-16 birimleri Windows `wchar_t` birimlerine tek tek kopyalanır. `remaining` hâlâ gerçek sağlayıcı/export/günlük/exception/worker bağlarını sağlamalıdır. Native adapter seçilmesi dosya veya DLL çağrısı yapabilir; bu modülün testleri yalnız mock sınırlar seçer.

Özgün kod null factory sonucu veya null profil C-string'i dereference eder. Rekonstrüksiyon bunları `OutOfEvidenceInput` ile açıkça sınır dışı ilan eder; bu exception özgün davranış iddiası değildir. Dönen C-string okunabilir, NUL ile sonlandırılmış ve callback kopyası boyunca geçerli olmalıdır. Factory'nin cookie parametresini çağrı sonrasında kullanma davranışı desteklenmez. C++ container/allocator hata zamanlaması, MSVC EH/CRT ABI'si, eşzamanlı global erişim ve sağlayıcının gerçek implementasyonu geri kazanılmış değildir. Genel `03cf00` için limitli/boş ayırıcı/argüman move ABI'si bu uzmanlaşmanın kapsamı dışındadır.

## Derleme ve doğrulama

`build.ps1`, yerel llvm-mingw ile `-std=c++20 -O2 -Wall -Wextra -Werror -static` kullanır. Bağımsız synthetic test executable'ı dışında hedef çalıştırmaz. Sonuç: **12.869 kontrol PASS**. Testler yükleme/export/sürüm dalları, profil/önbellek kombinasyonları, callback sırası, alan sayıları, kısa binary metinlerin round-trip özelliği, statü hata maskesi, normal-only cleanup/error-mode, null sınırları ve kurtarılan modüller arasındaki gerçek kaynak bağlantısını doğrular. `validation.json` kaynak ve executable hash'lerini kaydeder. Bunlar özgün DLL ile runtime eşdeğerliği kanıtlamaz.

Ayrı bir inceleme worker'ı `online.cpp` ve `recovered_effects.cpp` kaynaklarını yukarıdaki beş ana fonksiyonun tutulmuş assembly/pseudocode kanıtıyla karşılaştırdı; düzeltme gerektiren bulgu bildirmedi. İnceleme de sağlayıcı implementasyonunu veya çalışma zamanı eşdeğerliğini doğrulamaz.
