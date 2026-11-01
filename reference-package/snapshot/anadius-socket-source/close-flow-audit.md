# Socket close/cancel: bağımsız statik akış incelemesi

Kapsam: `0x180026e50` kapatma ve `0x180026fb0` iptal/kapatma/reset normal akışları. Yalnız retained REA pseudocode/assembly incelendi; özgün DLL/oyun, gerçek socket/ağ veya kullanıcı CFG/XML/INI içeriği çalıştırılmadı/okunmadı.

Hedef DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Retained listing hash'leri: pseudocode `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; assembly `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Yordam | Pseudocode Evidence | Assembly Evidence |
| --- | --- | --- |
| `0x180026e50` | `ev_e5ca5d521290c32a902e6ea9d557bdbb884ee65f15251bad3fcda5c36edc21e6` | `ev_5ba7cb4cd92f8a6b1a046387bb51dc6220269ea452ead106f538b767ff1fe6a7` |
| `0x180026fb0` | `ev_4ffe8b68e27a849ee81d94fa3520491aa59f139532cc5e31294d1bee89c8c4b4` | `ev_949e26c04afda34fe45f482346b1b127456e1debcf6ba15190922608977a89d9` |

## Kapatma 026e50

- `026e69/6d` invalid SOCKET tüm bitleri1 ise `026f9c` mantıksal0 döner. State ve error alanı yazılmaz, API veya kategori getter çağrılmaz.
- Socket girişte RBP'ye saklanır (`026e66`); tüm option/close/ioctl denemeleri bu aynı socket'i kullanır. Caller slotu bu yordam tarafından invalid yapılmaz.
- Destroying argümanı nonzero ve canlı state&8 nonzero ise `026e87/8d` state'e tekrar bit8 OR yazılır; `026ea6` setsockopt(SOL_SOCKET=65535,SO_LINGER=128,int0,size4) çağrılır. Option sonucu nonzero ise yalnız category028620→WSAGetLastError (`026eb0/b5`) çağrılır ve sonuçları atılır. Error çıktı alanına bu option hatası atanmaz.
- `026ebe` ilk closesocket çağrısıdır. Status0 ise yalnız error.value=0 yazılır (`026ece`); category korunur. Herhangi nonzero status failure yoludur, yalnız−1 değildir; dönüşte ham status korunur.
- Failure: category (`026edb`)→WSAGetLastError (`026ee3`), sonra error.value/category alanlarına yayın (`026ef7`). Native 16 bayt error_code kopyasının padding'i başlatılmış bir değer değildir; typed model bunu ABI eşitliği olarak sunamaz.
- `026efa` kategori getter'ından SONRA caller error.category ve error.value yeniden okunur. Kategori nesnesi pointer'ı değil, vtable+8 name function kimliği karşılaştırılır. Kimlik+10035 eşleşirse retry. Birleşik koşul başarısızsa ikinci getter `026f14` ve kimlik+1237 denenir. İkinci dalda10035 eşleşmesi retry değildir; ilk dalda1237 tek başına yeterli değildir. Getter çağrıları saf kabul edilmez.
- Retry: `026f40` ioctlsocket(cached socket,0x8004667e,&DWORD0); sonucu tamamen yok sayılır. SONRA canlı state `&=0xfc` (`026f46`); bu, ioctl sırasında state değişmişse yeni değerden bit0/1'i temizler. `026f4d` ikinci closesocket aynı cached socket'i kullanır.
- İkinci status0 da yalnız error.value=0 yapar, category korunur (`026f59`). İkinci nonzero status category→WSAGetLastError ile error'ı yeniden yazar (`026f79/81/95`); yeni retry döngüsü yoktur. Return ham ikinci status'tür (`026f98`); ioctl veya LastError sonucu değildir.
- WSA error64/1234 remap, sıfır hata için sentetik10022, sınırsız close döngüsü veya ilk baştan state bit0/1 temizliği eklenmemelidir.

## İptal/kapatma/reset 026fb0

- `026fd0/fd4` giriş socket'i invalid ise atomic engine erişimi, gate, cancellation ve026e50 atlanır. Buna rağmen aşağıdaki socket/state/resource/control sıfırlama ve son release yolu çalışır.
- Engine slotu param1+0x10 üzerinde `CMPXCHG.LOCK` (`026fdc`), expected0 ve desired0 kullanır. Bu sıradan load veya exchange(NULL) değildir. Nonnull engine slotu değişmeden kalır ve önceki pointer RBP'ye saklanır (`026fe2`); NULL slot ise tekrarNULL yazılır. Typed model gerçek `std::atomic<Engine*>` üzerinde expected=null/desire=null compare_exchange kullanabilir; native offset/ABI eşitliği ayrı sınırdır.
- Nonnull engine için canlı socket `026fe7` noktasında, engine gate'ine girmeden ÖNCE cache edilir. Engine gate+30'a enter `026ff6`; category028620 `027001`; error995/category hazırlanır; `027029` required026aa0(engine,cached socket,error995) çağrısı; gate leave `027032`.
- Cancellation sırasında object.socket veya engine slotu değişse bile cancel çağrısı cached engine/socket'i kullanır. Buna karşılık gate leave SONRASI close için object.socket yeniden okunur (`027058`). `026e50` çağrısının destroying argümanı1'dir (`027052`). Local close error başlangıçta value0/generic category'dir; hem status hem error sonucu bu caller'da atılır.
- `02706d` socket=-1, `027070` state0, `027073` pair.resource0. Eski control ancak bundan SONRA okunur (`027077`); pair.controlNULL daha sonra, release başlamadan önce yazılır (`02707b`). Close/cancel callback'lerinin değiştirdiği geç control değeri kullanılır.
- `027086` strong atomikfetch_sub: eski değer1 ise resource destruction (`027096`), ardından canlı weak fetch_sub (`027098`); weak eski değer1 ise control delete (`0270a8`). Strong eski değer1 değilse weak değişmez. Control slotu ve resource callback başlamadan temizdir. Callback tekrar pair atarsa source bunu sonradan yeniden silmez; weak değeri de callback'ten sonra okunur.

## Sınırlar

`026aa0` kendi hash-map/operation queue taraması, completion posting ve fallback wake mekanizmasını içerir. Bu aşamada onun içini yeniden kurmadan required cancellation effect olarak bırakmak normal caller akışını korur; gerçek ASIO cancellation/IOCP tamamlandı anlamına gelmez. `024db0` bu gövdeden sonra listeden çıkarma ve executor cleanup uygular; yeni close gövdesinin gerçek LifecycleEffects adapter'ına bağlanması bu sınırı daraltır, tüm servis ömrünü çözmez.

Category singleton/CRT guard yan etkileri, native allocation/global/metadata belleği, error padding, SEH/unwind ve genel concurrency/runtime eşitliği açık kalır. Gate/cancel effect'in exception durumundaki native unwind cleanup'ı bu normal-flow listing'den tamamlanmış sayılmaz. SDK importları enjekte testlerle doğrulansa bile gerçek socket/ağ yürütüldüğü iddia edilemez.
