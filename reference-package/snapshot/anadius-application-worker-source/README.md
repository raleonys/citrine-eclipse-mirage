<!-- PENDING-SOURCE-CHANGES -->
2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı. 296 nesne, 356 test grubu ve 19 framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.
<!-- END-PENDING-SOURCE-CHANGES -->

# B48 — Gerçek Resources ile uygulama worker köprüsü

`Resources.application_worker` fabrikası, `SourceBackend.bind_state` içinde gerçek RuntimeOwner configuration/runtime/Execution referanslarıyla bir kez kurulur. Controller özel belleğin sahibi kalır. Gerçek Domain NativeThreads server worker'ı kendi Graph/Policy/Profile, kategori/epoch ve TLS kaynaklarını oluşturur, mevcut `server_start_029ea0` akışını çalıştırır ve bu sahipleri worker üzerinde yok eder. Çağıranın Graph veya context fabrikası sağlaması gerekmez; dar SDK, content/license ve lifecycle sağlayıcıları açık kalır. Eski `Resources.server` yolu korunur ve iki yol aynı anda kullanılamaz.

Seçili **host politikası**, server hazır olup accept işlemi başlatıldıktan sonra session dispatch'i gerçek online worker dönüşü ve host setup tamamlanmasına kadar bekletir. Hazır baytlarının önceden 1 olması tamamlanma kanıtı sayılmaz. Bekleme Graph, Parent veya Execution kilitlerini tutmaz. Böylece normal startup'taki profil ve host alanı yazımları ilk dispatch'ten önce görünür olur. Bu sıralama özgün eşzamanlı startup davranışı veya genel shared-global senkronizasyon eşdeğerliği değildir. `configure`, yalnızca sabit ayarlar ve dar sağlayıcılar kurmalı; yakalanmış mutable startup hücrelerini okumamalı veya değiştirmemelidir.

AcceptEx zaten başlatılmışken stop, failure veya timeout başarılı drain sayılmaz: Graph/IO/bellek sahipliği korunur ve kapanış false kalır. İlk server-ready öncesindeki hatalar, eski online/library readiness beklemelerini açık bırakabilir; bütün bootstrap hata yollarının sonlanması kanıtlanmış değildir. Online worker'ın dışarı taşan hataları eski exception/terminate sınırını korur. Somut Worker'ın `noexcept` retention politikası özgün native EH eşdeğerliği iddiası taşımaz.

Normal kapanışta bütün Graph/sağlayıcı/TLS sahipleri worker üzerinde bırakılır. SourceBackend stop sinyalini join'den önce verir; tüm TrackingThreads ve rota lease'leri bittikten sonra ayrı bir controller finalization adımı gerçek worker/FLS çıkışını doğrular ve bellek sağlayıcısını bırakır. Join ile finalization retry'ları ayrıdır. Worker ve SourceBackend, opaque yıkıcılar çalışırken facade/metadata canlı kalacak şekilde açık Retiring mühürleri kullanır. Varsayılan Resources, bilinen boş JoinApi ile başlar; sıfır timeout yalnızca host tarafındaki nonblocking retry seçimidir.

Testler oluşturduğumuz sentetik program, heap/FLS/TLS/IOCP ve enjekte edilmiş SDK paketleri üzerinde çalışır. Terminal 995 paketinin durumu seçili test sınırıdır; gerçek ağ/kernel üretim drain sağlayıcısının yerini tutmaz. Şifreli peer örneği seçili hazırlanmış AES yoludur; tam özgün handshake kanıtı değildir.

Tam DLL henüz üretilmedi. Sonraki iş gerçek stop/cancel ve son-paket drain sağlayıcıları, kalan Resources ve loader attach/detach bağlantılarıdır. Yeni native gövde veya ABI kapanışı eklenmedi; güvenilir toplam DLL yüzdesi yoktur.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sınırlar](resources-audit.json)

## B48 ortak doğrulama

**162 çekirdek + 353 entegrasyon = 515 kontrol PASS.** Ortak build 270 nesne (266 C++ / 4 C), 325 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 920 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 926 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
