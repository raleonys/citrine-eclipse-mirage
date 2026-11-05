<!-- PENDING-SOURCE-CHANGES -->
2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı. 296 nesne, 356 test grubu ve 19 framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.
<!-- END-PENDING-SOURCE-CHANGES -->

# B45 — Somut oturum hata, log ve worker politikası

**Güncel kaynak adımı, henüz derlenmedi:** Canlı çocuk Parent JobPermit ile
yalnız o thread'in owned exit-wait kayıtlarını temizleyen giriş ve tam
JobHandoff bağlantısı eklendi. Diğer Policy işlemleri asıl worker'a bağlıdır.
Yeni pozitif entegrasyon test kaynağı henüz yoktur; test çalıştırılmadı.
[Kaynak sınırları](job-exit-cleanup.md). Aşağıdaki PASS sonuçları tarihseldir.

`session_policy::Policy::create(Options)` ve `install(Profile::Dependencies&)`, caller tarafından yazılan hata/category/log/guard/worker sınıfları yerine somut uygulamaları bağlar. Üç kategori tokenı ayrı ve owner-pinned; error API mevcut typed NativeApi’dir. NativeApi constructor yapılandırılmış importları çağırmaz; dedicated exit registry kendi owned SDK depolamasını kurar. Export edilen error/category shared alias’ları Policy’yi tutar, self-cycle yaratmaz; SDK portu Server başlangıcı/kapanışı için bind öncesi ve source-close sonrası geçerlidir.

Hata metni Parent domain’inde gerçek SSO/heap prefix olarak kurulur. Log, exact current address/count ile eşleşen owned bounded message snapshot kullanır; stack SSO adresini kör dereference etmez. Prefix önce gerçekten yazılır; append binary byte/NUL sayısını, ardından locale widening/put/flush sırasını korur. Her stream primitive kısa shared lease alır; formatter boyunca lock yoktur. Text/locale/ostream seçimi compiler HOST politikasıdır; native MSVC ABI/literal eşdeğerliği iddiası değildir.

Scoped guard ilk-init/register/footer/fastpath uygular. Worker exception context’i exact registered zero pair ile sınırlıdır; native pending exception aktarımı/rethrow açık kalır. Stop actual uint32 atomiktir; wake, actual uint64 alanın high32’sini koruyan same-worker serialized low32 güncellemesidir. Native interlocked/concurrency iddiası yoktur. Mevcut04a648 hata tablosu ve gerçek owned ExitWaitRegistry cleanup kullanılır; bootstrap/compilerCRT’nin alakasız kayıtları paylaşılmaz.

Create/bind/close ve son destruction aynı original worker/fiber üzerinde, loader lock dışında olmalıdır; dedicated exit registry’nin controller kimliği budur. Normal çağrılar bu worker/fiber ve bütün Parent Operation gerektirir. Snapshot yalnız o worker/fiber üzerinde reentrant metadata’dır. Terminal çağrılar Profile’ın kasıtlı poison’ından sonra ordinary Profile checkpoint’i tekrar çağırmadan intended compiler exception’ı üretir. Bilinmeyen prefix’ler fail-closed tutulur; otomatik rollback/retry yoktur.

Gözlenen completion owner seçimi, after-completion, worker-return ve dış drain kararları dört açık REQUIRED lifecycle callback olarak kalır. Sayaçlar kapanış kanıtı değildir. Profile tamamlanmış pending Application’ı tutup policy’yi bind eder; Jobs/Env/Deque’den sonra gerçek kaynak drain ve dış attestation ile Parent lease’i kapatır. Tam Resources/domain/config/event ve loader bağlantıları açık. Sonraki B46, öncelik1; genericCRT ertelenmiş durumda. Güvenilir toplam DLL yüzdesi yoktur.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sınırlar](resources-audit.json)

## B45 ortak doğrulama

**366 çekirdek + 306 entegrasyon = 672 kontrol PASS.** Ortak build 267 nesne (263 C++ / 4 C), 319 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 909 kaynak aynı; 1 önceki dosya açık hash kaydıyla güncellendi; toplam 914 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
