# Sunucu ve session uygulama çekirdeği

`server.cpp/.hpp`, `anadius64.dll` içindeki port duyurusu, session çerçeveleme ve yazma kuyruğu mantığını okunabilir C++20 olarak yeniden kurar. ASIO/IOCP ağı ve protokol Handler'ı zorunlu, türlenmiş bağımlılıklardır. Bu klasör tek başına yüklenebilir bir DLL değildir; gerçek socket açan bir backend içermez.

Hedef SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Adresler bu Windows DLL'sinin `0x180000000` tercih edilen tabanına aittir. Kanıtlar mevcut `../anadius-all-functions.asm.txt` ve `../anadius-all-functions.pseudo.c` dosyalarından alındı. Assembly ve pseudocode'un ayrı Evidence ID'leri, kapsam açıklamalarıyla `evidence-map.json` içinde tutulur. Yalnız yeni kaynaklardan derlenen mock EXE çalıştırılmıştır; orijinal DLL/EXE yüklenmemiş veya değiştirilmemiş, socket bind/listen ve ağ işlemi yapılmamıştır.

| Windows adresi | Gözlenen rol | Buradaki kapsam |
|---|---|---|
| `0x180029ea0` | Sunucu iş parçacığı gövdesi | Context → constructor → run → normal temizleme sırası; context içi ASIO dış sınırdır |
| `0x180024480` | Sunucu constructor | Loopback parse seçimi, portu okuma/yayımlama, readiness, ilk accept sırası |
| `0x1800270e0` | `do_accept` adayı | `0x180021b00` async accept sınırına yeniden başvurma |
| `0x180025720` | Accept completion, session start parçaları | Başarılı session'ın greeting/read başlangıcı, hata logu, tekrar accept |
| `0x180023000` | Session/shared ownership construction | Handler ilk reset'i temsil edilir; özgün allocation/move/shared_ptr ABI'si kurulmaz |
| `0x1800280d0` | Session `do_read` adayı | NUL sınırını arama ve tamamlanma akışı; ASIO lifetime/composed-operation mekanizması dış sınırdır |
| `0x180025c40` | ASIO `read_until` bileşimi | NUL çerçeve davranışı, commit-before-error ve receive chunk politikası |
| `0x180025480` | Read completion | Ayırıcı hariç kopya, detached worker, ardından tüketme ve yeni okuma |
| `0x180028d30` | Session `process_message` adayı | Boş mesaj kuralı, Handler çağrısı ve socket executor üzerinden yanıt dispatch'i |
| `0x180028270` | Session `do_write` adayı | FIFO'ya ekleme, ilk öğede yazmayı başlatma |
| `0x18002a500` | Session `write_loop` adayı | Öndeki öğenin 64 KiB parçalarla async gönderimi |
| `0x1800261d0` | ASIO write bileşimi | Kısmi gönderim sayacı, sıfır ilerleme ve hata tamamlanma kuralları |
| `0x180025a70` | Write completion | Başarıda pop/continue; hatada log ve kuyruğu koruma |

**Bu tablo tamamı eksiksiz kurtarılmış 13 fonksiyon anlamına gelmez.** Büyük ASIO fonksiyonlarının yalnız session'ın kullandığı davranış alt kümesi yeniden ifade edilmiştir; JSON her girişin sınırını belirtir. `0x180024db0`, socket nesnesinin destructor gövdesidir; genel sunucu `stop()` metodu olarak adlandırılmaz. `0x1800241b0` context init ve `0x1800298b0` scheduler run bu modülde backend sözleşmesidir.

## Sunucu başlangıcı ve port-ready

Constructor, `0x1800244ee` noktasında `127.0.0.1` metnini önce Windows family `23` ile parse etmeyi dener. Hata varsa `0x18002452e` family `2` ile tekrar dener. IPv4 normal loopback yoludur; ilk deneme hata vermemişse ilk sonucunun seçildiği dal da korunmuştur. `Endpoint` içindeki port host sırasındadır; kernel backend'i doğru sockaddr/network dönüşümünü yapmak zorundadır.

`0x180020e20` çağrısına `reuse_address=true` verilir. Bu yordamın statik gövdesi socket/IOCP setup, reuse option, bind ve `listen(...,0x7fffffff)` içerir. Kaynak bu ASIO altyapısını zorunlu `construct_acceptor_020e20` sınırı olarak bırakır; reuse/backlog değerlerini kaynak gönderir. Ardından bekleyen client socket'i başlatılır ve `getsockname` ile gerçek endpoint okunur. Hatalı endpoint ve `returned_size > 128` yolu readiness öncesinde kesilir. Kaynaktaki `raise_error` context metinleri entegrasyon etiketidir; özgün MSVC exception/INT3 ABI'sinin kopyası değildir.

`0x18002466d..0x180024676`, bound portu host sırasına çevirip `0x1800a3c90` alanına yazar. `0x180024690..0x1800246e4` bunu ondalık metin olarak `EALsxPort` ortam değişkenine verir. API'nin BOOL dönüşünün başarı kontrolü yoktur; kaynak, ortam değişkeni yazımı false dönse de `0x18002472c` ile readiness bayrağını tamamlar. Bunun için önceki `anadius-execution-source` modülünün gerçek kurtarılmış mutex/bayrak yolu kullanılır. İlk async accept ancak bundan sonra `0x180024758`'de başlatılır. Global port, bayrak, mutex ve frekans başlangıç değerleri caller tarafından sağlanır.

## Session okuma ve işleme

Başarılı accept, yeni session oluşturur ve Handler durumunu kurar; ardından `0x180025777` tekrar `0x18002bc70` sonucu ile Handler durumunu değiştirir. Kaynak constructor + `start_025720` olmak üzere iki reset çağrısını korur. Handler'ın `0x18002d540` initial-message çıktısı aynen gönderilir; bu kod kendi başına ayırıcı eklemez. Yanıtın NUL içermesi/şifrelenmesi Handler çıktısına bağlıdır.

Greeting yazması bloklayan write-all yoludur. `0x1800258bb` çağrısından sonra `0x1800258c0 ADD RBX,RAX`, `0x1800258c3 CMP RBX,RDI`, `0x1800258c6 JC 0x180025850` vardır. Döngü toplam ilerlemeyi sınar; dönüşte `RAX==0` için bitirme dalı yoktur. Bu yüzden hata yokken sıfır ilerleme, aynı toplamla tekrar denemeye yol açar; sürekli sıfır dönmesi bu kaynakta da tamamlanmaz. Hata slotu `0x18002585a` ve döngü sonundaki `0x1800258c8` civarında sınanır. Mock testinde bir sıfır başarılı dönüşün ardından ilerleme verilerek bu kuralın sonlu bir örneği doğrulanır.

Okuma ayırıcısı `\0`'dır (`0x1800280d0` state kurulumu ve `0x180025c40` byte karşılaştırması). Bir network parçasında birden fazla frame varsa sonraki frame'ler input buffer'da korunur; son yarım frame yeni receive'e kalır. Frame callback'ine verilen byte sayısı NUL dahil uzunluktur; `0x180025480` başındaki `param_3 - 1` ayırıcı hariç request kopyasını üretir. Detached worker **önce** başlatılıp detach edilir; bu başarısızsa input tüketilmez ve okuma yeniden başlamaz. Başarılı launch sonrası NUL dahil prefix silinir, sonra `0x1800280d0` tekrar çağrılır.

`0x180025c7d..0x180025ccd`, alınan byte sayısını prepared alandan commit edecek string resize/commit işlemini yapar; **ondan sonra** `0x180025cd2..0x180025ce0` error/zero-progress dalını sınar. Hata varsa `0x180026033` üzerinden callback'e gider; delimiter taraması yapılmaz. `0x180025480` hata dalı log/return eder, buffer erase veya read rearm yapmaz. Kaynak bu nedenle `bytes + error` tesliminde byte'ları buffer'a ekler fakat NUL içerse bile frame işlemez. Taşıma sınırı ASIO tarafından normalize edilmiş completion vermelidir: EOF, başarılı sıfır byte receive yerine hata olarak teslim edilir.

Boş frame için detached worker yine oluşur ama `0x180028d5d..0x180028d8b` Handler ve yanıt kuyruğunu atlar. Diğer frame'ler `0x180033d70` Handler işleyicisine verilir; yanıt socket executor üzerinden `0x180028270`'e dispatch edilir. Detached worker'ların tamamlanma sırası isteklerin geliş sırasından farklı olabilir; FIFO, executor'a gerçekten ulaşan yanıtların sırasını korur. Protokol parser/crypto/license davranışları burada taklit edilmez: `Handler` zorunlu sözleşmedir.

## Yazma kuyruğu

`0x180028341..0x18002834e`, ekleme sonrası kuyruk büyüklüğü yalnız `1` olduğunda `write_loop` başlatır. İlk mesajın tümü gönderilene kadar sonraki öğeler bekler; her kernel send isteği en çok `0x10000` byte'tır. `0x1800261ea` tamamlanan byte sayısını toplar. `0x1800261f5..0x180026210`, hatasız **sıfır** byte dönüşte bile composed write'ı bitirir; `0x180025a70` başarı callback'i mesajı kuyruktan çıkarır. Bu async davranış, yukarıdaki bloklayan greeting döngüsünden farklı gözlenen bir kontrol akışıdır.

Write error öğeyi çıkarmaz ve yeniden deneme başlatmaz. Sonradan ekleme olduğunda kuyruk zaten dolu olduğundan otomatik yeniden başlama da olmaz. Hem read hem write hata logunda `0x1800285b0` kategori kimliği ile kod `2` birlikte olduğunda (ASIO misc EOF) log bastırılır; başka kategoride kod `2` loglanır. `Transport` completion'ları bu kategori kimliğini doğru `ErrorCategory` değerine çevirmelidir. Kernel error remap ve ASIO category ABI'si backend sorumluluğudur.

## Derleme ve doğrulama

Çalışma alanı kökünden:

```powershell
& .\outputs\anadius-server-source\build.ps1
```

LLVM-MinGW 20261006 / Clang 23.1.3, C++20, `-O2 -Wall -Wextra -Werror` kullanılır. `server.o` ve statik runtime ile kendi `server_mock_tests.exe` üretilir. Test EXE yalnız mock scheduler, transport, Handler ve execution-platform kullanır; Windows socket backend'i yoktur. Gerçek sonuç ve derleyici/kaynak/kanıt/nesne SHA-256 değerleri `validation.json` içindedir.

Kontroller port-ready sırasını, ignore edilen ortam yazma sonucunu, hatalı endpoint yollarını, tüm network split konumlarında NUL framing'i, birlikte gelen/yarım/boş frame'leri, detached launch failure'ı, bytes+error önceliğini, worker sıra farkını, binary verinin kısmi/64 KiB send sonrasında korunmasını ve FIFO hata davranışını kapsar. Sentetik kontrollerin geçmesi orijinal DLL ile runtime eşdeğerliği kanıtı değildir.

## İsimlendirme ve kalan iş

Mac x64 symbol inventory'de `server::do_accept` (`0x3fd80`), `session::start` (`0x4bc80`), `do_read` (`0x4dfe0`), `process_message` (`0x53000`), `do_write` (`0x53340`), `write_loop` (`0x54930`) adları bulunur. Bu isimler Windows çağrı rollerini anlamlandırır; Mac gövdeleri bu turda decompile edilmediği için birebir Mac/Windows fonksiyon eşliği iddia edilmez. Windows mantığına ilişkin kanıt yukarıdaki Windows assembly'sidir.

Gerçek ASIO io_context/scheduler, IOCP/socket backend'i, MSVC allocator/string/deque/shared_ptr ABI'si, exception cleanup ve tüm nesne lifetime/refcount davranışı yeniden kurulmuş değildir. `server_start_029ea0` yalnız normal uygulama çağrı sırasını temsil eder; WSA referans sayacı, service registry, CRT/TLS ve istisna unwind temizlemesi backend'de tamamlanmalıdır. `accept_completion_025720` başarılı durumda dışarıda bağımlılıkları hazırlanıp bizim `Session` sınıfımızla oluşturulmuş nesne ister; özgün accepted socket move/allocation kısmını tamamlanmış saymaz.

Kaynak GNU C++ string/deque temsilini kullanır; dinamik string kapasiteleri ve allocation zamanlaması MSVC ile aynı olmak zorunda değildir. `receive_request_size` gözlenen `min(maximum-size,65536,max(capacity-size,512))` politikasını verilen geçerli `size <= capacity <= maximum` koşullarında ifade eder. GNU `max_size()` ile özgün MSVC sınırı aynı kabul edilmez.

Session callback'leri caller'ın sağladığı ömür ve sıralama sözleşmelerine bağlıdır. Worker capture'ı raw `this` kullanır; özgün `0x180025480` thread payload'ında da raw session pointer görülür. Buna yeni bir lifetime güvenliği varsayımı eklenmez. Gerçek concurrency, socket kapanması/cancellation, orijinal DLL yüklemesi ve host oyunda davranış eşdeğerliği sınanmamıştır.
