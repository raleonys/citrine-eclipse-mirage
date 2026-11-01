# N20 — Üç pool block-clear normal kaynak gövdesi

25 Ekim 2026. `pool_clear.hpp/cpp`,002d70/002dc0/002e10 için `clear_pool_blocks_*` normal gövdelerini ve `RecoveredPoolClearEffects::clear_pool_*` final kaynak bağlantılarını ekler. Üç bağlantı existing `NativeDocumentEffects` sağlayıcılarını kapatır. Her native hedefte normal+final iki tagged kaynak tanımı vardır; altı native fonksiyon değildir.002d70 iki embedded pool tarafından kullanılır; bu kullanım üç normal gövde/üç provider closure sayısını dört yapmaz. Historical source closure34→37;2 semantic session adapter ve1 ayrı allocation dispatcher değişmeden korunur.

## Kimlik ve retained kanıt

REA6.5.0 server/catalog aligned ve Ghidra kullanılabilir checkpoint'i `pool-clear-rea-session.json` içindedir; hedef açık değildir. Bu aşamada yeni native import veya data read yapılmaz. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Tam retained listing kimlikleri ve adresli sidecar'lar `pool-clear-evidence.json` içinde hash'lenir.

| Yordam | Caller EDX static hint | Assembly Evidence | Pseudocode Evidence |
|---|---:|---|---|
|0x180002d70|4080|`ev_06bc5dbb4ccc0a7c9ef5555e3b2c6beb617304d7002256a26cc3451fe1233c49`|`ev_4e130f68563bb04de14fda1c456fd436f3dacd767dd331b62bdd0b7ec1bc4588`|
|0x180002dc0|4056|`ev_942cee8f39a202985f358368870006d179df8d801e7c5ee6524b718b6cf62afb`|`ev_360a8ebddaae1ee10f83511de53328b963e4dbf5369444c578f3841fda48606d`|
|0x180002e10|4032|`ev_ffc8268becf2220ca2eeb649c6420381de74df66a6cf841c3f750d90fa5692ac`|`ev_d3753f429cb0573b35a6d038b49b7e94ccd954fe6542385b4948667e48fd523a`|

Dört full pool destructor001bb0/001c30/001cb0/001d30 ile önceki document constructor001830/parse004280/destructor001e00 context-only olarak saklanır; ek normal gövde claimed değildir. Önceki complete `pool-allocation-simd-data.json` ve `node-delete-next-vtables.json` byte olarak aynen kullanılır; yeni data-read iddiası yoktur. Bu kanıtlar initialized analysis memory'ye aittir; live runtime pointer/CPU gözlemi değildir.

## Gözlenen normal sıra ve alias sınırları

Her clear helper girişte raw DWORD pool+64 **nonzero** test eder. Zero count döngüyü atlar; negatif count boş kabul edilmez. Her iterasyonda live count tekrar okunur,eski count signed64 genişletilip cache edilir; old-1 raw32 wrapping ile pool+64'e **önce** yazılır. Ardından live data pointer pool+8 ve `data+sign_extended_old_count*8-8` slotundaki block pointer okunur. Slot eski cache edilmiş signed index'i kullanır; sonradan yazılan veya alias nedeniyle değişen count'u kullanmaz.

Lower04b0f8(block) çağrısı ardından live count tekrar nonzero test edilir. Lower effect count/data/slot değiştirebilir; next iteration bu değişikliği görür. Positive-count guard,hard iteration limit,ownership test veya rollback eklenmez. Negatif old=-1 örneğinde slot data-16 ve yeni count=-2 olur. Bounded mapping eksik/ambiguous ise UnsupportedInput mevcut committed prefix'i korur; native fault/unwind davranışı ileri sürülmez.

Döngü tamamlanınca pool+68,+70,+78'e bu sırada **üç QWORD zero store** yapılır. Bu stores root,current/total ve max/untracked çiftlerini temizler. Altı ayrı DWORD store'a dönüştürülmez; erişim genişliği/failure granularity korunur. Count+64 için ayrı bir son zero store yoktur. Slot mapping başarısızlığı öncesi count decrement committed kalır; lower free başarısızlığı final resets'e ulaşmaz; bir reset failure daha önceki QWORD reset'lerini geri almaz.

Clear vptr,data pointer,capacity DWORD+60,inline byte'lar ve stale array entries'yi korur. Pointer-array allocation'ını free etmez,data'yı inline'a döndürmez,vtable kurmaz,node destructor çağırmaz. Clear sonrasında aynı array storage yeniden kullanılabilir; full pool destruction ayrı aşamadır.

Caller EDX4080/4056/4032 değerleri static callsite hint olarak raporlanır. Existing `NativeDocumentEffects::free_04b0f8` interface'i **pointer-only** kalır. Bu hint'ler yeni sizedfree ABI,native allocator ownership veya recovered CRT free implementation kanıtı değildir.04b0f8 lower free mandatory kalır; eski block04b0bc allocation/0793d0 fill ve array04b100 allocation/078d30 copy etkileri de önceki katmanlarda mandatory'dir. Yeni CRT/terminal sağlayıcı eklenmez.

## Document bağlantısı ve doğrulama

Existing004280 parse-error yolu delete_nodes003110 sonrası002d70(document+108),002d70(document+188),002e10(document+208),002dc0(document+288) çağırır; ardından live error DWORD+6c tekrar okunur. Böylece üç final kaynak bağlantısı dört embedded pool temizliğinde kullanılır. Bu direct source zinciri native parser/DOM materialization veya full document/pool lifecycle eşdeğerliği değildir. Existing001e00 document destructor dört full pool destructor sağlayıcısını ters embedded sırada çağırır; bunlar N20'de recovered sayılmaz.

117 çekirdek +13.024 entegrasyon =13.141 yeni kontrol PASS. Authoritative sonuçlar ortak build `socket_pool_clear` / `socket_pool_clear_integration` gruplarıdır; standalone modül scripti çalıştırılmış sayılmaz. Controlled memory/free effects ile clear sırası ve bağlı kaynak zinciri sınanır. Parser/error projection kullanıldığı yerde parser lower fixture'dır; tam native parser/message/DOM/heap/fault/EH kanıtı değildir. Bağımsız bounded static source review üretim cpp/hpp hash'lerine bağlıdır; full native equivalence review false kalır.

Ortak build201 üretim nesnesi(197 C++/4 C),215 normal +19 terminal grup,88 socket grubu. Önceki641 kaynak/CRT byte olarak korunur;646 current source snapshot girdisi vardır.44 validation mirror çifti=43 historical+1current;86 historical config referansı korunur.121 dependency hash;37 historical typed source closure,2 semantic session adapter,1 ayrı allocation adapter vardır. Bunlar DLL tamamlanma yüzdesi değildir. Full DLL tamamlanmadı; toplam yüzde ölçülmedi.

Original DLL/oyun/network/IOCP yürütülmez;gerçek CFG/token/achievement içeriği okunmaz. Hash-only input integrity ayrıdır;bilinen engelli version.dll/generated version DLL tekrar denenmez. Genel CRT unchanged/paused,aşama5'e kadar bekler. Pointer-only free,full pool destructor,node virtual/parser/DOM/base destructor/native lifecycle/ABI/EH/concurrency açık kalır.

Sıradaki N21:dört pool destructor001bb0/001c30/001cb0/001d30 inventory/contracts. Bu hazırlık henüz yeni normal gövde iddiası değildir. Öncelik1→2→3→5→4 korunur.
