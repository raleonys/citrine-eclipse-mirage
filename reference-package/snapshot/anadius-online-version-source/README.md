# B25 — Online version_check için sahipli çağrı

OnlinePlatform.Resources içindeki optional version binding, seçili **version_check** yolunu somut typed çağrıya bağlar. Aynı platformun load_copy sonucunu kaydeder; yalnız exact module üzerinde çözülen nonzero version_check adresini ve açık CodeLease sahibini kabul eder. Farklı veya o anda bağlı olmayan procedure çağrılmaz. Seçili Windows x64 çağrı şekli int32_t(*)()’dır; bu foreign DLL’in tam formal prototipinin kanıtı değildir.

**Seçili yolda1/7 foreign çağrı bağlandı; kalan6 çağrı zorunlu alt sağlayıcıdadır.** Optional seçim yapılmayan legacy yolda yedi forward korunur. Yeni Resources düzeni bütün tüketicilerin yeniden derlenmesini gerektirir.

## Çağrı, sahiplik ve retirement

Load/resolve/call/free işlemleri kısa metadata kilidinde rezervasyon alır; lower loader/lookup, lease factory ve version export çağrılarında kilit tutulmaz. Aktifken aynı/başka thread’in managed load/rebind/call/free isteği lower etki başlamadan reddedilir; metadata snapshot okunabilir. CodeLease, çağrılabilir kodu gerçekten yaşamda tutmalıdır; keyfi nonnull token modül pin veya adres provenance kanıtı değildir. Raw handle/address ABA, bağımsız dış unload ve foreign reentry’nin bütünü desteklenmiş sayılmaz.

Lookup eski callable kaydını önce kapatır. Eski code lease ve factory capture’ları kilit dışında, **active rezervasyonu hâlâ tutulurken** bırakılır. Böylece deleter yeniden girse bile yeni lookup/free veya yeni version çağrısı başlayamaz. Binding destructor da factory capture’ını metadata kilidi hâlâ yaşarken bırakır; aktif managed çağrı sırasında destruction desteklenmez. Raw module veya foreign object için örtük RAII cleanup yoktur.

Normal lookup0 veya lease başarısızlığı, yüklenmiş module prefix’ini korur ve callable kayıt açmaz. Lease bookkeeping, lookup’ın WinLastError değerini korur. Synchronous version exception’ı aynı exception_ptr/error ve yazılmış source prefix’iyle çıkar; binding yaşamda kalır ve otomatik retry yapılmaz.

## Cleanup ve failure prefixleri

Free, procedure’u lower çağrıdan **önce** geçersiz kılar; code sahibi lower çağrı boyunca yaşar. Normal TRUE veya FALSE dönüşte tracked module sıfırlanır ve kendi lease referansı bırakılır. Bu, değişmeyen0366f0’ın normal dönüşte module=0 davranışıdır; FALSE unload başarısı değildir. Lower free exception’ı ise önceki object=0 prefix’ini, nonzero module’ü ve code sahibini tutar; binding poison olur, lower free kör tekrarlanmaz. Unknown load exception’ı da managed reuse’u durdurur.

035bd0/035b60/0366f0 source algoritmaları değiştirilmedi. Destroy exception’ı object/module’ü bırakır ve FreeLibrary’ye geçmez. Bu nesne ömrü hâlâ kalan foreign sağlayıcının sorumluluğundadır.

## Test ve kanıt

**50 core + 44 integration = 94 yeni kontrol PASS.** Core gerçek own typed zero-input fonksiyonda düşük32 bit sonucunu, incoming/outgoing WinLastError’ı, exact module/procedure retlerini, blocked SDK overlap ve code/factory deleter retirement reentry’sini doğrular. Dört managed etki, retirement sırasında sıfır lower çağrıyla reddedilir; snapshot kilit dışında çalışır.

Entegrasyon gerçek recovered worker/load/cleanup zincirini kendi sentetik SystemCalls/export/UI ve bellek içi dosyalarıyla çalıştırır. Typed version2 sonucu ardından kalan altı foreign çağrının sırası, version3 mismatch dialog/log sırası, missing version/factory, lease throw/null, version exception, destroy exception, FreeLibrary FALSE ve unknown load/free exception prefixleri doğrulanır. Lease factory hata kodunu değiştirse bile typed version, lookup’ın outgoing WinLastError’ını alır. Hedef DLL/game veya gerçek online DLL çalıştırılmaz; actual LoadLibrary/FreeLibrary ve gerçek user profile I/O yapılmaz.

Assembly:0x180035e8c CALL RAX,0x180035e8e CMP EAX,2. Kaynak source-evidence.json içinde retained function VA/Evidence ID/line/hash ve bütün yedi foreign callsite envanterine bağlıdır. Yeni source binding, yeni recovered native fonksiyon diye sayılmaz. B22 REA6.5.0/Ghidra12.1.4 oturum kimliği retained kullanılır; B25’te target açılmadı veya yeni original byte/decompile sorgusu yapılmadı.

## Açık kalan kapsam

B18–B24 callback PTD/admission, aynı Execution, shared stream, compiler locale ve original SDK sınırları değişmedi. Foreign factory/object lifecycle ve5virtual çağrı, gerçek module pin/adres provenance, foreign tam prototip/EH/ABI, configuration/license/legacy document, B10 review borcu ve nativeCRT/unload/PE kapıları açık kalır.

Original/rebuilt DLL/game değiştirilmedi veya çalıştırılmadı; gerçek CFG/token/profile/achievement içeriği okunmadı, AV engelli öğeler yeniden denenmedi. Yeni native gövde/kapanış0;487mapped/491eligible/124normal claim/52tarihsel kapanış korunur. Tam DLL hazırlığı BLOCKED; toplam DLL yüzdesi bu bileşimden türetilmez.

Sonraki B26: create_online factory ve nesne ömrü. İstenen sıra1→2→3→5→4; genel CRT kuyruğu bekler.

## Foreign çağrı envanteri

| Sınır | Çağrı VA | Durum |
|---|---|---|
| version_check | `0x180035e8c`, `0x180035e8e` | Seçili typed source binding |
| create_online | `0x180035fac` | Zorunlu lower / açık |
| status_virtual_08 | `0x180036002` | Zorunlu lower / açık |
| username_virtual_10 | `0x180036039` | Zorunlu lower / açık |
| persona_virtual_18 | `0x18003606c` | Zorunlu lower / açık |
| user_id_virtual_20 | `0x180036098` | Zorunlu lower / açık |
| destroy_virtual_00 | `0x180035b8c`, `0x180036703` | Zorunlu lower / açık |

## B25 ortak doğrulama

**50 çekirdek + 44 entegrasyon = 94 kontrol PASS.** Ortak build 243 nesne (239 C++ / 4 C), 281 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 820 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 826 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
