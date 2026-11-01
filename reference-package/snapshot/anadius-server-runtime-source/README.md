# B39 — Sahipli server Runtime ve kaynak fabrikası

`Runtime`, accept başlatma, gerçek `0298b0/027c10` worker zinciri, coherent completion ve önbellek temizliğini birleştirir. `make_server` / `install_server` yalnız `bootstrap_backend::Resources.server` bileşenini kurar; mevcut server üzerine yazmaz, worker veya soket başlatmaz.

Graph kalıcı owner içine önce kaydedilir, sonra aşamalı bağlanır. `ServerView` değer olarak kopyalanır; çağrı-local referans saklanmaz. Run-local admitted API, kalıcı proxy üzerinden yalnız run süresince kullanılır ve istisna yolunda da ayrılır. Bilinmeyen kısmi etkilerde kaynaklar tutulur; kanıtlanmamış geri alma uygulanmaz.

Harici worker/raw-job/kernel sonlanma kanıtından sonra Parent, IO, accept, kuyruk ve iş sayacı sıfır koşulları denetlenir. Accept kapanmadan bu koşullar sağlanmalıdır. Application ve graph açıkça emekli edilir; Base/Parent kapanışını B35 yapar. Runtime yok edilirken Retiring aşaması lifecycle girişlerini kapatır; sağlayıcılar Impl/kilit canlıyken kilit dışında bırakılır.

Uygulamanın frame/read/write/message bağlamları, sabit kapasiteli session kayıtları, ortam/exception/atomik politikaları ve harici sonlanma sağlayıcısı hâlâ gereklidir. N5 normal-return mesaj köprüsü native033d70 ara-callback/EH eşdeğerliği değildir. Gerçek kernel ağı ve özgün DLL/oyun çalıştırılmadı.

**Yeni native gövde ve ABI kapanışı yoktur.** Önceki 882 kaynak korunur. Bu kaynak bileşimi toplam DLL tamamlanma yüzdesi değildir; tam Resources ve loader bağlantıları açık kalır. Sonraki B40, öncelik1; kullanıcı sırası1–2–3–5–4 ve CRT ertelemesi korunur.

[Kanıt](evidence.json), [bağımsız inceleme](architecture-review.md), [kaynak sınırları](resources-audit.json).

## B39 ortak doğrulama

**184 çekirdek + 179 entegrasyon = 363 kontrol PASS.** Ortak build 260 nesne (256 C++ / 4 C), 307 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 882 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 888 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
