# B28 — Mesaj zincirindeki gerçek boşluk denetimi

**Seçim: `0x180023000` native accepted-session/shared-owner constructor.** Kaynak envanterinde yalnız kayıt var; tanım ve derlenmiş sembol yok. Bu, mevcut bir adapterin çevresine yeni kontrol eklemek değil, uygulamanın eksik sahiplik/durum kurma algoritmasıdır.

B27 ortak build PASS;834 kaynak/245 nesne/285 grup/19 framework terminal senaryo/52 native kapanış. Bu denetim bütün834 kaynak hash’ini frozen snapshot ile karşılaştırdı. Üretim/test dosyası değiştirilmedi; yeni binary sorgusu, original byte okuması veya hedef yürütmesi yok.

## Yeniden eksik sayılmaması gerekenler

`025c40→025480/0280d0→028d30→033d70→032d00→028270→02a500→0261d0→025a70` için gerçek kaynaklar ve final derived adapterler zaten bulunuyor. Eski README paragraflarındaki mandatory ifadeler sonraki N/R paketlerinden önceki tarihsel durumu anlatır. Handler registry39 isim/33 unique hedef içerir; hepsinin kaynak tanımı vardır.31’i canonical envanterde;02d720 ve0314a0 bağımsız disassembly ile kaynakta bulunan outside-canonical LAB gövdeleridir ve2076 paydasına eklenmez. `MessageHandler`, `Router`, gerçek tinyxml2 compatibility reader ve `SessionCipher` de mevcut; bunlar test fixture’ı değildir. Native ABI/full parser eşitliği ayrıca açıktır.

## Eksik üretim girişi

`SourceBackend::Resources.server` hâlâ abstract `ServerBackend` ister. Üretimde `AddressBackend/AcceptorBackend` ve `SynchronousTransport` kısmi alt sınıflardır; async transport/scheduling/session/acceptance bütününü kurmazlar. Tam sunucu sınıfları fixture’larda bulunur. `accept_completion_025720`, dışarıda hazırlanmış `Session*` alarak asıl yeni-session/shared-owner oluşturma işini atlar. Semantic `Session` constructor yalnız ilk handler reset’ini yapar.

`023000` retained assembly:02301b–023054 allocation0x1c8 + strong/weak1 + vtable084750;023057–023086 iki023d20 socket move;02308f–0230d8 owner+a8 input string, owner+c8 deque/proxy;0230db–0230eb owner+f0 gerçek cipher reset + moved temporary socket destructor;0230f5–023188 sonuç pair’i ve weak-self atomic yaşam süresi. Bunlar mevcut read/message/response kaynaklarının ihtiyaç duyduğu aynı owner alanlarıdır.

Pseudocode Evidence: `ev_1cbac20a9d856e56adbd7a8a140ddc1dc8071cc859ea7bc052cd5ba9b24a722f`.

Assembly Evidence: `ev_2c0f5d78836aff0cdce58b5618135e67ae13fbb3dff7606435c88f2a6b37d575`.

`023d20` socket-move gövdesi de eşlenmemiştir; birlikte kurtarılmalı veya açık mandatory etki tutulmalıdır. `024db0` destructor ve `02bc70` cipher reset kaynakları yeniden kullanılabilir. Allocator/throw/native shared_ptr ABI’si tamamlanmış gibi gösterilmemelidir.

Sonraki somut işler: native025720 ownership/greeting/release bağlama; sonra eksik021b00 AcceptEx initiation ve0271a0 completion. Tek başına0270e0 küçük wrapper’ını yapmak kabul edilen oturumu üretmez. Native registry/DOM/generic virtual parser ayrı boşluklardır; çalışan semantic reader/router yeniden yazılacak eksik app algoritması değildir. Numeric0033c0 ve genel CRT bekler. Öncelik1→2→3→5→4.

Adres, Evidence ID, tüm kaynak tanımları, gerçek dosya/satır/hash referansları JSON dosyasındadır. Denetimin kendisi yeni native gövde/kapanış eklemez ve toplam DLL yüzdesini artırmaz.
