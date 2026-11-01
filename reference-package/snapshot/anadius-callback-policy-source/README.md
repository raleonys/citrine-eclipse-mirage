# B16 — Somut callback policy ve UTF-16 depolama köprüsü

`Policy`, B15’in gerçek Runtime state/original-slot dizisine bağlanan Context’i için somut `OriginalEffects` uygulamasıdır. Önceden dışarıda kalan **15 effect route** bağlandı; mevcut 10 typed original forwarder ve 2 wide route yeniden kullanılır. Bu sınıf yeni bir özgün native fonksiyon gövdesi olarak sayılmaz.

**52 çekirdek +34 entegrasyon =86 yeni kontrol PASS.** Bağımsız inceleme PASS; reviewer kaynak/EXE hashlerini kontrol etti, testleri tekrar çalıştırmadı. Önceki780 takipli kaynak aynı;5 yeni dosyayla toplam785. Ortak build kaydı final teslim adımında eklenir.

## Kurulan bağlar

- Dört baytlık fabricated key belleği, caller’ın aynı DomainStorage’ına ait `ApplicationMemory` üzerinden ayrılır. Policy kendi ticket’larını tutar; yabancı/çift free reddedilir. Başlangıç fabricated seti boş olmalıdır; eski native pointer’lar sahiplenilmez.
- Registry ve network beklemeleri gerçek borrowed Execution’a gider. Filesystem conversion mevcut explicit-length kodunu kullanır; embedded NUL, ANSI/OEM/UTF-8 seçimi ve hatalar korunur.
- Log VA’ları yalnız beş doğrulanmış literal için metne çevrilir. Network text/port/finish sırası compiler ostream ile uygulanır. Arbitrary VA okunmaz veya çağrılmaz.
- Dialog mevcut UTF-8/SDK adapter’ına, exit açıkça seçilen terminal fonksiyona gider. Fonksiyon dönerse terminate olur. Testler yalnız own fake UI ve own terminal alt süreç kullanır.
- `ByteWide`, caller’ın wchar_t/char16_t/hizasız depolamasına yalnız memcpy ile erişir. Compiler CRT karşılaştırma/kopyalama çağrıları için gerçek wchar_t nesneleri kurulur. Geçersiz kopyada actual output ilk code unit’i handler çağrısından önce temizlenir.

## Sahiplik ve kapanış

`Storage` base’i önce kurulur, sonra OriginalEffects onun wide adapter’ına bağlanır; ters yıkım kaynakları yaşatır. Policy immovable’dır. One-shot factory kopyaları aynı tüketim kaydını paylaşır. B15 `make_factory(seed, make_policy_factory(resources), permanent_errors, quiescence)` yoluna verilir. Constructor callback yayınlamaz.

Source close fabricated üyeliğini **free’den önce siler**. Known free failure policy/memory ownership’ünü korur, fakat kaynak setini geri eklemez. Testte açık policy.free retry belleği serbest bırakır; client tracking cleanup veya deleted log kendiliğinden tamamlanmaz. Live handle ile policy yıkımı terminaldir. Unknown allocator outcome ApplicationMemory’nin mevcut poisoned/ticket sözleşmesine tabidir.

Context’in kapanması için disabled-hooks/no-new-native-admission kanıtı hâlâ ayrı ve zorunludur. Boş handle listesi veya active_calls=0 bunun kanıtı değildir. Native Errors ve late-entry ömrü B15 sözleşmesiyle korunur.

## Kapsam sınırları

**Bütün callback ve ortak state/stream erişimi dışarıdan tek denetleyici thread üzerinde, callback reentry olmadan sıralanmalıdır.** Policy’nin15 override’ı başka thread’i reddeder; inherited final original/wide metotları bu kontrolü yapmaz. Bu kontroller bütün callback için race/admission guard değildir. ApplicationMemory’nin foreign host-thread PTD desteği yoktur; bu aşama bunu çözmüş sayılmaz.

Stream owner streambuf’ı da yaşatmalıdır. UTF-16 input tamamen okunabilir NUL-terminated string, output belirtilen capacity kadar yazılabilir storage olmalıdır. Capacity INT_MAX ile sınırlıdır; overlap domain_error ile reddedilir. Staging allocation/EH, compiler locale/invalid-handler/debug-fill ve özgün static CRT/ABI eşdeğerliği kurulmaz. Compiler CRT yalnız bu seçilen yeniden derlenmiş source domain’inin politikasıdır.

## Kanıt ve testler

`source-evidence.json` retained fonksiyon VA/Evidence ID/line/hash bağlantılarını içerir. REA6.5.0/Ghidra12.1.4 doğrulandı; anadius64.dll SHA-256 önceki ile aynı. Üç targeted statik sorguda144 byte gözlendi. Son literal’in eksik kuyruğu ayrı read-only PE VA→file mapping ile tamamlandı; beş complete NUL-terminated literal file evidence içinde tutulur. Payload’lar örtüşür; bunlar unique-function kapsamı değildir. Oturum kapatıldı.

Own SDK imzalı original fonksiyonlar actual rebuilt native shim→B15 Context→somut policy zincirinde kullanıldı. Gerçek wchar_t query/folder, UTF-16 surrogate/unaligned storage, capacity/error-prefix, fabricated open/query/close, allocator failure/retry, exact slot identity ve LastError doğrulandı. Testler own process heap/FLS storage ve own terminal children kullanır; gerçek registry/socket/process/hook/MessageBox/foreign DLL veya orijinal DLL/game çalıştırılmaz. Yeni entegrasyon testinin root’u B15 Owner’dır; B4 Runtime/PE birleşiminin tam üretim çalışması iddia edilmez.

Orijinal dosyalar değiştirilmedi. Gerçek kullanıcı CFG/token/profile/achievement içeriği okunmadı. AV engelli original/generated version.dll öğeleri yeniden denenmedi.

## İlerleme ve sonraki adım

Yeni canonical native gövde/kapanış **0**;487mapped/491eligible/124normalclaim/52tarihsel kapanış korunur. Bu source composition ilerlemesinden toplam DLL yüzdesi türetilmez. Tam DLL hazırlığı **BLOCKED**.7online foreign çağrı, configuration/license/legacy document ve B10 review borcu sürer.

**B17:** whole-callback admission/state synchronization, foreign-thread PTD sınırı ve Runtime lifecycle birleşimi. Wait/original-call/reentry altında kaba global kilit eklemek yerine kanıtlı kapsam kurulmalı. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B16 ortak doğrulama

**52 çekirdek + 34 entegrasyon = 86 kontrol PASS.** Ortak build 234 nesne (230 C++ / 4 C), 263 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 780 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 785 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
