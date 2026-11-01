# N22 — Node-base destructor001ea0 normal kaynak gövdesi

25 Ekim 2026. `node_base.hpp/cpp`, `destroy_native_node_base_001ea0` normal gövdesini ve `RecoveredNodeBaseEffects::destroy_node_base_001ea0` final kaynak bağlantısını ekler. Bağlantı existing `NativeDocumentEffects` sağlayıcısını kapatır; `RecoveredPoolDestructEffects` kalıtımı önceki document/node/pool final kaynak bağlarını korur. Bir native hedefe normal+final iki tagged tanım bağlanır; iki native fonksiyon değildir. Historical source closure41→42;2 semantic session adapter ve1 ayrı allocation dispatcher aynen korunur.

## Kimlik ve retained kanıt

REA6.5.0 server/catalog aligned ve Ghidra kullanılabilir checkpoint'i `node-base-rea-session.json` içindedir; hedef açık değildir. Bu aşamada yeni native import/data read yapılmaz. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Tam retained listing hash'leri/adresli sidecar'lar `node-base-evidence.json` içindedir.

| Yordam | Rol | Assembly Evidence | Pseudocode Evidence |
|---|---|---|---|
|0x180001ea0|Yeni normal gövde|`ev_21355add25b8123d7a7643cd6b01256197966701b4c2309f7557a5cdd95d7192`|`ev_e7c4be9c1717eb0483ad0b59aaf3cbeb61047a721af9832aa8b1d49694e9dd87`|
|0x180003110|Existing direct child,context only|`ev_635328ecb5bc687965c6bb53ce282983fd334bc548208011ed74d62e645db5bf`|`ev_1309c5f52e5b3d9c95ec97c721c6a49f19fcded8db1c04d21e5faf6c6874beb4`|
|0x180001e00|Existing parent,context only|`ev_b1ea754068b65d01028de2c76b93bcfaf0dca2608322419cc6418811342f3097`|`ev_310b19c5bb059cd1c7953ed5ff55dac04b85c180edb01c590a4176041e650755`|
|0x18004b0f8|Mandatory lower boundary,no normal free recovery|`ev_c742d3bdc5602e2293140e3408473befa826c5e7f02f988ef02afb1709aa3adf`|`ev_4b68e7012c821e22c8f38d541b6e4e613a7e14ea81822e58a96efac29dd593ef`|

001ea0 gerçek CALL hedefleri003110(`001ebc`) ve04b0f8(`001f3b`) ile sınırlıdır;0046f0 çağrısı veya bilinmeyen string-helper eklenmez.001f0f internal branch'tir.07fa98 base-node identity retained LEA immediate operandıdır; table contents/virtual target'lar bu immediate'dan tek başına established değildir. Kimlik callable host function pointer'a çevrilmez. Önceki complete SIMD/vtable data evidence aynen korunur; yeni acquisition yoktur.

## Gözlenen normal sıra

Önce node+0'a QWORD07fa98 identity yazılır; sonra **existing003110(node)** gerçek kaynak sağlayıcısı çağrılır. Table publication child-delete failure öncesinde committed'dir. Parent,string flags veya storage child deletion öncesinde prefetch edilmez.003110 child virtual probe/destructor ve pool etkileri önceki explicit lower sınırlarını korur; bu child yeni normal gövde olarak tekrar sayılmaz.

003110 döndükten sonra live parent QWORD node+10 okunup cache edilir. Parent null ise own parent/sibling unlink ve node+50/+48/+10 zero stores tamamen atlanır; own previous/next alanları korunur. Flagged value-storage cleanup aşağıda yine çalışır.

Nonnull cached parent dalında önce parent+38 head okunur; node'a eşitse cached head/node+50 next değeri parent+38'e yazılır. Ardından **live** parent+40 tail okunur; node'a eşitse cached tail/node+48 previous parent+40'a yazılır. Live previous node+48 nonzero ise live next node+50 okunup previous+50'ye yazılır. Sonra next node+50 **tekrar** okunur; nonzero ise previous node+48 tekrar okunup next+48'e yazılır. Alias-visible head/tail/neighbor reread sırası korunur. Yalnız bu dalda own next+50,previous+48,parent+10'a bu sırayla QWORD zero stores gelir.

Own unlink003030 remove-parent yordamıyla ikame edilmez: o yordam registry,virtual node destruction ve pool release gibi bu bölümde bulunmayan ek etkiler taşır.001ea0 own-unlink bölümünde node_probe,virtual destructor veya pool release çağrısı yoktur.

Sonra live DWORD flags node+18 okunur. Bit0x200 set ise live QWORD node+20 pointer-only04b0f8'e verilir; null pointer guard veya size argümanı eklenmez. Optional free döndükten sonra **DWORD node+18=0,QWORD node+20=0,QWORD node+28=0** store'ları bu sırada yapılır. Padding+1c..1f korunur. Lower free storage alanlarını değiştirse de bu son resets yapılır; free failure son resets'e ulaşmaz. Önceki unlink/publication veya earlier reset store'ları rollback edilmez.

Bu value/string-storage prefix cleanup'ıdır;inline pointer-array/capacity/pool-array free işi değildir. Node object'i deallocate etmez veya pool item'ını release etmez. Native string ownership/allocator/fault/unwind/ABI/EH/concurrency hâlâ lower sınırdır.04b0f8 pointer-only interface korunur; yeni CRT free/terminal sağlayıcı eklenmez.

## Document zinciri ve doğrulama

Existing001e00 document destructor clear ardından dört gerçek N21 pool destructor kaynağını reverse embedded sırada001c30(+288),001cb0(+208),001bb0(+188),001d30(+108) çağırır;document-owned vector/flagged storage cleanup sonrası001ea0(document)'a tail-call yapar. Yeni final bridge bu son dependency'yi gerçek001ea0 kaynak gövdesine bağlar. Controlled node/empty-tree fixtures native DOM/parser veya full document lifetime eşdeğerliği değildir.

78 çekirdek +97 entegrasyon =175 yeni kontrol PASS. Authoritative gruplar ortak build `socket_node_base` / `socket_node_base_integration`;standalone script çalıştırılmış sayılmaz. Aynı owned mapped memory üzerinde node-base davranışı ve mevcut001e00→dört pool destructor→node-base kaynak zinciri controlled lower node/free effects ile sınanır. Yeni parser,DOM,native heap,fault veya EH çalıştırması iddiası yoktur. Bağımsız bounded static source review üretim cpp/hpp hash'lerine bağlıdır;full native equivalence review false kalır.

Ortak build203 üretim nesnesi(199 C++/4 C),219 normal +19 terminal grup,92 socket grubu. Önceki651 kaynak/CRT byte olarak korunur;656 current source snapshot girdisi vardır.46 validation mirror çifti=45 historical+1current;90 historical config referansı korunur.133 dependency hash;42 historical typed source closure,2 semantic session adapter,1 ayrı allocation adapter vardır. Bunlar DLL tamamlanma yüzdesi değildir. Full DLL tamamlanmadı;toplam yüzde ölçülmedi.

Original DLL/oyun/network/IOCP yürütülmez;gerçek CFG/token/achievement içeriği okunmaz. Hash-only integrity ayrıdır;bilinen engelli version.dll/generated version DLL tekrar denenmez. Genel CRT unchanged/paused,aşama5'e kadar bekler. Native allocator/copy/fill/free,node virtual/parser/DOM/global/vtable lifecycle/ABI/EH/concurrency açık kalır.

Sıradaki N23:kalan node virtual destruction inventory/contracts incelemesi. Bu hazırlık henüz yeni normal gövde iddiası değildir. Öncelik1→2→3→5→4 korunur.
