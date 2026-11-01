# B23 — Sahipli original callback sağlayıcısı

Seçili callback Policy artık original SDK slotlarını **exact son RuntimeSeed dizisine bağlı bir OriginalProvider** üzerinden okuyabilir. `resources.originals = original_provider::make_factory(code_owner)` seçimi, provider’ı runtime taşındıktan sonra kurar. Base OriginalEffects aynı dizi kimliğini zorunlu tutar; eşit değerli farklı bir dizi reddedilir. Eski boş factory yolu korunur. Resources ve OriginalEffects düzeni değiştiği için bütün tüketiciler yeniden derlenmelidir.

`code_owner`, seçili code/trampoline alanını gerçekten yaşamda tutan caller sahibi olmalıdır. Kaynak katmanı DLL yüklemez veya modül pin etmez. Nonnull keyfi token, executable adres geçerliliği veya hook başarısı kanıtı değildir. HostDomain’in callback_lifetime nesnesiyle sahiplik döngüsü kurmak yerine ayrı engine/API/code sahibi kullanılmalıdır.

## Setup ve yayınlama

Created durumunda load reddedilir. Owner host_setup_begin önce admission’ı setup’a alır, sonra original provider setup’ı açar. Setup boyunca yalnız kurucu controller thread güncel nonzero slotu okuyabilir; böylece Context’in önce yayımlanması ve engine’in slotları sonra doldurması desteklenir. İlk Context.publish anında slotlar dondurulmaz.

Host_setup_complete, **foreign admission hazır olmadan önce** on slotun tamamını mühürler. Ready çağrısında seçili canlı değer, mühürlenmiş nonzero değerle eşleşmelidir. Gözlenen nonzero/zero slot değişimi sağlayıcıyı kalıcı Stopped durumuna geçirir; eski değeri geri yazmak çağrıyı açmaz. Eksik ve baştan boş slot çağrılamaz. Array’e kör onarım, kopya pointer adopt etme veya replacement çağrısı yoktur. Raw concurrent slot writes ve geçici değiştirip geri koyma tespiti desteklenmez.

Protokol tek kullanımlıdır; reset veya reseal yoktur. Remaining ready proof exception atarsa mühürleme geri alınmaz; proof düzeltildikten sonra mevcut ForeignDomain.activate_foreign yolu kullanılır. Seal, hook kurulumunun başarılı olduğunu iddia etmez.

## Sahiplik ve cleanup

OriginalProvider mutex’i yalnız kısa metadata/slot kontrolündedir; gerçek SDK çağrısına taşınmaz ve metadata WinLastError’ı korur. In-flight native Binding lease, Context/effects/provider yaşamını tutar. Başarısız pause veya false/throwing close, code sahibini bırakmaz. Provider yalnız admission pause başarılı ve native aktif sayısı sıfır olduktan sonra durdurulur. Başarılı Binding.close sonrası bile code sahibi Context ve kalan dış sahipler yok edilene kadar yaşar.

Moved-from std::function, küçük callable içindeki shared sahibi hâlâ tutabilir. Lifetime testleri son weak-expiration iddiasından önce bu kalan dış nesneleri açıkça bırakır. OriginalEffects destructor gövdesi original_effects.cpp içinde açık tanımlıdır.

## Başarısız hook prefix’i

Kurtarılmış046970/00f120 zincirinde engine create başarısız status ile dönerken original çıktı slotunu yine yazabilir. Yeni sağlayıcı bu prefix’i silmez. Nonzero mühürlenmiş slot, enable başarısı veya foreign hazır olma kanıtı değildir. Entegrasyon kontrollü create failure8 için nonzero prefix’i, enable yapılmamasını ve ayrı ready proof false iken foreign admission’ın reddini doğrular; sentetik prefix adresi çalıştırılmaz.

## Kanıt ve kapsam

Retained Ghidra kayıtları on callback’i,00e310 host setup,00f120 install,046970 lookup/create ve0102b0 cleanup adreslerini Evidence ID/line/hash ile bağlar. Original global slotları0x1800a3838–0x1800a3880: connect1, registry5, process2, folder2. source-evidence.json bu kanıtı yeni source tasarımından ayırır. B22 REA6.5.0/Ghidra12.1.4 oturum kimliği retained olarak kullanılır; B23 source-only, yeni original byte veya decompile sorgusu yoktur.

Core41 ve bağımsız yazarın integration24 kontrolü PASS. Gerçek own native shim/FLS/heap senaryoları; setup controller ve foreign seal sırası, exact SDK arguments/HRESULT/output/LastError, kilit dışında metadata erişimi, in-flight pause/close retention ve son owner yıkımını sınar. Own terminal çocuklar231warming/233changed/234zero için typed fault, sıfır stub/replacement çağrısı ve PTD/admission/native lease drain doğrular; generic abort239 veya unexpected return230 başarı değildir.

Original/rebuilt DLL/game çalıştırılmadı veya değiştirilmedi; gerçek kullanıcı CFG/token/profile/achievement içeriği okunmadı, AV engelli öğeler yeniden denenmedi. Yeni native gövde/kapanış0;487mapped/491eligible/124normal claim/52tarihsel kapanış korunur. Tam DLL hazırlığı BLOCKED; toplam DLL yüzdesi bu bileşimden türetilmez.

B18–B22 PTD/admission, aynı Execution, shared stream, owned compiler locale/encoding ve external quiescence şartları sürer. Original SDK alt çağrılarına yeni PTD pre/post kontrolü bu aşamada eklenmedi. Explicit engine uninitialize/unload tutulan code sahibiyle otomatik engellenmez. 7online foreign çağrı, configuration/license/legacy document, B10 review borcu ve nativeCRT/EH/ABI/unload/PE açık kalır.

Sonraki B24: original SDK çağrılarındaki seçili PTD alt-çağrı sınırları. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## Slot adresleri

| API | Callback VA | Original pointer VA |
|---|---|---|
| connect | `0x18000fd00` | `0x1800a3838` |
| RegOpenKeyExA | `0x18000f800` | `0x1800a3840` |
| RegOpenKeyExW | `0x18000f960` | `0x1800a3848` |
| RegCloseKey | `0x18000f5b0` | `0x1800a3850` |
| RegQueryValueExA | `0x18000fac0` | `0x1800a3858` |
| RegQueryValueExW | `0x18000fb60` | `0x1800a3860` |
| CreateProcessA | `0x18000f3f0` | `0x1800a3868` |
| CreateProcessW | `0x18000f4d0` | `0x1800a3870` |
| SHGetFolderPathA | `0x18000fc00` | `0x1800a3878` |
| SHGetFolderPathW | `0x18000fc80` | `0x1800a3880` |

VA/Evidence ID/line/hash ilişkileri source-evidence.json içindedir; callback gövdeleri yeni native kapanış olarak sayılmadı.

## B23 ortak doğrulama

**41 çekirdek + 24 entegrasyon = 65 kontrol PASS.** Ortak build 241 nesne (237 C++ / 4 C), 277 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 809 kaynak aynı; 5 önceki dosya açık hash kaydıyla güncellendi; toplam 818 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
