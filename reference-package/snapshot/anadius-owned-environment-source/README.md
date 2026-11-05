# B43 — Profile-owned ortam ve zamanlama

**Güncel kaynak adımı, henüz derlenmedi:** Canlı Parent JobPermit ve tam oturum
kimliğine bağlı çocuk projection kapsamı ile `JobHandoff.clone_environment`
eklendi. Geçici kopyalar kapanıştan önce yok edilir; scheduling ve mesaj yürütme
henüz açılmadı. Yeni test kaynağı çalıştırılmadı; çocuk kapsamın pozitif entegrasyon
testi henüz yoktur. Ayrıntı: [job-environment.md](job-environment.md). Aşağıdaki
PASS sonuçları önceki kaynak sürümüne aittir.

`Dependencies.environment` bütünüyle boş bırakıldığında Profile artık kendi vtable, type-query, executor factory, handler/response/accept scheduler ve copy bağlarını kurar. Varsayılan yol için uygulama Table/Types/thunk veya Policy session/accept override’ı sağlamaz. Açıkça verilen eski konfigürasyonlar korunur; eksik zorunlu legacy parçalar varsayılanlarla karıştırılmaz.

Context almayan callbackler raw integer→object cast veya TLS singleton yerine weak registry ve monoton owner/projection capability kullanır. Her callback canlı Control pin’i alır. Copy yeni referans/capability oluşturur, move mevcut kaydı yeni adrese taşır, destroy tek release yapar. Empty helper’lar unspecified payload okumaz. Native partial move, source executor’u doğrular; destination’ın henüz yazılmamış alanlarını okumaz.

Source admission checkpoint’i çalışırken yeniden source girişine dar bir mühür uygulanır. Task gövdesindeki meşru nested copy/destroy işlemleri sürer. Owned handler slotu call öncesi null olur ve aynı worker’da inline çağrılır; yanıtlar mevcut `022e40` ile force-post1 kullanır. Bunlar seçilmiş host politikalarıdır; `0228c0`, native RTTI/vtable, EH ve concurrency eşdeğerliği değildir.

Bind, tamamlanmış pending Application elde tutulduktan sonra yapılır; bilinmeyen prefix constructor unwind ile silinmez. Slot environment’ları observer weak bırakılmadan önce emekliye ayrılır. Kapanış, dış drain kanıtına ek olarak envrefs/aktif callback ve gerçek Parent/IO/accept/work/queue sıfır koşullarını kontrol eder. Envrefs0 tek başına posted response functor sonlanması değildir. Parent lease Runtime drain kontrolünden önce bırakılır.

Export edilen raw table/executor/schedule pointer’ları owner ömrünü aşmamalıdır. Stale integer capability reddi, owner yok olduktan sonra raw virtual pointer çağrısını güvenli yapmaz. Raw struct copy retain sayılmaz. Bilinmeyen prefix tekrar çalıştırılmaz ve tahmini cleanup yapılmaz.

Önceki 902 kaynağın 900’ü aynı; Profile.hpp/.cpp açık hash kaydıyla güncellenir, dört yeni dosya eklenir. Yeni native gövde/ABI kapanışı0. Tam Resources, kalan uygulama/hata/SDK/job sağlayıcıları, gerçek asenkron kapanış ve loader bağlantıları açık; sonraki B44, öncelik1. Toplam DLL için güvenilir tamamlanma yüzdesi yoktur.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sınırlar](resources-audit.json)

## B43 ortak doğrulama

**200 çekirdek + 244 entegrasyon = 444 kontrol PASS.** Ortak build 265 nesne (261 C++ / 4 C), 315 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 900 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 906 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
