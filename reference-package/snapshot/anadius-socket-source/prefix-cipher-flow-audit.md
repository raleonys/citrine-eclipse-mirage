# N11 — Live string prefix, padding ve AES/ECB/CBC kaynak akışı

25 Ekim 2026. `prefix_cipher.hpp/cpp`042480/042610 üst akışlarını ve045620/045630 ECB wrappers /0454e0/045580 CBC normal gövdelerini ekler. Altı normal gövde, altı final kaynak bağıdır: iki NativeMessageApi cipher metodu ve dört PrefixCipherBlocks metodu. Yeni AES round algoritması sayılmaz; önceki045670/0458d0 block kaynakları kullanılır. Önceki596 kaynak ve CRT korunur.

## Kimlik ve kanıt

REA6.5.0 server/catalog hizası ve kullanılabilir Ghidra sağlayıcısı `prefix-cipher-rea-session.json` ile doğrulandı. Hedef açılmadı/yürütülmedi; Ghidra12.1.4 retained listing SHA'ları kontrol edilerek kullanıldı. Bağlı anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Altı hedefin tam assembly/pseudocode bölümleri ve on context işlevi evidence sidecar'larında; ID/hash kayıtları `prefix-cipher-evidence.json` içindedir.

| Adres | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|042480|`ev_d0fccb2775fdb815d1a580c813946b1cdefdb26fb8590bbb2b2a03d7b562e8e4`|`ev_e27b923a47363280bcc0e8f0d5f1d84d309c13cbcf938d944e3bf4f18460fe56`|
|042610|`ev_46790ff61e14371172a73e53871ee23da992983094393729fe2474de81caee15`|`ev_84196222f2218d2b0a060bb317b12da12a77b2f964a5902100ed78e154eb8375`|
|045620|`ev_91602a126449722f4c42c22e772d9d2afdcaf65714aa5b88b89077d4bfdfca4f`|`ev_9c4ae0650cc1a802bb9b81a7408ddd7ccd1c43b7f78ee86dfaf95e0bcbb01969`|
|045630|`ev_aa0732814e7f08faa575592800a67bc9f2448f5e541cab2d933d9a7ad44b619e`|`ev_ce515a04c612b867ced7f7d2522cebb5b74ec98b99de8507bd0fe7aec9d70fa5`|
|0454e0|`ev_ec8e488c9470bfc5a91fa0eb4a8a7ce7b044f0c6f346c6515836286e71443ef3`|`ev_2f22af2c88e0c02db0783988bf6b8d67eb469b0f6802fb5ec641d0c0ad0d06f0`|
|045580|`ev_feb0127f5eb8c794b4ad41840bc3118c7f03d5d3a9188b8d0179c3c959bb262b`|`ev_d09002f392ba53373ac77b1c1cbf192b7033a4e4bfa093ba2c3bf57670891ee4`|

Tüm sayısal instruction adresleri `0x180` image prefix'i taşır. Native return için yalnız AL anlamlıdır; üst RAX baytları C++ bool sözleşmesine taşınmaz.

## Üst akışın adresli sırası

| Native aralık /callsite | Gözlem ve kaynak davranışı |
|---|---|
|04248a–0424b1|Decrypt mode cached; mode0 ve unsigned(size-offset)%16!=0 blok çağrısından önce false|
|0424c1–0424e5→045620|ECB her blokta live cap/data, callback sonrası live size; cached offset+16 ve cached mode kullanır|
|0424ee–042511→0454e0|CBC tek batch çağrısı, size-offset ham64bit; sonra size yeniden yüklenir|
|042515–042570|Tam string'in son signed byte'ı padding;1–16 ve tüm tail doğrulanır. Offset yalnız blok aralığını etkiler; padding tüm size üzerinden|
|042572–0425ef|Başarılı padding sonrasında size tekrar okunur ve inlined resize/NUL yapılır; shrink/growth sözleşmesi mevcut010040/008350 kaynağıyla birleşir|
|042629–042677|Encrypt mode0 önce false; padding=16-((size-offset)&15). Ham unsigned newsize wrap shrink/NUL yolunu korur|
|04268c–0426c6|Yeni size fill/growth öncesi; in-capacity0793d0 veya doğrudan008350, sonra cached tail NUL|
|0426d3–04271c|Mode padding etkilerinden sonra yeniden okunur. Geçersiz nonzero mode padding yapıp false dönebilir|
|0426f0–042713→045630|ECB loop live cap/data/size kullanır; seçilmiş mode blok callback'ları boyunca cached kalır|
|04271e–042736→045580|CBC live padded size-offset ile tek batch; offset prevalidation guard eklenmez|

Native üst gövdelerde010040'a CALL yoktur; yeni C++ kod aynı inlined resize sözleşmesini yeniden kullanır. Bellek taraması ile resize arasında dış callback yoktur; native cached-cap ile sequential typed projection eşdeğerliği bu domain'le sınırlıdır. Concurrency/fault eşdeğerliği iddia edilmez.

ECB045620→0458d0 ve045630→045670 register-argument sırasını mevcut block source metoduna bağlar. CBC0454e0'da045520 orijinal ciphertext16'yı XMM0'da saklar,045529 decrypt block,045540–04554d IV XOR,045553 IV'ye orijinal ciphertext store yapılır: **decrypt IV her blok sonunda değişir**. CBC045580'de0455d0–0455df IV/önceki ciphertext XOR,0455e7 encrypt block;0455fe/04560c son ciphertext'i IV'ye kopyalar: **encrypt IV batch sonunda değişir**. Count0 decrypt input okumaz; encrypt mevcut IV'yi kendisine kopyalar. Nonzero count için ceil(count/16) tam blok işlenir; aligned olmayan alt batch count sessizce normalize edilmez.

## Doğrulama

**5.039 çekirdek +4.224 entegrasyon =9.263 yeni kontrol PASS.** Ortak build192 üretim nesnesi (188 C++/4 C),197 normal +19 terminal grup,70 socket grubudur. Beş yeni kaynak/test dosyasıyla601 kaynak snapshot girdisi vardır; önceki596 kaynak byte olarak korunur. Ortak build son otoritedir; standalone script ve CMake ayrıca çalıştırılmış sayılmaz.

Çekirdek testler önceki fixture'lardaki dört-blok bilinen ECB/CBC cevaplarını, ek padding bloğunu ve tam byte sonuçlarını kontrol eder. Binary/embedded NUL,0–64/128/2047/4095/8192 payload, offset0/1/65, spare/growth ve multi-call CBC zinciri mevcut semantic crypto bağımsız orchestration oracle'ıyla karşılaştırılır. Yeni AES implementasyonu olmadığı için bu karşılaştırma ek bir AES implementation independence iddiası değildir; sabit bilinen cevaplar ayrıca AES sonucunu sınar.

Tüm256 padding byte değeri,16 geçerli padding uzunluğu ve her bozuk tail konumu denenir. Invalid padding plaintext değişimini ve CBC IV güncellemesini geri almaz. Mode0 eşlenmemiş prefix'i okumaz. Unknown mode encrypt padding sonrası false döner. Fill callback mode/size değişimi, ECB callback live pointer/size ve cached-mode ayrımı sınanır. Offset>size davranışına std::string guard eklenmez: native unsigned aligned extent ECB bloklarını atlayıp tam-string unpad yapabilir; encrypt offset padded size'ı aşıyorsa blok çağrısız true dönebilir.

Boş input native pre-base padding okumasına gider; bounded mapping eksikse UnsupportedInput olur. Mapped pre-base padding1, unsigned resize growth/length terminaline ulaşabilir. Symbolic size wrap metadata/NUL sırasını korur. Fill-before/after, small/aligned NULL ve block exception prefix'leri kontrol edilir. CBC count17 ve yalnız bir blok mapping ile ikinci blok failure'ında encrypt IV eski kalırken decrypt IV ilk ciphertext'e ilerler. Bunlar gerçek native page-fault/SEH eşdeğerliği değildir.

Entegrasyonda eski identity-cipher sağlayıcısı kullanılmaz. Gerçek042480/042610 ve dört ECB/CBC kaynak bağı, N10 append gövdeleriyle033d70'ye bağlanır. Ayrı peer oracle önce request'i, sonra response ve event'i **aynı sıradaki IV zinciriyle** hesaplar. Her request sonrası typed server IV ile peer IV karşılaştırılır. ECB/CBC × plain/explicit encrypted/version-retry/autodetect × borrowed/owned yollarında13 yanıt, deque0→8→16,150000bayt payload/event'in ayrı NUL'u, tam ciphertext ve64KiB send parçaları doğrulanır. Ertelenmiş task/copy/free ve gerçek AES decrypt sonrası parser exception'ında kalan plaintext de sınanır. Controlled cipher/growth substitute sayaçları sıfır kalır.

## Sayım ve açık sınırlar

042480/042610 eski semantic kaynakta zaten eşliydi; yeni typed body ve final caller binding önceki tanımları silmez. Dört alt adres önceden yalnız `module_record_only` idi; yeni kaynak/compiled eşlemesi bu dördü için artar. Anadius source mappings450→454; compiled canonical correlations451→455; explicit normal-body claims80→86. Altı kaynak bağı17→23 closure getirir: ikisi önceden açık NativeMessageApi sağlayıcısı, dördü bu aşamada tanımlanan block arayüzünün final kaynak seçimidir. Dört alt final metot aynı anda normal body/binding'dir ve iki farklı native fonksiyon diye sayılmaz. Bunlar DLL tamamlanma yüzdesi değildir.

PrefixCipherState, native object byte layout'u değildir. Aynı handler için coherent live mode/IV ve mevcut AES schedule ömrü integratör tarafından sağlanır. Testte sabit sentetik anahtarlar kullanılır.0423d0/042760/042780 initialization/key/IV session bağı ve ChallengeResponse/reset'in aynı schedule durumunu paylaşması **henüz kurulmadı**. Schedule'ın eski Cipher içindeki private IV'si kullanılmaz; yeni typed IV native+b4 alanını temsil eder. Tam session/native global provenance kapanmaz.

NativeMessageApi'nin tarihsel üçüncü uint32 alanı033d70'de daima0'dır; yeni final cipher bridge bunu offset0 olarak iletir. Doğrudan yeni gövdeler full uint64 offset taşır. Geniş offset ABI'si NativeMessageApi üzerinden iddia edilmez. XML/032d00/log/getter/mutex/heap/terminal sağlayıcıları ve native MSVC ABI/EH/SEH/concurrency açık kalır. Bounded pointer/memory domain, native arbitrary alias/fault veya XMM/stack ABI'si değildir. Catch/rollback/unwind/unlock icat edilmez. Orijinal DLL/oyun, gerçek network/thread/IOCP çalıştırılmaz. Aynı ajan incelemesidir; bağımsız final reviewer PASS yoktur.

Orijinaller değiştirilmez; gerçek CFG/token/achievement içeriği okunmaz. Antivirüsün daha önce engellediği orijinal version.dll tekrar okunmaz; güncel hash'i doğrulanmış sayılmaz. Eski üretilmiş version.dll paket dışında kalır. Diğer dört girdinin hash'i paket kaydında ayrıca doğrulanır.

**N12:**0423d0/042760/042780 init/key/IV normal kaynak ve aynı session cipher state bağı. Genel CRT aşama5'e ertelidir. Tam DLL henüz tamamlanmadı; toplam yüzde ve güvenilir bitiş tarihi ölçülmüş değildir.
