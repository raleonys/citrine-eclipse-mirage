# Donor kapanışı ve doğrulanan hata yolları

**Brownfield ve Bluefield emekliye ayrılmadı.** Önceki emeklilik önerisi geri
çekildi. Bu tur, kanıtlanmış üç hata yolunun düzeltmesini ve karşılaştırmalı
incelemenin devamını içerir; başka hata kalmadığı iddiası içermez.

## Uygulanan düzeltmeler

| Hata yolu | Değişiklik | Kalıcı regresyon |
|---|---|---|
| `async_accept` başlangıcında tahsis hatası | `accept_pending` geri alınır; exception saklanmaz. Yeniden başlatma artık eski bayrağa takılmaz. | `net_failure_contracts`: gerçek acceptor, tahsis hatası, gerçek loopback peer ile tekrar kabul. |
| Handler hatasından sonra cleanup post tahsisinin de başarısız olması | Worker yalnız atomik stop işaretini yazar. IO döngüsü işareti görüp kendi listener/client kaynaklarını kapatır. | Gerçek worker, gerçek socket, sıradan handler hatası için sağlıklı oturum kontrolü ve cleanup-post tahsis hatası. |
| `FreeLibrary` başarısızlığında ek DLL kaydının kaybolması | Başarılı slotlar sıfırlanır; başarısız slotlar aynı sahibin sonraki cleanup denemesine kalır. Load öncesinde sahiplik slotu ayrılır. | Gerçek inert fixture DLL, iki lease, karışık başarı/hata, ardışık hata ve tekrar; tahsis hatasında hiç native load yapılmaması. |

Transport stop artık yeni bir `asio::post` tahsisine ihtiyaç duymaz. IO runner
100 ms'lik `run_for` dilimleriyle stop işaretini gözler; bu yeni bir CFG ayarı
veya protokol timeout'u değildir. 100 ms, yürütülmekte olan callback'in bitmesi
koşuluyla kontrol gözlem aralığıdır; toplam shutdown süresi garantisi değildir.
Windows Asio `io.stop()` yolundaki IOCP wake işlemi de exception üretebildiği
için tahsis hatası düzeltmesi yalnız `io.stop()` çağrısına dayandırılmadı.
Socket'ler worker thread'den kapatılmaz. IO runner socket'leri kapattıktan sonra
kontrol thread'i onu join eder; aktif handler'lar bundan sonra drain edilir.
Join sonrası cancellation callback'i exception üretirse sonraki `stop()` kalan
callback'leri yeniden drain edebilir. Kullanıcı/provider callback'lerini zorla
sonlandırma veya genel bounded-shutdown iddiası eklenmedi.

Referans `libraries::unload_035ab0` gövdesi değiştirilmedi: özgün kaynağın
FreeLibrary sonucunu görmezden gelen davranışı referans olarak kalır. Retry
sahipliği somut `ExtraLibraries` uygulama adaptöründe düzeltilmiştir. Native
yüklemeden önce sahiplik ayrıldığı için başarısız bir vector tahsisinden sonra
başarısız olabilecek FreeLibrary rollback'ine ihtiyaç kalmaz.

## Test kanıtı ve sınırı

İki kalıcı test executable'ı sekiz ayrı hata senaryosu içerir. Aynı test kaynakları
önceki ve düzeltilmiş üretim CPP'leri üzerinde derlendi: eski kodda sekiz senaryo
FAIL, düzeltilmiş kodda sekizi PASS. [RED_GREEN.json](../metadata/evidence/recovery/RED_GREEN.json)
ve ham case logları saklanır. Testler hata injection'ını kendi executable'larıyla
sınırlar; üretime allocator veya Win32 API override'ı eklenmedi.

İlk Debug koşusunda `network_policy_contracts` mevcut bir oracle yarışı gösterdi:
IO thread socket'i kapatabilirken worker henüz hata sayacını artırmamış olabilir.
Test artık hem kapanmayı hem asenkron hata kaydını süre sınırında gözler; oversized
response kabulü veya hata beklentisi gevşetilmedi. İlk FAIL logu
[DEBUG_INITIAL_TESTS.log](../metadata/evidence/recovery/DEBUG_INITIAL_TESTS.log) olarak korunur.
Son koşular, tekrar sayıları ve kaynak QA sonucu
[VALIDATION.json](../metadata/evidence/recovery/VALIDATION.json) içindedir.

Üç üretim C export'u/ordinal'ı, dört vendor, TinyXML derinliği500, sabit SDK/port,
connect kapsamı, PNG/VERSIONINFO ve Golden linker/metadata politikası korunur.
Public header veya fonksiyon imzası değiştirilmedi. Geliştirme DLL'inin otomatik
C++ symbol tablosu aynı compiler altında ayrıca karşılaştırıldı; compiler'ın
implicit inline destructor emisyonu ve yeni iç metotlar farklıdır. Bu tablo sabit
üretim C ABI'si değildir; ayrıntı [SOURCE_API_DELTA.json](../metadata/evidence/recovery/SOURCE_API_DELTA.json).

LLVM-MinGW/Wine sonuçları native Windows/MSVC, Golden veya oyun kabulü sayılmaz.
Eski kabul edilmiş DLL'in SHA256'sı
`4a0f37c9328ecf31d0815a9e9b85634bc481e77e2f07b636d4ea65bb4783f26b`;
o DLL güncel değişmiş kaynak olarak sunulmaz. Yeni native Windows/Golden ve oyun
kabulü bekliyor. Yerel çıktı dist'e terfi ettirilmedi.

## Genişletilen karşılaştırma

1805 first-party donor dosyasının hash/rol envanteri korunur. Bu turda XML,
Config2, route binding/selector, startup ve IOCP backpressure alanlarından
30 dosya daha seçilmiş branch'ler düzeyinde karşılaştırıldı: toplam67 kayıt.
**67 tam dosyanın bütün semantiği kapatılmadı; 1738 dosyada henüz manuel kayıt
yoktur.** Bunların hepsi uygulama CPP'si değildir; test, araç ve belge de içerir.
Lexical fonksiyon ve benzerlik indeksleri semantik eşdeğerlik kanıtı sayılmaz.

| Aile | İncelenen fark | Karar |
|---|---|---|
| XML framing / RequestView / child index | Brownfield literal LSX çerçevesi, tek envelope/command ve owned DOM span/index kullanır. Bizde NUL-framing ve RE belge/dispatch sözleşmesi var. | Daha katı kabul politikası sessizce aktarılmadı; ownership/sınır test fikirleri ayrı tutuldu. |
| Entity/UTF8/response formatter | Brownfield decoded/source ayrımı, checked-size formatter ve strict XML karakter doğrulaması içerir. Bizde TinyXML ve RE printer yürür. | Donor codec/allocator'ı toptan alınmadı. Erken serializer ve toplam bellek bütçesi hâlâ açık adaydır. |
| TinyXML adapter | Bluefield implementation-local sınırı100'e indirir; ayrıca staging ve wrapper tahsislerini içerir. | Kullanıcı depth500 kararı ve mevcut ölçülen tahsis iyileştirmesi korunur. |
| Config2 / settings | Donor raw quoted spans, depth33, staged visitor, section/key authority ve farklı trailing-comment politikaları kullanır. | RE escape action'ları, partial state ve hata pozisyonları değiştirilmedi. SDK/port CFG override'ları geri alınmadı. |
| Path / store | Brownfield module-relative ghc helper ve private model/arena storage içerir. | EXE-relative config ve std::filesystem kararı korunur; Gulrak veya özel CRT/548-byte ABI geri getirilmedi. |
| Route inventory / auth / selectors | Donor primary, compatibility, extension ve local-social route'larını ayırır; bazı selector/default ve auth lease sınırları farklıdır. | Yeni komutlar veya farklı commit politikaları eksik RE aktarımı diye ithal edilmedi. Tam route-output differential kampanyası hâlâ açık. |
| Startup / teardown | Donor stop-before-initialize ve stop-before-listener kontrolleri, ayrı logical/physical owner retirement kullanır. | Mevcut DLL worker/Hook sahibiyle faz bazlı fault karşılaştırması sürmeli; singleton/model mimarisi alınmadı. |
| IOCP backpressure | Donor testi 32MiB non-reading peer ve native-IOCP compile gate kullanır. Bizde actual AsioService slow greeting/response/deadline testleri var. | Var olan test karşılığı görülüyor; Wine'da çalıştırmak gerçek Windows IOCP kabulü sayılmıyor. Tam completion/cancellation/fault matrisi açık. |

Dosya/hash/okunan branch kayıtları:
[MANUAL_REVIEW_LEDGER.json](../metadata/evidence/recovery/MANUAL_REVIEW_LEDGER.json),
[ALL_DONOR_FILES.csv](../metadata/evidence/recovery/ALL_DONOR_FILES.csv),
[CANDIDATES.json](../metadata/evidence/recovery/CANDIDATES.json).
Bu aday listesi ilk bulguların tarihsel durumunu korur; bu turda DC01–DC03 için
uygulanan düzeltmeler ve kalan işler yukarıdaki güncel kayıtla okunmalıdır.

## Emeklilik için açık işler

Socket/listener close hatasında gerçek Winsock sahipliği ve pending callback
retirement; timer kurulum/iptal hatasında cached deadline; sınırsız hook quiesce
beklemesi; registry original-close/handle-reuse provenance; toplam serializer,
event ve config bellek bütçeleri; console/environment yan etkileri ve startup
fazları kapanmadı. Kalan kaynak/test/araç aileleri manuel karşılaştırılmalı;
her aday için uygulandı, zaten karşılanıyor, bilinçli politika farkı, uygun değil
veya açık kararının somut kaynak/test kanıtı olmalı. Ardından değişmiş kaynakla
native Windows/Golden ve oyun kabulü gerekir. Bunlar tamamlanmadan "başka
iyileştirme kalmadı" veya "donor obsolete" sonucu verilmez.

Kaynak parent kimliği, bir önce teslim edilmiş kaynak ZIP'in birebir hash'idir:
`1f2a6581a08dbafa940031353c8723e1947e00f0a4681f3bceb1d9f65e3d509e`.
Accepted binary/log ZIP'i veya sonradan kabul belgeleri eklenmiş çalışma kopyası
bu parent yerine kullanılmaz. Review edilen değişen dosya listesi ve byte pin'leri
[REVIEWED_CHANGES.json](../metadata/evidence/recovery/REVIEWED_CHANGES.json) içinde kayıtlıdır.
