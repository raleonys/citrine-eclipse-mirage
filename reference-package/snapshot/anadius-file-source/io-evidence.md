# Dosya giriş/çıkış çekirdeğinin bağımsız statik denetimi

Bu not `anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` içindeki `0x18003b3f0` ve `0x18003c1d0` davranışını, korunmuş Ghidra assembly/pseudocode ve kimliği doğrulanan dosyadan sınırlı salt okunur tablo okumalarıyla inceler. DLL yüklenmedi/çalıştırılmadı; CFG, token veya kullanıcı kimliği okunmadı. Adresler bu PE'nin tercih edilen `0x180000000` image base'ine göredir.

## Okuma: `0x18003b3f0`

Gözlenen sıra şöyledir: boş MSVC string oluştur; geniş karakter yoluyla binary ifstream aç; **yalnız ilk stream durumunun sıfır olduğunu** denetle; sona seek; tell; konumu string uzunluğuna dönüştürüp sıfırla doldur; başa seek; bu uzunluk kadar oku; açıkça kapat; string'i döndür. İlk açılış başarısızlığı boş string döndürür. Bu sonuç tek başına bulunamayan dosyayı boş dosyadan ayırmaz.

| Gözlem | Kesin assembly çapası |
|---|---|
| Boş string / SSO kapasite 15 | `0x18003b41d`–`0x18003b42f` |
| Ham openmode `0x20`, paylaşım `0x40` | `0x18003b44b`, `0x18003b451`, çağrı `0x18003b45a` |
| Yalnız başlangıç durumunu sınar | `0x18003b490`: CMP durum,0; `0x18003b495`: JNZ temizlik |
| Seek(0,end), tell | `0x18003b49b`–`0x18003b4b8` |
| İlk iki fpos kelimesini toplar; resize(size,0) | `0x18003b4bd`, `0x18003b4c1`, `0x18003b4c4`, `0x18003b4ca` |
| Seek(0,beg), read(string.data,string.size) | `0x18003b4cf`–`0x18003b4f4` |
| Kapat; kapanış başarısızlığında flag ekle, exception mask ile kesiştir | `0x18003b4fe`–`0x18003b530` |
| String uzunluğunu read sonrasında değiştirmez | read çağrısı `0x18003b4f4` ile dönüş `0x18003b59b` arasındaki tam yol |

Assembly Evidence: `ev_cb304cf806c8f77471c7aad5e5d42745fb5c6a321687823f1a11e5a06a733148`; pseudocode Evidence: `ev_ad6cc2139952e6a5b4801ee44818938562b836f6b702e7924bc854bcd3096820`.

`0x18003b2a0` read yardımcı gövdesi, `0x18003b319` streambuf sanal okumasının döndürdüğü gerçek sayıyı `gcount` alanına yazar (`0x18003b31c`). İstenen sayıdan farklıysa `0x18003b320`–`0x18003b328` flag `3` (eofbit|failbit) ekler. Çağıran bu sayıyı okumaz. **Kısa okuma halinde string istenen eski dosya boyunda kalır; yazılmamış son ek, resize'ın oluşturduğu NUL baytlarından oluşur.** Buna örnek, tell'den sonra küçülen dosyadır; bunun çalışma zamanında gerçekten gerçekleştiği gözlenmemiştir. Streambuf'un zaten kopyaladığı ön ek korunur. read helpers Evidence: assembly `ev_cc0aa90f9de7e7906701a4e9023cf7b35b557c01032d56d1f1f791f37bc0aaeb`; pseudocode `ev_06d0995ac266899ad894ad111cccab1df71f2f547de35561889331c7ed4d887a`.

`0x180010040` resize uzunluğu büyüttüğünde yeni aralığı çağrıdaki üçüncü parametre `0` ile doldurur; bu yalnız kapasite ayıran reserve değildir. Evidence: assembly `ev_4eed657fe242e320b79de3469b4ff268d87d2033487b16f14133e304afc75ba1`; pseudocode `ev_8b172ce1ae7a2a8f6e7d19d9433d0f9681d58a8e0fba88dbe42da06deb17bac4`.

## Yazma: `0x18003c1d0`

String uzunluğu `0x18003c1ef`'te alınır; gömülü NUL'lar yazım uzunluğunu kısaltmaz. Yol ve string için MSVC SSO gösterimi çözülür; binary ofstream açılır. İlk durum sıfırsa verilen bayt sayısı yazılır ve dosya kapatılır. **Normal dönen yolda bool, son yazma/kapanış başarısını bildirmez: ilk durum iyiyse AL=1; ilk durum kötüyse AL=0.**

| Gözlem | Kesin assembly çapası |
|---|---|
| Yazılacak string uzunluğu | `0x18003c1ef`: MOV RDI,[RDX+0x10] |
| Ham mode `0x20`, paylaşım `0x40` | `0x18003c20f`–`0x18003c221` |
| Başlangıç durumuna göre dal | `0x18003c257` CMP durum,0; `0x18003c25c` JZ yazma yolu |
| İlk durum kötü: temizlikten sonra false | `0x18003c2ba`: XOR AL,AL |
| Yazma, sonra kapatma | `0x18003c2cc` CALL `03be80`; `0x18003c2d6` CALL `03ae70` |
| Kapanış başarısız: flag ekle / mask sınaması | `0x18003c2db`–`0x18003c30d` |
| İlk durum iyi: temizlikten sonra true | `0x18003c378`: MOV AL,1 |

Evidence: assembly `ev_97eaf0d1a5c0d1e7ad14855daa321d9f92e4aabb4a1d614bc467ef53f3010812`; pseudocode `ev_69190e83bcae202b03f4fa8834b567484e7a048bd0cad2004da94e2895f74606`. Pseudocode'un üst RAX baytlarını tanımsız göstermesi bool için yalnız AL'nin kurulduğunu yansıtır; 64 bit bir durum kodu değildir.

`0x18003be80`, streambuf yazım dönüşünü istenen sayıyla karşılaştırır (`0x18003bf30`–`0x18003bf3b`); eksik yazımda badbit `4` ekler. Sıfır/negatif streamsize için sanal yazma çağrısı atlanır (`0x18003bf19`–`0x18003bf1c`); stream sentry/temizlik yine vardır. Normalde exception mask `0` olduğu için stream flag'leri tek başlarına `ios_base::failure` fırlatmaz. Bu, bellek ayırma, locale, alt çağrı veya başka C++ istisnalarının olamayacağını söylemez. Evidence: assembly `ev_3cf8d3a7fb2de6142e727cbb684596006bc225cb26e093f0a499b9b9f4c55cbc`; pseudocode `ev_0ddba33553e0acda2dcca9c75dede5b31b344841af8bf0c08280fbc898e9bdc9`.

`0x180019440` stream başlatıcısı exception mask alanını `[this+0x14]=0` yapar (`0x18001946f`); bu gövdeler maskeyi değiştirmez. Evidence: assembly `ev_f941b643096f54712f8d3153bdf55a4bbfcc4c1139636727ab1d92786287d0a2`; pseudocode `ev_2c728170b002f970860455c64a5e1f3809ace9ce1a77f6dcf573673e91ce512d`.

## Açma modu ve paylaşım

ifstream constructor `0x180039f70` mode'a `1` ekler (`0x18003a088`), ofstream constructor `0x18003a1a0` mode'a `2` ekler (`0x18003a2af`). Dolayısıyla bu iki çağrının CRT modları sırasıyla `0x21` ve `0x22` olur. `0x18004a8c4`, temizlenmiş mod değerini `0x1800a14f0` tablosunda bulur, `0x180087e10` pointer tablosundan UTF-16 modu seçer, ardından `0x1800523d4`'ü çağırır (`0x18004a934`–`0x18004a938`).

Kimlik doğrulamasından sonra doğrudan okunan PE baytları:

| Mod | Mod tablosu VA / dosya ofseti | Pointer tablosu VA / dosya ofseti | UTF-16 dize VA / dosya ofseti | Sonuç |
|---|---|---|---|---|
| `0x21` | `0x1800a1500` / `0x9fb00` | `0x180087e30` / `0x86630` | `0x180087ec4` / `0x866c4` | `rb` |
| `0x22` | `0x1800a1504` / `0x9fb04` | `0x180087e38` / `0x86638` | `0x180087ecc` / `0x866cc` | `wb` |

Yazma yolu append değildir: `wb` var olan dosyayı keser veya yeni dosya oluşturur. Otomatik üst klasör oluşturma, atomik geçici dosya/rename ve veriyi kalıcı diske zorlayan `FlushFileBuffers` bu iki üst gövdede yoktur. Bu sınırlandırılmış gövde gözlemidir; tüm program için yokluk iddiası değildir.

Paylaşım argümanı `0x40`, `common_fsopen<wchar_t>` (`0x1800523d4`) ve `_wopenfile` (`0x1800680d0`) üzerinden sopen yoluna taşınır. `0x180073e10` içinde `0x180073efc`–`0x180073f0e` karşılaştırmaları sonrası bu değer `0x180073f36`'da DWORD `3` üretir; `0x18007422c` bunu `CreateFileW` paylaşım alanına geçirir. Bu yolda değer `FILE_SHARE_READ | FILE_SHARE_WRITE` anlamındadır; delete paylaşımı eklenmez. Mevcut başka handle'ların çakışan paylaşım kuralları yine açmayı başarısız kılabilir. Yalnız `std::fstream` kullanmak, farklı CRT/stdlib'nin aynı paylaşım/handle davranışını uyguladığını kanıtlamaz.

Constructor Evidence: `ev_fb6406575d3357186579c5a4154994c91b032eb2a440e573b2e98a01c204549e`, `ev_b5522aa88153c4e76fb64ed36ff9d33fbf298818e706f6fa4eaeab341d2a5f05`. Açıcı Evidence: `ev_dc91ab6d59d1dbaafdee8406eaa11a1a76cfaf012e00a3dd9698d418c83f91ce`. CRT zinciri Evidence: `ev_4f680c7900cb1e77a5bda038031dd51b4735394ee01eff788964ce002002a802`, `ev_542edfae22ccbf6ba67c3009d8a2ddf5338fa4865f33eb7b637a7ee766ba5089`, `ev_8e9fc191d9232413d1d4ae6707cdaaa48c6ed55ade18a54d31a128488e4e2178`, `ev_537fcf332a66a41651928c6ce34443c335c30729d079dd7ca8e16b8d5d06b3ef`.

## Seek, tell ve kapatma sınırları

`0x18003b610` seek önce eofbit'i temizler (`0x18003b65c`), mevcut failbit/badbit'i korur. Bu bitlerden biri varsa sanal seek'i çağırmaz (`0x18003b6a6`–`0x18003b6ab`). Sanal sonuçtaki ilk iki 64 bit konum kelimesinin toplamı `-1` ise failbit `2` ekler (`0x18003b6cb`–`0x18003b6de`). `0x18003ba50` tell, fail/bad durumunda `{-1,0,0}` döndürür (`0x18003baea`–`0x18003bafb`); iyi durumda seekoff(0,cur,in) kullanır. Evidence: `ev_472e5a8e51a7d561a59a7891c267458d9488571dfa7f00dc86ea1a5c36c4ff03`, `ev_275ab77235c64757e4c1b2b4b17baa985376c046cef6646fe1507c793a9656d8`.

`0x18003b810` streambuf seekoff, yerel tek karakter putback durumunda cur ofsetini bir azaltır (`0x18003b838`–`0x18003b84a`); sonra gerekli seek'i CRT'ye geçirir ve fgetpos kullanır. Başarısızlıkta `{-1,0,0}` üretir. Bu iki üst yordamdan yalnız `(0,end)`, `(0,cur)`, `(0,beg)` kullanılır. Üst okuma yordamında negatif tell'i boş string'e çeviren veya dosya boyutuna özel bir üst sınır koyan kontrol yoktur. `-1`, 64 bit unsigned string boyutuna taşındığında aşırı büyük resize isteği olur; standart kütüphane uzunluk/ayırma hatası fırlatabilir. Rekonstrüksiyonda bunu sessiz başarı/boş sonuçla değiştirmek özgün akışa uymaz. Evidence: `ev_a3894fd23a40274a1415d55f7f10542d04380ddd1db9b7abc8941b7b537cb7ef`.

`0x18003ae70` filebuf close: handle yoksa null döndürür. Varsa `0x18003a760` dönüşüm sonlandırmasını dener ve **bu başarısız olsa bile** CRT close `0x180052600` çağrısını yapar (`0x18003aec5`–`0x18003aedb`). Yalnız dönüşüm sonlandırması doğru ve CRT close dönüşü sıfırsa filebuf pointer döner. Her yolda buffer/handle alanları temizlenir, sahiplik bayrağı sıfırlanır (`0x18003af0e`, `0x18003af51`). Üst gövdeler başarısız close'a failbit ekler, maskeye göre fırlatabilir; yerel mask sıfırdır. Destructor `0x18003a3c0` sahiplik bayrağı kalmışsa close'u çağırır; açık close sonrası ikinci close yapılmaz. Evidence: close `ev_f07708d5bc881573a96dcf1f90ec6e0045e3c2f98e34246add685a10f0e6d81b`; dönüşüm sonlandırma `ev_32f615943aac3e155b34a35c93717a871bedba7d2f475751ed6711027efd04ac`; destructor `ev_1d39e0fa752f567a0c0eed8b20b92789b03b119f505789ebb9dc95cd4cd168a0`.

Bu denetim MSVC object layout/locale/exception ABI'sini taşınabilir C++'a eşitlemez. Gerçek OS dosya paylaşımı, nadir CRT/device/locale hataları, eşzamanlı dosya değişimleri ve özgün DLL'le çalışma zamanı eşdeğerliği test edilmedi. Tasarım uyumluluk backend'i bu sınırlamaları açıkça korumalıdır.
