# N29 — 0x1800035e0 GetStr kaynağı

27 Ekim 2026. `node_string.hpp/cpp`, `materialize_node_string_0035e0` normal gövdesini ve `RecoveredNodeStringEffects::materialize_0035e0` final bağlantısını ekler. Önceki document/parser/Identify/constructor/insertion kaynakları korunur. Numeric decoder, copy, compare ve classifier alt sağlayıcıları zorunludur.

## Kimlik ve kanıt

Anadius64 SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. `node-string-rea-session.json`, REA6.5.0 server/catalog kimliği, kullanılabilir Ghidra ve kapalı hedef checkpoint'ini içerir. Yeni native import/data read yapılmaz; eski complete retained kayıtlar kullanılır.

| Native yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|0x1800035e0|`ev_704e716e4fcf7eead3762f34f5b8e311ff9f14a674ad542b9c79429b3efad5e0`|`ev_5e71fdafa86c9cb6c66ad0d57972845920ad9cc67d23951b21164d5b586a0c01`|

Önceki N28 envanterinde0035e0 `module_record_only` idi: `anadius-xml-source/reader-evidence.json` fonksiyon kaydı vardı, tanım yoktu.0definition/uncompiled/normalfalse olan tek giriş `source_definition_mapped` olur. Module-only−1,mapped+1;no_definition_mapping değişmez. Aynı native adrese normal+final iki numeric tanım bağlanır. Historical typed closure51→52 olur; seçili node-parse adapteri1,semantic session adapter2 ve allocation adapter1 korunur.

## Flag ve ilk geçiş sırası

`0035f8` flags100 testinde bit yoksa yalnız mevcut pair.start döner. Bit varsa pair.end'deki BYTE önce0 yapılır (`003613`), ardından flags canlı XOR100 ile güncellenir (`003616`). XOR sonucu0 ise ilk geçiş atlanır; canlı flags4 whitespace testi ve final AND200 yine uygulanır. İlk geçiş cached input/output cursor tutar, her iterasyonda flags'ı ve sınırda pair.end'i yeniden okur. Pair.end yeniden yazılmaz.

Flags2, CRLF ve LFCR çiftlerini tek LF'ye çevirir; tek CR/LF de LF olur. Flags1 ve '&' için numeric veya named entity yolu seçilir. Olağan kopya, kaynak byte'ını store anından hemen önce yeniden okur (`003740`). Bounded map hataları ve alt sağlayıcı hataları önceki committed store'ları geri almaz; yapay null/length guard veya rollback eklenmez.

## Named entity tablosu ve no-store yolu

| Kayıt adresi | Name pointer | Name | Signed length | Replacement |
|---|---|---|---:|---|
|`0x18007f760`|`0x18007f7b0`|`quot`|4|`0x22`|
|`0x18007f770`|`0x18007f7b8`|`amp`|3|`0x26`|
|`0x18007f780`|`0x18007f7bc`|`apos`|4|`0x27`|
|`0x18007f790`|`0x18007f7c4`|`lt`|2|`0x3c`|
|`0x18007f7a0`|`0x18007f7c8`|`gt`|2|`0x3e`|

Tablo07f760..07f7af beş16bayt kayıttır. Compare çağrısı live signed32 length'i64bite genişletir (`003700`); başarılı compare sonrası semicolon için length yeniden okunur (`003714`). Replacement byte store'undan sonra length DWORD yeniden yüklenir;ADD EAX2 modulo32 ve CDQE ile input ilerletilir (`003733..00373b`). Karşılaştırma için pointer equality kısayolu yoktur.

Beş compare başarısızsa veya eşleşen prefix sonrasında ';' yoksa `003728→003745` yolu input ve output cursor'larını artırır, **destination'a byte yazmaz**. Daha önce CRLF/entity sıkıştırması olmuşsa eski destination byte'ı korunabilir. Bu yol sıradan '&' kopyasıyla değiştirilemez. Test oracle'ı bu assembly ayrımını kapsar.

## Numeric sınırı ve whitespace

Numeric '&#' yolunda her0033c0 çağrısından önce count DWORD0 ve scratch10byte0 hazırlanır. Decoder0 döndürürse normal fresh source-byte copy yapılır. Başarılı dönüşte count **signed32→64** genişletilir (`0036d9`) ve zorunlu078d30 copy sağlayıcısına iletilir. Output ilerlemesi cached genişletilmiş count'u kullanır; input decoder dönüşüne gider. `NativeNodeStringEffects::decode_0033c0` ve `copy_string_078d30` host span/ref projeksiyonlarıdır; native stack adresleri, stack corruption/fault modeli veya özgün decoder/copy gövdesi sayılmaz. Count'a yeni clamp eklenmez; bounded sağlayıcı kendi destek sınırını açıkça uygulayabilir.

Flags4 whitespace aşamasında80..FF byte'ları classifier051fc0'ı atlar. Leading scan zero byte'ını da sınıflandırır; başlangıç cursor'ı pair.start'a yazıldıktan sonra byte yeniden okunur. Outer cached byte/classifier ile inner fresh byte/classifier ayrımı ve yinelenen çağrılar korunur (`0037c0..0037f9`). Inner scan çıkışında fresh byte0 ise boşluk yazılmaz; aksi halde ASCII20 yazılır ve source byte tekrar okunarak kopyalanır. Bu aşama pair.end sınırını kullanmaz. Final flags canlı AND200 ve live pair.start dönüşüdür (`00381b/003822`).

## Retained veri ve çağrı ilişkileri

Entity kayıtları/names, eski complete raw read `ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082` kaydından gelir. Parent SHA-256:`c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495`;07f000..0a0dff aralığı138752/138752bayt complete'tir. Tablo80byte SHA-256:`2f95e69e4e56392cad524e29c11fcebb53c93132d7bf4eb8c98cab2918c8d3db`. Exact row/name bytes ve offsets `node-string-retained-data.json` içinde doğrulanır. Initialized static Ghidra memory,runtime gözlemi değildir.

004970 caller'ı isim/closing pair karşılaştırmaları için0035e0 kullanır.0035e0 doğrudan0033c0,05c550,078d30,051fc0 ve security-cookie04b220 çağrılarını içerir.0033c0/004970 context dökümleri yeni kaynak sayılmaz. Security cookie/native stack/ABI/EH, CRT locale ve diğer parser/DOM/heap/virtual hedefler açık kalır.

## Doğrulama ve sınırlar

**65 çekirdek + 57 entegrasyon = 122 yeni kontrol PASS**. Authoritative gruplar ortak build'deki `socket_node_string`/`socket_node_string_integration` olur;module standalone script çalıştırılmış sayılmaz. Bağımsız sınırlı statik kaynak review exact CPP/HPP hash'lerine bağlıdır;full native equivalence review açık kalır.

Ortak build210 üretim nesnesi(206 C++/4 C),233 normal +19 terminal grup,106 socket grubu içerir. Önceki686 kaynak/CRT byte olarak korunur;691source/243documentation snapshot girdisi,53validation mirror=52historical+bircurrent,104historical config referansı,182dependencyhash ve52historical typed closure vardır. Bu sayılar DLL tamamlanma yüzdesi değildir.

Orijinal DLL/oyun yürütülmez/değiştirilmez;gerçek CFG/token/achievement içeriği okunmaz. Hash-only integrity ayrıdır. Engelli version.dll/generated DLL tekrar okunmaz veya koruma ayarları değiştirilmez. Öncelik1→2→3→5→4;genel CRT aşama5 beklemede.

Sıradaki N30, 0x1800033c0 numeric entity decoder yordamının kayıtlı kanıt ve sözleşme incelemesidir; hazırlık yeni decoder kaynak gövdesi sayılmaz.
