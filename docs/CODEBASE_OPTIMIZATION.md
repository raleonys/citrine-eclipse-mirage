# Kaynak temizliği ve ölçülen optimizasyonlar

Bu belge ilk optimizasyon aşamasının tarihsel ölçümlerini tutar. Güncel üretim
ayrımı60/78 ve dar temizlik/TLS uygulaması [BUILD_CLEANUP.md](BUILD_CLEANUP.md)
içindedir; aşağıdaki eski test, TU ve export sayıları güncel toplamlar değildir.

İnceleme 138 first-party CPP ve derleyici bağımlılıklarında görülen 139 başlığın
tamamını kapsıyor. 64 üretim ve 74 korunan referans CPP ayrımı değişmedi. Bir CPP
veya genel API fonksiyonu silinmedi. Kullanılmayan özel depolama ve erişilemeyen
bir dal kaldırıldı; XML işleme ve Release tanılama çağrıları iyileştirildi.

Önceki sahibin kabul ettiği DLL SHA256:
`4a0f37c9328ecf31d0815a9e9b85634bc481e77e2f07b636d4ea65bb4783f26b`.
O DLL için Windows Native 193/193 ve sahibin bildirdiği oyun kabulü geçerli tarihsel
kanıttır. Aşağıdaki kaynak değişiklikleri için yeni native Windows/Golden ve oyun
kabulü henüz çalıştırılmadı. Yerel sonuçlar LLVM-MinGW/Wine ve Linux sanitizer
kanıtlarıdır; MSVC veya oyun performans sonucu olarak sunulmuyor.

## İnceleme ve kararlar

Gerçek Windows x64 compile database kullanılarak bütün 138 CPP Clang-Tidy21.1.6
ve Clang AST ile tarandı; derleyici hatası yok. Seçilen kontroller analyzer
core/cplusplus/deadcode, bugprone, performance, misc-unused ve redundant grupları.
Vendor başlıklarının uyarıları first-party bulgu sayısına eklenmedi ve vendor
kaynaklarında düzeltme yapılmadı.

1779 açık AST tanım kaydı çıkarıldı. Bu sayı template örneklerini ve constructor
kayıtlarını da içerir;1779 bağımsız mantıksal fonksiyon iddiası değildir.
969 kayıtta doğrudan DeclRefExpr/MemberExpr başvurusu gözlenmedi. Bu sonuç
"969 ölü fonksiyon" anlamına gelmez: sanal dispatch, Windows callback'leri,
constructor/destructor, CRT/TLS başlangıcı ve geliştirme DLL export kökleri bu
grafikle bütünüyle kanıtlanamaz. 738 kayıt virtual/override niteliğindedir.
Geliştirme DLL'inin önce/sonra 3347 export adı da korundu.

53 benzersiz first-party Clang-Tidy bildiriminin tamamı gerekçesiyle kaydedildi.
Örneğin messages.cpp içindeki c_str() önerisini uygulamak ilk-NUL sınırını
değiştirebilir; Asio error_code overload'ları ise yalnız dönüş değerine bakılarak
"hata kontrol edilmiyor" diye değerlendirilemez. Otomatik düzeltme uygulanmadı.
Asio destructor'ından stop() çağrısının olağanüstü tahsis/join hatasında exception
üretebilmesi ayrı takip konusudur. Bu yol için native hata enjeksiyonu yapılmadı;
uyarı düzeltilmiş veya güvenliği kanıtlanmış sayılmadı. Canlı thread sahipliğini
riske atacak şekilde exception yutma eklenmedi.

Dosya başına güncel/eski hash ve kapsam:
[FILE_MATRIX.csv](evidence/optimization/FILE_MATRIX.csv).
Fonksiyon kayıtları ve saklama kararları:
[FUNCTION_MATRIX.csv](evidence/optimization/FUNCTION_MATRIX.csv).
Bütün uyarılar ve tek tek kararları:
[STATIC_DIAGNOSTICS.csv](evidence/optimization/STATIC_DIAGNOSTICS.csv).
Derleyici kapsamı ve sınırlar:
[COMPILER_SCOPE.json](evidence/optimization/COMPILER_SCOPE.json).

## Uygulanan değişiklikler

| Değişiklik | Gerekçe ve korunan davranış |
|---|---|
| XML parse öncesindeki staging string kaldırıldı | TinyXML2 zaten sahip olduğu sonlandırılmış tamponu kopyalıyor. Sayılı view içinde ilk NUL sınırı, boş girdi, kesilmiş view, tam ErrorStr ve belge sahipliği korunuyor. |
| XML Element wrapper map düğümünde tutuluyor | Her düğüm için ikinci heap tahsisi kalkıyor. unordered_map rehash sırasında düğüm adreslerini korur; vendor belge wrapper'lardan sonra yok edilir. |
| İç tanılama çağrılarına inline Debug kapısı | Golden LTO'suz derlendiği için başka TU'daki boş Release log gövdeleri çağıranın stringlerini temizlemiyordu. Release çağrıları artık kendi TU'sunda elenir; mevcut out-of-line geliştirme API log sembolleri korunur. |
| Environment özel alan temizliği | Kullanılmayan xml_ referansı kaldırıldı. Yalnız Debug'da kullanılan log_mutex_ Release sınıfında tutulmuyor. XML constructor girdisi gerçek Storage kullanımı için korunuyor. clock_/formatter_ gerçek kullanım nedeniyle tutuluyor. |
| Sıfır receive boyutu dalı kaldırıldı | Önceki size>=maximum kontrolünden sonra maximum-size,65536 ve max(spare,512) pozitiftir. Dolayısıyla minimumları sıfır olamaz. Yardımcı fonksiyonun API'si ve dolu tampon davranışı korunuyor. |

Hook'lar, shutdown/rollback sahipliği, FIFO/event commit, atomik dosya yazımı,
sabit SDK/port, dört immutable vendor, PNG/resource/PE metadata politikaları,
üç C export ve ordinal'ları değişmedi. Derleyici/linker ayarları değiştirilmedi;
Golden için mevcut /LTCG:OFF sözleşmesi korunuyor. Kamuya açık C++ kaynak API
imzaları korunuyor; iç Environment yerleşimi değiştiğinden eski C++ OBJ dosyaları
yeniden kullanılmamalı, kaynaklar yeniden derlenmeli.

## Ölçüm ve regresyon

Tek probe, aynı LLVM-MinGW Release ayarları ve aynı XML dışı core kütüphanesiyle
dört varyant karşılaştırıldı: önceki kod, yalnız tampon değişikliği, yalnız
wrapper değişikliği, ikisi birlikte. Her varyant için 5 dönüşümlü sırada tur,
her turda 3 senaryo ve senaryo başına 500 işlem çalıştırıldı. Üretim allocator'ı
değiştirilmedi; tahsis sayacı yalnız ayrı test executable'ındadır.

| Senaryo | Tahsis/işlem önce → sonra | İstenen heap baytı/işlem önce → sonra |
|---|---:|---:|
| Küçük LSX isteği |13 →9|9482 →9370|
| 64KiB metin |8 →6|140200 →74648|
| 256 alt düğüm |539 →281|83808 →79184|

Tahsis sayıları ve baytları allocator metadata'sını içermez. p50 ortancaları
iyileşti, ancak p95/p99 değerleri her senaryoda iyileşmedi; Wine zamanlaması
dalgalanıyor. Örneğin çok düğümlü senaryoda p95 ortancası 255900→291900 ns oldu.
Bu ölçümden native hız veya kuyruk gecikmesi kazanımı çıkarılmıyor. Tüm varyantlar
ve tüm örnekler [XML_BENCHMARKS.json](evidence/optimization/XML_BENCHMARKS.json)
içinde tutuluyor.

Yeni xml_reader_allocation_contracts testi önceki kodda ikinci input büyüklüğünde
tampon nedeniyle başarısız, yeni kodda başarılıdır. Test ayrıca cache lookup'ın
yeni tahsis yapmamasını ve wrapper tahsis hatasından sonra belge kaybetmeden
yeniden denenmesini doğrular. Süreye bağlı kırılgan test eşiği yoktur.
Kalıcı xml_reader_corpus_contracts testi aynı 12087 girdiyi ve önceki koddan
kaydedilen tam çıktıları tests/fixtures/xml_reader altında tutar; her Native
Acceptance çalışmasında yeniden karşılaştırır. XML unit testleri sayılı/sonlandırılmamış dilim, çağıran tamponun sonradan
değiştirilmesi ve 1025 çocukla rehash/pointer kimliğini de denetler. Server testleri
receive-size pozitiflik ve üst sınırını SIZE_MAX dahil sınırlarla kontrol eder.

Eski/yeni XML okuyucu ayrıca 12087 normal, bozuk, kesilmiş, NUL içeren, UTF8/rasgele
bayt ve depth sınır girdisinde Linux ASAN+UBSAN altında karşılaştırıldı. Error
code/flag, tam ErrorStr, bütün element/attribute/text çıktıları bayt düzeyinde
aynı, sanitizer hata çıktısı yok. Bu sonlu corpus tam program eşdeğerliği veya
oyun kabulü iddiası değildir. Sonuç ve corpus/output hash'leri
[XML_CORPUS_PARITY.json](evidence/optimization/XML_CORPUS_PARITY.json) içinde.

Yerel son sonuç: Release 196/196, etkilenen Debug 21/21, kaynak QA 17/17
grup başarılı. XML unit testinde 649 kontrol ve kalıcı corpus testinde 12087
girdi ASAN+UBSAN altında geçti. Temiz ZIP açılımından testler ve geliştirme DLL
kapalı üretim derlemesi ile üç gerçek yükle/başlat/profil/kapat/unload döngüsü
başarılı. Aynı LLVM Release çıktısı 1081856 → 1077760 bayt, yani 4 KiB küçüldü.

Güncel derleme/test sonuçları, gerçek önce/sonra DLL bölüm boyutları ve native
bekleyen işler [VALIDATION.json](evidence/optimization/VALIDATION.json) içinde.
LLVM çıktısındaki küçülme sahibin önceki MSVC DLL boyutuna oranlanmamalı; farklı
toolchain çıktıları karşılaştırılarak kazanım iddiası yapılmıyor.

## Windows'ta gerçek boyut kanıtı toplama

Normal Production Build tamamlandıktan sonra proje kökünden:

```powershell
python -B scripts/collect_size_evidence.py --build-dir build/golden_msvc --dll dist/preloader_l.dll --output build/qa/native-size-evidence.zip
```

VS Developer ortamında gerçek dumpbin de PATH'teyse `--with-symbols` eklenebilir.
Araç mevcut kaynak manifestini doğrular; DLL'i çalıştırmaz, derlemeyi veya link
ayarlarını değiştirmez, dist'e terfi/yayın yapmaz. Var olan raporun üzerine yazmaz.
ZIP ve yanına SHA256 dosyası üretir. Bilinmeyen/LTCG object biçimleri için PARTIAL
sonuç verir; boş kanıtı tam sonuç diye sunmaz.

Kaynak sözleşmeleri, final/raw DLL bölümleri/export'ları, üretim MAP'i ve yalnız
güncel CMake hedef girdilerindeki ordinary/BigObj COFF OBJ bölümleri toplanır.
Diskte kalmış eski OBJ'ler güncel grafikte yoksa dahil edilmez. 14 altyapı testi
bu sınırları, VS object yolu çözümünü, bozuk COFF'u ve üzerine yazma engelini
doğrular. Gerçek LLVM üretim grafiğinde 145 güncel OBJ başarıyla okundu.

OBJ bayt toplamı final DLL katkısı değildir: linker kullanılmayan/aynı COMDAT'ları
çıkarabilir veya birleştirebilir. MAP public-symbol aralığı da kesin fonksiyon
boyutu sayılmaz. Gerçek native önce/sonra incelemesi, aynı MSVC/SDK/Release
ayarlarıyla yeniden derlenmiş iki sürümün MAP/OBJ/DLL kanıtlarını gerektirir.
Şu an eski kabul edilmiş native DLL ve logları mevcut, karşılık gelen eski
native OBJ/MAP elde değil. Yeni collector bu eksik kanıtı toplamayı mümkün kılar.

## Sonraki optimizasyonların kabul ölçütü

Yeni bir silme adayı için gerçek production link katkısı, gelişim DLL/test/virtual/
callback/lifetime kökleri ve hata yolları birlikte incelenmeli. İyileştirme ayrı
değişiklik olarak ölçülmeli; önceki davranışla kontrollü regresyon, ilgili Debug
ve tam Release testleri, source QA, temiz üretim derlemesi ve native/game kabulü
tamamlanmalı. Vendor, ABI veya ürün politikasını sessizce değiştirerek küçültme
yapılmamalı. Bu tarama "başka hata veya ölü kod kalmadı" garantisi değildir.
