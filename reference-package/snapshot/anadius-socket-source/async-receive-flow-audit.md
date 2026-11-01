# Async receive — 0x180021d10

Bu aşama yalnız **bir yeni normal gövde** ekler: `start_async_receive_021d10`. `async_receive.hpp/cpp` bağımsız derlenebilir C++20 kaynağı, SDK imzalı `NativeAsyncReceiveApi` ve gerçek `023dd0` kaynağını kullanan `RecoveredAsyncReceiveEffects` köprüsünü içerir. `025c40` read-until gövdesi bu aşamada tamamlanmadı.

## Adres ve kanıt eşlemesi

Hedef SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Pseudocode kanıtı `ev_d716892e207f1ae8230bf4ba5b7cce514f147811c107c7c9812d3b205fe10abf`; assembly kanıtı `ev_cb69360436ddc454d9ea6ddc971cd6cbc8fa39ab9ec36acaba00a999bd388274`. Tam yerel kesitler `async-receive-evidence.*`, kullanılan eski gövdeler `async-receive-context.*` dosyalarındadır. REA 6.5.0 kimliği ve Ghidra 12.1.4 kullanılabilirliği oturum sorgusunda doğrulandı; saklanan aynı hedef hashli kanıtlar yeniden kullanıldı. Yeni native analiz veya hedef yürütmesi yapılmadı.

| Native adres | Gözlem ve kaynak karşılığı |
|---|---|
| `021d46 / 021d4b` | `0xe0` baytlık operation isteği; gerçek `allocate_io_storage_0269b0`, mevcut TLS iki slotlu cache ve son marker sözleşmesi kullanılır. |
| `021d6c / 021daf / 021e5d` | Socket weak control için geçici increment, operation increment ve geçici decrement; son decrement eski değer 1 ise slot08 delete. Aynı saklanan control kullanılır. |
| `021d70..021db3` | İlk socket state baytı saklanır; callback `027710`, OVERLAPPED, queue/packet alanları ve retained weak pair kurulur. SDK OVERLAPPED ile mantıksal ASIO metadata ayrı tutulur. |
| `021db3..021e39` | İlk buffer pointer/full64 length ve tanımlı handler alanları kopyalanır; handler+20 byte ile +24 DWORD ayrıdır. Owned pair +40/+48 kaynağından operation+a0/+a8 alanlarına taşınır, kaynak sıfırlanır. |
| `021e4e` | Gerçek `023dd0` constructor çağrısı. Yeni köprü allocator kimliğini canlı kaynak binding'i ile eşler; eksik/çift eşleşme model sınırı hatasıdır. |
| `021e70..021e9f` | Buffer ve socket state constructor/weak cleanup sonrasında yeniden okunur. Operation ilk görüntüyü tutar; SDK yeniden okunan buffer'ın düşük 32 bit uzunluğunu kullanır. Zero shortcut full64 boyuta bakar. |
| `021ea5 / 021eca` | Her normal başlatma dalında bir work-count increment; unsigned32 taşması korunur. |
| `021ed1..021ee6` | Invalid socket için `10009`, sıfır byte ile gerçek `028b50` publication. State bit10 ve full64 length0 shortcut'ı socket geçerliliğinden önce gelir ve success0 yayınlar. |
| `021f34 / 021f3c` | Tam SDK `WSARecv` imzası; bir buffer, private transferred/flags DWORD, gerçek SDK OVERLAPPED ve null completion routine. Hemen ardından, başarılı sonuçta da `WSAGetLastError`. |
| `021f42..021f5e` | Receive'e özgü `64→10054`; ayrıca `1234→10061`. Adapter raw status döndürür, eşleme gövdede yapılır. |
| `021f5e..021f87` | Result0 veya pending997 için gerçek `028c00`; diğer durumlarda mapped error/transferred DWORD ile gerçek `028b50`. Early completion handshake ve post başarısızlığındaki queue fallback mevcut kaynaktan gelir. |
| `021fa1` | Null operation/allocation içeren normal tail cleanup: gerçek `0293f0`. Publication başarılıysa operation ömrü bitmiş olabilir; sonraki operation erişimi eklenmez. |

Adresler anadius64 görüntüsüne aittir; tabloda ortak `0x180` öneki kısaltılmıştır. Callback kimlikleri veri token'ıdır; görüntü adresleri çağrılmaz.

## Testler ve kapsam

**691 çekirdek + 1.270 entegrasyon = 1.961 kontrol PASS.** Çekirdek testler açık WinSock senaryoları, full64/low32 sınırları, callback ile değişen canlı alanlar, weak taşması/delete, owned handler aktarımı, allocation/cache, early completion/fallback ve gerçek operation ömrünün bitmesini kapsar. SDK adapter testlerinde iki import da enjekte edilir; gerçek socket/ağ API'si çağrılmaz.

Entegrasyon testleri yeni `021d10` ile gerçek `023dd0` empty/factory dallarını, IO dispatcher yönlendirmesini ve `027710 → 021a40` completion kaynaklarını çalıştırır. EOF, message-size normalization, error mapping, strong/weak cleanup ve storage cache/free doğrulanır. Son `025c40` sağlayıcısı kontrollü gözlem sınırıdır. Tam worker döngüsü bu yeni entegrasyon testinin kapsamı değildir; mevcut worker/dispatcher regresyonları ortak build içinde ayrıca çalışır.

Kontrolleri aynı ajan hazırlayıp kaynakla karşılaştırdı; bu aşama için ayrı bir bağımsız reviewer onayı iddia edilmez. Beklenen exception testleri yalnız tamamlanan normal prefix'i sınar; native SEH/unwind/rollback davranışı kurulmuş sayılmaz.

## Açık kalanlar

`010040/008350` tampon sözleşmesi, `025c40` composed receive, `025480/0280d0` native frame bağları ve native allocator/factory/query/move/destroy/RTTI/category/global/ABI sahipliği açık. CRT üretim kaynakları değiştirilmedi ve beklemede. Özgün DLL/oyun yüklenmedi veya çalıştırılmadı; gerçek CFG/token/achievement dosyaları okunmadı. Bu bir tam DLL veya native STL/IOCP/concurrency/SEH eşdeğerliği kanıtı değildir.

Toplam DLL için doğrulanmış tamamlanma yüzdesi yoktur. Somut artış: bir normal fonksiyon gövdesi, SDK adapter'i ve constructor bağlantısı. Sıradaki paket R1 tampon boyut/büyüme sözleşmesidir.
