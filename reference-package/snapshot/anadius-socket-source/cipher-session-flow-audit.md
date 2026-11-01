# N12 — Owned cipher initialization/key/IV/reset ve semantic session bağı

25 Ekim 2026. `cipher_session.hpp/cpp`0423d0/042760/042780/02bc70 dört normal kontrol projection'ı ile SessionCipher/Factory kaynak adapter'lerini ekler. Tek owned AES schedule, live mode/IV ve c4/c5 bayrakları vardır. `PrefixCipherBinding` aynı `SessionCipher::native()` nesnesini seçer; semantic ChallengeResponse da aynı SessionCipher arayüzünü kullanır. N11'in separate caller-owned key/state tutarlılığı burada seçilen owned session zincirinde source-bound olur. Native object materialization/global lifecycle/ABI tamamlanmaz.

## Kimlik ve adresli kanıt

REA6.5.0 server/catalog hizası ve kullanılabilir Ghidra sağlayıcısı `cipher-session-rea-session.json` ile doğrulandı. Hedef açılmadı/yürütülmedi; Ghidra12.1.4 retained listing hashleri kontrol edilerek kullanıldı. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Dört hedef ve on yeniden kurtarılmış sayılmayan context işlevinin tam bölümleri evidence sidecar'larındadır; bütün ID/hashler `cipher-session-evidence.json` içindedir.

| Native adres | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|0423d0|`ev_1a5eecedd9b2929ae9f1e885705987927e1309cc6991de8c01925d839d99ad34`|`ev_5faa15cdd4a0e251b11f4a4b841a7d36cb933d79a06a0a53b3bd926fb86c2613`|
|042760|`ev_997fe5368937536a0756fb50ee6f4d70854abb04ae62c70396ae16dedd5bbafd`|`ev_b7191ccefe4f94753ba7877dd859eb4a7530a22a4d086b700584ccf2da34de7e`|
|042780|`ev_0c01cb3386957026020066a6feefbf7a41b6111237a7001393542ccc8e6732b7`|`ev_2d855481c04f0ce9821269ccf7945fb9c438f9de1000b64bfab4cca8c180c6ce`|
|02bc70|`ev_4fa4de12d674eff8476f9cf6f72bdef77e52de8ef76d04cd9a0e596bd21a0db1`|`ev_d6716c5d525f5e8e2eb566ef802896e45423abc1be5b6a78f2da5f052740c118`|

Instruction adreslerinin image prefix'i0x180'dir.0423d0 önce mode0 yazar, stack'te192 sıfır bayt oluşturur ve context+4'e aktarır:176 schedule +16 IV. c4/c5 bu aralığın dışındadır. Yeni owned projection mevcut AES schedule reset kaynağını ve live IV sıfırlamasını kullanır; native stack/CRT memset fault prefix'i yeniden kurulmaz.

04276d→045650 ve045659→045ce0: anahtar genişletilir;04565e/045661 IV16'yı expansion sonrasında okur/kopyalar;042772 ancak sonra mode1 yazar.04278d→045640 thunk→045ce0 ve042792 mode2: ECB anahtar değişimi IV'ye dokunmaz. Existing AES expansion source yeniden kullanılır;045640/045650/045ce0 bu pakette yeni normal gövde sayılmaz.

02bc79→0423d0,02bc7e receiver return identity,02bc81 word[c4]=0: yeni reset normal gövdesi context temizliğinden sonra encrypted/unknown bayraklarını sıfırlar. C++ projection'ın iki byte store'u native concurrent word-store/ABI eşdeğerliği sayılmaz. Key/IV array'lerinin expanded schedule'a arbitrary alias olması veya native partial fault/SEH bu domain dışında kalır.

## Permanent semantic kaynak adapter'leri

SessionCipher, mevcut `messages_recovered::Cipher` arayüzünün init/fixed-key/key/encrypt/decrypt metotlarını aynı owned materyale bağlar. SessionCipherFactory, mevcut semantic ChallengeResponse için her seferinde yeni unkeyed SessionCipher sağlar. Eski crypto/messaging kaynakları değiştirilmedi. Native request/response `PrefixCipherContext` doğrudan aynı owned state'i kullanır; burada std::string staging yoktur.

Semantic helper'ın yerel std::string'i için borrowed host buffer projection kullanılır: host string'in yaşayabilen char aralığı önceden genişletilir, logical native size eski boyut olarak kalır, heap-tag prefix host data'ya işaret eder. Yeni042480/042610 ve gerçek ECB/CBC gövdeleri bu buffer üzerinde çalışır. Yeterli reserve nedeniyle native grow/free burada beklenmez; beklenmeyen allocation/free açık UnsupportedInput sınırıdır. Mode0 host string'i okumadan/reserve etmeden false döner. Native bool false da prefix'te kalan plaintext/padding uzunluğunu host string'e aktarır.

Adapter catch bloğu yalnız prefix logical length'i host string'e eşler; payload/IV/key geri alınmaz. Bu, native EH cleanup/rollback kurtarımı değildir. Gerçek request prefix'i bu adapter'a girmez. Host allocation/capacity/SSO ve callback-intermediate host string size native ABI sayılmaz; uyumlu borrowed local-string domain'idir. Existing Cipher sınıfının private mode/IV bookkeeping'i native alan olarak kullanılmaz; block kaynakları yalnız onun176-byte schedule'ını okur, tek aktif CBC IV `native().iv_b4` alanıdır.

İki semantic kaynak adapter'i ayrı registry'de tutulur. Önceki23 typed native provider closure sayısı korunur; iki yeni adapter, native heap/vtable ABI kapanışı veya iki yeni native fonksiyon diye sayılmaz. Dört kontrol adresi önceden semantic kaynakta eşliydi; normal kaynak eşlemesi454 olarak kalır. Explicit normal-body claims86→90 olur. Fixed-key084c98 bir data adresidir; yeni canonical fonksiyon sayılmaz.

BorrowedApi'nin allocation/free/terminal metotları açık unsupported projection sınırlarıdır. Mapper bu dosya/metot kapsamını `bounded_projection_boundary_definition` diye etiketler;04b0f8/00ce80/00cd30 yalnız bu tanımlara sahip olduğundan `bounded_projection_boundary_only` durumundadır. Native gövde veya kapanmış heap sağlayıcısı sayılmazlar. Raw compiled address correlation455→458 artışı yalnız bu üç interface sembolünden gelir.04b0bc/05c974 için önceden var olan gerçek kaynak tanımları korunur. Böylece kaynak varlığı/compiled signature gözlemi ile native yordam kurtarımı ayrılır.

## Doğrulama

**716 çekirdek +6.622 entegrasyon =7.338 yeni kontrol PASS.** Ortak build193 üretim nesnesi (189 C++/4 C),199 normal +19 terminal grup,72 socket grubudur. Beş yeni kaynak/test dosyasıyla606 snapshot girdisi; önceki601 kaynak/CRT byte olarak korunur. Ortak build son otoritedir; socket standalone scripti ve CMake ayrıca çalıştırılmış sayılmaz. Yerel/common kayıtlar aynı build hashine bağlıdır.

Çekirdek64 key/IV örneğinde176-byte schedule'ı mevcut crypto oracle ile karşılaştırır. CBC key/IV, ECB IV preservation, context init'in bayrakları koruması ve session reset'in sıfırlaması sınanır. Sabit bilinen AES cevabı, binary local-string0–79 boyutları, CBC/ECB roundtrip, invalid padding plaintext mutation, empty bounded failure length eşlemesi, mode0 no-reserve ve unknown-mode padding-before-false kontrol edilir. İki session/factory temporary isolation ve native prefix→semantic local kesintisiz IV zinciri testlidir.

Entegrasyon mevcut032d00/02d7e0 semantic alt gövdesini gerçek XML compatibility reader ile çağırır; native032d00/02d7e0 XML object/vtable ABI'si kurtarıldı sayılmaz. Version eksik/1/2/3 dalları, fixed veya034c30 response-derived key seçimi için independent reference kullanılır. Yeni factory'nin fixed-key cipher'ı gerçek typed prefix şifrelemesini kullanır; callback'in seçtiği session key, sonraki native API'nin schedule'ıyla byte olarak karşılaştırılır.

Her handshake ardından13 gerçek encrypted request/response, bir event ve150000bayt payload; borrowed/owned, auto-detect sonrası encrypted flag, deque büyümesi ve send/finish normal akışı sınanır. Reset/rekey, kuyruktaki eski ciphertext'i değiştirmeden sonraki yanıtı yeni anahtara geçirir. Eksik key, mevcut CBC mode/key/IV'yi değiştirmeden invalid-message yoluna gider; cleanup icat edilmez. N11'in CBC/ECB, normal/retry/autodetect, ertelenmiş task ve gerçek AES decrypt sonrası parser exception regresyonları yeni session adapter'leri üzerinden tekrar çalışır. Eski identity cipher ve split semantic cipher sayaçları sıfır kalır.

## Açık sınırlar

Seçilen context'te key/IV/session source coherence kurulmuştur; gerçek host session nesnesinin+f0/+c4 byte materialization'ı, aynı global graph'a init/destruction, allocator/native ABI/EH/SEH/concurrency ayrıca açıktır. Native document001830/004280/001e00, dispatch032d00, log/getter/mutex/terminal/heap sağlayıcıları tamamlanmadı. Generic CRT zero/COPY kaynakları yeniden kurtarılmadı. Gerçek DLL/oyun/network/thread/IOCP çalıştırılmaz. Aynı ajan incelemesidir; bağımsız final reviewer PASS yoktur.

Orijinaller değiştirilmez; gerçek CFG/token/achievement içeriği okunmaz. Antivirüsün engellediği orijinal version.dll okuması tekrar denenmez; güncel hash'i doğrulanmış sayılmaz. Eski üretilmiş version.dll ZIP dışında kalır; diğer dört girdinin hash'i paket kaydında ayrıca doğrulanır.

**N13:**032d00 dispatch normal akışı ve document/handler kaynak bağlantıları. Genel CRT aşama5'e ertelidir. Tam DLL henüz bitmedi; ölçülmüş toplam yüzde veya güvenilir bitiş tarihi yoktur.
