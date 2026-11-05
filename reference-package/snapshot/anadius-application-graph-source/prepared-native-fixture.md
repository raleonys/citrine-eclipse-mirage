# Prepared Graph/Profile + NativeJobs senaryo kaynağı

Sonraki kaynak aşaması: [bildirim sonrası gecikmeli çıkış](prepared-native-delayed-exit.md).
Aşağıdaki inceleme ve kanıt önceki kaynak hash'ine aittir; yeni senaryo da henüz çalıştırılmadı.

Durum: **kaynak senaryoları eklendi; derleme/test/link/DLL çalıştırılmadı**.
Değişen C++ dosyası `graph_integration_tests.cpp` statik incelemeden geçti.
Bu aşamada üretim algoritması veya DLL tamamlanma yüzdesi artırılmış sayılmıyor.

Mevcut iki default/explicit same-worker senaryosu korunur. Eklenen native
senaryolar aynı test EXE'sinin ayrı argümanlarıyla, izole süreçlerde çağrılacak
şekilde yazıldı. Hiçbir süreç bu çalışma sırasında başlatılmadı.

`--prepared-native` gerçek varsayılan DomainStorage/heap/PTD/FLS, controller
PreparedMemory, aynı Domain'in gerçek NativeThreads original worker'ı ve gerçek
NativeJobs çocukları üzerinden tam selected HOST akışını kurar. Graph mevcut
`install_server_prepared` ile bağlanır. IOCP kendi kernel nesnesidir; socket
kimlikleri ve AcceptEx/WSARecv/WSASend sonuçları enjekte ve sentetiktir. Ağ
bağlantısı, gerçek configuration/token/license dosyası, orijinal image kodu veya
foreign DLL çalıştırılmaz.

Tek oturumda70 mesaj beş tür arasında dönüşür: GetProfile, GetAllGameInfo,
GrantAchievement, GetAuthCode ve QueryEntitlements. Somut Information, Content
ve Licensing sağlayıcıları, aynı Runtime/Online görünümü ve yaşayan
AuthenticationCode ile kurulur. Online nesne/modülü yüklenmez; auth fake-auth
rotasını kullanır. Clock/formatter kontrollüdür. Achievement total1000 olduğundan
14 artış sonunda progress14 kalır ve completion event'i üretilmez. Boş persistence
path'leri dosya erişimini atlar; memory file sağlayıcısının hiç kullanılmadığı
ayrıca denetlenir. Bu seçim tüm foreign/file davranışlarını doğrulamaz.

NativeJobs ve Profile iş kapasitesi1'dir. Her mesajın önceki SDK send completion'ı,
hazır yanıt tüketimi ve fiziksel reaping'i görülmeden sonraki mesaj verilmez.
`native_delivered` sayacı, receive callback'i geldiğinde bir sonraki çocuk henüz
işleme başlamamış olsa bile aynı isteğin ikinci kez beslenmesini engeller.
70 farklı request/response id'si,70 body/receipt/start/detach/continuation ve
70 private handle/barrier kapanışı; çocuk ve original için toplam140 ayrı
04a47c çağrısı kaynak beklentileri arasındadır. **Bunlar yürütülmüş sonuçlar değil.**

Test driver, NativeJobs'un private wait duplicate'inden ayrı proof HANDLE'ları
tutar. Fiziksel dispatcher.get Parent Operation dışında bu proof'ları bounded
wait ile join eder. Bu sayede senaryo OS zamanlamasının tesadüfine bağlı kalmaz;
NativeJobs'un kendi wait0/exit-code/close ve PreparedMemory duplicate reaping
yolları yine gerçek kaynak kodundan geçer. Bu driver politikası, receipt kabulü
sırasında çocuk hâlâ fiziksel olarak canlıyken oluşabilecek bütün interleaving'leri
kanıtlamaz. Son backing, original wrapper/FLS çıkışı gözlendikten sonra yalnız
controller'da kapatılır; NativeThreads ve en son Domain orada bırakılır.

| Süreç modu | Kaynak beklentisi | Beklenen çıkış |
|---|---|---|
| `--prepared-native` | 70 mesaj, kapasite1 reuse, FIFO ve tam source/physical drain | 0 |
| `--prepared-native-post` | Ready post reddi; row/closure/control/work/private handle/barrier tutulur; tekrar post yok | 191 |
| `--prepared-native-duplicate` | Bir child publication iki receipt verir; ikinci body/decrement yok; Profile/Jobs poisoned | 192 |
| `--prepared-native-wait` | Physical wait reddi; private handle/barrier kapatılmaz, work değişmez | 193 |
| `--prepared-native-close` | Private close reddi; gerçek CloseHandle çağrılmaz, kayıt/sahiplik korunur | 194 |

Hata süreçleri retained prefix'i doğruladıktan sonra `_Exit` kullanacak şekilde
yazıldı; C++ unwind ile tahmini cleanup yapılmaz. Bütün worker/proof beklemeleri
bounded'dır, başarısız wait'te canlı stack sahipleri boşaltılmadan süreç failfast
eder. Spy SDK callback'leri exception dışarı taşırmaz. Hemen alınan SDK error
değeri bookkeeping'den sonra korunur. Paylaşılan trace için mutex, genel assertion
sayacı için atomic kullanılır; diğer mutable uygulama alanları whole Parent ile
serialize edilir veya fiziksel join sonrasında okunur.

Son statik kabul: `prepared-native-fixture-review.json`. Yeni kaynak compiler
kontrolünden geçmedi. Testlerin geçmesi, production ABI/foreign-code/kernel
quiescence veya loader finality kanıtı yerine geçmeyecek. Önceki ZIP ve tam
derleme sonuçları kendi eski snapshot'larına aittir.

Sonraki kaynak hedefleri: receipt kabulünden sonra geciktirilmiş physical/FLS
exit interleaving'i; erken stop/detach, kapanış erteleme/retry ve daha geniş
negative-prefix senaryoları. Çalışma zamanı doğrulaması yalnız açık yeniden
derleme talimatıyla yapılacak. İki üretim loader hook'u ve CRT/image/foreign
ömrü sözleşmeleri ayrıca açıktır.
