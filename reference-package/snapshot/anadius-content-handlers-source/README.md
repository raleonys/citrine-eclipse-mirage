# İçerik, entitlement, achievement ve avatar işleyicileri

Bu klasör, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olan `anadius64.dll` için adres ve kanıt destekli C++20 uygulama mantığıdır. Orijinal dosya değiştirilmedi, DLL/EXE yüklenmedi veya çalıştırılmadı. Sentetik mock testleri kendi üretilen test programında çalışır. Bu kaynak bağımsız bir replacement DLL değildir; MSVC nesne yerleşimi/ABI ve bütün çalışma zamanı eşdeğerliği doğrulanmış değildir.

`content_handlers.hpp` API'si mevcut `anadius-config-source` veri tiplerini ve `anadius-handlers-source::Xml` yazıcı arayüzünü kullanır. `RequestTree`, tinyxml2 tarafından çözülmüş sıralı attribute ve child-text görünümüdür. Adapter gerçek document sırasını, boş/missing ayrımını ve GetText davranışını korumalıdır. XML parse veya gerçek ağ/dosya/online işlemi bu modülde yapılmaz.

| Kaynak giriş noktası | Yeniden yazılan kararlar |
| --- | --- |
| `QueryContent` `0x180030ee0` | Content vektörünün mevcut sırasını koruyan `Game` kayıtları; iki version attribute aynı kaynak alanından gelir; progressValue sabit 0. |
| `QueryEntitlements` `0x180031030` | Filtrelerin OR birleşimi; filtrelenen satırlar dahil her kaynak kayıt için artan 1.000.000.000.001 tabanlı ID; 11 attribute ve sabit tarih alanları. |
| Filtre yardımcısı `0x18002c6a0` | Ignore listesi, attribute/child sırası, boş değerlerin atlanması, unknown log/dialog sınırı. `ItemId` attribute'u ilk/offer kümesine; `FilterItems` child'ı üçüncü kümeye gider ve entitlement **tag** alanıyla karşılaştırılır. Child text virgül/noktalı virgül ile bölünmez. |
| `QueryAchievements` `0x180030d30` | Mutex altında lexical ID sırası; `All` yalnızca tam `true` olduğunda bütün kayıtlar; aksi halde Count != 0 kayıtları. |
| `GrantAchievement` `0x180030010` | Eksik ID -> false; Progress eksik -> 1; bilinmeyen ID'nin Total alanı ilk increment olur. Signed progress < total durumunda x86 32-bit toplama, zaman güncelleme, tamamlanma/cap, event, XML persistence ve Steam persistence sırası. |
| Achievement yazıcıları `0x18002bf60`, `0x18002c080` | Gerçek element ve attribute isimleri, signed sayılar, placeholder tarihler; DISPLAY_NAME lookup'u yoksa boş map kaydı ekler. |
| `QueryImage` `0x1800314f0` | Width/Height/ImageId mevcudiyeti; ilk `:` sonrası sahip ID'si; own/default avatar seçimi; online cache yolu; genişlik/208 ile yalnızca daha büyük kendi avatarını kopyalama ve success durumunda saved-size değişimi. |

Achievement map value alanları bu kullanımda netleşir: `field00=Total`, `field04=Count`, `field08=Progress`, `timestamp=GrantDate`, `display_name=Name`, `steam_id=Steam ID`, `field70=zaman yardımcısının yazdığı sayısal değer`. Native map node başına value offset `+0x40` olduğundan Count node `+0x44`, Progress `+0x48`, timestamp `+0x50`, sayısal zaman `+0xb0` olur. Bu eşleme gerçek C++ struct boyutunun/yerleşiminin aynı olduğunu iddia etmez.

Önemli kenar davranışları korunur: tamamlanan achievement yeniden artırılmaz; sıfır/negatif Total başlangıçta ilerlemez; mevcut kayda negatif increment verilebilir; toplama signed overflow durumunda modulo 2^32 sarar; persistence yalnızca Progress != 0 kayıtlarını yazar. Completion event, achievement mutex tutulurken ayrı event mutex altında kuyruğa eklenir. Kuyruk ilk kullanımında `IGOEvent State="DOWN"` başlangıç mesajı alır. Event/XML yazımı başarısız olursa daha önce yapılmış state değişiklikleri geri alınmaz.

Avatar ImageId'de `:` yoksa online sağlayıcı çağrılmaz ve seçili avatar kalır. Online cache dosya adı `_AVATAR_` + owner + `_` + **orijinal Width metni** olur; Height cache seçiminde kullanılmaz. Başarılı cache sonucu diğer kullanıcı için de ResourcePath olabilir, ancak last-avatar kopyası yalnızca kendi kullanıcı ID'si ve daha büyük signed `atoi(width)/208` için yapılır. Cache append yardımı `0x180006d70` içinde `__std_fs_code_page()` kullanır; output ResourcePath dönüşümü UTF-8 code page 65001'dir.

`constants.json` içindeki bütün element/attribute/sabit metinler, PE section eşlemesiyle orijinal DLL'den salt okunur alınmıştır; VA, raw offset ve NUL dahil baytlar kayıtlıdır. `evidence.json` her fonksiyonun retained REA evidence ID'sini, pseudocode/assembly aralığını ve metin digest'ini içerir. Statik bulgu yürütme gözlemi değildir.

## Derleme ve doğrulama

```powershell
.\build.ps1
```

Mevcut taşınabilir LLVM-MinGW toolchain ile `-std=c++20 -O2 -Wall -Wextra -Werror` kullanılır. Sonuç: `build/content_handlers.o` ve statik bağlı `build/content_handlers_tests.exe`. **1.023 sentetik kontrol geçti.** Testler filtre OR/sıra/ID boşlukları, case/empty/missing ayrımı, negatif/sarma durumları, completion ve tekrar grant, event/persistence sırası, mutex hata ve exception cleanup, avatar cache/copy success/failure kararlarını kapsar. `validation.json` compiler ve kaynak hash'lerini kaydeder.

## Açık entegrasyon sınırları

- Gerçek tinyxml2 parse/print backend ve native request/state ABI adapter gereklidir. Bu kaynak allocation/SSO kapasitelerini veya allocator-failure zamanlamasını aynen taklit etmez.
- Mutex ve event queue storage adapters, `04a6ac/04a6b4` ve native recursive count alanını sağlamalıdır; thread error 5/6 production'da geri dönmemelidir.
- UTC helper, dosya yazma, filesystem code-page/path append, online avatar virtual +0x28, CopyFileW overwrite ve log/dialog etkileri zorunlu typed dependency'dir; üretim için boş implementasyon sağlanmaz.
- `01f9a0` Steam persistence alt sistemi ayrı zorunlu sınırdır; burada iç dosya formatı/path mantığı kurtarılmış sayılmaz. Achievement XML persistence kararları ve alanları bu modülde yazılmıştır.
- Bu testler mock sözleşmeleri ve yeniden yazılan mantığı doğrular; orijinal DLL ile çalışma zamanı/birebir protokol eşdeğerliğini kanıtlamaz.
