# B54 — Gerçek backend durumuna bağlı sahipli online factory

Bu modül, mevcut online kaynak algoritmalarını büyük bir `Effects` alt sağlayıcısı gerektirmeden birleştirir. `RecoveredEffects` içindeki on core işlem, beş platform sınırı, seçilmiş yedi version/object/query işlemi ve üç sahipli getter kopyası kullanılır. İki phase işlemi gerçek backend'in `Execution` ve `online_ready` alanlarına gider. Toplam 24 zorunlu Effects metodu ve üç owned-copy metodu vardır; yeni native fonksiyon gövdesi eklenmez.

Yeni `OnlineComponent`, mevcut `Effects` arayüzünü değiştirmeden durum kimliği kontrolü ve açık kapanış işlemi ekler. `Resources.online_factory`, legacy `online` ile karşılıklı dışlanır. Factory, configuration/runtime/Execution adresleri sabitlendikten sonra `bind_state` sırasında kurulur. Dönen bileşen önce sahiplenilir; aynı üç adrese bağlı olduğu doğrulanır. Mevcut legacy online yolu korunur.

`Options` gerçek OS/path/file/encoding/log/dialog yapraklarını, açık bir `CodeLease` ve zorunlu `foreign_quiescent` etkisini taşır. Testsiz bir shared token, modül kodunun gerçekten pinlendiğini kanıtlamaz. Seçilmiş Win64 çağrı şekilleri tam foreign C++ ABI, prototype veya EH eşdeğerliği değildir. Borrowed getter sonuçları tüketici kopyalayana kadar dış seri kullanım ister; owned-copy rotaları Binding rezervasyonunu kopya boyunca tutar.

Yeni sahipli kurulum için `install_online(Resources&, Options&)` ve Runtime oluştururken `bootstrap_runtime::make_runtime(..., resources, ...)` kullanılır. Installer yalnız boş online hedefine kurulur. `take_options` ve B53 `take_resources` tüketen transferleri callable donorlarını temizler; libc++ small-buffer function move'unun donor boşaltacağı varsayılmaz. Genel by-value/`make_unique` forwarding zincirlerine constructor-failure güvenliği genellenmez.

Yeni sahipli bileşim, delege edilen core/foreign işlem exception fırlatırsa muhafazakâr olarak Poisoned durumuna geçer ve retry'ı kapatır. Bağımsız Binding bazı senkron version/getter exception'larından sonra kullanılabilir kalsa da bu yeni façade aynı retry davranışını iddia etmez. Açık `filesystem_error_018130` hata üretme rotası ayrı tutulur. Exception/EH davranışının özgün DLL ile eşdeğerliği kanıtlanmış değildir.

Kapanış, gerçek source join ve exit cleanup sonrasında ilerler. Aynı Runtime'ın modül/nesne alanları sıfır olmalı; Binding boş, aktif olmayan ve zehirlenmemiş durumda olmalıdır. Ayrıca dış çağrı/job admission'ının durduğunu ve işlerin tamamlandığını bildiren zorunlu etki çağrılır. Bu etki açık bir host sözleşmesidir; boş snapshot tek başına global foreign finality değildir. `FreeLibrary` sınırının FALSE dönüşü gerçek unload başarısı sayılmaz. Unknown load/factory/destroy/free sonuçları için otomatik temizleme veya kör retry yapılmaz.

Geç kapanış etkisi, exit cleanup'ın daha önce yaptığı destroy/free işlemini geriye dönük güvenli hâle getirmez. `stop_sources` ve dış sahiplik sözleşmesi, foreign nesne kullanıcılarını `cleanup_0366f0` öncesinde durdurup tamamlamış olmalı; alternatif olarak gerçek foreign destructor kendi join sözleşmesini karşılamalıdır. Geç `foreign_quiescent` kontrolü kalan provider/code sahiplerinin bırakılmasına ilişkindir; otomatik foreign job iptali sağlamaz.

Başarılı close, çağrıları kapatır fakat sahipleri hemen bırakmaz. Diğer Resources ve opaque yıkıcıları bitene kadar online metadata ve yaprak sahipler korunur; sonra oluşturulduğu controller thread ve fiber üzerinde bağlı Retiring durumunda bırakılır. Kullanılmamış, gerçekten boş ve unpublished önekler ayrı değerlendirilir. İlk girdileri live/unknown olan başarısız kurulumlar için çağıran gerekli alias'ları tutmalıdır; hayali rollback sağlanmaz.

Kanıt kapsamı sentetik EXE fonksiyonları ve mevcut kaynak uygulamalarıdır. Özgün DLL/oyun yüklenmez veya çalıştırılmaz; gerçek cfg/token/profile/achievement içeriği ya da ağ kullanılmaz. Hızlı testlerde kullanılan HOST `Resources` tüketicileri taze derlenir; tüm tüketicilerin ortak yeniden derlemesi ayrıca doğrulanır.

Tam DLL hazırlığı açık kalır. Host setup thread uyuşmazlığı, dört gerçek startup milestone sonrası controller aktivasyonu, gözlenen hook disable/uninitialize sonuçları ve dış callback/thunk finality sıradaki somut konulardır. Kalıcı Errors ömrü, rawjob/kernel/foreign sahipliği, native ABI/EH ve loader-lock attach/detach de tamamlanmış sayılmaz. Toplam DLL yüzdesi ölçülmüş değildir.

[Kaynak kanıtı](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Kapsam sınırları](resources-audit.json)

## B54 ortak doğrulama

**69 çekirdek + 686 entegrasyon = 755 kontrol PASS.** Ortak build 274 nesne (270 C++ / 4 C), 337 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 939 kaynak aynı; 3 önceki dosya açık hash kaydıyla güncellendi; toplam 946 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
