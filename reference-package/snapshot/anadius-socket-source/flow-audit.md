# Socket işlemleri: bağımsız statik akış incelemesi

Kapsam: `0x180028690` adres ayrıştırma, `0x18002a200` senkron gönderim ve `0x180028620` kategori getter'ının çağrı etkisi. Yalnız retained REA pseudocode/assembly incelendi; özgün DLL/oyun çalıştırılmadı, WSAStartup/socket/network veya gerçek kullanıcı CFG/XML/INI dosyası kullanılmadı. Bu kayıt yeni üretim kodunun incelemesi değildir.

DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Retained kaynaklar: `outputs/anadius-all-functions.pseudo.c` SHA-256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; `outputs/anadius-all-functions.asm.txt` SHA-256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Yordam | Pseudocode Evidence | Assembly Evidence |
|---|---|---|
| `0x180028690` | `ev_68e5e01ef5c49226f1e664a3680d60bb8a9a73b30b25570c3b2f281991fabfed` | `ev_f5e208e60a00b9e67c6ec94288e3198044cf274f0ddb46a5dd76b7989feb14f3` |
| `0x18002a200` | `ev_f742a98635e622694b3ed7e3d308cf9cebd4433c8b26ca837d086ae0e52413eb` | `ev_f13b91d447e6018ba93152c0b2fed70292acebd91afbd2c5c5496c6cd93ffe7b` |
| `0x180028620` | `ev_fd2098bcb37a61e83ff7f76f095ba6d0fe598933164420bda64f6c6ef3920095` | `ev_73535f6dec0ad639cf35346f8021a5b39597a421eb724714a6aec9986996b9f6` |

## Adres ayrıştırma 028690

- `0286d0` önce `WSASetLastError(0)` çağırır; family geçersiz olsa da bu çağrı vardır. Family yalnız2 veya23 kabul edilir. Diğer family `0286e2..0286fd` ile10047/system hata ve düşük32bit -1 dönüşü üretir; adres/scope hedefleri yazılmaz.
- Kaynak C string'dir: `028716..02871f` ilk NUL'e kadar uzunluğu bulur ve signed32 `strlen+1` üretir. Native dinamik stack scratch kapasitesi bu sayı kadar WCHAR'dır. `028761` MultiByteToWideChar argümanları CP0, flags0, kaynak length−1, çıktı kapasitesi strlen+1'dir. Dönüş sayısı **denetlenmez**; `02877d` WSAStringToAddressW yine çağrılır. ProtocolInfo null, sockaddr scratch boyutu başlangıçta128'dir (`02870c`).
- `028783` API sonucunu saklar; `028786` kategori getter'ı, **sonra** `02878e` WSAGetLastError çağrılır. Sıra korunmalıdır. API sonucu −1 dışındaki herhangi int ise başarı dalıdır; dönüş normalleştirilmiş1 olur (`028817..02881f`).
- IPv4 başarı `0287af..0287b2` yalnız ilk4 adres baytını yazar, diğer12 ve scope değişmez. IPv6 başarı `0287dc..0287ee`16 bayt yazar; scope pointer null değilse scope da yazılır. Başarı hata değerini0 yapar (`028815`), kategori korunur.
- Başarısız IPv4 normalde adresi değiştirmez. Ancak strcmp ile literal `255.255.255.255` eşleşirse `0287cb` ilk4 baytaFFFFFFFF yazar. `0287d3` geçici olarak hatayı0 yapar, fakat `0287d5` doğrudan `0287f9`'a gider; sonuç **10022/system ve −1**'dir. Bu özel durum başarıya düzeltilmemelidir.
- Diğer başarısız durumda yakalanmış error0 ise `0287f9..02880d`10022/system atanır; error nonzero ise korunur. IPv6 failure adres/scope yazmaz. Dönen sockaddr length'i için burada yeni doğrulama yoktur; yalnız API'nin tanımlı başarılı çıktı alanları kaynak domainine dahil edilebilir.
- Native unsupported-family yolu RAX'ı tam64bit −1 yapar; desteklenen family failure EAX'tanFFFFFFFF döndürür. Mantıksal int32 API bu farkı temsil etmez; full return-register/ABI eşitliği iddia edilmemelidir. Stack boyutu taşması, conversion failure sonrası başlatılmamış wide scratch ve kısmi/uninitialized sockaddr okumaları ayrı bilinmeyen bellek domainidir. Güvenli SDK adapter bu domaini açık UnsupportedInput ile sınırlayabilir; bunu özgün hata kodu diye sunmamalıdır.

## Senkron gönderim 02a200

- Invalid SOCKET bütün pointer bitleri1 ise `02a244..02a25d`10009/system ve0 dönüşü; WSASend çağrılmaz. Bu kontrol empty shortcut'tan öncedir.
- `all_empty` baytı nonzero ve state&0x10 nonzero ise `02a27d..02a281` yalnız error.value=0 ve0 dönüşü vardır. Error category alanı değişmez; WSASend yoktur. Başka kombinasyonlarda buffer listesi boş olsa da API yolu çalışır.
- Her denemede transferred DWORD önce0 (`02a2d3`); WSASend socket, buffer pointer, count düşükDWORD, ham flags DWORD ve null overlapped/completion alır (`02a2b0..02a2d7`). Öncesinde WSASetLastError(0) çağrısı yoktur. Sonra **kategori getter → WSAGetLastError** (`02a2df →02a2e7`) çağrılır; status0 olsa da LastError alınır.
- Yakalanan64,10054'e;1234,10061'e dönüştürülür; her dönüşüm ayrıca kategori getter'ını çağırır (`02a305..02a346`). WSASend status0 ise hata değeri tekrar0 yapılır. Transferred DWORD signed32'e genişletilir (`02a351`); nonnegative ise bu değer int64 olarak döner. Sign bit set ise0-error ile failure koluna düşer. Diğer nonzero status değerleri de −1 gibi failure koluna girer.
- Failure kolunda state&1 nonzero ise0 dönülür; select yoktur. Bu bit, yalnız would-block koşuluna değil bütün bu failure koluna uygulanır (`02a360..02a364`).
- Retry için kategori nesnesi pointer eşitliği değil, vtable slot+8 function pointer eşitliği kullanılır (`02a376`, `02a390`). İlk kategori getter sonucu ile kimlik eşleşir ve error10035 ise select'e gidilir. Bu birleşik koşul başarısızsa getter **ikinci kez** çağrılır ve kimlik+1237 denenir; eşleşmezse0 dönüşü vardır.
- `02a39e` nfds değerini socket düşük32bit+1 wrap ile oluşturur. Select'te read/except/timeout null; yalnız write fd_set, count1 ve aynı SOCKET vardır. `02a3c7` sonuç negatifse **kategori getter → WSAGetLastError** ile ham error atanır ve0 döner;64/1234 remap burada uygulanmaz. Sonuç0 dahil nonnegative ise error.value0 yapılır ve WSASend tekrar denenir (`02a3d1..02a3dd`). Yeni timeout/retry limiti veya select0 hatası eklenmemelidir.

## 025720 çağırıcısı: senkron köprü bağları

Retained assembly Evidence `ev_dd2e4ac31861fc8ef574bca994412618157ab792cafb2381c051fec67b48caaa`, pseudo Evidence `ev_3e78124546b5e0a51f195c1b0be03ae62d45ef4336e5cbc3f29792ddf071ce39`: `02574b XOR EBP,EBP` sıfırı tanımlar; send çağrısına kadar EBP yeniden yazılmaz ve `0258a4 MOV dword ptr [RSP+0x20],EBP` flags argümanını0 yapar. `0258a8` buffer count1, `0258ae` tek WSABUF pointer'ı, `0258b3` canlı `[RSI+0x20]` state baytı, `0258b7` `[RSI+0x18]` socket ve `0258bb` hedef02a200'dür. `025890 TEST R8,R8` / `025893 SETZ AL` / `0258a0` all_empty argümanını chunk length==0'dan kurar. `025850` üst chunk sınırı0x10000'dır. Bu kesit SynchronousTransport'un flags0/tekbuffer/canlısocket-state bağlarını destekler; tüm025720 lifecycle veya gönderim döngüsünün bu adapter içinde tamamlandığını göstermez.

## Kategori getter'ı 028620 ve sınırlar

028620 saf sabit getter değildir. `028624..028645`, TLS epoch+4 ile global guard0a38e8 arasında signed karşılaştırma yapar. Soğuk yolda header04b838, guard==−1 ise atexit07dff0 ve footer04b7cc çağrılır (`028653..02867b`). Aynı global kategori nesnesi0a1110 döner. Bu çağrılar LastError'ı koruyor varsayımı statik kesitten çıkmaz; socket kaynak modeli getter çağrı sırasını/effect hook'unu korumalı veya açıkça daha dar sabit-kategori uyumluluk domaini sunmalıdır.

Typed SDK adapter'da ek geçici WSABUF vektörü veya mutable wide-string kopyası kullanılırsa bunların API dönüşü ile kategori/GetLastError arasındaki destruction/tahsis etkileri ayrıca değerlendirilmelidir. Native kesitte bu noktada böyle bir allocator cleanup yoktur. API LastError değerinin adapter-owned cleanup boyunca korunması, kategori hook'unun daha sonra gerçek yan etkisini uygulamasından farklı bir sınırdır; yalnız cached error kullanmak da aradaki kategori yan etkisini silebilir. Bu ayrıntı çözülmediyse backend daha dar seçilmiş uyumluluk olarak sunulmalıdır.

Bu iki socket yordamının normal kontrol akışının kaynaklaştırılması native WSA/global başlangıç, gerçek ASIO/IOCP, socket yaşamı, native kategori singleton/CRT guard internalleri, MSVC buffer/error padding, heap/stack/allocator, SEH/unwind ve tüm DLL/runtime eşitliği değildir. Mantıksal hata değeri/kategori modeli native16bayt error_code'un başlatılmamış padding'ini yeniden kurmaz. Testlerde injected API kullanımı gerçek socket/network yürütmesi olarak sunulamaz.
