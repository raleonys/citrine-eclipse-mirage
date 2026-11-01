# SHA-1 çekirdeği ve dosya hash karşılaştırıcısı

Bu klasör, `anadius64.dll` içindeki SHA-1 hesaplamasını ve `0x180042d20` dosya karşılaştırıcısının uygulama akışını derlenebilir C++20 olarak yeniden yazar. Gerçek dosya erişimi ve iki ayrı yerel ayar davranışı zorunlu arayüzlerdir. Tam DLL, özgün kaynak kod veya özgün MSVC nesne düzeni değildir. Özgün DLL yüklenmedi veya çalıştırılmadı.

Girdi `C:\Users\Skysect\Downloads\anadius64.dll`; SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Bu kimlik yeniden kontrol edildi; dosya değiştirilmedi. VA adresleri image base `0x180000000` için geçerlidir. `evidence.json`, aşağıdaki on adresi retained assembly/pseudocode Evidence ID'leri ve gövde hash değerleriyle bağlar. Bunlar on bağımsız tam fonksiyonun kaynak olarak kurtarıldığı anlamına gelmez; kitaplık sınırları ayrıca gösterilir.

| Adres | Kaynakta karşılığı |
|---|---|
| `0x180042aa0` | 64 baytı big-endian 16 sözcüğe dönüştürme; `Sha1::transform_043900` içine katıldı |
| `0x180043900` | 80 SHA-1 turu, beş durum sözcüğüne ekleme, 64 bit blok sayacı |
| `0x180043270` | Bit uzunluğu ve doldurma, beş çıktı sözcüğü, durum reseti; metin biçimi `TextEnvironment` sözleşmesinde |
| `0x180042d20` | Dosya açma, peek/read/gcount döngüsü, kapatma, küçük harfe çevirme ve tam dize karşılaştırması |
| `0x18003d2c0` | İşaretli baytlarla CRT `tolower`; `lowercase_03d2c0` |
| `0x180039f70` | `FileFactory::open_input_039f70` zorunlu dosya/stream kurma sözleşmesi |
| `0x180043790` | `InputStream::peek_043790` zorunlu sentry/durum sözleşmesi |
| `0x18003b2a0` | `InputStream::read_03b2a0` zorunlu gcount/durum sözleşmesi |
| `0x18003ae70` | `InputStream::close_03ae70` zorunlu kapatma sözleşmesi |
| `0x1800427a0` | C++ locale `num_put` biçimleme bağımlılığı; `TextEnvironment::format_digest_043270` |

Somut davranış ve kanıt:

- `0x180042d71`–`0x180042d8d` beş standart SHA-1 başlangıç sabitini kurar. `0x180042f68` big-endian dönüştürücüyü, `0x180042f7f` sıkıştırmayı çağırır. `0x180044ff9` blok sayacını artırır. Kaynak, açılmış 80 turu aynı 32 bit toplama/döndürme işlemleriyle döngü halinde yazar.
- `0x1800432b4`–`0x1800432c0`, bit uzunluğunu `(blok_sayısı * 64 + kalan_bayt) * 8` olarak hesaplar. `0x1800432e7`, `0x80` ekler. `0x180043361`/`0x180043365`, doldurmanın 56 baytı geçmesi halinde ek blok üretir. `0x18004338c`/`0x180043393`, uzunluğun düşük/yüksek 32 bitlerini son iki sözcüğe yazar. `0x18004362a`–`0x18004365f` durumu yeniden başlatır. `finalize_words_043270` sonrasında nesne yeni bir mesaj için kullanılabilir.
- Dosya açıcıya `mode=0x20`, `protection=0x40`, kurma bayrağı `1` verilir; `039f70` kendi içinde giriş bitini OR eder. `0x180042e06` başlangıç stream durumunun **tam sıfır** olmasını ister; diğer tüm değerler `false` yoluna gider. Fabrika, açma başarısızlığını bile geçerli bir stream nesnesi ve sıfır olmayan durumla temsil eder.
- `0x180042eb3` ilk `peek` çağrısıdır; `0x180042ed3` her defasında 8192 baytlık `read` ister. `0x180042ed8` gerçek `gcount` değerini alır. Kısa okumada alınmış baytlar hash hesabına katılır; sonraki `peek` EOF verirse bu önekin özeti karşılaştırılır. Kaynak ek bir “dosyanın tamamı okundu” veya “40 hex karakter” kabul koşulu icat etmez.
- `0x180042fd9` kapatma çağrısından sonra başarısızlık stream durumuna işlenir. `0x180043010`, sadece etkin exception-mask bitleri varsa istisna yoluna gider. Kapatma başarısız diye doğrudan `false` dönülmez. Bu nedenle `close_03ae70()` hem durum güncellemesini hem özgün istisna politikasını üstlenir.
- `043270`, her 32 bit sözcüğü hexadecimal, genişlik 8 ve doldurma karakteri `0` ile stream'e yazar. Ortamın **C++ locale** facet'leri çıktıyı etkileyebilir. `classic_hex_digest` yalnız açıkça klasik yerel ayar için kesin 40 küçük harfli hex karakter verir; eşleştirici bu yardımcıyı zorunlu varsaymaz. C++ facet nesneleri veya başarısız stream biçimleme durumları burada yeniden oluşturulmadı.
- `0x18004302e` önce hesaplanan metni, `0x180043036` sonra beklenen metni küçük harfe çevirir. `0x18003d2f1` **MOVSX** kullanır: her bayt `-128..127` işaretli aralıkta `tolower` çağrısına gider. Kaynak baytları sessizce unsigned'a çevirmeden `TextEnvironment`'a iletir. Döngü, gömülü NUL dahil gerçek `std::string` uzunluğunu kullanır. C `tolower` yerel ayarı C++ stream locale'inden ayrıdır.
- `0x18004307a` ortak öneki karşılaştırır, `0x180043083` sonrasında uzunluk eşitliği ayrıca denetlenir. Beklenen değer çağrıda kopyalanır; yapılandırmadaki metin değiştirilmez.

`InputStream`, `FileFactory` ve `TextEnvironment` saf sanal arayüzlerdir. Bunların üretim uyarlayıcısı sağlanmadan dosya doğrulaması çalıştırılamaz. Başarı üreten boş gövdeler yoktur. Adaptörün boş stream döndürmesi veya gcount'u hedef tampon boyutundan büyük bildirmesi açık entegrasyon hatasıdır; özgün kodun belirsiz davranışını taklit etmez.

Somut std::fstream ve ambient locale uyumluluk bağlayıcısı artık `../anadius-file-source` içindedir. Gerçek dosya üzerinde bilinen SHA-1 sonucu ve lisans bağlantısı yalnız üretilen fixture'larla doğrulandı. Backend'in sharing/CRT/ABI ve negatif C karakter alanı sınırları kendi README'sinde açıklanır.

`build.ps1`, mevcut taşınabilir LLVM-MinGW derleyicisiyle nesne ve test programı üretir. Testler özgün dosyalar yerine sentetik stream, locale ve veri kullanır. Empty/`abc`/56 baytlık standart dizi/milyon `a` bilinen sonuçları; bağımsız .NET SHA1 ile hesaplanıp sabitlenen 17 ikili veri uzunluğu; her biri için 1–129 arası parça boyutları; doldurma sınırları; reset; kısa/boş okuma; kapatma/istisna sırası; işaretli baytlar ve gömülü NUL kontrol edilir. Güncel sonuç ve kaynak hash değerleri `validation.json` içindedir.

Bu kontroller SHA-1 ve uygulama akışını doğrular. Özgün DLL ile çalışma zamanı eşdeğerliği, MSVC STL/istisna ABI'si, allocator davranışı, gerçek stream locale nesneleri ve dosya sisteminin hata/yarış durumları doğrulanmış değildir. Çok büyük sayaç taşması pratik veriyle sınanmadı; unsigned 64 bit aritmetik ve retained listing üzerinden korunur.
