# Dosya I/O ve lisans dosya bağlantıları

`binary_io.cpp`, anadius64.dll içindeki `0x18003b3f0` tüm-dosya okuma ve `0x18003c1d0` yazma gövdelerini C++20 olarak yeniden kurar. `io-evidence.md` / JSON, 19 yordamı ve kesin assembly noktalarını tutulan REA kayıtlarına bağlar. Girdi SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Okuma yalnız başlangıç stream durumu tam sıfırsa ilerler: sona seek, tell konum toplamı, sıfır doldurarak resize, başa seek, istenen uzunlukta read ve close. Gerçek gcount çıktı uzunluğunu kısaltmaz; kısa okumada kalan sıfırlar korunur. Negatif tell boş başarıya çevrilmez; unsigned boyutla resize uzunluk/ayırma istisnasına gidebilir. Yazma, saklanan uzunluğu NUL dahil kullanır. Başlangıç iyi ise daha sonraki write/close durum bitlerine bakmadan true döner; etkin stream exception maskesi varsa istisna yine yayılır. Bu bool, kalıcı diske başarılı yazma garantisi değildir.

Özgün üst çağrılar `mode=0x20`, paylaşım `0x40`, kurma bayrağı1 geçirir; constructor input/output bitini ekler. Ham CRT tablosu bunları `rb` ve `wb` olarak çözer. Yazma keser/oluşturur; append değildir. CRT→CreateFileW paylaşım alanı3 (read+write, delete yok) olarak izlendi. Özgün stream maskesi bu yolda0'dır. Ayrıntılar Evidence dosyalarında vardır.

`native_io.cpp`, **seçilen std::fstream uyumluluk backend'idir**. UTF-16 Path'i Windows std::filesystem yoluna çevirir; binary input/output/truncate kullanır. Gerçek dosya okuyabilir/yazabilir. Farklı CRT/stdlib'nin paylaşım, codecvt, yarış, handle, allocator, exception veya MSVC nesne ABI'sini aynen uyguladığı iddia edilmez. Backend, streamsize değerini aşan isteği açıkça length_error ile reddeder; özgün üst gövdede bu ayrı kontrol yoktur. Native backend'in exception maskesi0'dır; tüm özel exception-mask yolları somutlaştırılmış sayılmaz.

`license_files.cpp`, yeni birleştirme katmanıdır, ayrı özgün fonksiyon değildir. `LicenseFiles`, lisans `Effects` arayüzünü uygular ve üç işlemi somut kaynakla karşılar:

- `file_hash_matches_042d20`: gerçek dosya stream'i→kurtarılmış SHA-1→ortam biçimleme/küçük harf→karşılaştırma.
- `load_token_02afa0`: zorunlu status kontrolü→somut tüm-dosya okuyucu→kurtarılmış AES-CBC→ham etiket araması.
- `write_ticket_03c1d0`: somut uzunluk bilgili dosya yazıcı.

Kalan status/istisna, yol/code-page, saat, online sağlayıcı, log, UI, çıkış ve no-response işleri çağıranın verdiği `Effects` nesnesine devredilir. Başarı üreten varsayılan uygulama yoktur. `NativeTextEnvironment` ambient C++ ostringstream locale'iyle beş sözcüğü hex/genişlik8/dolgu0 biçimler, C tolower kullanır. `-1` dışındaki negatif C karakter girdileri bu backend'de açık kapsam-dışı hatadır. Hash çekirdeği özgün MOVSX işaretli bayt davranışını aynen zorunlu arayüze geçirir; backend'in dar alanı özgün genel davranış diye sunulmaz.

Status ve yol/code-page alanlarını somut kaynakla karşılayan `WindowsEffects` artık `../anadius-windows-source` içindedir. Doğru zincir `LicenseFiles(WindowsEffects(app,...),...)` biçimindedir; böylece token status'u da Win32 katmanına ulaşır. Ayrı 23 ortak kontrol bu alanlar için mock kullanmadan, yalnız üretilen fixture'larda çalışır. Genel stats/CRT/ABI ve kalan uygulama etkileri bu bağlantıyla tamamlanmış sayılmaz.

## Doğrulama

`build.ps1` mevcut LLVM-MinGW ile kaynakları derler; bizim iki test EXE'mizi çalıştırır. **78 I/O kontrolü** sentetik stream sırası/hata/short-read yollarını ve gerçek binary/Unicode dosya adı/truncate okumalarını kapsar. **33 ortak kontrol** gerçek dosya+SHA-1 bilinen `abc` özeti+AES-CBC+token çıkarma+lisans gövdesi+XML çıktısını birlikte sınar; normal/cached dosya denemeleri ve invalidate/blocked durumunun kalmasını da doğrular.

Gerçek I/O yalnız testlerin oluşturduğu `build/file-io-fixtures` klasöründe yapılır. Fixture'lar sentetik metin ve token içerir; kalırlar ve inceleme için pakete dahildir. Testler özgün DLL/EXE/CFG veya gerçek token dosyalarını okumaz/değiştirmez/yüklemez/çalıştırmaz. Status/yol ve diğer provider sonuçları ortak testte kontrollü uyarlayıcılardır. Bu doğrulama tüm DLL çalışma zamanı veya özgün CRT/ABI eşdeğerliği değildir. Ortak proje `../source-workspace` içindedir.
