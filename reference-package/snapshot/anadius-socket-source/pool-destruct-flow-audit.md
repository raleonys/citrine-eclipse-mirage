# N21 — Dört pool destructor normal kaynak gövdesi

25 Ekim 2026. `pool_destruct.hpp/cpp`,001bb0/001c30/001cb0/001d30 için `destroy_native_pool_*` normal gövdelerini ve `RecoveredPoolDestructEffects::destroy_pool_*` final bağlantılarını ekler. Dört bağlantı existing `NativeDocumentEffects` sağlayıcılarını kapatır. Her hedefte normal+final iki tagged kaynak tanımı vardır; sekiz native fonksiyon değildir. Historical source closure37→41;2 semantic session adapter ve1 ayrı allocation dispatcher değişmeden korunur. `NativePoolDestructContext` existing clear context'inin alias'ıdır; mandatory pointer-only free interface'i değişmez.

## Kimlik ve retained kanıt

REA6.5.0 server/catalog aligned ve Ghidra kullanılabilir checkpoint'i `pool-destruct-rea-session.json` içindedir; hedef açık değildir. Bu aşamada yeni native import/data read yapılmaz. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Tam retained listing kimlikleri ve adresli sidecar'lar `pool-destruct-evidence.json` içinde hash'lenir.

| Yordam | Item stride | Type vtable immediate | EDX block hint | Assembly Evidence | Pseudocode Evidence |
|---|---:|---|---:|---|---|
|0x180001bb0|80|0x18007fe38|4080|`ev_7f94b4e17d4fc968551742e8037bd685c0d16490f0e4a589eb7b722000771cb7`|`ev_092ecbcf8771634efb8602a3d6cd0c898285159da964af33c3c60fc8ea0d6f6e`|
|0x180001c30|104|0x18007fe98|4056|`ev_f88b02234b39e4be1d70ef9f1d8f69cb3447507b0a25e2a40a9d178765b573e3`|`ev_b0d1ec372d4b8ad00df52da43c93679046facd73ad4d9f5863369ece94248459`|
|0x180001cb0|112|0x18007fe68|4032|`ev_1fec953cfb59eb28d532eb735335cff7cbee4916e7cbf4c23a119a7c379197f5`|`ev_443260d7e20484b82b22a88a38d81ed81dc3b6284636a4b11009555f363f64cd`|
|0x180001d30|120|0x18007fe08|4080|`ev_9e9eabb75164aad26441c55c6926835ef60fd2bf0461f0bf1fd1f4c442db8975`|`ev_586172bc5ef241fce0e41a4d51bfd926c1ba166521c02a285dba1e7337c7c816`|

Type table değerleri ve final base table `0x18007fa18` retained code içindeki LEA immediate operandlarından gelir. Bunlar yeni table-byte acquisition veya callable host function pointer değildir. Önceki complete `pool-allocation-simd-data.json` / `node-delete-next-vtables.json` kanıtları aynen korunur; initialized analysis memory live runtime gözlemi değildir. Existing001830 document constructor,001e00 document destructor ve üç002d70/002dc0/002e10 clear gövdesi context-only saklanır; yeni recovery sayılmaz.

## Gözlenen normal işlem sırası

Her destructor raw DWORD pool+64 için nonzero test sonucunu **type-vtable store öncesinde cache eder**. Ardından ilgili type table pool+0'a yazılır; entry branch önceki CMP sonucuna göre alınır. Alias backing nedeniyle vtable store count'u değiştirirse yeniden entry test yapılmaz. Initial count nonzero dalında loop do/while olarak girilir; bu yüzden normal clear helper'ını doğrudan çağırıp entry testini tekrar okumak eşdeğer bir projeksiyon değildir.

Girilen iterasyon live count'u tekrar okur; signed eski index'i cache edip old-1 raw32 değerini pool+64'e önce yazar. Ardından live data pointer pool+8 ve data+signed_old_count×8-8 slotundan block pointer okunur; pointer-only04b0f8(block) çağrılır. Free ardından live count nonzero test edilerek tekrar girilir. Negatif count boş kabul edilmez; positive clamp,hard iteration cap,ownership guard veya rollback eklenmez. Callback-visible count/data/slot mutation ve bounded failure prefix korunur.

Loop sonrasında pool+68,+70,+78'e bu sırada üç QWORD zero store gelir. Ardından **live** array data pointer pool+8 tekrar okunur; pool+10 inline adresinden farklıysa04b0f8(data) çağrılır. Bu array-free callsite için yalnız RCX kesin tanımlıdır; block-free EDX hint'ini array-free parametresi olarak yeniden atamak veya yeni sizedfree interface uydurmak yoktur. Son olarak base-vtable identity07fa18 pool+0'a yazılır.

Data pointer,capacity,inline bytes/stale entries reset edilmez. Lower array free count/root/data alanlarını değiştirirse sonraki tek store base-vtable'dır; diğer mutation'lar geri alınmaz. Block free/reset/array free failure'ları o ana kadar committed store'ları korur ve ilerideki base-vtable store'a ulaşmaz. Native allocation ownership,fault/unwind/SEH/concurrency/vtable ABI eşdeğerliği ileri sürülmez.

Block callsite EDX4080/4056/4032/4080 değerleri yalnız static hint'tir. Existing04b0f8 pointer-only free mandatory kalır; generic CRT free gövdesi eklenmez. Önceki composition'ın block04b0bc allocation/0793d0 fill ve array04b100 allocation/078d30 copy etkileri de mandatory'dir. Vtable identity değerleri host tarafında function pointer'a cast edilmez.

## Document bağlantısı ve doğrulama

Existing001e00 document destructor clear002e60 sonrasında destroy001c30(document+288),001cb0(document+208),001bb0(document+188),001d30(document+108) sırasını kullanır. Dört final bridge bu reverse embedded pool sırasını gerçek kaynak gövdelerine bağlar. Document-owned array/optional buffer frees ve001ea0 node-base destructor lower sağlayıcıları kalır. Controlled empty-tree projection native DOM/parser veya full document lifetime kanıtı değildir.

204 çekirdek +10.580 entegrasyon =10.784 yeni kontrol PASS. Authoritative sonuçlar ortak build `socket_pool_destruct` / `socket_pool_destruct_integration` gruplarıdır; standalone modül scripti çalıştırılmış sayılmaz. Controlled owned memory/free effects üzerinde dört tür için allocation→append→track/release→clear/destruct kaynak zinciri ve mevcut001e00 reverse pool sırası sınanır. Bounded alias/failure kontrolleri native heap/fault/EH çalıştırması değildir. Bağımsız bounded static source review üretim cpp/hpp hash'lerine bağlıdır; full native equivalence review false kalır.

Ortak build202 üretim nesnesi(198 C++/4 C),217 normal +19 terminal grup,90 socket grubu. Önceki646 kaynak/CRT byte olarak korunur;651 current source snapshot girdisi vardır.45 validation mirror çifti=44 historical+1current;88 historical config referansı korunur.127 dependency hash;41 historical typed source closure,2 semantic session adapter,1 ayrı allocation adapter vardır. Bunlar DLL tamamlanma yüzdesi değildir. Full DLL tamamlanmadı; toplam yüzde ölçülmedi.

Original DLL/oyun/network/IOCP yürütülmez;gerçek CFG/token/achievement içeriği okunmaz. Hash-only integrity ayrıdır;bilinen engelli version.dll/generated version DLL tekrar denenmez. Genel CRT unchanged/paused,aşama5'e kadar bekler. Pointer-only free,node virtual/parser/DOM/base/global/native pool/lifecycle/ABI/EH/concurrency açık kalır.

Sıradaki N22:kalan document/node-base lifecycle inventory/contracts incelemesi. Bu hazırlık henüz yeni normal gövde iddiası değildir. Öncelik1→2→3→5→4 korunur.
