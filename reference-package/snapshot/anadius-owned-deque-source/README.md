# B42 — Somut yanıt deque büyütme

**Güncel kaynak adımı, henüz derlenmedi:** Canlı çocuk Parent işlemine bağlı
`JobPermit` ve açık yetkili Deque büyütme yolu eklendi. `JobHandoff.grow_output`
yalnız eşleşen oturum kuyruğunu mevcut026670 ile büyütür. Varsayılan worker ve
kapatma denetimleri korunur. Yeni test kaynakları çalıştırılmadı; ayrıntı:
`job-deque.md`. Aşağıdaki derleme sonuçları tarihsel kaynak sürümüne aittir.

`Profile`, kuyruk büyümesini artık uygulamadan beklenen Policy callback’i yerine `owned_deque::Deques` üzerinden mevcut `0x180026670` gövdesine bağlar. Eski override’lar derlenmeye devam eder; yeni yol bunları çağırmaz ve override gerektirmez.

Map allocation’ları Parent içinde gerçek C++ pointer array olarak başlar. Küçük/büyük native boyutlar ve 32-byte hizalama aynı allocation/header üzerinden sağlanır. Kopyalama, sıfırlama, eski map’i free etme, canlı map_size okuması ve yeni pointer yayın sırası kurtarılmış gövdede kalır.

Parent’ın canlı typed-map defteri tek yetkili kayıttır. Binding metadata const, pointer slotları algoritma için mutable’dır. Borrowed span her allocation/free denemesinde geçersiz sayılır. Normal grow dışındaki doğrudan session `026880` temizliği de map kaydını backing free callback’inden önce düşürür.

Provider bir Parent lease tutar. Profile harici sonlanma kanıtını alıp session/job kayıtlarını emekliye ayırdıktan sonra lease’i Runtime drain kontrolünden önce bırakır. Reentry/bilinmeyen prefix kalıcı durdurma getirir; source tekrar denenmez, tahmini rollback/free yapılmaz. Parent’ın bilinen retired-raw free retry’si, deque algoritmasını tekrar çalıştırma izni değildir. Retained descriptor bir stack lifetime pin değildir ve checkpoint capture retirement hatasında descriptor zaten bırakılmış olabilir; lease/map yükümlülüğü tutulur.

Bu aynı worker/fiber ve bütün Parent Operation kapsamındaki seçilmiş C++ sağlayıcıdır. `018460` terminali std::length_error uyumluluk payload’u kullanır; native allocator/class/EH ABI veya kernel/job sonlanma eşdeğerliği iddia edilmez.

Önceki 898 kaynağın 893’ü aynı kalır; beş dosya açık hash kaydıyla güncellenir, dört yeni dosya eklenir. Yeni native gövde ve ABI/provider kapanışı0. Üretim ortamı/zamanlama, diğer sağlayıcılar, tam Resources, gerçek asenkron kapanış ve loader bağlantıları açık; sonraki B43, öncelik1. Güvenilir toplam tamamlanma yüzdesi yoktur.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sınırlar](resources-audit.json)

## B42 ortak doğrulama

**159 çekirdek + 244 entegrasyon = 403 kontrol PASS.** Ortak build 264 nesne (260 C++ / 4 C), 313 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 893 kaynak aynı; 5 önceki dosya açık hash kaydıyla güncellendi; toplam 902 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
