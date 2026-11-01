# B44 — Somut inline işler ve SDK varsayılanları

Profile’ın Jobs/receive/send dependency’leri boş olduğunda artık somut sağlayıcılar seçilir. Jobs için `InlineJobs`, taşıma için mevcut `NativeAsyncReceiveApi`/`NativeAsyncSendApi` kullanılır. SDK importları typed konfigürasyonla verilebilir; constructor hiçbir SDK çağrısı yapmaz. Açıkça verilen eski provider’lar korunur.

InlineJobs, bound worker ve bütün Parent Operation altında runner’ı ticket dönmeden önce tam bir kez çalıştırır. İş slotu generation’ı artmak zorundadır; source-only handle/id çiftleri global benzersizdir ve OS HANDLE/thread ID değildir. Nonreentrant policy nested start/detach/close girişini kalıcı hata yapar; metadata snapshot serbesttir.

Incoming callable ve moved-from donor active seal altında tutulur. Completed kayıt, normal start dönüşü ve exact detach olmadan tekrar kullanılmaz. Callable target metadata/gate canlıyken, mutex dışında emekliye ayrılır. Bilinmeyen runner/probe/retirement sonucu source tekrar çalıştırılmaz; callback/kayıt elde kaldığı ölçüde ve Parent lease tutulur. Profile’ın mevcut extra-strong/tombstone kuralı ikinci kez uygulanmaz.

Bind, tamamlanmış pending Application eldeyken yapılır. Close yalnız ticket/call sayısından türetilmez: dış drain kanıtı ve gerçek worker-return/Parent/IO/Accept/work/queue koşulları gerekir. Jobs lease’i Env/Deque ve Runtime drain2 kontrolünden önce bırakılır. Bu seçilmiş source execution politikasıdır; native `05e8e0`/`04a458`, detached CRT thread, foreign ABI/EH/concurrency eşdeğerliği değildir.

Önceki 906 kaynağın 903’ü aynı; Profile.hpp/.cpp ve geçersiz bağımlılık beklentilerini güncelleyen profile_tests.cpp açık hash kaydıyla güncellenir ve dört yeni dosya eklenir. Yeni native gövde/ABI kapanışı0. Hata/log/worker ve uygulama domain/config/event politikaları, tam Resources, gerçek asenkron kapanış ve loader bağlantıları açık. Sonraki B45, öncelik1; genericCRT ertelemesi korunur. Güvenilir toplam DLL yüzdesi yoktur.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sınırlar](resources-audit.json)

## B44 ortak doğrulama

**230 çekirdek + 298 entegrasyon = 528 kontrol PASS.** Ortak build 266 nesne (262 C++ / 4 C), 317 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 903 kaynak aynı; 3 önceki dosya açık hash kaydıyla güncellendi; toplam 910 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
