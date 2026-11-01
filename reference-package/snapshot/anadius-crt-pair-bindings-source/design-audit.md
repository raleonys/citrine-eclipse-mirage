# Typed 16-pair dispatch tasarım denetimi

Bu salt okunur tasarım denetimi mevcut initializer, pair-leaves, lowio, cache, locale, MB, argv, environment ve onexit kaynaklarına dayanır. Üretim dispatcher yazılmaz, özgün DLL çalıştırılmaz/değiştirilmez, yeni REA oturumu açılmaz. Önceki16-pair ledger tablosu aynı kalır; bu aşamada yeni modüller gerçek leaf/lowio core'larını sağlar. Yeni dispatcher **composition** sayılmalı, yeni özgün fonksiyon gövdesi sayılmamalıdır.

## Constructor'da doğrulanabilir koşullar

1. `&termination.encoded_callback_a4f10 == &encoded.a4f10`: değer eşitliği yeterli değildir, gerçek referans alias'ı istenir. Farklı hücreyi reddet.
2. `runtime.cookie == heap.cookie_0a1580 == termination.onexit.cookie_0a1580`: seçilmiş cookie aynı olmalı. Runtime cookie const'tur; cookie seçimi dispatcher kurulumundan önce bitmelidir. Bu eşitlik nesne ömrü boyunca caller sözleşmesi olmalı; constructor gelecekteki harici mutasyonu engelleyemez.
3. `&command_lines.narrow == &argv_cleanup.argv`: command-line publication ve cleanup aynı argv State'i kullanmalı. Ayrı explicit argv State alınıyorsa üç adres eşit olmalı. Aynı veri değerini taşıyan iki farklı State kabul edilmemeli.
4. Dispatcher explicit narrow-environment State de alıyorsa `&environment_cleanup.narrow == &startup_environment`. Yalnız cleanup State alıyorsa ikinci nesne uydurulmasın; StartupBridge kurulumunda aynı referans verilmesi caller sözleşmesi olarak yazılsın.
5. Dispatcher explicit application Table veya Entry State alıyorsa `&entry.onexit_table == &termination.onexit.application_0a47a8`. Pair15'in CRT ve CRT-quick tabloları `termination.onexit.crt_0a52a8` ve `termination.quick_table_a52c0` üzerinden doğrudan çağrılmalı; application-quick A47C0 ile karıştırılmamalı.
6. Eğer dispatcher caller'dan16pair backing span alıyorsa size16 kontrolü; storage ömrü/fixed extent/callback mutation izinleri açık olmalı. Kendi `std::array<Pair,16>` sahibi olursa borrowed mutable span döndürebilir; sahibi taşınmamalı veya span geçersizleşmesi belgelenmeli.

Constructor yalnız bu concrete reference/scalar denetimlerini yapsın. Native initialization, FLS getter, commandline acquisition, arbitrary Effects identity getter veya callback invocation ile “validation” yapmasın: lock/heap/FLS henüz kurulmamış olabilir ve provider çağrıları gözlenebilir etkilerdir. Provider içindeki gizli referansların aynı domain'i kullandığı constructor'dan kanıtlanamaz; bunlar borrowed bundle/caller sözleşmesidir.

## A5178: dürüst en küçük kapsam

`EncodedGlobals.a5178` ve `heap::State.encoded_handler_0a5178` mevcut API'de ayrı **by-value** hücrelerdir. Gerçek shared alias değildir. En küçük doğru aşama, dispatched0644b0 gerçek core'u çağırdıktan hemen sonra `heap.encoded_handler_0a5178 = encoded.a5178` şeklinde **initialization projection** yapmaktır. Core tüm beş encoded-null grubu başlatır; projection aynı initialization olayının heap consumer görünümünü günceller. Callback yeniden dispatch edilirse o yeni initialization olayında projection da yeniden yapılmalıdır.

Constructor'da encoded.a5178==cookie beklenmemeli: pair1 öncesi zero/default encoded State geçerlidir. Projection sonrası iki hücrenin gelecekte birinin değişmesi diğerini güncellemez. Bu aşama encoded new-handler mutation/setter emülasyonu veya bidirectional synchronization iddiası yapmamalıdır. Test iki nesnenin ayrı adres olduğunu ve yalnız initializer sonrası aynı bit değerini aldığını gösterebilir; “same global alias” testi diye sunulmamalıdır. Runtime handler mutation gerekiyorsa sonraki ayrı aşamada gerçek tek hücre provider/ref API değişimi gerekir. Diğer encoded consumer'lar a4f00/a4f20/a5a18 için de gölge value state'leri varsa aynı sınırlama geçerlidir; a4f10 için gerçek referans alias'ı zaten mümkündür ve zorunlu tutulmalıdır.

## Dispatch imzaları ve mutable tablo

- Opaque rebuilt IDs veya scoped enum-derived integer IDs kullan; original VA'lar hiçbir cast/call yoluna girmesin. Metadata VA→rebuilt ID eşlemesi ayrı tutulsun. Null entry0 kalsın.
- `invoke_boolean_initializer` yalnız pair initializer IDs'ini kabul etsin. Bool core'ları uint64 0/1'e dönebilir. Thunk init067538 gerçek uint64 döndürüyor; dönüş aynen geçirilsin ve generic walker AL sınasın. Tam uint64==1 kabul koşulu değildir.
- `invoke_uninitializer(Address,uint32 argument)` dönüşleri ignored. Thunks06755c `static_cast<uint8_t>(argument)` almalı:0x100 cleanup,0x101 skip.0644f0 gerçek leaf'i çağırmalı; leaf VCRT core'a **zorunlu0** gönderir, argument/outer terminating capture edilmez.
- Uninitializer-only IDs boolean domain'de veya initializer-only IDs uninit domain'de açık reject edilmelidir.006500 her iki signature domain'inde bilinçli success/no-op'tur. C/C++ void/int table invoke metodları bu modülün kapsamı değilse ayrı mandatory delegate veya açık reject kullan; yanlış imza cast'iyle çalıştırma.
- Pair array mutable kalır; walker her ulaşılan entry'yi yeniden okur. Callback'ler sonraki entry'yi başka **geçerli bound ID** veya0 yapabilir. Konstruktor16entry oluşturup sonradan snapshot liste üstünden dispatch etmesin. IDs sabit registry'den gelir; entry değişebilir.
- Normal reverse uninit!=0 olan her entry'yi çağırır. Rollback başarısız entry'yi dışlar; önceki entry'de init ve uninit birlikte nonnull şarttır. Outer terminating0645cc walker'ı bypass eder. Dispatcher bu politikaları yeniden yazmamalı; mevcut gerçek walker'ları kullanmalı.

## Gerçek core bağlantıları ve lifetime

Locale pair0 mevcut SourceEffects ve gerçek initialize_locale core'u; cache pair1 core+yalnız initialization projection; cache pair2 aynı runtime State+CacheEffects; locks pair4 aynı State/native bank; heap pair5 aynı shared State; pair6 aynı outer VCRT State/Effects; pair7 aynı ACRT PTD State/Effects; pair8 gerçek lowio lifecycle State/ArrayEffects; pair9 gerçek pair-leaves CommandLines+provider; pair10 gerçek MB core; pair12 gerçek argv cleanup; pair13 gerçek MB shutdown; pair14 gerçek environment cleanup; pair15 gerçek onexit initializer bağlanmalı. Callback gövdeleri provider stub'ıyla taklit edilmemeli.

Lowio alt ensure/inherited/standard provider'ları hâlâ mandatory'dir. Bounded state128slot/capacity, recursive lock7, gerçek64CS destructor provider'ı bu domain'de kalmalı. Native OS handle acquisition yapılmış gibi iddia edilmez. MB init/shutdown ayrı Effects tiplerindedir; ikisi aynı mutable current slot, initial identity, native counter registry ve heap domain'ine yönlenmeli. Private provider referansları constructor tarafından introspect edilemez; fixture bunu shared-state mutation üzerinden doğrulamalı.

Borrowed States/Effects/encoded hücreleri, pair array, commandline spans, cleanup views, locale identities ve native lock/counter nesneleri bütün dispatch/walker ve late FLS callbacks boyunca yaşamalı. ACRT ve VCRT PTD State'leri ayrı tutulmalı. Array/record/CS/C++ nesne ömürleri ve heap free/unbind dış provider'a aittir. İşlemler dışarıdan serialize edilmeli; dispatcher yeni lock/RAII/SEH/rollback politikası eklememeli. Native FLS callback provider'ları dispatcher'ın arkasında daha uzun yaşamayı gerektirebilir.

## Minimal anlamlı owned fixture

Constructor yanlış a4f10 ref, cookie mismatch ve farklı argv State'lerini provider çağrısı yapmadan reddetmeli. Gerçek16pair initialize/reverse/rollback, one-time a5178 projection,067538 yüksek cookie bitleri,06755c low-byte0/1/0x100/0x101,0644f0 forced0, bilinmeyen/wrong-domain IDs, sonraki entry mutation, null-init rollback skip ve shared heap/argv/environment/onexit identity etkileri kontrol edilmeli. Providers gerçek core'ların zorunlu dış sınırlarını modelleyebilir;14 veya başka bir sayı “native tam callback execution” iddiasına dönüşmemeli. C/C++ initializer tablo dispatcher'ı, tüm runtime providers, full CRT/DLL ABI ve SEH bu aşamanın dışında kalabilir; bunlar açıkça belirtilmelidir.
