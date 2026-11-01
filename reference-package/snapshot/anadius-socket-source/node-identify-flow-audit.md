# N26 — 0x1800038b0 Identify kaynak gövdesi

26 Ekim 2026. `node_identify.hpp/cpp`, `identify_node_0038b0` normal byte-state gövdesini ve `RecoveredNodeIdentifyEffects::identify_0038b0` final bağlantısını ekler. N25 seçili kaynak yönlendirmesi korunur; yalnız mevcut Identify sağlayıcısı kaynak koduna bağlanır. Beş constructor ve classifier/compare gövdeleri bu aşamada çözülmüş sayılmaz.

## Kimlik, kanıt ve sayım

Anadius64 SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA 6.5.0 server/catalog kimliği, kullanılabilir Ghidra ve kapalı hedef checkpoint'i `node-identify-rea-session.json` içindedir. Yeni binary import veya data read yapılmaz; önceki tam REA kayıtları kullanılır.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
| 0x1800038b0 | `ev_e46dfd93352ef14c6e16936e419152588cc3aae4c071f99fdc01fe8f8145aae9` | `ev_16c4bf124a34657d7d2d217f79a017802a074ddf7e1c559929f47750b79fdee5` |

Önceki N25 envanteri 0038b0 için `module_record_only`, 0 definition, uncompiled ve normal-claim false idi. Bu aşama yalnız bu bucket'ı `source_definition_mapped` yapar; `no_definition_mapping` değişmez. Normal gövde ve final override aynı native adres için iki numeric kaynak tanımıdır. Historical typed closure 44→45 olur. N25 `node_parse_source_adapters` listesi (bir kayıt), iki semantic session adapter ve ayrı allocation adapter byte olarak korunur; yeni ayrı adapter eklenmez.

## Giriş, whitespace ve yayın sırası

RCX=document, RDX=input cursor, R8=out-node QWORD adresi, RAX=dönüş cursor'udur. Girişte out-node sıfırlanmaz; bu alanı önce sıfırlayan N25 caller ayrı bir işlemdir. `0038d2` initial line DWORD'ünü ilk input byte'tan önce cache eder. Cached line address document+a0'dır; original ve trimmed cursor ayrı tutulur.

High-bit byte classifier'ı atlar. Diğer byte0..127, NUL dahil, mandatory051fc0'a verilir. Nonzero dönüşte cached lineAddress sıfır değilse current byte tekrar okunur; LF için live DWORD satır modulo32 artırılır. Skip sonrası byte yeniden okunur. NUL yolunda `00390c` out-node0 yayımlar ve trimmed cursor döner; token/constructor çağrılmaz. Hardcoded whitespace, erken NUL guard veya ikinci BOM geçişi eklenmez.

## Token çağrı ilişkileri

Her global token pointer'ı sırası geldiğinde LIVE okunur. Candidate pointer token pointer'a eşitse compare çağrısı atlanır; aksi halde compare05c550(candidate,token,limit) zorunlu alt etkisidir. Nonzero sonuç bir sonraki pointer slotuna geçer. Sabit limitler tam string değeri iddiası değildir.

| Slot / compare adresi | Limit | Constructor / çağrı | Pool | Match cursor |
|---|---|---|---|---|
| 0x1800a10a8 /003934 | 2 |001610 /003a63 |doc+288 |candidate+2 |
| 0x1800a10b0 /00395a | 4 |001590 /003a4e |doc+288 |candidate+4 |
| 0x1800a10b8 /003980 | 9 |001720 /003a2c |doc+208 |candidate+9 |
| 0x1800a10c0 /0039a2 | 2 |0017b0 /003a1b |doc+288 |candidate+2 |
| 0x1800a10c8 /0039c0 | 1 |001690 /0039d3 |doc+108 |candidate+1 |
| fallback | — |001720 /0039f7 |doc+208 |original input |

Matched constructor sonrası live document line node+30'a yazılır; out-node en son yayımlanır. CDATA yolu `003a38` live line'ı BYTE node+68=1 yazımından ÖNCE cache eder, ardından cached line ve out-node yayımlar. Fallback constructor sonrası node line live satırdan alınır, document line başlangıç snapshot'ına geri yazılır, out-node yayımlanır ve **orijinal input cursor** döner. Trimmed cursor'u döndürmek veya rewind'i atlamak aynı davranış değildir.

Constructor0 veya input-null için yeni guard yoktur. Bounded mapping/alt etki hataları committed yazım prefix'lerini korur; native fault/EH, rollback ya da otomatik free iddiası eklenmez. Yayın alanları ve line/flag alanları physical alias olduğunda gözlenen store/load sırası korunur.

## Retained veri sınırı

`node-identify-retained-data.json` tam retained asm/pseudo kimliklerini ve exact0038b0 bölümlerini doğrular. Ayrıca önceki broad read `ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082` ile karşılaştırır. Parent SHA-256 `c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495`, aralık 0x18007f000..0x1800a0dff, 138752/138752 byte complete'tir. **0x1800a10a8..0x1800a10cf token pointer slotları bu aralık dışındadır.** Slot adresleri, sıra, constructor operandları ve compare limitleri assembly kanıtıdır; gerçek pointer/string baytları bu kayıtta edinilmiş sayılmaz. Fixture pointer'ları native raw-slot kanıtı değildir; başka retained dosyalar için genel yokluk iddiası kurulmaz.

## Doğrulama ve kalan işler

**122 çekirdek + 47 entegrasyon = 169 yeni kontrol PASS**. Authoritative ortak build grupları `socket_node_identify` ve `socket_node_identify_integration` olur. Standalone module script bu aşamada çalıştırılmış sayılmaz. Bağımsız sınırlı statik review üretim CPP/HPP hash'lerine bağlıdır; tam native eşdeğerlik incelemesi açık kalır.

Ortak build: 207 üretim nesnesi (203 C++ /4 C), 227 normal +19 terminal grup, 100 socket grubu. Önceki 671 kaynak korunur; 676 source ve 240 documentation snapshot girdisi vardır. 50 validation mirror =49 historical +bir current, 98 historical config referansı ve 161 dependency hash. 45 historical typed closure; N25 seçili node-parse adapteri bir kayıt olarak korunur. Bu sayılar toplam DLL tamamlanma yüzdesi değildir.

Beş constructor, classifier051fc0, compare05c550, token-pointer raw data, diğer parser primitifi/heterojen hedefler, heap/formatter/scratch/error argument transport/native ABI/EH/concurrency ve tam DOM açık kalır. Orijinal DLL/oyun yürütülmez veya değiştirilmez; gerçek CFG/token/achievement içeriği okunmaz. Hash-only integrity ayrı kaydedilir; önceki engelli version.dll/generated DLL tekrar okunmaz, koruma ayarları değiştirilmez. Öncelik1→2→3→5→4; genel CRT aşama5 beklemede.

Sıradaki N27, beş node constructor yardımcısının (001590/001610/001690/001720/0017b0) envanter ve sözleşme incelemesidir; bu hazırlık yeni constructor gövdesi sayılmaz.
