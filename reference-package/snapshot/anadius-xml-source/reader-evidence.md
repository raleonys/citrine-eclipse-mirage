# XML okuyucusu: özgün DLL'den çıkarılan sözleşme

Bu belge `anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için tutulmuş REA/Ghidra assembly ve pseudocode kayıtlarını inceler. Adresler `0x180000000` imaj tabanındadır. Özgün dosya çalıştırılmadı, yüklenmedi veya değiştirilmedi. Makine tarafından okunabilir fonksiyon kanıtları, instruction anchor'ları ve ham hata tablosu `reader-evidence.json` içindedir.

## Mesaj yolunda gerekli yapılandırma

`0x180033dab` R8D'yi sıfırlar, `0x180033dae` DL=1 yapar, `0x180033db5` XMLDocument kurucusu `0x180001830`'yu çağırır. Kurucu `+0x69` alanına entity işleme bayrağını, `+0x70` alanına whitespace seçimini yazar. Böylece bu yol `processEntities=true`, whitespace değeri `0` kullanır. Korunan metinde CR, LF, CRLF ve LFCR satır sonları LF'ye normalize edilir; XML boşlukları tek boşluğa daraltılmaz. Bu son ayrım `0x180004ea0`'da whitespace değeri yalnızca `1` olduğunda StrPair bayraklarına `4` eklenmesinden görülür.

`0x180033e1a` R8=-1, `0x180033e26` Parse çağrısıdır. `0x180004280` önce `0x180002e60` ile eski ağacı, hatayı ve giriş tamponunu temizler. `size_t(-1)` durumunda ilk NUL'a kadar uzunluk hesaplar, yeni `length+1` tamponuna kopyalar ve sonuna NUL koyar. `std::string_view` uzunluğunu doğrudan Parse'e iletmek bu çağrı sözleşmesini değiştirebilir: gömülü NUL sonrası mesaj verisi XML okuyucusuna ulaşmaz. Yeniden deneme özgün durumda aynı document nesnesinde yapılır; Clear bayrakları `processEntities` ve whitespace modunu değiştirmez.

`0x1800041b0` baştaki ASCII whitespace'i atlar, LF'leri sayar, sonra tek UTF-8 BOM (`EF BB BF`) algılar. İlk Parse girdisi null, boş veya uzunluk sıfırsa hata 13 oluşur; whitespace/BOM sonrası boş belge de hata 13 üretir. Parse başarısız olduğunda Parse çocukları ve dört pool'u temizler. Adapter belgenin sahipliğini düğüm wrapper'larından uzun tutmalıdır; özgün tamponda lazy StrPair dönüşümleri vardır.

## Düğüm ve attribute erişimi

| İşlev | Adres | Gözlenen davranış |
|---|---|---|
| FirstChildElement | `0x180003300` | `node+0x38` ilk çocuktan başlayıp `+0x50` kardeş zincirinde yalnızca elementleri seçer; ad null ise ilk element, aksi halde tam ve büyük/küçük harfe duyarlı karşılaştırma. |
| NextSiblingElement | `0x180003fe0` | Aynı seçim kardeş `node+0x50` başlangıcıyla yapılır. |
| Attribute | `0x180002cc0` | `element+0x70` attribute listesini `attribute+0x40` ile dolaşır; tam ada göre arar; yoksa null, boş değerde null olmayan boş C string döndürür. Üçüncü argüman null değilse değeri de karşılaştırır. |
| GetText | `0x180003860` | Baştaki comment düğümlerini atlar, ilk comment olmayan düğüm XMLText ise onun Value'sunu döndürür. Diğer elementlerin içine inmez ve parçaları birleştirmez. Metin yoksa null döndürür. |
| Value / StrPair::GetStr | `0x180006260` / `0x1800035e0` | Lazy tampon sonlandırma, entity çözümü ve gerekli satır sonu normalizasyonu uygular. |

`GetText` için `<!--c-->text` metin döndürür; `<child/>text` null döndürür; `a<child/>b` yalnızca `a` döndürür. Bu sonuçlar assembly dal akışından türetilmiştir; özgün DLL çalıştırılarak gözlenmemiştir. Attribute sırası ilk görülen sıra olarak bağlı listede korunur. Yinelenen attribute adı `0x180004370` yolunda hata 7'ye gider; son değeri kazanır biçiminde sözlük kurmak doğru değildir.

`StrPair::GetStr` entity tablosu `quot`, `amp`, `apos`, `lt`, `gt` adlarını ve ayrıca sayısal karakter başvurularını işler. Bilinmeyen adlandırılmış entity yolu `0x180003728 -> 0x180003745 -> 0x180003748` okuma/yazma pointer'larını kopyalama yapmadan birer artırır; bunu temizleme veya hata üretme davranışıyla değiştirmek ek bir semantik değişikliktir. Sayısal başvuruların tüm geçersiz Unicode/taşma uç durumları bu odaklı incelemede bağımsız olarak doğrulanmamıştır.

## Hata kodları ve log metni

Ham `.data` tablosu VA `0x1800a1010`, dosya ofseti `0x9f610`, 19 adet 64 bit string pointer'ı içerir. Hata 7 `XML_ERROR_PARSING_ATTRIBUTE`, 8 `XML_ERROR_PARSING_TEXT`, 13 `XML_ERROR_EMPTY_DOCUMENT`, 14 `XML_ERROR_MISMATCHED_ELEMENT`, 18 `XML_ELEMENT_DEPTH_EXCEEDED`'dir. Tam liste JSON'dadır. Hata noktaları: attribute `0x180004370`, text/CDATA `0x180004ea0`, comment `0x180004750`, declaration `0x180004800`, unknown `0x180005030`, genel ağaç/mismatch/depth `0x180004970`.

Derinlik sayacı `0x18000499e`'de artırılır; `0x1800049a4` tam `100` ile karşılaştırır, eşitse hata 18 set edilir. Bu sayım document ParseDeep çağrısını da içerir; yalnızca kullanıcı elementlerini sayıp sınır 100 demek off-by-one yaratabilir. Uyumluluk adapter'ı gerçek parser'ın sayacını aynı noktada sınırlandırmalı ve sınırın iki yanını test etmelidir.

`0x180033fd7` çevresindeki hata logu, `0x1800032d0`'nun döndürdüğü **ErrorStr** metnini kullanır; yalnızca enum adı olan ErrorName değildir. `0x180005820` özgün formatter'ı VA `0x180080068`'den alır: `Error=%s ErrorID=%d (0x%x) Line number=%d`. İsteğe bağlı `: ` ve node bağlamı eklenir. Hata yokken ErrorStr boş C string'dir. Şu anki `XmlDocument::error_name()` arayüz adının yanıltıcı oluşu, adapter'ın ErrorName döndürmesini haklı çıkarmaz.

Mesaj sarmalayıcısı `0x180033d70` hata durumunu document alanından ayrıca kontrol eder. Dönüş kodu 8 ve henüz şifreli olmayan girişte tam `<LSX>` dizisi yoksa şifre bayrağını kalıcı olarak açıp tekrar dener. Dönüş kodu 7 ise en fazla bir defa ilk `version=""` alt dizisini kaldırmayı deneyip parse akışına geri döner. Yeniden denemede şifre çözme de tekrar çalışır. Bu iki davranış parser'a gömülmemelidir; mevcut messages katmanında kalır.

## Upstream tinyxml2 ile sınır

Mac sembol envanteri tinyxml2 sınıf/işlev adlarını doğrular: x86-64 dilimde XMLDocument(bool, Whitespace) `0x7d20`, Parse(char const*, unsigned long) `0x8410`, GetText `0x7480`, ErrorStr `0x8740`, FirstChildElement `0x5c60`, NextSiblingElement `0x5d50`. Mac adresleri Windows RVA'ları değildir; bu kayıt aynı derleyici/sürüm/kaynak eşitliğini kanıtlamaz.

Root'un yerel olarak tuttuğu resmi tinyxml2 **11.0.0**, commit `9148bdf719e997d1f474be6bcc7943881046dba1`, seçilen Parse, GetText, StrPair::GetStr, ErrorStr formatter ve depth equality akışlarıyla örtüşür. Upstream varsayılan depth 500'ün özgün gözlenen 100'e ayarlanması gerekir. Bu, seçilmiş davranışları eşleyen bir uyumluluk bağımlılığıdır; özgün DLL'nin tinyxml2 sürümünün 11.0.0 olduğunu veya tüm parser davranışlarının eşitliğini kanıtlamaz. Özgün sürüm numarası/commit kimliği bu incelemede saptanmadı. Başka sürümlerin veya vendor değişikliklerinin aynı seçilmiş akışları paylaşması mümkündür. XMLDocument/XMLElement/allocator/MSVC ABI yerleşimleri bağımsız olarak eşitlenmiş değildir.

Bu inceleme kaynak veya binary değişikliği yapmaz ve adapter uygulamasını içermez. Özgün XML parser'ının tüm instruction'larını karşılaştıran bir kanıt veya çalışma zamanı diferansiyel test sonucu değildir.
