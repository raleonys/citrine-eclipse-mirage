# Config2 yapılandırma mantığının C++ kurtarımı

`configuration.cpp`, `anadius64.dll` içindeki yapılandırma ayarlarının uygulanmasını ve başlangıç sırasını okunabilir C++20 olarak yeniden kurar. **Bağımsız bir DLL veya tamamlanmış DLL dönüşümü değildir.** Dosya okuma, ham CFG dilbilgisi, işletim sistemi işlemleri ve henüz kurtarılmamış alt sistemler `Dependencies` üzerinden zorunlu sözleşmelerdir; üretim için boş uygulamaları bulunmaz.

Başlangıç bağlantısı güncellemesi: Üç argümanlı `initialize_configuration(state,deps,ConfigurationPass&)` overload'u yeni kaynak loader'ını aynı mevcut post-configuration gövdesine bağlar. İki argümanlı giriş eski document sözleşmesini korur. `anadius-config-startup-source::FilePass/BootstrapConfiguration`, gerçek dosya/parser/hash/application akışını bu overload ve Bootstrap üzerinden kullanır. Bu kaynak arayüzü native MSVC class/ABI düzeni değildir; eski validation/review kayıtları tarihsel kalır, güncel ortak sonuç ayrıca kaydedilir.

Kaynak ikilinin SHA-256 değeri `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`, boyutu 694.272 bayttır. Aşağıdaki adresler bu DLL'nin tercih edilen `0x180000000` görüntü tabanındaki VA değerleridir. `version.dll`, gömülü preloader ve host EXE adresleriyle karıştırılmamalıdır.

## Kurtarılan kapsam

İki ana yordamın uygulama mantığı ve dokuz yardımcı yordamın kullanılan davranışı karşılandı. `03cf00` için yalnızca burada çağrılan tek karakterli ayırıcı / `limit=0` durumu uygulanmıştır. Altı bölümde **46 bölüm/alan anahtarı**, dinamik içerik ve entitlement kayıtları ile achievement isim/toplam/Steam eşlemeleri kapsanır. Tam anahtar listesi, 41 durum alanının özgün global adresi, kaynak satırları ve REA kanıt kimlikleri [evidence.json](evidence.json) içindedir.

| Özgün VA | Kaynaktaki karşılığı | Kurtarılan davranış |
|---|---|---|
| `0x180019ad0` | `load_and_apply`, `apply_config2` | Host yanındaki `anadius.cfg`, ardından `anadius_override.cfg`; Config2 ayarları, override ayrımı ve koleksiyon güncellemeleri |
| `0x18001c870` | `initialize_configuration` | EXE yolu, iki config geçişi, kullanıcı yolları, avatar işlemlerinin sırası, SDK normalleştirmesi, persistence ve ortam değişkeni çağrıları |
| `0x18001e1e0` | `assign_if_nonempty` | Boş kaynak eski değeri silmez |
| `0x18001de40` | `apply_boolean` | Tam eşleşen `true`/`1`, `false`/`0`; boşta eski byte; diğer metinde çağıranın fallback değeri |
| `0x18001d6c0` | `parse_decimal` | İşaretli 32 bit ondalık dönüşüm, sayısal sonekten sonraki metni kabul, invalid/range exception sonuçları |
| `0x18003cf00` | `split_delimited` | Sınırsız, tek karakterli ayırma; boş parçalar korunur |
| `0x18001df60` | `apply_csv_paths` | Dolu girişte eski liste temizlenir, boş parçalar atlanır; dönüşüm hatasında önce eklenen yollar kalır |
| `0x18001e0c0` | `apply_registry_path` | Registry bayrağı UTF-8 dönüşümünden önce yazılır |
| `0x18001da80` | `apply_achievement_ids` | Dolu set adı koşuluyla `;` kimlikleri eklenir/mevcut kayıtlar sıfırlanır |
| `0x18001e180` | `path_if_nonempty` | Dolu yol, özgün yol çözümleyicisi sözleşmesine aktarılır |
| `0x18003d2c0` | `lowercase_hash` | Signed-char girdisiyle CRT `tolower` işlemi; locale adaptöre aittir |

Başlıca kanıtlar: `019ad0` pseudocode `ev_5ec5f52f67388ea3296af7964bac7ecdff2ed88218b0b532744a87ae797c72dd`, assembly `ev_b75b58538a829894dee04456d214f29001ee3a09dd2d8c04d5b393fa8534df88`; `01c870` pseudocode `ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e`, assembly `ev_42beaf13c42bb51716fcf407bd74587506fbb6acf17bfba9e16c31bdd31f8fba`. Yeni REA analizi çalıştırılmadı; mevcut tam pseudocode/assembly dökümleri tekrar kullanıldı ve kaynak SHA-256 değerleri kaydedildi.

## Davranışı etkileyen ayrıntılar

Base geçişi `Game/Name`, `Game/Version`, `Game/ContentId` alanlarını uygular. Override geçişi bunları ve Content/Entitlements/Achievements koleksiyon bloklarını atlar; diğer ayarlar uygulanmaya devam eder. License yolu iki geçişte de eldeki ContentId ile tekrar oluşturulur. Boş sıradan değerler mevcut durumu korur; başlangıç global değerleri tahmin edilmez, `State` çağıran tarafından verilmelidir.

`ServerPort=default` değeri 3216, `random` değeri 0 olur. Sayısal değer assembly'de `0x18001a52a` adresindeki word yazımıyla daralır; örneğin 70000 → 4464 ve -1 → 65535. Bu kaynak yeni bir 0–65535 kontrolü eklemez. DenuvoToken alanının uzunluğu 1–99 ise temizlenir; boşsa boş kalır. Token işleme veya lisans oluşturma alt sistemi burada uygulanmaz.

Argument parser tarafından `language_from_arguments` set edilmişse config `Language` ve `LanguageRegistrySpoof` uygulaması atlanır. `LanguageRegistryKey` daha önce işlendiğinden yine uygulanabilir. Bu bayrakların başlangıçta nasıl ayarlandığı ayrı başlangıç modülünün sorumluluğudur.

Content ve Entitlements vektörleri base geçişinde **eklenerek** güncellenir; tekrar base çağrısı eski vektörleri temizlemez. Achievement IDs/Names mevcut kayıtları sıfırlayabilir; Totals ve SteamIdOverrides yalnızca var olan kayıtlara, boş olmayan değerlerle uygulanır. Tarih başlangıcı `0000-00-00T00:00:00` olur. Anlamı kesinleşmeyen sayısal achievement alanları `field00/04/08/70` adlarıyla bırakılmıştır.

`01c870`, kullanıcı yolunda isim içinde tam ve büyük harfli `_AVATAR_` bulunan girdiler için remove çağrısı yapar; uzantı filtresi yoktur. Kaynak bu işlemi yalnızca `Dependencies` sözleşmesine iletir. Testler gerçek dizinleri dolaşmaz veya dosya silmez. SDK içindeki noktalar virgüle çevrilir; `01eb00` achievement persistence çağrısından sonra `ContentId` ve `EAGameLocale` ortam çağrıları yapılır; dönüş değerleri göz ardı edilir.

## Denetimde düzeltilen iki önemli fark

İlk kaynak taslağındaki `std::map` sırasıyla dolaşma doğru değildi. `0x180012820`, FNV-1a hash sabitlerini; `0x180014a90`, hash/list konteyner başlangıcını gösterir. Özgün Config2 koleksiyon sırası leksikografik kabul edilemez. `Node.children` yalnızca lookup deposudur. Eski document adaptörü her dolaşılan koleksiyonun bağlı liste sırasını `iteration_order` ile sağlamalıdır; eksik, mükerrer veya bilinmeyen anahtar entegrasyon hatasıdır. Yeni `anadius-config-hash-source` ve `anadius-config-application-source` normal akışın büyüme/rehash/konumsal kopya sırasını kendi mutating modeliyle uygular; `anadius-config-startup-source` bunu üç argümanlı başlangıç girişine bağlar. Native allocator/ABI/fault/SEH eşitliği açık kalır. Hatalı Total metninden önce hangi kaydın güncellendiği de sıraya bağlıdır.

Dosya sistemi kod sayfası ile UTF-8 yolu ayrıldı. `0x180010540` → `__std_fs_code_page()` → `017330`, dosya adı ve ContentId yol dönüşümlerinde kullanılır. Registry dönüşümü `0x1800145d0` açıkça 65001 kullanır. Dolayısıyla iki ayrı sözleşme bulunur. Özgün UTF-16 `Electronic Arts` / `EA Services` / `License` sabitleri doğrudan UTF-16 tutulur. Testler dönüşüm sırası ve çağrı yönlendirmesini kontrol eder; gerçek Windows locale dönüşümünü taklit ettiğini iddia etmez.

## Derleme ve doğrulama

Workspace kökünden:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\outputs\anadius-config-source\build.ps1
```

Varsayılan derleyici workspace içindeki `work/toolchain/extracted/llvm-mingw-20261006-ucrt-x86_64` dizinidir; farklı kurulum `-ToolchainRoot` ile verilebilir. C++20, `-O2 -Wall -Wextra -Werror` ile derlenir. CMake dosyası da sağlanır. `build/configuration.o`, `build/configuration_contract_tests.exe`, test çıktısı ve derleyici/dosya hash'lerini içeren [validation.json](validation.json) üretilir.

Son doğrulama **150 sentetik/mock assertion PASS**. Kapsam: boş/geçersiz boolean, signed sınırlar ve NUL/sonek, port daralması, token uzunluğu, override ayrımı, liste temizleme/kısmi hata durumu, registry dönüşüm hatasının sırası, achievement reset/toplam/prefix/override, özgün koleksiyon sırasını zorunlu kılma, avatar yolları, dosya hatasında durma ve ortam çağrı sırası. Yalnızca bizim sentetik test EXE'miz çalıştırıldı. Özgün DLL, yeniden oluşturulan DLL ve host EXE yüklenmedi veya çalıştırılmadı. Testler gerçek CFG'yi okumaz; tüm kimlikler sentetiktir.

Bu sonuçlar kaynak uygulamasının sözleşmelerini doğrular. Özgün DLL ile karşılaştırmalı runtime eşdeğerliği kanıtı değildir.

## Açık kalan zorunlu işler

`019ad0` parser sonucunu kontrol eder; fakat incelenen `014310` normal dönüşleri EOF'de true, uyumsuz yolları throw helper çağrılarıdır. Normal bir false dönüş yolu gözlenmedi. `load_document` sözleşmesi bu guard'ı karşılamayı gerektirir; parse hatalarının olağan biçimde boş belgeye çevrildiği iddia edilmez. Ayrı [anadius-parser-source](../anadius-parser-source/README.md) modülü ham byte dilbilgisini daraltır; dosya I/O, özgün hash/list sırası ve ABI adaptörlerinin yerini almaz.

- Eski document girişinin parser/hash adaptörü caller'a aittir; yeni file/hash/application/startup kaynak bileşimi normal akış için bu bağlantıyı sağlar. Native fault/exception ABI ve tüm runtime eşitliği açık kalır.
- Global başlangıç değerleri, özgün MSVC string/konteyner/exception ABI ve yerleşimi; `State` doğrudan özgün belleğe cast edilemez.
- `0192d0` yol çözümleme/genişletme, Windows dosya sistemi işlemleri ve hata yayılımı, encoding/locale adaptörleri.
- `02a830` lisans işlemesi ve `01eb00` achievement persistence alt sistemleri. Burada iç işleyişleri yerine çağrı noktası ve zorunlu sözleşme vardır.
- Logger iç işleyişi, CRT errno yan etkileri, allocator hata zamanlaması ve MSVC destructor/unwind yolları.
- DLL/host entegrasyonu ve gerçek davranış karşılaştırması. Yalnızca bu modülü derlemek çalışan bir `anadius64.dll` üretmez.

Kullanıcının özgün dosyaları değiştirilmedi; kullanıcı CFG'sindeki kimlik, cookie veya token değerleri bu pakete kopyalanmadı.
