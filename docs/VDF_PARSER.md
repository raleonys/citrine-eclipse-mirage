# PEGTL ile mevcut Config2/VDF grameri

Kullanıcının “pegtl vdf grameri vardi” düzeltmesi ve ardından “devam edelim”
talimatı, RE'de zaten belgelenen gramerin PEGTL'ye bağlanmasının yetkisidir.
Önceki “gramer belirlenmedi” kaydı eksikti: RE kaynaklarında kurallar ve action
sırası bulunur. Bluefield karşılaştırma için okunur; uygulama kodu alınmaz.

Temel kaynak, kullanıcının `Reverse Engineering/anadius-parser-source/` altındaki
`README.md`, `evidence.json`, `parser.cpp` ve `parser_tests.cpp` dosyalarıdır.
`anadius-all-functions.pseudo.c` içindeki `vdf::parser::KeyValue`, `Object`,
`Seps` ve `tao::pegtl` rule metinleri bu kaynağı destekler.

| RE yordamı | Kural / action |
|---|---|
| 014310 | Root: EOF'ye kadar EOL, separator veya quoted object; uyumsuzlukta hata |
| 013810, 013640, 0141a0 | Quoted byte dizisi; yalnız quote ve backslash escape'i çözülür |
| 013460, 0135b0 | Satır yorumu ve HT/LF/VT/FF/CR/SP whitespace |
| 013ed0, 013520, 013ab0 | Block comment; direct tüketim ve rewind ayrımı |
| 0138e0, 013b70, 013cc0 | Önce scalar sonra object; üyeler, mandatory scalar separator |
| 012ff0 | Object adı okunduğunda, açılış `{` doğrulanmadan frame oluşturma |
| 012cf0, 012bf0, 0164f0 | Tamamlanmış atama; duplicate subtree replacement |

`include/preloader_l/config/vdf_grammar.hpp` byte kurallarını tanımlar.
`src/config/parser.cpp` PEGTL action/control'lerini, owned `ParseError` arayüzünü
ve mevcut Node dönüşümünü taşır. `parser::parse` daima gerçek
`tao::pegtl::parse<vdf::Document,...>` çağırır; eski elle yazılmış parser veya
başarısızlıkta başka backend'e geçiş bulunmaz.

Root'ta scalar kabul edilmez; object adları, key ve value quoted'dur. Scalar
key/value arasında separator gerekir; tamamlanmış üyeler bitişik olabilir.
Block comment ilk `*/` ile biter. Bilinmeyen escape'ler, NUL ve geçersiz UTF-8
byte'ları korunur; ham CR/LF quoted token içinde hata verir. Bir UTF-8 BOM
kaldırılır. LF sayımı host'tan bağımsızdır. Input rewind action `State`'ini geri
almaz; hata öncesindeki atamalar ve açık/orphan frame'ler korunur. Duplicate key
önceki node'un tüm değer/çocuklarını değiştirir. Her belge için yeni `State`
kullanılır ve input parse boyunca yaşamalıdır.

Windows'ta `preloader_l_vdf_objects`, `preloader_l_pegtl` hedefiyle derlenir;
objeleri hem `preloader_l_core` test arşivine hem gerçek `preloader_l` DLL'ine
girer. DocumentPipeline ve mevcut config uygulama akışı bu parser'ı kullanır.
Dinamik source host, DLL üzerinden BOM'lu nested Config2 belgesini, duplicate
atamaları ve hata konumunu denetler. Sayı fixture'ı ayrıca yalnız vendor API testidir.

Bu bağ `surge.cfg` / `surge_override.cfg` dosyalarının seçimini veya önceliğini
belirlemez. Asio/filesystem üretim adapter'ları, otomatik bootstrap, Consumer ABI,
gerçek Windows/MSVC ve oyun kabulü ayrı kalan işlerdir. XML sahibi TinyXML2'dır;
bu gramer XML/LSX parser'ı değildir. Vendor dosyaları ve TinyXML2 500 değişmez.

Mevcut 115 parser assertion'ı değiştirilmeden korunur. Yeni
`vdf_parser_regressions` testi 23.417 girdide eski RE-tabanlı Yellowfield
parser'ından alınmış golden gözlemleri denetler: 4.724 kabul, 18.693 parse hatası.
Girdiler kısa token kombinasyonları, bütün 256 byte'ın farklı bağlamları,
belge kesme/silme/ekleme/değiştirmeleri ve deterministik üretilen belgelerdir.
Kullanıcının gerçek config dosyaları okunmaz.

Gözleme input, sonuç/hata sınıfı, byte/line/column, tüm AST, atama sırası,
açık frame'ler, accumulator/key/value ve BOM durumu dahildir. Sayılar sabit
little-endian biçimde FNV64'e katılır. Bu bir regresyon fingerprint'idir;
kriptografik eşdeğerlik, orijinal native DLL veya exception ABI kanıtı değildir.

- Baseline paket: `yellowfield-vendor-compatible-source.zip`
- Paket SHA-256: `c2a4d4622bb9cf03dd889d5bb9b0040e817d94a25fe2fcb44f4bcece52aa0767`
- Eski `src/config/parser.cpp` SHA-256: `bc93d8badfb33c89b268470ee95b27384bb4a6b700842a4f4f476f2c2075d139`
- Beklenen full-state FNV64: `ea70f5d56d33f404`

Eski implementasyon ürün ağacına eklenmez. Baseline kontrolü aynı test harness'ını
eski parser ve configuration kaynağıyla ayrı executable olarak derleyerek yapıldı.
Normal CTest yeni PEGTL parser'ını bu sabit gözlemlere karşı sınar.
Güncel ham kayıtlar ve sonuçlar
[VDF_PARSER_STATUS.json](evidence/VDF_PARSER_STATUS.json) içindedir.

Güncel runtime bağı: [RUNTIME_BINDING.md](RUNTIME_BINDING.md). Buradaki önceki vendor/parser test sayıları tarihsel snapshot kanıtıdır; yeni app testleri [RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json) içindedir.
