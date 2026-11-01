# Lisans yanıt işleyicileri

`license.cpp`, anadius64.dll içindeki son üç LSX lisans işleyicisinin görülen uygulama akışını C++20'ye taşır. `Handlers`, mevcut `dispatch_recovered::LicenseHandlers` arayüzünü somut olarak uygular. Bağımsız, oyunda kullanılabilir bir DLL değildir; lisans sağlayıcısı, dosya okuma, token çözme ve hash doğrulama gibi alt sistemler zorunlu `Effects` sözleşmeleridir. Kaynak başarı cevabı uyduran boş bir uygulama içermez.

Dosya/hash/token sözleşmeleri artık `../anadius-hash-source`, `../anadius-token-source` ve `../anadius-file-source` kaynaklarıyla karşılanabilir. `LicenseFiles`, üç dosya işlemini somut kaynağa bağlar; kalan status/path/online/log/UI/exit etkilerini zorunlu çağıran nesnesine devreder. Bu ek bağlantı gerçek I/O/SHA-1/CBC/XML ile yalnız üretilen fixture'larda sınandı; özgün CRT/ABI eşitliğini tamamlamaz.

Girdi SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. İmaj tabanı `0x180000000`. `evidence.json` üç işleyicinin REA kanıt kimliklerini, 11 destek fonksiyonunu, dal adreslerini ve dosyadan salt okunur alınan 47 ASCII sabitini içerir. Kişisel CFG/token değerleri kaynaklara veya testlere alınmadı.

| İşleyici | Özgün VA | Yeniden kurulan davranış |
|---|---|---|
| GetAuthCode | `0x18002e3b0` | `ClientId` varlığı; çevrimiçi callback; boş callback cevabında yerel değeri koruma; offline/FakeAuth dalları; gerçek XML çıktısı |
| RequestLicense | `0x1800320c0` | Bloklama, DLL/EXE hash uzunluğu ve dosya durum kontrolleri, kalıcı dosya deneme sayacı, mevcut/yüklenen lisans cevabı, ticket metni ve log/dosya/dialog/çıkış sırası |
| InvalidateLicense | `0x180030c50` | Yalnız bellekteki lisansı temizleme ve `License Deleted.` cevabı |

GetAuthCode `ClientId` yoksa `false` döner; mevcut ama boş değeri kabul eder. Çevrimiçi callback'in boş olmayan cevabı yerel kopyanın yerine geçer, boş cevap yapılandırılmış fallback'i korur. Offline ve FakeAuth kapalıysa kod `-1577058293` olur. Yapılandırılmış auth değeri değişmez. Callback'in null pointer döndürmesi özgün kodda geçerli bir C-string oluşturmaz; taşınabilir kaynak bu kapsam dışı girdiye `invalid_argument` tanısı verir. Bu koruma özgün davranış diye sunulmaz.

RequestLicense önce `0x1800a39c0` blok bayrağını denetler. Bayrak set ise no-response istisnasına gider. DLL hash metni tam 40 bayt olmalıdır; burada ek bir hex sözdizimi denetimi yoktur. Dolu bir lisans dahi DLL ad/durum/hash doğrulamasını atlamaz. Dosya durum hatasının tam koşulu `error != 0 && ((uint32(kind)-1)&0xfffffff7) != 0` olup devam etmek için `kind == 2` gerekir. Durum istisnası blok bayrağını kendi başına set etmez. Hash yardımcısına beklenen metnin kopyası verilir; global metin değiştirilmez.

`0x1800a39bc` sayacı çağrılar arasında korunur. Sayaç sıfırken normal `.dlf`, diğer değerlerde `_cached.dlf` eki lisans taban yoluna eklenir. Tamamlanan okuma sayacı artırır; dolu lisans döndüğünde sonraki dosya okunmaz. Sayacın negatif olması kapsam dışı varsayılmadı: özgün signed `< 2` ve `!= 0` dalları aynen uygulanır. Lisans bulunamazsa blok bayrağı **iki ticket özniteliği okunmadan önce** set olur; öznitelik eksikse `false` dönmesine rağmen sonraki çağrı no-response yoluna gider. InvalidateLicense bu bayrağı veya sayacı sıfırlamaz, dosya silmez.

Ticket metni display name, sürüm, EXE dosya adı, yapılandırılmış hash metinleri, gerekirse DLL adı, `RequestTicket|TicketEngine|ContentId` sırasıyla oluşturulur. `dbdata.dll` kıyaslaması büyük/küçük harfe duyarlıdır. Farklı DLL adı dalında UTF-8 yol çevirisi özgün kod gibi iki kez çağrılır. Eksik oyun alanları map erişimiyle boş olarak eklenir. Ticket öznitelikleri C-string uzunluğunu, yapılandırılmış string'ler depolanan uzunluğu kullanır. Dosya adı `Denuvo_ticket_` ve zaman metninden oluşturulur; ikinci parça path join yerine ekleme işlemidir. Daraltma `01e3f0` UTF-8 iken genişletme `006d70` dosya sistemi code page sözleşmesidir; birbirinin yerine varsayılmaz.

DLL hash uzunluğu hatası `-4`, DLL adı/durum hatası `-5`, DLL hash uyumsuzluğu `-6`, geçersiz ContentId `-2`, EXE hash uzunluğu `-3`, EXE hash uyumsuzluğu `-7` çıkış yoluna gider. EXE uyumsuzluğu dialog başlığının özgün kodda da `Bad dll hash` olması korunur. Ticket kaydetme/uyarı yolları `-1` ile çıkar. Testler bu çıkışları sentetik istisnalarla yakalar; gerçek süreç sonlandırma veya dialog açma yapmaz.

## Sınırlar ve derleme

Dosya durumu/istisnası, SHA-1 dosya okuma ve karşılaştırma yardımcısı `042d20`, şifreli lisans yükleme/token çıkarma `02afa0`, çevrimiçi nesnenin vtable `+0x30` metodu, yol ayrıştırma/çevirme, saat, log, ticket yazma, dialog, süreç çıkışı ve no-response istisna ABI'si bu modülde somut işletim sistemi uygulaması değildir. Her biri zorunlu saf sanal bağımlılıktır. Yerel değişkenlerin allocation/EH ayrıntıları ve MSVC nesne yerleşimleri korunmuş sayılmaz. `StateReferences` başlangıç değerlerini çağırandan alır. Sınıf ve adaptör yapısı yeniden kurma tasarımıdır; özgün C++ sınıf tanımı olduğu iddia edilmez.

`build.ps1` C++20 nesnesini ve yalnız sentetik test EXE'sini derler. **992 deterministik kontrol geçti.** Kontroller gerçek MemoryPrinter ile XML baytlarını; auth fallback/değiştirme; dosya durumu/hata matrisi; tüm çıkış dalları; kalıcı sayaç/bloklama; dosya sırası; ticket golden metni, dosya yolu, log/dialog sırası; NUL ve eksik alanlar; bağımlılık istisnalarında durumun korunmasını kapsar. Yol, saat, hash, token ve online sonuçları testte sentetiktir; bu alt sistemlerin doğru çalıştığını kanıtlamaz. Özgün DLL/EXE veya yeniden oluşturulan DLL çalıştırılmadı. Çalışma zamanı eşdeğerliği kanıtlanmadı.
