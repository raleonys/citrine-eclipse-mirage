# B15 — Exact Runtime state ve native Context bağlama

B14'te açık kalan **moved RuntimeSeed / native Context state kimliği**, seçilen kaynak attachment yolunda çözüldü. Yeni optional factory gerçek ve sabit configuration/RuntimeSeed/Execution referanslarını alır. Owner aynı değerli bir original-slot kopyasını reddeder; Context map, config flag ve server-port referanslarını doğrudan gerçek depolamaya bağlar. Tam üretim callback policy factory'si ve otomatik dış quiescence sağlandığı iddia edilmez.

**34 çekirdek +30 entegrasyon =64 yeni kontrol PASS.** İki kendi terminal alt süreci core sayısına dahildir. Bağımsız reviewer `/root/b15_binding_audit`: **PASS**. Root testlerini source/EXE hashleriyle doğruladı, yeniden çalıştırmadı.5yeni dosya ve4eski kaynak değişikliği incelendi. Önceki775 kaynaktan771aynı; toplam780 kaynak. [Dört değişikliğin kayıtları](validation.json) ortak build sonrasında oluşur.

## Bağlama ve kapanış sırası

1. SourceBackend `bind_state`, seed final Impl depolamasına taşındıktan sonra unpublished Owner kurar. Factory callback yayınlamaz veya worker başlatmaz. Null/throw/wrong-array sonuçları publication olmadan reddedilir.
2. `between_workers`, host initialize/create/enable öncesinde native Context'i yayınlar. Repeated publication yeni binding üretmez.
3. B4 kapanışı source stop/join→exit cleanup drain→ayrı hook-disabled/no-new-native-admission kanıtı→in-flight kontrolü/native Binding.close→registry.finish sırasını izler.

**Boş exit kuyruğu cleanup kanıtı değildir.** Başarısız exit registration sonucu kaynakta yok sayılır ve hook cleanup hiç kaydedilmeyebilir. Böyle bir yolda Owner false döner; source davranışına uydurma disable/cleanup eklemez. Test kendi fakeengine'ini açıkça kapattıktan sonra retry yapılır. False/exception/Busy durumları Context/policy/state'i korur; tüketilmiş exit prefix'i yeniden çalışmaz.

`OriginalEffects::original_slots_identity()` salt okunur adres kontrolü sağlar. Owner factory sonucu `&view.runtime.originals` ile aynı diziye bağlı olmalıdır. Bu accessor layout/trampoline ABI değiştirmez. Resources'a appended std::function alanı ise **source ABI'sini büyütür: bütün nesneler yeniden derlenmelidir**. İlk quick denemede eski B14host binder nesnesinin karışması düzeltildi; current quick updated backend/factory ve değişmemiş host binder'ı da yeniden derler. Ortak build tüm üretim ve test birimlerini yeniden üretir.

## Kimlik kanıtı

Fake engine create, kendi SDK imzalı original RegClose fonksiyonumuzun adresini Runtime'ın final internal slotuna yazar. Fake enable sırasında actual rebuilt native Resolver/shim çağrılır ve OriginalEffects bu aynı slottan kendi original'ımıza gider. Native folder/process callback'leri sonradan değişen gerçek internal map'i; registry callback değişen registry_spoof flag'ini; connect callback değişen server_port'u görür. Own in-flight callback thread'i close'u Busy olarak tutar. Gerçek socket/registry/process veya hook kurulumu yapılmaz.

Testteki input_originals assertion'ı move öncesi **değer kopyası snapshot'ıdır**; moved-from belleğin değişmediğini kanıtlamaz. Exact-address karşılaştırması, copied-array rejection ve callback etkileri selected identity kanıtını verir. [Kanıt ve kaynak hashleri](source-evidence.json), [güncel gap durumu](native-context-gap.json).

Özgün binary sabit global slot/map adresleri kullanır; bu yeni factory özgün native virtual ABI olarak sunulmaz. Retained038970 bootstrap,00e310 host setup,038950/0102b0 cleanup ve00f5b0/00f3f0/00fc00/00fd00 callback kayıtları referans alınır. Tercih edilen orijinal VA'lar executable callback adresi olarak kullanılmaz; test original'ları yalnız kendi derlenmiş fonksiyonlarımızdır.

## Sahiplik ve açık sınırlar

Attachment provider'lardan önce yıkılır. Owner immovable'dır; effects/Context/seed sabit yaşar. Published Owner'ı kapatmadan yıkmak abort'tur. Native Errors, kapanış sonrasındaki late entry'ler dahil process-lifetime olmalıdır; native last_errors pointer'ı owner tarafından sahiplenilmez. External quiescence gerçek caller sözleşmesidir; active_calls snapshot veya lifetime token onun yerine geçmez.

Testler controlled engine/SDK/join/callback ve own typed shim/trampoline kullanır. Framework root Runtime path, failed registration, timeout/retry, quiescence false/throw/reentry ve terminal sınırlar doğrulanır. Gerçek hooks/native concurrent provider/foreign DLL/CRT/ABI/unload/PE eşdeğerliği kanıtlanmaz. Production callback allocation/locale/wide/log policies hâlâ factory girdileridir. [Resources denetimi](resources-audit.json).

Yeni canonical native gövde/kapanış0;487mapped/491eligible/124normalclaim/52tarihsel kapanış aynı. Toplam DLL yüzdesi türetilmez; tam DLL **BLOCKED**. Orijinal/rebuilt DLL veya gerçek kullanıcı CFG/token/profile/achievement içeriği çalıştırılmadı/okunmadı. AV engelli version girdileri yeniden denenmedi. B10 review borcu,7online foreign çağrı ve configuration/license boşlukları açık.

B16: exact-state Context için somut callback policy/handle-memory/byte-safe wide-operation sahipliği ve senkronizasyon. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B15 ortak doğrulama

**34 çekirdek + 30 entegrasyon = 64 kontrol PASS.** Ortak build 233 nesne (229 C++ / 4 C), 261 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 771 kaynak aynı; 4 önceki dosya açık hash kaydıyla güncellendi; toplam 780 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
