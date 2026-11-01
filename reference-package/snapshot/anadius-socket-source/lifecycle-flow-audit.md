# Socket nesnesi ömrü: bağımsız statik akış incelemesi

Kapsam: `0x1800211b0` constructor, `0x1800237f0` shared resource/owner değişimi ve `0x180024db0` destructor normal akışları. Retained REA Ghidra pseudocode ve assembly birlikte okundu. Bu rapor yeni kaynak kodun incelemesi değildir. Özgün DLL/EXE, ağ, socket veya gerçek kullanıcı CFG/XML/INI dosyaları çalıştırılmadı/okunmadı.

Hedef DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Retained tam liste hash'leri: `anadius-all-functions.pseudo.c` = `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; `anadius-all-functions.asm.txt` = `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Yordam | Pseudocode Evidence | Assembly Evidence |
| --- | --- | --- |
| `0x1800211b0` | `ev_0a99c668ad187f12ba22bcb5a084965bb3619bfa398a64dd105f65647c8a7c33` | `ev_632ecefa1757518f06b13606a94918118593165c553357228e6939dd09997b9d` |
| `0x1800237f0` | `ev_9d032f90b7525b7ede209b0cd270417daa9266d8e3a7a8c00e1c70cc49a616d6` | `ev_de485f776e49d7ff2d789f8827fe4b134f48db3815f521d6d4cde097753b314c` |
| `0x180024db0` | `ev_26181ed4d015f17fcc8c6e68ec415c48aa1ddc0fca6cc7e7870406406e38f9d3` | `ev_8f785f4afe16ef05caf020d8ab2bbdb264eebef7c56405097cc4dfec666d8357` |

## Constructor 0211b0

`0211c8..0211d1` context'in ilk pointer'ı ile context'in kendisini zorunlu service resolver `0239c0`'a geçirir. Dönen service `0211d6` nesne+0'a yazılır. Kaynak service lookup/RTTI/lazy creation içini yeniden yazmış sayılmamalıdır.

`0211e2/eb`, shared pair nesne+18/+20 alanlarını önce sıfırlar. Protokol+40=2, marker+44=0; +48=2, +50/+58=0, +60 dword=0 (`0211f2..02120f`). `02121a` +88'e `0x180084908`, `02122c` +78'e `0x180084928`, `021237` +90'a `0x180084860` kimliklerini yazar. +68 context, +74 dword0, +80 nesnenin kendi+68 adresidir. +70 dört bayt padding/diğer veri bu kesitte yazılmaz; mantıksal model native padding/offset ABI'si değildir. Bu kimlikler typed kaynakta çalıştırılmayan metadata olabilir; image adresleri fonksiyon pointer'ına çevrilmemelidir.

`02123f`, service'i nesne+0'dan **yeniden okur**, sonra bu kimliği gate işlemleri boyunca saklar. `021242/45` socket=-1 ve state=0 yapar. `021249`, owner+20'yi yükler; fakat bu alan `0211eb` ile önceden0 olduğundan tanımlı tek-thread constructor yolunda release kolu çalışmaz. `02124d/51` pair'i yeniden0 yapar. Bu yüzden constructor, çağrıdan önce logical object'te bulunan owner'ı release etmemelidir. Native aynı storage'a geçersiz yeniden construction/concurrent data race davranışı kanıtlanmış değildir.

`021281/85` aynı cached service+50 gate'e girer. `02128b/8f` head'i node.next'e yazar; node.prev=0 (`021293`). Head tekrar okunur (`021297`); doluysa eski head.prev=newnode (`0212a0`). Sonra service.head=newnode ve Leave (`0212a8/ac`). Native head node nesne+8'dir; typed `SocketObject*` bu kimliğin mantıksal temsilidir, byte overlay değildir.

## Shared owner değişimi 0237f0

`023800/805` allocator'a tam24 (`0x18`) byte isteği verir. Caller akışındaki NULL dalı korunur. Nonnull sonuç `02381e..023832` ile başlangıç state'ini kurar: strong=1, weak=1, vtable `0x180084820` (noop_deleter resource control), resource=girdi. Partial construction/vptr sıfırlama zamanlaması typed C++ ABI'si olarak sunulamaz.

`023836` pair.resource=girdi yapar. **Ardından** `023839` pair.owner'dan eski control'ü yükler; `02383d` yeni control (NULL olabilir) owner alanına atanır. Allocation/new-handler sırasında reentrant değişen pair.owner snapshot'ı çağrının başında alınmamalıdır. Eski owner'ın destruction callback'i çalıştığında yeni resource ve yeni owner zaten yayımlanmıştır. Callback'in yaptığı pair değişiklikleri sonradan tekrar eski/new değerle ezilmez.

Eski control nonnull ise `02384d` strong atomik decrement yapar. **Önceki** strong değer1 ise slot0 resource-destroy çağrılır (`02385d`); sonra weak atomik decrement (`02385f`) yapılır. Weak'in **önceki** değeri1 ise slot+8 delete çağrılır (`02386f`). Strong eski değeri1 değilse weak'e dokunulmaz. Callback sonrası weak canlı okunur; resource callback'i weak değerini değiştirebilir. Callback sonrasında aynı control kimliği kullanılır; pair.owner tekrar okunmaz. Typed atomik 32bit işlemler bu sıralamayı korumalı; yeni saturation/underflow guard eklenmemelidir.

Önemli allocator sınırı: retained `04b0bc` normal gövdesi `_malloc_base` ve `0634b0` new-handler retry'ından sonra `047f80`/`00cd30` throw'a gider; bu allocator'ın gerçek exhaustion yolunun NULL döndürdüğü gösterilmemiştir. Caller'daki NULL branch ayrı statik gözlemdir. Seçilen `new(std::nothrow)` allocator, bu CRT/new-handler/throw semantiğinin yeniden kurulması değildir.

## Destructor 024db0

`024dc7`, service'i **cleanup çağrısından önce** cache'ler. `024dd5`, cached service+28 ve socket node nesne+8 ile zorunlu `026fb0` close/cancel helper'ını çağırır. Bu helper ayrıca socket/state/shared-pair reset/release içerir; body burada yeniden yazılmış sayılmaz. Callback nesnenin service alanını değiştirse de list/gate için ilk cached service kullanılmalıdır.

`024dda/de` cached service+50 gate'e girer. Head bu node ise head=node.next (`024de4..dee`). Prev varsa prev.next=node.next (`024df2..dff`); next varsa next.prev=node.prev (`024e03..e10`). Sonra node.next ve node.prev0 (`024e16/1a`) ve Leave (`024e22`). Links/gate kapsamı cleanup'tan sonra, executor callback'inden öncedir. Otomatik C++ object destructor/list normalization bu explicit akışın yerine geçirilmemelidir.

`024e28`, executor subobject nesne+68'i RCX'e koyar. +78'deki callback-table slot0 çağrılır (`024e2c..e33`). Ancak **bu callback'ten sonra** `024e36` pair.owner yüklenir. Executor cleanup owner'ı değiştirirse yeni owner release edilir. Gate bu callback boyunca tutulmaz. Final owner release strong→resource destroy→weak→delete sırasıyla yukarıdaki kalıptır (`024e46/56/58/68`). Destructor bu son owner slotunu ayrıca NULL yapmaz; native storage daha sonra ömrünü bitirir. Typed modelin dangled owner slotuna ikinci kez release yapması geçerli kullanım değildir.

## Dayanak sınırları

- `026fb0`: pseudo `ev_4ffe8b68e27a849ee81d94fa3520491aa59f139532cc5e31294d1bee89c8c4b4`, asm `ev_949e26c04afda34fe45f482346b1b127456e1debcf6ba15190922608977a89d9`; socket cancellation/close internalleri ayrı provider sınırıdır.
- `0239c0`: pseudo `ev_c935d546ffcfd9a015759eba276b6b86808a598ce42947d5ddbe6ef939153abb`, asm `ev_245079b916f12d89c98287073bb82e89b00650e14f7fa967621e26699d2344a4`; service inventory/RTTI/creation ayrı provider sınırıdır.
- `04b0bc`: pseudo `ev_9c975496fb48d4e6fca6f8c0535c16fa44feb14e8dafa92ef6b45a98915fa0ec`, asm `ev_4b9d54867bdd71cc2cd07945df16f49f84fdecaa3c3535386e97bd20f64624ec`; allocator/new-handler/throw CRT'si açık kalır.
- Native ASIO/service layout, vtable/padding, std resource-control allocation ABI, shared_ptr/general concurrency, original CRT exception/unwind, real socket/IOCP lifecycle ve tam DLL runtime eşitliği bu akışların kaynaklaştırılmasıyla kanıtlanmaz. Kontrol blokları ve list node'ları caller-owned geçerli ömür, geçerli links ve provider sözleşmeleri gerektirir.
