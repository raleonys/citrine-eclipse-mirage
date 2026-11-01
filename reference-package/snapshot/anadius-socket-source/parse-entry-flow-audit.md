# N23 — Parse-entry0041b0 normal prelude kaynağı

26 Ekim 2026. `parse_entry.hpp/cpp`, `parse_document_entry_0041b0` giriş gövdesini ve `RecoveredParseEntryEffects::parse_0041b0` bağlantısını ekler. Mevcut `NativeDocumentEffects` parse-entry sağlayıcısı kaynak koduna bağlanır; yeni `NativeParseEntryEffects` sınıflandırıcı ve dolaylı parse bağımlılıkları açık kalır. Tam parser/DOM yeniden oluşturulmuş değildir. Bir native hedefe iki kaynak tanımı bağlanır. Önceden kaydedilen kaynak bağlantısı sayısı 42→43 olur; iki semantic session adapter ve ayrı allocation dispatcher korunur.

## Kimlik,önceki durum ve kanıt

REA6.5.0 server/catalog aligned,Ghidra kullanılabilir ve hedef kapalı checkpoint'i `parse-entry-rea-session.json` içindedir. Yeni native import veya data read yapılmaz. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Tam retained listing kimlikleri/adresli sidecar'lar `parse-entry-evidence.json` içindedir.

0041b0 önceki inventory'de **module_record_only**,0definition/uncompiled/normal-claimfalse idi;no_definition_mapping değildi. Bu aşamada yalnız module_record_only→source_definition_mapped geçişi vardır;no_definition_mapping bucket sayısı bu hedef nedeniyle azalmaz.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|0x1800041b0|`ev_0161e7e54c7c2f628bb6bbf99adff0b97b85e09b7c8528365aa858c02fdd4a08`|`ev_d4270b9a2ca885be4b12cfa24f257c4c48568ce7a4cbd3234c71d483394ca0de`|

Gerçek sınırlı çağrı sınırları:CALL051fc0(`0041ef`),tail JMP005820(`00424e`),indirect tail JMP [live vtable+80](`004275`).005820 error,051fc0 classifier ve static slot target004970 gövdesi yeni normal recovery sayılmaz;004280 existing parent wrapper context-only'dir.

N16 dar document-vtable read07fec8 için128/128 byte Evidence`ev_6f833c2318f756f405188b1d9c7ac22f59489362ca3ee1f71598ed665f9f3e33`,+7f sonunda biter;slot+80 bu dar aralık dışındadır. Ancak önceki **complete broad retained** `outputs/anadius-rea-evidence.json` Evidence`ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082`,07f000..0a0dff aralığında138752/138752 byte içerir. Parent SHA-256:`c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495`. Bu eski kanıtta07ff48 slotu,parent offset3912,exact8 byte`7049008001000000` olarak **0x180004970** hedefini doğrular. Küçük extract/provenance kaydı `parse-entry-retained-slot.json` içindedir.004970 body context-only/mandatory olup yeni normal recovery sayılmaz. Fixture target native kimlik kanıtı değildir;source generic live-vtable lookup/mandatory virtual-parse etkisini korur. Yeni original read veya table acquisition yoktur. Initialized analysis memory/live runtime ayrımı korunur;raw target host function pointer olarak çağrılmaz.

## Gözlenen normal işlem sırası

Girişte document+98 buffer cursor cache edilir;sonra cached line-counter address document+a0 için DWORD1 ve document+30 için DWORD1 yazılır. Lower classification doc+98 alanını değiştirse de cached cursor korunur.

Her iterasyon current cursor byte'ını okur. High bit set ise051fc0 classification atlanır ve whitespace skipping biter. Aksi halde unsigned byte0..127 mandatory classifier'a verilir. Dönüş0 ise skip biter;nonzero ise line address nonzero olduğunda current byte callback **sonrasında tekrar okunur**. YalnızLF0a olduğunda live DWORD line counter modulo32 artırılır. Cached cursor artırılıp devam edilir. Hardcoded ASCII whitespace/locale tablosu veya classification öncesi ayrıNUL stop eklenmez;lower byte/counter mutation'ları korunur.

Sonra BYTE document+68 BOMflag0 yazılır ve input byte tekrar okunur. ByteEF ise sırayla+1BB ve+2BF kontrol edilir. Her mismatch unchanged cursor/BOMflag0 ile doğrudan virtual parse yoluna gider. Full BOM cursor+3 yapar;BYTE BOMflag1 store'dan sonra advanced cursor byte okunur. BOM sonrasında ikinci whitespace pass yoktur. Ordinary nonEF/full-BOM yolunda cached current byte0 ise mandatory005820(document,13,0,0) tail-call yapılır;ardından DOM parse yoktur. Partial BOM mismatch'e ordinary empty-input kontrolü eklenmez.

Virtual yol tüm lower effects/stores sonrasında **live** document vtable ve slot+80 target'ını okur. Mandatory `parse_virtual(target,document,cached cursor,0,document+a0)` effect'i çağrılır;native target pointer'ı yürütülmez.0041b0 void projection return değerini kullanmaz. Missing bounded mapping/lower failure committed line/field/BOM prefix'ini korur;synthetic rollback/native fault/unwind iddiası yoktur.

## Bağlı doğrulama ve açık sınırlar

78 çekirdek +142 entegrasyon =220 yeni kontrol PASS. Authoritative gruplar ortak build `socket_parse_entry` / `socket_parse_entry_integration`;standalone script çalıştırılmış sayılmaz. Existing004280 wrapper gerçek entry kaynak gövdesine bağlanır;indirect parser,error/classifier fixture'ları declared lower effects olarak kalır. Source error cleanup önceki pool/node clear kaynaklarını kullanır;bu native classifier/error-formatting/DOM/parser materialization kanıtı değildir. Bağımsız bounded static source review üretim cpp/hpp hash'lerine bağlıdır;full native parser/equivalence review false kalır.

051fc0 locale classification,005820 set-error implementation,004970 parse body/DOM,node virtual/native heap/copy/fill/free/global/vtable ABI/EH/concurrency açıktır. Tek parse-entry provider source bridge'i bu lower sınırları kapatmaz. Generic CRT production unchanged/paused;classifier gövdesi eklenmez.

Ortak build204 üretim nesnesi(200 C++/4 C),221 normal +19 terminal grup,94 socket grubu. Önceki656 kaynak/CRT byte olarak korunur;661 source snapshot girdisi.47 validation mirror çifti=46 historical+1current;92 historical config referansı.140 dependency hash;43 historical typed closure,2 semantic session adapter,1 ayrı allocation adapter. Bu sayılar DLL tamamlanma yüzdesi değildir. Full DLL/parser tamamlanmadı;toplam yüzde ölçülmedi.

Original DLL/oyun/network/IOCP yürütülmez;gerçek CFG/token/achievement içeriği okunmaz. Hash-only integrity ayrıdır;bilinen engelli version.dll/generated version DLL tekrar denenmez. Öncelik1→2→3→5→4;genel CRT aşama5'e kadar bekler.

Sıradaki N24:error005820 inventory/contracts incelemesi. Bu hazırlık henüz yeni normal gövde iddiası değildir.
