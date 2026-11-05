<!-- PENDING-SOURCE-CHANGES -->
2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı. 296 nesne, 356 test grubu ve 19 framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.
<!-- END-PENDING-SOURCE-CHANGES -->

# B46 — Somut uygulama grafiği ve event sahipliği

**Güncel kaynak adımı, henüz derlenmedi:** Dar çocuk JobLogic kapsamı, açık
Jobs seçimi, Information'ın iki çocuk çağrısı ve somut Content'in dar çocuk
kapsamı eklendi. Mevcut033d70 kullanılır; Licensing çocuk yetkisi ve Profile'ın
tam mesaj bağlantısı henüz açık.
[Content ayrıntısı](../anadius-content-effects-source/job-content.md).
Aynı-worker seçili
Jobs entegrasyon test kaynağı eklendi, çalıştırılmadı; çocuk Graph testi değildir.
[Ayrıntılar](job-graph.md). Aşağıdaki PASS sonuçları tarihseldir.

`application_graph::Graph::create(Options)` ve `install_server(...)`, gerçek Policy/Profile/NativeLogic/XMLReader/Router/kripto/random kurulumunu tek fabrikada toplar. Caller SessionLogic/Router/XML/context/event sınıfları kurmaz. Exact StateView canlı configuration/online/avatar hücrelerini sahipleriyle tutar; state kopyası yapılmaz, her dispatch’te Router span/referansları yeniden kurulur. Controlled update yalnız aynı worker üzerinde yapılır.

Event ring gerçek Parent-backed SSO/heap prefix’lerini sahiplenir. İlk gerçek kullanım mevcut68byte DOWN seed’ini kopyalar; Content/IGO ve native033d70 aynı authoritative queue’yu kullanır. Native pop/free/reset ve offset/count doğrulanır. Fixed power-of-two kapasite bir HOST seçimidir; native deque/class/allocator ABI kapanışı değildir.

Her dış info/content/license/clock/writer/XML çağrısı ve her stream primitive sonrası sticky kontrol vardır. Opaque writer, postcheck öncesi persistent inventory’ye alınır; yalnız bilinen normal dispatch sonunda emekliye ayrılır. Unknown sonuç rollback/retry ile tahmin edilmez. Kurulumda tamamlanmış unpublished backend finalcheck öncesi tutulur; unknown prefix için kasıtlı retained owner cycle oluşabilir, normal kapanışta cycle yoktur. Saat ve bağımsız opaque sahipleri Graph Retiring metadata’sı hâlâ canlıyken mutex dışında bırakılır.

Gerçek dış kernel/job kanıtı sonrası explicit Parent Operation’da app.reap ve bilinen owned event retirement yapılır. Graph Drained olduktan sonra başka source allocation Profile close’u engellerse, known free ardından yeni gerçek dış proof ile tekrar kapanış desteklenir. Sayaçlar drain kanıtı değildir. Narrow bilgi/content/license/SDK/lifecycle alt sağlayıcıları mandatory kalır.

**Kapsam aynı original controller/worker/fiber’dir**, loader lock dışındadır. Domain/provider/memory factory’leri controller-only, B45 Policy worker-affine olduğundan bu fabrika controller’ın başlattığı ayrı bootstrap worker’ı henüz desteklemez. Wrapper yabancı thread/fiber girişini SDK’dan önce reddeder. Tam Resources/başlatma ve loader bağlantıları açık; B47 iki aşamalı controller/worker sınırını ele alır.

Test peer202 explicitly fixed key/encrypted flag ile hazırlanır; gerçek prefix AES ve event yolu test edilir, tam default challenge-response/autodetection kanıtı değildir. Önceki914 kaynak aynı,4yeni dosya. Yeni native gövde/ABI kapanışı0; güvenilir toplam DLL yüzdesi yok.

[Kanıt](evidence.json) · [Bağımsız inceleme](architecture-review.md) · [Sınırlar](resources-audit.json)

## B46 ortak doğrulama

**503 çekirdek + 325 entegrasyon = 828 kontrol PASS.** Ortak build 268 nesne (264 C++ / 4 C), 321 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 914 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 918 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
