# B40 — Somut session profili ve kaynak grafiği

**Güncel kaynak adımı, henüz derlenmedi:** `JobHandoff` gerçek iş/oturum nesli,
closure ve kontrol bloğunu hazırlanmış Parent bellek yetkisine bağlar. Çocuk
worker kimliği denetleyebilir, açık izinle çıktı Deque kapasitesini büyütebilir
ve sınırlı Environment kopyasını oluşturup yok edebilir. Ayrıca somut Policy
üzerinden yalnız kendi thread'inin owned exit-wait kayıtlarını temizleyebilir;
mesaj yürütmez. Temizleme ayrıntısı:
`../anadius-session-policy-source/job-exit-cleanup.md`.
Açık kayıt eski runner'ı tutar. Environment ayrıntısı:
`../anadius-owned-environment-source/job-environment.md`.
Yeni preflight test kaynakları çalıştırılmadı; pozitif gerçek prepared-Profile
round trip hâlâ doğrulama bekliyor. Ayrıntı: `job-handoff.md`. Aşağıdaki sonuçlar
önceki kaynak sürümünün tarihsel doğrulamasıdır.

`Profile` ve `Application`, frame/read/write/response/message bağlamlarını ve sabit kapasiteli session kayıtlarını kurar. Çağıran kod tüm bağlam grafiğini kurmaz; mesaj mantığı, SDK, ortam/scheduler, hata ve harici sonlanma alt sağlayıcılarını verir.

Her session ayrı slot, generation, logic, IO ve environment kaydına sahiptir. Yalnız aynı domain içindeki gerçek factory owner/control çifti kaydedilir. Slot temizliğinde tüm bindingler ve owner’a referans veren logic nesnesi silindikten sonra observer weak bırakılır. Ertelenmiş iş Policy hook içinde bitmişse slot aynı callback sonunda yeniden kullanılabilir. Kapasite aşımı bilinmeyen native factory prefix’ini geri almaz; kaynakları tutup durur.

Frame işi başlamadan ayrı bir strong alınır; yalnız bilinen normal `021770` dönüşünde bırakılır. Job kaydı tamamlanma, detach ve start dönüşü olmadan tekrar kullanılmaz. Runner kopyaları paylaşılan tombstone taşır; App kapandıktan sonra eski bir kopya App’e erişmeden reddedilir. Desteklenen model aynı admitted worker thread/fiber üzerindedir. Deferred işler native worker dönmeden bitmelidir; kapanışta yeni source işi üretilmez.

`Strings`, SSO/heap içeriklerini ve büyük allocation header kayıtlarını yönetir; gerçek `009eb0` kopyası da kullanılır. Mesaj köprüsü normal dönüşte request yazımı/response üretimi sağlar; native `033d70` ara-callback/EH sırasının eşdeğerliği değildir.

Harici job/kernel drain Root Operation dışında alınır. Ardından kısa ayrı Operation içinde observer’lar bırakılır. Sağlayıcılar ve std::function capture’ları lifecycle kapalıyken, metadata/kilit canlıyken bırakılır.

**Yeni native gövde ve ABI kapanışı yoktur.** Önceki 888 kaynak korunur. Tam bootstrap Resources, loader bağlantıları ve gerçek native kernel/thread/EH eşdeğerliği hâlâ açıktır. Bu kaynak grafiği toplam DLL tamamlanma yüzdesini ölçmez. Sonraki B41, öncelik1; sıra1–2–3–5–4 ve CRT ertelemesi korunur.

[Kanıt](evidence.json), [bağımsız inceleme](architecture-review.md), [kaynak sınırları](resources-audit.json).

## B40 ortak doğrulama

**403 çekirdek + 85 entegrasyon = 488 kontrol PASS.** Ortak build 262 nesne (258 C++ / 4 C), 309 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 888 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 894 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
