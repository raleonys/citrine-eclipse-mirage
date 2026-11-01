# B47 — Controller sahipli bellek ve worker/join sınırı

Bellek sağlayıcısı controller üzerinde hazırlanır ve son sahipliği orada kalır. Aynı Domain'in gerçek NativeThreads worker'ı, bu belleği yalnızca bir kez alabilen kullanım hakkıyla Graph/Policy/server kurar. Önceki aynı-thread API korunur. Parent erişimleri worker/thread/fiber/PTD kimliğini yeniden denetler; alias shared_ptr ile sole-owner kontrolü aşılmaz.

Worker kaynakları açıkça drain edip kullanım hakkını iade eder. Controller, worker'ın bağımsız kopyalanmış gerçek thread handle'ını bekler; `Returned` durumu tek başına yeterli değildir. Thread ve FLS çıkışı tamamlandıktan sonra bellek sağlayıcısını controller bırakır. Timeout sahipliği korur; bilinmeyen veya terk edilen sonuç bileti zehirler. Graph/Policy/server referanslarının son yıkımı worker üzerinde, thread fonksiyonu dönmeden önce yapılmalıdır.

Bu paket uygulama/server bileşeninin iki aşamalı sınırını sınar. Tam SourceBackend/RuntimeOwner Resources bağlantısı, eşzamanlı online state politikası, gerçek stop/kernel/raw-job drain ve loader attach/detach uygulaması sonraki iştir. Yeniden derlenmiş tam DLL henüz üretilmedi. Yeni native gövde/ABI kapanışı yok; güvenilir toplam DLL yüzdesi hesaplanmıyor.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sahiplik ve sınırlar](resources-audit.json)

## B47 ortak doğrulama

**112 çekirdek + 343 entegrasyon = 455 kontrol PASS.** Ortak build 269 nesne (265 C++ / 4 C), 323 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 910 kaynak aynı; 8 önceki dosya açık hash kaydıyla güncellendi; toplam 922 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
