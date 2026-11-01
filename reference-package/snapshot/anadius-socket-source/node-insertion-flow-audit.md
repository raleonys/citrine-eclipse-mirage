# N28 — 0x180003aa0 insertion hazırlama kaynağı

27 Ekim 2026. `node_insertion.hpp/cpp`, `prepare_native_insertion_003aa0` normal gövdesini ve `RecoveredNodeInsertionEffects::prepare_003aa0` final bağlantısını ekler. Mevcut parser/Identify/constructor zinciri korunur. Bu bir node destructor, yeni owner'a append veya full native DOM gövdesi değildir.

## Kimlik ve kanıt

Anadius64 SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA6.5.0 server/catalog kimliği, kullanılabilir Ghidra ve kapalı hedef checkpoint'i `node-insertion-rea-session.json` içindedir. Yeni binary import/data read yapılmaz; eski complete retained kayıtlar kullanılır.

| Native yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|0x180003aa0|`ev_0b08898f12f5db24f44abc354495178632463f25a053f9ce71e2a4157571b4ef`|`ev_ce74b4f983309d2cc1be9c243fc3798203afe86b437ad9ffead5aaaad8d7c3cb`|

Önceki N27 envanterinde003aa0 `no_definition_mapping`,0definition/uncompiled/normalfalse idi. Tek giriş `source_definition_mapped` olur;no_definition_mapping−1,mapped+1,module_record_only değişmez. Normal gövde ve final override aynı native adrese iki numeric tanım verir. Historical typed closure50→51 olur; mevcut node-parse seçili adapteri1,semantic session adapter2 ve allocation adapter1 korunur.

## Linked branch: yalnız detach

Gelen owner argümanı okunmaz; `003aa0` parent'ı child+10'dan alır. Parent nonzero ise sırasıyla parent.first+38, ardından live parent.last+40 kontrol edilir. Child first/last olduğunda child.next/previous üzerinden uç pointer güncellenir. Sonra previous.next yazımı ve bu store SONRASINDA yeniden okunan child.next için next.previous yazımı gelir. En son child.next+50,previous+48,parent+10 bu sırada sıfırlanır (`003b03/003b07/003b0b`). Padding/alias ve committed mapping-failure prefix'leri korunur.

Bu yol allocation-vector,pool track,node probe,destructor,release veya free çağırmaz. Önceki003030 adı unlink içerse de node'u sonrasında destroy/release eder; bu yordamın yerine kullanılamaz.003aa0 yeni owner'a append etmez;004970 bunu dönüşten sonra live owner.last okuyarak yapar.

## Unlinked branch: outstanding kayıt ve track

Child.parent0 ise child+8 document cache edilir. Document+104 initial count signed32→64 okunur;count<=0 ise arama atlanır. Positive count ve document+a8 entries arama boyunca cache edilir. Slot child'a eşitse replacement tail count **live DWORD, ZERO-extended** olarak yeniden yüklenir (`003b4b` MOV EAX);match index signed32→64 genişletilir. Son entry matched slot'a yazılır;DEC document+104 swap SONRASINDA live DWORD'ü yeniden okur (`003b5e`). Decompiler'ın live loop-bound izlenimi assembly'deki cached R10 sınırını değiştirmez.

Count<=0,notfound ve match yollarının tümü live child+60 pool,live pool vtable ve slot+20 target okumasıyla track etkisine gider. Child link alanları bu yolda sıfırlanmaz. Native tail return sözleşmesi yeniden kurulmaz:004970 sonucu kullanmadığından void projection korunur. Yapay null/owner guard,rollback,node free veya native EH iddiası yoktur.

## Önceki kaynak ve veri kanıtı

Bilinen005a20 hedefi mevcut `RecoveredPoolReleaseEffects`/`track_pool_005a20` kaynağına bağlanabilir; generic hedefler alt sağlayıcıda kalır. Yeni005a20 recovery,closure veya selected adapter sayılmaz.005a20 yalnız pool+7c untracked DWORD'ünü azaltır;free-list/current allocation sayaçlarına yeni işlem eklenmez.

`node-insertion-retained-data.json`, eski complete raw read `ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082` kaydına bağlıdır. Parent SHA-256:`c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495`;07f000..0a0dff aralığı138752/138752 bayt complete'tir.07fe08/07fe38/07fe68/07fe98 tablolarının+20 slotları exact8 byte `205a008001000000` ile005a20 kimliğini doğrular. Initialized Ghidra memory,runtime çağrı gözlemi değildir.

## Doğrulama ve sınırlar

**71 çekirdek + 108 entegrasyon = 179 yeni kontrol PASS**. Authoritative gruplar ortak build'deki `socket_node_insertion`/`socket_node_insertion_integration` olur;module standalone script çalıştırılmış sayılmaz. Bağımsız sınırlı statik kaynak incelemesi exact CPP/HPP hash'lerine bağlıdır;full native equivalence review açık kalır.

Ortak build209 üretim nesnesi(205 C++/4 C),231 normal +19 terminal grup ve104 socket grubu içerir. Önceki681 kaynak/CRT byte olarak korunur;686source/242documentation snapshot girdisi,52validation mirror=51historical+bircurrent,102historical config referansı,175dependencyhash ve51historical typed closure vardır. Bu sayılar DLL tamamlanma yüzdesi değildir.

Generic pool hedefleri,GetStr0035e0,diğer child parser/DOM,heap/classifier/compare/token data,formatter/scratch/error-argument transport/native ABI/EH/concurrency açık kalır. Orijinal DLL/oyun yürütülmez/değiştirilmez;gerçek CFG/token/achievement içeriği okunmaz. Hash-only integrity ayrıdır. Engelli version.dll/generated DLL tekrar okunmaz veya koruma ayarları değiştirilmez. Öncelik1→2→3→5→4;genel CRT aşama5 beklemede.

Sıradaki N29, 0x1800035e0 GetStr yordamının kayıtlı kanıt ve sözleşme incelemesidir; hazırlık yeni kaynak gövdesi sayılmaz.
