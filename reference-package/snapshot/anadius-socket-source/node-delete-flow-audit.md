# N16 — Parent/node unlink ve standalone node deletion kaynak bağlantıları

25 Ekim 2026. `node_delete.hpp/cpp`,003030/003240 normal gövdelerini ve `RecoveredNodeDeleteEffects : NativeNodeClearEffects` final `remove_parent_003030`/`delete_node_003240` bağlantılarını ekler. N15 clear/child cleanup ve N14 document yolları aynı kontrollü ham bellek modeli üzerinden bu kaynaklara ulaşır. Gerçek native DOM/parser/pool/vtable/heap/ABI/EH tamamlanmış sayılmaz.

## Kimlik ve kanıt

REA6.5.0 server/catalog hizası ve Ghidra sağlayıcısı `node-delete-rea-session.json` içinde korunur. Orijinal hedef yürütülmedi. N16 gövdelerinde retained listing kullanıldı; N17 hazırlığında bilinen vtable alanları ayrıca read-only REA/Ghidra ile okundu ve hedef kapatıldı. Bu hazırlık yeni source-body sayımına dahil değildir. Retained Ghidra12.1.4 listing kimlikleri `node-delete-evidence.json` içindedir. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. İki hedef ve altı context işlevinin tam bölümleri evidence/context sidecar'larındadır; context yeni kurtarılmış sayılmaz. Adresler0x180 image prefix'ini kullanır.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|003030|`ev_62653814fe73bda4ddc3c13653105d5eae5d8b8e23438a418eb6eb770c525aad`|`ev_f0d8f947ec8be75ff8f8d98756eef10381d18d179adac4a13a2b449556abbc64`|
|003240|`ev_66a8781816232d9983f66d11708eff8ac7b7af8864cb6b8663030e90b072427d`|`ev_85f5d40be5f36d1fa85d93ac5e561c5df22a3923879f0ddc4fbe2ef681ff3a04`|

003030 parent+38 first ve+40 last kimlik karşılaştırmalarını sırayla yapar; node+48 prev ve+50 next komşularını onarır.003081 node vtable'ını link resetlerinden önce cache'ler; ardından node+50,+48,+10 sıfırlanır.003095 captured table+38 probe hedefini çağırır;003098 full64-bit RAX sonucunu test eder. Parent üyelik doğrulaması,null parent/node guard veya bulk parent reset eklenmez.

Probe zero ise canlı node+8 document ve signed32 document+104 count okunur. Pozitif ilk count search bound,document+a8 table pointer cache olur.0030ca match anındaki count'u EAX'e yeniden okuyarak zero-extend eder;0030d4 son slot adresi bu uint32 count ile hesaplanır.0030d9 swap-store sonrasında0030dd canlı DWORD count decrement edilir. Son slot sıfırlanmaz; bulunamama destruction'ı atlatmaz. Bu sözleşme003110 signed reload yoluyla birleştirilmez.

003240 null node için hiçbir bellek/virtual etki yapmadan döner. Nonnull node+38 probe full64-bit sonuç üretir; sibling/parent linklerini resetlemez. Probe zero dalında signed32 count ve table pointer bir kez cache olur.003292 son slot adresinde cached sign-extended count kullanılır; match anında count yeniden okunmaz.00329a swap-store sonrası00329e canlı DWORD decrement eder. Bu sözleşme003030 live zero-extended reload yoluyla birleştirilmez.

Her iki yordam destructor öncesinde canlı node vtable'ını okur ve node+60 pool pointer'ını cache'ler. Node slot+78(node,flags0) sonrasında cached pool'un canlı vtable'ı ve slot+18 release(pool,node) hedefi okunur.003030 son release'i tail JMP biçimindedir;003240 CALL/RET biçimindedir. C++ normal effect sırası korunur; native tail-call ABI/unwind eşdeğerliği iddia edilmez. Raw target değerleri mandatory effects üzerinden geçer; shipped image adresleri callable host pointer'a çevrilmez. Fixture vtable kimlikleri native hedef adresi kanıtı değildir.

## Doğrulama ve sınırlar

386 çekirdek +40.107 entegrasyon =40.493 yeni kontrol PASS. Ortak build `socket_node_delete` ve `socket_node_delete_integration` gruplarının gerçek sonuçları authoritative kayıttır; standalone modül scripti çalıştırılmış sayılmaz. Sentetik ham graph/mandatory effect oracles normal gövdelerin field ve effect sırasını sınar. Compatibility parser için kontrollü ham graph kullanımı full native DOM materialization,parser allocation veya pool yapısı kanıtı değildir. Upstream document/dispatch/cipher/response queue/send/finish zincirinin kapsama sınırı integration kayıtlarında tutulur.

Ayrı ikinci ajan retained assembly ile iki üretim gövdesini ve final bağlarını karşılaştırır: bounded static source review PASS. cpp/hpp tam hash'leri ve review kapsamı `node-delete-source-review.json` içindedir. Bu,full native equivalence review değildir. Node probe/destructor,pool release/track,0041b0 parser,remaining pool/DOM/native allocator/free/vtable/object lifecycle/ABI/EH zorunlu açık sağlayıcılardır. Normal failure prefix'lerine rollback veya native unwind uydurulmaz; fixture teardown native EH kanıtı değildir. Orijinal DLL/oyun/network/IOCP çalıştırılmaz; gerçek CFG/token/achievement içerikleri okunmaz. Hash-only input integrity kontrolü ayrıdır; bilinen engelli version.dll yeniden okunmaz.

İki yeni unique adres eşlemesi,iki normal-body claim veiki typed source closure eklenir. Önceki621 C++/header kaynağı ve CRT byte olarak korunur. Ortak build197 nesne(193 C++/4 C),207 normal +19 terminal grup/80 socket;626 kaynak snapshot girdisi ve31 typed source closure vardır.39 eski yerel validation kaydı tarihsel kalır;40 validation mirror çifti ve78 historical config referansı korunur. Bu sayılar DLL tamamlanma yüzdesi değildir. Full DLL henüz tamamlanmadı; toplam yüzde ölçülmedi.

Sıradaki N17:shipped vtable evidence ile node virtual target/pool backend inventory. Native hedef adresleri seçilmeden önce gerçek vtable kanıtı gereklidir; fixture adresleri kullanılmaz. Öncelik1→2→3→5→4; genel CRT aşama5'e kadar bekler.


N17 hazırlık kanıtı: dört pool vtable'ının+18 slotu0x1800033a0,+20 slotu0x180005a20 adresini içerir.192/192 bayt okuma Evidence `ev_7349a6f3b8db519dad314c7de8d04be1e89ad0c44e1b592d75731e5c20e44706`; document vtable128/128 bayt okuma `ev_6f833c2318f756f405188b1d9c7ac22f59489362ca3ee1f71598ed665f9f3e33`. Tam kayıtlar ve loader-fixup/initialized-memory sınırlamaları `node-delete-next-vtables.json` içinde korunur. Bu ek okuma N16'nın iki yeni kaynak gövdesi sayısına eklenmez.
