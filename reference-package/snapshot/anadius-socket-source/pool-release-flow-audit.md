# N17 — Pool free-list release ve tracking kaynak bağlantıları

25 Ekim 2026. `pool_release.hpp/cpp`,0033a0/005a20 normal gövdelerini ve `RecoveredPoolReleaseEffects : RecoveredNodeDeleteEffects` final generic `pool_release`/`pool_track` bağlantılarını ekler. Bridge adlarında numeric tag yoktur: her hedefte bir tagged normal tanım bulunur. İki bilinen native target için guard bounded dispatch kuralıdır; ek native gövde değildir.

## Kimlik ve kanıt

REA6.5.0 kimliği ve Ghidra sağlayıcısının kullanılabilirliği `pool-release-rea-session.json` içinde kayıtlıdır. Ghidra12.1.4 sürümü retained analiz kanıtına aittir; original yürütülmedi. Tam retained listing kimlikleri `pool-release-evidence.json` içindedir. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. N16 complete shipped-vtable read evidence `node-delete-next-vtables.json` aynen reuse/hash edilir; yeni vtable read iddiası yoktur. Pool read192/192 Evidence:`ev_7349a6f3b8db519dad314c7de8d04be1e89ad0c44e1b592d75731e5c20e44706`. Dört shipped pool table07fe08/07fe38/07fe68/07fe98 aynı slot+18→0033a0 ve+20→005a20 hedeflerini paylaşır. Initialized analysis memory/loader fixups,live runtime pointer değildir. Fixture kimlikleri native hedef kanıtı değildir.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|0033a0|`ev_b7d15db3601446cde12580c10ac5ad7d9eb1c2aa6ce5bfb3646f09c3c2800809`|`ev_503f4ea3c84034bdcba37c5bab8c21622f3c49f5ed6db330dd2f530060498520`|
|005a20|`ev_fc5e2131422789819e9bf0acf480c663ab946afa810969adb78f817d69d2ffb2`|`ev_df5bc8292ee95a900d1b02c812064d25be2cc214ec36d768fec18c3da8b9e8c8`|

0033a0 null item için pool'a hiç erişmeden döner. Nonnull item dalı pool+68 root'u cache eder; pool+70 current DWORD decrement yapar; cached root'u item+0'a yazar; item'ı pool+68 root olarak yayımlar. Sıra ve alias davranışı korunur. Raw DWORD0→ffffffff wrapping vardır; item'ın eski vptr alanı free-list link olur. Membership/null-pool/count guard veya HeapFree eklenmez.

005a20 pool+7c untracked DWORD decrement yapıp döner. Raw32 wrapping vardır; item/heap/global/free çağrısı yoktur. Bunlar bellek içi normal projections olup native pool allocation/destruction tamamlanması değildir.

Generic final bridges yalnız shipped target0033a0/005a20 eşleşmesinde bu kaynaklara ulaşır. Unknown target explicit unsupported boundary olarak kalır. Raw image target değerleri callable host pointer'a çevrilmez. Node probe+38/destructor+78,parser0041b0,remaining pool helpers/vtable/allocator/native object lifecycle/ABI/EH zorunlu açık sağlayıcılardır.

## Doğrulama ve sınırlar

613 çekirdek +148.389 entegrasyon =149.002 yeni kontrol PASS. Ortak build `socket_pool_release`/`socket_pool_release_integration` gerçek sonuçları authoritative kayıttır; standalone modül scripti çalıştırılmış sayılmaz. Sentetik ham bellek/effect oracles null no-access,cached-root/decrement/store sırası,alias veDWORD wrapping durumlarını sınar. Integration aynı ham bellek üzerinde açıkça mapped0x80 pool prefix ve controlled parser roots kullanır; native pool allocation veya full DOM/parser materialization kanıtı değildir. Upstream parent/standalone clear/document/dispatch/cipher/response queue/send/finish kapsamı integration kaydında sınırlıdır.

Ayrı source author'dan bağımsız review retained assembly ile iki üretim gövdesi/final guard bağlarını karşılaştırır: bounded static source review PASS. cpp/hpp tam hash'leri ve adresleri `pool-release-source-review.json` içindedir; full native equivalence review false. Normal failure prefix için rollback/native unwind uydurulmaz; fixture teardown native EH kanıtı değildir. Orijinal DLL/oyun/network/IOCP çalıştırılmaz; gerçek CFG/token/achievement içerikleri okunmaz. Hash-only input integrity kontrolü ayrıdır; bilinen engelli version.dll ve engellenmiş generated version DLL yeniden denenmez. CRT unchanged/paused; HeapFree veya yeni CRT source yoktur.

İki yeni unique adres,iki normal-body claim veiki typed generic source closure eklenir; her native hedefte bir tagged definition vardır. Önceki626 C++/header ve CRT byte olarak korunur. Ortak build198 nesne(194 C++/4 C),209 normal +19 terminal grup/82 socket;631 kaynak snapshot girdisi ve33 typed source closure vardır.40 eski yerel validation kaydı tarihsel kalır;41 validation mirror çifti ve80 historical config referansı korunur.102 dependency hash vardır. Bu sayılar DLL tamamlanma yüzdesi değildir. Full DLL tamamlanmadı; toplam yüzde ölçülmedi.

Sıradaki N18:shipped vtable üzerinden pool allocation002b10/002600/002960/0027b0 inventory/contracts. Bu hedeflerin yeni normal gövdeleri henüz claimed değildir. Öncelik1→2→3→5→4; genel CRT aşama5'e kadar bekler.


Test kapsamı ayrıntısı: 4 pool prefix ×3 kontrollü root durumu mesaj/cipher/queue/send zincirinde sınanır; current/untracked sayaçları sıfıra ve free-list başı bırakılan root'a gelir. Diğer pool prefix'leri korunur; eski pool release/track stub çağrıları sıfırdır. Allocation alanları compatibility parser tarafından kontrollü hazırlanır, native allocator sayılmaz. Eksik current-count mapping testi gerçek yeni release gövdesini destructor sonrasında durdurur; publish edilmiş yanıt ve canlı parser buffer geri alınmaz. Son root store ile ilk root read aynı statik span'i kullandığından, araya callback uydurularak bağımsız publication-mapping kaybı testi yapılmaz; alias testleri yayın sırasını doğrular.
