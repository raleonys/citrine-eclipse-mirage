# N19 — Pool block-array append005440 normal kaynak gövdesi

25 Ekim 2026. `pool_append.hpp/cpp`, `append_pool_block_005440` normal gövdesini ve `RecoveredPoolAppendEffects::append_block_005440` final bağlantısını ekler. Final metot önceki N18 `PoolAllocationEffects::append_block_005440` sağlayıcısını kapatır. Aynı native adrese iki tagged kaynak tanımı bağlanır; iki native fonksiyon değildir. Historical source closure sayısı33→34;2 semantic session adapter ve1 ayrı allocation dispatcher değişmeden korunur.

## Kimlik ve retained kanıt

REA6.5.0 server/catalog aligned ve Ghidra kullanılabilir checkpoint'i `pool-append-rea-session.json` içindedir; checkpoint'te hedef açık değildir. Bu aşamada yeni native import veya data read yapılmaz. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Retained tam assembly ve pseudocode kimlikleri `pool-append-evidence.json` içinde korunur.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|0x180005440|`ev_805b1e389566c011341fa6b160b23ec49ecacccb6f0a1d123af5d4d5169b5cef`|`ev_3a74ec35882305703da94fdd0e4de6801b34e1449914f6d7d963f4cdd4ea201c`|

Context-only dört allocation002b10/002600/002960/0027b0 ve release0033a0/track005a20 eski normal gövdeleri yeniden recovered sayılmaz. `pool-allocation-simd-data.json`96-byte complete read ve `node-delete-next-vtables.json` complete vtable kanıtları byte olarak aynen kullanılır; yeni data-read iddiası yoktur. Bunlar initialized analysis memory kanıtıdır; live runtime pointer/CPU gözlemi değildir.

## Gözlenen işlem sırası

005440 array parametresi pool+8'dir. Array+0 QWORD data pointer,inline storage+8,capacity DWORD+58,count DWORD+5c. İlk count+1 raw32 wrapping ile hesaplanır ve capacity ile signed karşılaştırılır (`0x180005455`/`JLE`). Growth yoksa allocation/copy/free ve capacity publish atlanır.

Growth dalında required count ikiyle raw32 çarpılır (`0x18000546b`); doubled değer signed64 genişletilir ve unsigned64×8 çarpılır. Yüksek64bit nonzero ise allocation boyutuUINT64_MAX olur (`0x180005476`/`CMOVO`). `0x180005480`04b100 allocation çağrısı cache edilir. Allocation ardından **live** count sign-extend/shift3 ve data pointer yeniden okunur; `0x180005496`078d30 copy çağrılır. Sonra live eski pointer yeniden okunur; array+8 inline ile aynı değilse `0x1800054a7`04b0f8 free çağrılır. Free sonrasında cached yeni pointer (`0x1800054ac`) ve cached doubled capacity (`0x1800054b4`) yayımlanır. Callback-visible önceki state/alias sırası korunur; null-return guard,retry veya rollback eklenmez.

Ortak append tail live count'u signed64 ve data pointer'ı tekrar okur (`0x1800054bc`/`54c0`); cache edilen giriş block pointer'ını data+count×8'e yazar (`54c3`). Sonra live count DWORD artırılır (`54c7`); yazılan öğe count hücresine alias ederse increment o yazıdan sonraki değeri kullanır. Native pointer/fault/stack/SEH eşdeğerliği ileri sürülmez; bellek eşlemesi eksik/ambiguous ise mevcut bounded projection UnsupportedInput sınırı geçerlidir.

Final bridge aynı mapped memory ve mandatory `PoolAppendEffects` sağlayıcısı üzerinden gerçek005440 gövdesine gider. N18'den gelen04b0bc block allocation ve0793d0 fill ile array04b100 allocation,078d30 copy ve04b0f8 pointer-only free olmak üzere **beş lower etki** mandatory kalır; bu aşamada native gövde olarak eklenmez. Yeni CRT/terminal sağlayıcı veya native pool destruction yoktur.

## Doğrulama ve sınırlar

87 çekirdek +22.736 entegrasyon =22.823 yeni kontrol PASS. Authoritative sonuçlar ortak build `socket_pool_append` / `socket_pool_append_integration` gruplarıdır; standalone modül scripti çalıştırılmış sayılmaz. Controlled integration dört pool türünün gerçek allocation→append growth→track→release→reuse kaynak zincirini owned mapped storage ve supplied lower heap/copy/free effects üzerinde sınar. Tam parser/message/DOM materialization veya native heap/fault/EH kanıtı değildir. Bağımsız bounded static source review üretim cpp/hpp hash'lerine bağlıdır; full native equivalence review false kalır.

Ortak build200 üretim nesnesi(196 C++/4 C),213 normal +19 terminal grup,86 socket grubu. Önceki636 kaynak/CRT byte olarak korunur;641 current source snapshot girdisi vardır.43 validation mirror çifti=42 historical+1current;84 historical config referansı korunur.115 dependency hash;34 historical typed source closure,2 semantic session adapter,1 ayrı allocation adapter vardır. Bunlar DLL tamamlanma yüzdesi değildir. Full DLL tamamlanmadı; toplam yüzde ölçülmedi.

Orijinal DLL/oyun/network/IOCP yürütülmez; gerçek CFG/token/achievement içeriği okunmaz. Hash-only input integrity ayrıdır; bilinen engelli version.dll ve generated version DLL tekrar denenmez. Genel CRT unchanged/paused ve aşama5'e kadar bekler. Lower allocation/copy/free/fill,native pool lifecycle,node virtual/parser/DOM/ABI/EH/concurrency açık kalır.

Sıradaki N20:pool lifecycle inventory/contracts; seçilecek hedefler retained kanıta göre incelenecektir. Bu hazırlık henüz yeni normal gövde iddiası değildir. Öncelik1→2→3→5→4 korunur.
