# Native socket sahipliği ve IO retirement

Önceki incelemede doğrulanan uygulama-owned socket boşluğu için kapanış sahibi
eklendi. Asio close hatasından sonra kaybolan kayıt artık sessiz başarıya
dönüşmez. Güvenli retry ile belirsiz sonuç birbirinden ayrılır; pending IO ve
tampon ömrü kapanış kabulünün parçasıdır. Donor emekliliği hâlâ BLOCKED.

## Native sonuç politikası

Microsoft, `WSAEWOULDBLOCK` durumunda handle'ın geçerli kaldığını açıkça belirtir.
Aynı belge handle'ın yeniden kullanılabileceğini ve close dönüşünün pending IO
completion'larının bittiği anlamına gelmediğini de belirtir.
[closesocket sözleşmesi](https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-closesocket).

| Sonuç | Uygulanan sahiplik davranışı |
|---|---|
| Başarılı close | Native kayıt bırakılır; ilgili callback'ler bitene kadar oturum tutulur. |
| Would-block | Exact raw handle retained durumunda tutulur. Retry öncesi uygulamanın varsayılan linger-off politikası kurulur; başarılı native close yalnız bir kez kabul edilir. |
| Diğer close hatası | Sayısal değer uncertain durumunda kaydedilir; kör native close, socket sorgusu veya tekrar kullanım yapılmaz. Cleanup başarısı engellenir. |
| Retry sırasında linger ayarı hatası | Close çağrılmaz. Retained sahiplik saklanır; descriptor/not-initialized hatası varsa uncertain'a geçilir. |

Uncertain durum otomatik olarak temizlenmez. Kaydın hâlâ aynı kernel socket'ini
gösterdiği kanıtlanamadığında yeniden kapatma güvenli değildir. Bu durumda DLL
worker'ı aynı Application/IO sahibini ve image lease'i tutar; shutdown cleanup
failure döndürür. Bir sonraki istek durumu yeniden raporlar, sayısal handle'a
yeniden dokunmaz. Force-clear CFG anahtarı veya üretim export'u eklenmedi.
Bu, her native arızanın otomatik kurtarılacağı iddiası değildir. Retained retry,
uygulamanın socket üzerinde tek sahibi olmasına ve dengeli Winsock scope
ömrünün korunmasına dayanır.

## Sahipliğin kapsamı

First-party retained wrapper'lar Client socket'ini, IPv4/IPv6 acceptor'larını,
bind adaylarını ve accepted callback'e geçen geçici socket'leri kapsar. Native
close başarısızlığından sonra wrapper yok olacaksa kayıt önceden ayrılmış deferred
listeye aktarılır. Client constructor/handler hatası da native kaynak sahibini
kaybettirmez. İlk deferred failure admission'ı durdurur; başarısız bind cleanup'ı
arkada bırakılıp sonraki port açılmaz. Normal occupied-port taraması3226–3236
aynı kalır.

Deferred kapasite, policy session cap +8 olarak socket ediniminden önce ayrılır.
Published client/listener'lara en fazla iki accept callback'i ve iki bind adayı
katılabilir; ilk başarısız retirement sonrası yeni edinim durur. İlgili IO ve
deferred kayıtlar callback target'larından sonra yok edilir. Bu liste retirement
yolunda yeni heap tahsisine dayanmaz.

Client listesi artık strong ownership taşır; Client yalnız Impl'i borrow eder,
ownership cycle kurulmaz. Native close, timer ve read/write callback'leri bitmeden
oturum kapasiteden çıkarılmaz. Sayımlar initiation exception'ında geri alınır,
completion'ın her çıkış yolunda bir kez düşürülür. Tamamlanmış client'lar sonraki
admission sırasında prune edilir.

Kapanış IO runner'ın socket'leri kapatması, runner join, aktif handler drain ve
IO context drain sırasını korur. Son IO drain100 ms'lik deneme kullanır; zaman
dolarsa yeniden denenebilir sahibi saklar. Başarı için context'in doğal olarak
stopped olması, accept callback'lerinin tamamlanması, native/deferred kayıtların
boşalması ve client retirement'ı birlikte gerekir. Bu süre, kullanıcı/provider
kodunu veya tek bir callback'i preempt eden toplam shutdown garantisi değildir.

Mevcut response completion closure'ı PreparedResponse tamponunu zaten sahiplenir;
bu davranış korundu. Cancel sırasında negative receipt verilir, kernel tarafından
kullanılan tampon completion'a kadar tutulur. Tamamlanmayan veya iptal edilen
gönderim için positive delivery acknowledgement eklenmedi.

Bu değişiklik Asio vendor baytlarını, private service implementation'ını veya
header makrolarını üretimde değiştirmez. Public Asio API kullanılır; Windows7
hedefini yükselten `release()` zorunluluğu getirilmez. Sınır, first-party tarafından
edinilmiş/teslim alınmış socket'lerdir. Vendor'ın kendi iç geçici nesnelerindeki
bütün olası OS failure yollarının kapandığı iddia edilmiyor.

## Test kanıtı

`socket_retirement_contracts` gerçek IOCP socket/timer ve üretim implementation'ı
üzerinde dokuz senaryo içerir: retained retry, option failure, belirsiz açık/kapalı
sonuçlar, write-buffer/receipt completion ömrü, timer retirement, Client constructor
rollback, bind rollback ve listener/accept retirement. Native hata injection'ı
yalnız test executable'ında belirlenmiş socket'e uygulanır. Sayısal reuse
gözlenmediyse gözlenmiş gibi raporlanmaz; esas oracle belirsiz değere tekrar native
çağrı yapılmaması ve unrelated socket'in canlı kalmasıdır.

`socket_dll_retirement_contracts` üç izole child process'te gerçek DLL worker'ı,
Application ve Asio service'i çalıştırır. Hooks bu fixture'ın Options'ında kapalıdır;
bu bir hook patching testi değildir. Would-block retry aynı Application'ı kapatır,
tekrar native close yapmaz ve image lease'i bırakır. Diğer iki çocukta close'un
gerçekte yapılmadığı ve yapıldığı halde hata bildirildiği kontrollü sonuçlar
üretilir. Her ikisinde de caller FreeLibrary yapsa bile worker lease'i erken
unload'u engeller. Quarantine sahibini yalnız test process exit'i/OS reclaim eder;
üretime test amaçlı unquarantine API'si eklenmez. Parent test, çocuklara ayrılmış
fixture dizinlerini çocuklar sonlandıktan sonra temizler.

Aynı son harness kaynakları ve aynı eski core archive kullanılarak exact önceki
kaynak ve düzeltme derlenir. 12 sözleşme senaryosu önce FAIL, sonra PASS.
[Önce/sonra kayıtları](../metadata/evidence/socket-retirement/PINNED_RESULTS.json) ve build
receipt'leri source/harness/core archive hash'lerini taşır. Bunlar test sırasında
kontrollü Win32 sınır hatalarıdır; gerçek Windows hata sıklığı veya native MSVC
kabulü değildir.

Yerel tam sonuç, Debug ve kaynak QA kayıtları
[VALIDATION.json](../metadata/evidence/socket-retirement/VALIDATION.json) içindedir.
Public header bildirimi ve üretim C ABI'si değişmez: DllMain@1, SURGE@2,
DllShutdown@3. Geliştirme DLL'inin otomatik compiler/helper sembol farkları
[SOURCE_API_DELTA.json](../metadata/evidence/socket-retirement/SOURCE_API_DELTA.json) içinde
ayrı tutulur; private template emisyonu sabit C ABI değildir.

Native Windows/Golden ve oyun kabulü değişen kaynakla yeniden yapılmalıdır.
Registry provenance, aggregate bellek bütçeleri, provider/handler liveness ve
kalan donor kaynak/test/araç incelemeleri açıktır. Manuel donor branch ledger'ı
76 dosyadadır; bu tur tüm donor semantiğinin kapanışı sayılmaz.

Exact parent kaynak ZIP SHA256:
`e0b6964082c4343359d824684b5c60e667867db54f089796862080a81f73d8ac`.
Güncel değişen dosya pin'leri
[REVIEWED_CHANGES.json](../metadata/evidence/socket-retirement/REVIEWED_CHANGES.json) içindedir.
