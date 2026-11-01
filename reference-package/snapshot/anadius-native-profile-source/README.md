# B41 — Canlı native mesaj yolu

`native_profile::Logic`, mevcut Profile üzerinden kurtarılmış `033d70` gövdesini canlı request/output prefixlerine bağlar. Hex çözümü, version onarımı, AES, string büyütme ve event yolu mevcut typed kaynakları kullanır. Session’ın kendi cipher/flag durumu paylaşılır; bu yolda bütün mesajı std::string kopyasında işleyip sonunda geri yazma yoktur. Eski SessionLogic yolu korunur.

Greeting ve iç içe XML/document/node/printer/cipher/random/effects çağrıları giriş/çıkış kontrollerinden geçer. Parent bölgeleri allocation/free ve callback dönüşlerinde yenilenir; yerel ve event SSO bölgeleri çakışmasız eklenir. Büyük allocation header kayıtları mevcut Strings ile paylaşılır.

Bilinmeyen prefix veya yutulmuş iç hata kalıcı durdurma oluşturur. Invocation tekrar çalıştırılmaz; tahmini unlock/pop/free/rollback yapılmaz. Greeting scratch invocation’a aittir. Mesaj exception’ından sonra retained descriptor içindeki dış prefix referansları artık geçersiz olabilir; bunlar okunmaz ve stack ömrünü uzattığı iddia edilmez.

**Seçilen sağlayıcı sınırı:** XML/DOM/printer/registry/032d00, mevcut C++ semantik sağlayıcılarıdır; native sınıf ABI’si değildir. Dönen opaque nesneler, wrapper unwind’i sırasında silinmez; bilinen normal invocation sonunda kontrol altında bırakılır. Bu ömür uzatma açık bir host politikasıdır, native destructor/EH eşdeğerliği değildir. Event queue/mutex ve uygulama sağlayıcılarının canlı backing’i hâlâ zorunludur.

Profile.cpp typed route/greeting seçimi, Strings.hpp parent kimliği ve Router.hpp/.cpp guarded serialization/node erişimi olmak üzere dört önceki kaynak açık hash kaydıyla güncellenir. Router’ın gerçek MemoryPrinter/Element sözleşmesi guarded proxy üzerinden çalışır; eski somut yol da korunur. Router’ın altındaki uygulama Effects iç davranışları ayrıca seçilen sağlayıcının sözleşmesidir. Yeni native gövde veya ABI/provider kapanışı yoktur. DLL linki ve gerçek kernel/job finality, tam Resources ve loader bağlantıları ayrı engellerdir. Sonraki B42 bunları önceliklendirir; CRT ertelemesi ve 1–2–3–5–4 sırası korunur. Güvenilir toplam tamamlanma yüzdesi yoktur.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sağlayıcı sınırları](resources-audit.json)

## B41 ortak doğrulama

**333 çekirdek + 99 entegrasyon = 432 kontrol PASS.** Ortak build 263 nesne (259 C++ / 4 C), 311 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 890 kaynak aynı; 4 önceki dosya açık hash kaydıyla güncellendi; toplam 898 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
