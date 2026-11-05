# Bilgi sağlayıcısının çocuk iş bağlantısı

Durum: **yalnız kaynak kodu; derleme, proje testi, link veya DLL çalıştırılmadı**.
Dört dosya bağımsız statik incelemeden geçti. Test kaynakları çalışma zamanı
kanıtı değildir. Eski derleme/ZIP kayıtları tarihsel kaynak sürümlerine aittir.

Somut `Information` sağlayıcısına iki nonvirtual giriş eklendi:
`log_profile_name_job` ve `current_time_job`. Mevcut log prefix/name/newline/flush
işlemleri ve mevcut `utc_time_041b60` algoritması kullanılır. Olağan log/time,
checkpoint bind/clear ve son destruction asıl worker/fiber'a bağlı kalır.
Public Information facade yerleşimi ve base virtual slotları değiştirilmedi.

Çocuk girişleri canlı `SessionMemoryDomain::JobPermit` ile aynı Parent Operation
ve actor'u doğrular. Yanlış Parent/actor veya süresi bitmiş izin, Information
rezervasyonu ve opaque alt çağrılardan önce reddedilir. Mutex yalnız metadata
içindir; Parent, kabul checkpoint'i, clock/formatter veya stream çağrısı sırasında
tutulmaz. Busy/checking/binding işaretleri yutulan yeniden girişi kalıcı poisoned
duruma çevirir ve sonraki alt çağrıya ilerlemeyi engeller.

JobCall descriptor'ı bu senkron çağrının stack nesnesidir. Impl içindeki raw
işaretçi yalnız busy rezervasyonu boyunca yayımlanır; normal/exception dönüşte
descriptor yok olmadan önce temizlenir. Checkpoint mevcut POD `CallCheckpoint`
(context + function pointer) biçimindedir; sahip olunan std::function aktarımı
veya capture retirement yoktur. Çağıran facade, Parent Operation, callback
context'i ve bütün lower backing'lerini çağrı bitene kadar korur. Parent izni
tek başına tam Graph/job claim veya dış lower eşzamanlılık kanıtı değildir.

Graph'ın Information adapter'ı çocuk kapsamı varsa seçili somut Information'ı
shared olarak tutar, aynı Graph JobCall Parent/permit'ini ve senkron
`Impl::admit` checkpoint'ini geçirir. Bu callback mevcut Graph/session/claim
kabul zincirini her primitive öncesi/sonrası yeniden denetler. Legacy/custom
sağlayıcıya çocuk fallback yoktur. Asıl worker yolu mevcut CheckedCalls bağını
korur. Çocuk methodu döndükten sonra dış Graph opaque postcheck'i de çalışır.

`handoff_tests.cpp` içindeki yeni `child_information_permit` senaryosu, mevcut
gerçek Domain/NativeThreads/prepared Parent fixture'ını ve kontrollü clock/
formatter'ı kullanacak şekilde yazıldı. Aynı Domain'de yanlış Parent, yanlış
actor, default-child reddi, UTC/log normal yolu, LastError, yutulan admission
reentry, eski Operation izninin reddi, taze izin ve asıl worker'a geri dönüşü
tarif eder. Mantıksal dönüş ile gerçek thread/FLS çıkışı ayrı kontrol edilir.
**Bu test çalıştırılmadı; tam Graph/Profile çocuk mesaj zincirini kapsamaz.**

Kalanlar: Content/Licensing ve onların dosya/encoding/Online alt kaynaklarının
dar çocuk yetkisi; Profile'ın JobLogic seçimi, gerçek request/response/closure
ve021770 son temizleme zinciri; native Jobs scheduler/exit bağlantıları ve tam
prepared entegrasyon fixture'ı. İki üretim loader hook'u ayrıca açıktır.
