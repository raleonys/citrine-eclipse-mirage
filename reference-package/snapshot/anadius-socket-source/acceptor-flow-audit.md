# Acceptor ve socket kapatma: bağımsız statik akış incelemesi

Kapsam: `0x180020e20` acceptor constructor ve `0x180025310` socket kapatma yardımcısının retained REA pseudocode/assembly incelemesi. Bu rapor kaynak incelemesinden ayrıdır. Özgün DLL/oyun, gerçek socket/ağ/WSAStartup yürütülmedi; kullanıcı cfg/token/XML/INI içeriği okunmadı.

Hedef DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Retained pseudocode SHA-256: `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; assembly SHA-256: `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Yordam | Pseudocode Evidence | Assembly Evidence |
| --- | --- | --- |
| `0x180020e20` | `ev_3c0f133878db1fec30ab6ef19ef7b2595fed8a0ba99ea8e0ec75a708ab5925af` | `ev_ff338f1411812fb5f0cbb9ad6393b0698c2c0a40146ad6aab6f4aaeffaf2bfea` |
| `0x180025310` | `ev_26632449c219b976e8c944c3f39220e450cf9f69c0bb73f371721826920c0fb4` | `ev_2645e8dfee6bdef124f50586c9504ff413676b420d87ddf68f69b549a20cc93c` |
| `0x1800211b0` | `ev_0a99c668ad187f12ba22bcb5a084965bb3619bfa398a64dd105f65647c8a7c33` | `ev_632ecefa1757518f06b13606a94918118593165c553357228e6939dd09997b9d` |
| `0x1800237f0` | `ev_9d032f90b7525b7ede209b0cd270417daa9266d8e3a7a8c00e1c70cc49a616d6` | `ev_de485f776e49d7ff2d789f8827fe4b134f48db3815f521d6d4cde097753b314c` |

## Acceptor 020e20

- `020e53` önce `0211b0` initializer'ı çağırır. `020e59` endpoint family'yi sonra okur; family yalnız 2 ise socket family2, diğer bütün değerlerde23 seçilir (`020e5d..020e6c`). Object'ın service pointer'ı `020e70`'de yerel R13'e alınır.
- `020e73..020e96` initializer sonrası object socket hâlâ invalid değilse `0285b0` misc kategori + hata1 ile `028240` raise yoluna gider. Faithful `0211b0` socket'i invalid yapar; dal native gövdede yine de vardır.
- `020eb6` WSASocketW seçilen family, type1, protocol6, protocol-info null, group0, flags1 alır. Native önce WSASetLastError çağırmaz. Başarısız socket için `020efe` kategori getter'ı, sonra `020f06` WSAGetLastError çalışır.
- Başarılı socket ve cached endpoint family!=2 ise `020ef3` setsockopt(socket,41,27,int0,4) çağrılır. Return/LastError tamamen yok sayılır. Bu çağrıya göre hata yükseltilmez.
- Geçici socket invalid ise `020f37` kapatma yardımcısı yine çağrılır; helper invalid için hiçbir API çalıştırmaz. Yakalanan WSA error nonzero ise başlangıç/open raise'a gider; error0 ise object socket invalid kalır, transfer/shared-reset yapılmaz, fakat protocol alanları resetlenir (`020f45`→`020fd3`). Sonraki option veya bind invalid kontrolü10009 üretir.
- Geçici socket valid ise `020f5d` CreateIoCompletionPort(socket, service->scheduler->handle,0,0) çağrılır. Sonuç null olduğunda **önce GetLastError** (`020f68`), **sonra kategori getter'ı** (`020f70`) çağrılır. WSA hata sırasıyla karıştırılmamalıdır. Captured error nonzero ise helper ile geçici socket kapatılır (`020f8d`), ardından başlangıç/open raise vardır. Helper'ın sonraki LastError yan etkileri captured hata değerini değiştirmez. Null sonuç + captured error0 başarı yoluna devam eder.
- Transfer yolu `020f97` geçici guard'ı invalid yapar, `020f9f` object socket'e valid handle yazar, `020fa3` state byte'ını0x10 yapar, sonra `020fb2` shared pair+0x18 için `0237f0(null)` çağırır. Bu çağrı sadece iki pointer'ı sıfırlamak değildir: yeni24bayt ref-count-resource kontrol bloğu allocation ve eski shared pair refcount/lifetime işlemleri içerir. Provider olarak kalırsa kaynak bu işi yeni kurtarılmış saymamalıdır.
- `020fd3..020ffc` reset alanları: dword+0x40=seçilmiş socket family; byte+0x44=0; qword+0x48=2 (**family23 olsa bile sabit2**); qword+0x50=0; qword+0x58=0; dword+0x60=0. State0x10 ve shared-reset yalnız transfer yolundadır; WSASocket invalid/error0 yolunda bu iki işlem yoktur.
- Reuse byte nonzero ise setsockopt(socket,0xffff,4,int1,4) çağrılır (`02103c`). Öncesinde object socket yeniden okunur; invalid ise category+10009 ve set_option raise. Nonzero API status'ta kategori getter (`021046`) → WSAGetLastError (`02104e`) vardır; captured error0 ise hata yükseltilmez ve bind'e devam edilir.
- Bind öncesi socket yeniden okunur; invalid category+10009/bind raise. `021085` endpoint family'yi **yeniden** okur: family2 için16, diğerleri için28 bayt sockaddr. Bind (`021091`) nonzero status'ta category→WSAGetLastError; captured0 ise listen'e devam edilir.
- Listen öncesi socket yeniden okunur; invalid category+10009/listen raise. Listen (`0210d4`) backlog0x7fffffff alır. Nonzero status'ta category→WSAGetLastError; captured0 ise normal return yapılır. Başarılı reuse/bind/listen için category/get-error çağrısı eklenmemelidir.
- Raise bağlamları assembly'de başlangıç `DAT_180084728`, set_option `DAT_180084730`, bind `DAT_180084604`, listen `DAT_18008473c` adresleridir. Retained defined-string inventory `ev_c97664560efc0e5cf3207e174b0defd0b96f84c49fdf20bd0644ce960d07d716`, 084730=`set_option` ve08473c=`listen` metinlerini doğrular. 084728/084604 için bu inventory içinde exact kayıt yoktur; inventory yokluğu metin yokluğu sayılmaz. Bağımsız read-only PE section mapping ve baseline DLL hash doğrulaması ayrıca `work/acceptor-context-strings.json` kaydını doğruladı: 084728/raw82f28=`6F70656E00` (`open`),084604/raw82e04=`62696E6400` (`bind`);084730/raw82f30=`7365745F6F7074696F6E00`,08473c/raw82f3c=`6C697374656E00`. Bu ham PE okuması REA string Evidence diye sunulmaz; kayıt SHA-256=`24038d03d8ad7eb090084acb01e04efe5c5d3d4480c8c58a3e6dedfb77d1f704`.
- Geçici socket IOCP failure sırasında açıkça helper ile kapatılır. Socket transfer edildikten sonraki raise yollarında bu gövdede explicit helper çağrısı yoktur; MSVC EH/destructor/unwind temizliği ayrı kapsamdır. Kaynak kendi RAII cleanup'ını ekliyorsa bunu native normal akışın gözlemi diye sunmamalıdır.

## Socket kapatma 025310

- `025316` input slot'tan SOCKET'i bir kez yerel RBX'e alır. Slot sonradan değişse de kullanılan handle bu cached değerdir. Native gövde input slot'u invalid yapmaz.
- Invalid SOCKET bütün64bit1 ise return; close/category/GetLastError/ioctl yoktur. Diğer socket için closesocket (`025326`) çalışır; return0 ise doğrudan return. Herhangi nonzero return failure dalıdır.
- Failure sırası `02533e` kategori getter → `025346` WSAGetLastError → `02534e` kategori getter. Kimlik karşılaştırması object pointer'ı değil +8 slot kimliğidir (`025353..025357`). İlk kimlik ve error10035 birlikte eşleşirse retry yoluna geçilir. Değilse getter `025365`'te ikinci karşılaştırma için yeniden çağrılır; kimlik+1237 eşleşmelidir. İlk getter/LastError çağrısının yan etkileri yok sayılmamalıdır.
- Retry yolunda DWORD0 değeriyle ioctlsocket(socket,0x8004667e,&zero) (`025391`) çağrılır; ioctl dönüşü ve LastError okunmaz. Ardından aynı socket'e closesocket ikinci kez çağrılır (`02539a`). İkinci close failure ise kategori getter (`0253a4`) → WSAGetLastError (`0253a9`) çalışır ve sonuç atılır. Üçüncü close yoktur; dışarıya hata return edilmez.

## Kapsam ve bilinmeyenler

İki normal akışın kaynaklaştırılması `0211b0` service acquisition/intrusive liste/critical section kurulumu, `0237f0` allocator/sharedref ömrü, `028620/0285b0` CRT lazy kategori singleton'ları, `028240` native exception ABI'si, ASIO/IOCP scheduler, gerçek socket sahipliği ve concurrency, MSVC padding/stack/SEH/unwind veya tüm DLL eşitliği değildir. Typed adapter'ın sockaddr boyutunu/initialized-memory alanını sınırlaması seçilmiş bir güvenli domain'dir; özgün koda eklenen hata doğrulamasını özgün davranış diye sunmamalıdır. Enjekte API testleri gerçek ağ yürütmesi değildir.
