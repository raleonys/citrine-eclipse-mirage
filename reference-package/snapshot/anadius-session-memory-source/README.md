# B33 — Somut session allocator/free/context domain

**Güncel kaynak adımı, henüz derlenmedi:** `enter(job)` açık yetkiyle tam Parent
işlemine giriş sağlar; yetki işlem boyunca sabit kayda tutunur ve erken iade
edilemez. Varsayılan girişin worker denetimleri korunur. Yeni test kaynakları
çalıştırılmadı. Ayrıntı: `job-parent.md`; aşağıdaki sonuçlar tarihsel sürüme aittir.

`SessionMemoryDomain` artık session kontrol bloğu, proxy, deque blokları/map'i ve
receive/message tamponlarını **aynı gerçek ApplicationMemory heap/PTD domain'i**
üzerinde sahiplenir. Bu yeni bir kaynak bileşimidir; yeni native yordam veya
ABI/provider kapanışı sayılmaz. B30/B32 normal kurucu/temizleyici gövdeleri kullanılır.

Control için native1c8 etiketi, gerçek C++ `sizeof(RecoveredSessionControl)`
tahsisinden ayrılır. Proxy16/blok32 ve typed pointer-map ayrı kayıtlardır. Büyük
map ve stringler +39/header/32-byte alignment protokolünü paylaşır. Map pointer
elemanlarının C++ ömrü placement initialization ile başlar. Memory projeksiyonu
control'ün yalnız input.inline16 kısmını, blokların inline16 kısmını ve raw
allocation alanlarını tek kez kaydeder; çakışan header/control bölgeleri eklenmez.

## Ömür ve hata sözleşmesi

Bir Operation, bütün recovered kaynak çağrısı/zinciri boyunca tutulur. Registry
erişimini serialize eder; thread/fiber ve backing-domain native FLS PTD kimliğini
kontrol eder. Domain API'leri ile backing allocation/free öncesi/sonrası ve scope
çıkışında kimlik tekrar kontrol edilir. Gözlenen ihlal, callback eski değeri geri
koysa da stickyPoison olarak kalır. Bu kontrol bütün native callback/foreignABI,
PTDfree/ABA veya geçici değişimlerin evrensel kanıtı değildir. Saf memory.view
borrow'ları da Operation ve kendi allocation ömürleriyle sınırlıdır.

Ledger ve region kapasitesi heap etkisinden önce ayrılır. Typed destructor bir
kez çalışır. Başarısız free'de yalnız backing getter'ın **Live** doğruladığı
retired raw depolama tekrar bırakılabilir; destructor ve başarısız native kaynak
algoritması yeniden çalıştırılmaz. Missing/Busy/Unknown veya gözlem hatası
poison/retention gerektirir. Fiziksel free olup dönüşü bilinmeyen blok tekrar
free edilmez. İki eski ApplicationMemory dosyasına yalnız enum+saltokunur
tracking_poisoned/allocation_state gözlemleri eklendi; eski heap davranışı değişmedi.

construct_session gerçek023000'ı çağırır ve partial-throw'da gerçek scratch
nesnesini domain içinde tutar. Stack adresi kaydetmekle yetinmez. Native EH
rollback uydurulmaz. Live control/observerweak/partial/pending/poison durumunda
close false döner; false allocation admission'ını kapatmaz. Caller önce async
işleri durdurup drain etmelidir. Başarılı close Retiring durumunu provider
destructor/deleter'larından **önce** yayımlar ve refs'i gate dışında bırakır.
Nested close false, enter reddedilir. Destructor için Closed zorunludur.

## Doğrulama

56 core +45 integration =101 kontrol PASS. Core gerçek heap/FLS,6000-byte input,
512-slot alignedmap,5000-byte queuedpayload, finalweak/controlcontext ömrü,
yanlış/doublefree reddi, knownrawfree retry/destructoronce, provider-deleter
reentry ve foreignthread→explicitCallbackPTD admission'ını sınar. Dokuz ownchild,
partialconstructor,unknownalloc/free,PTDpre/postalloc/postfree/exit ve caught+restored
stickyPoison prefixlerini doğrular; failedstate'i tutup kendi process'ini bitirir.
Bu child'lar özgün veya rebuiltDLL/game çalıştırmaz.

Integration'da customSessionAllocator/Free ve fakebufferpool kaldırılmıştır.
Gerçekprocessheap/FLS üzerinde actualfactory/readEOF/027710/frame025480→021770→028d30,
GetConfig/XML/cipher ve actualB32disposal kullanılır. Sonweak allocationledger'ını
sıfırlar; close gerçek backinglease ve service-root'u bırakır. SDK spy, gerçek
HeapAlloc/HeapFree/FLS çağrılarını gözler; startupcalloc flags8 ve malloc flags0
ayrılır. Response scheduler/IO OperationStorage/FrameClosure allocation'ları
kontrollü test sağlayıcılarıdır; fullIOCP/kernelborrower/rawjobdrain iddiası yoktur.

B32'deki852 kaynaktan850 aynı,2 readonly-gözlem editi;4 yeni dosyayla856 kaynak.
Native mapped494/eligible498/normal131 ve52tarihsel kapanış değişmez; yeni domain
75-source/37-object composition defterindedir. Toplam DLL yüzdesi bu sayaçlardan
türetilmez. B31 üretim attach/detach hook'ları, gerçekserver/accept/executor,
licensing/foreignABI/loader ömrü açık; genericCRT kuyruğu duraklatılmıştır.

Orijinal byte/decompile sorgusu yapılmadı; orijinal/rebuiltDLL/oyun çalıştırılmadı,
gerçek CFG/token/achievement içeriği okunmadı. Kesin ortak build sonucu aşağıdadır.

## B33 ortak doğrulama

**56 çekirdek + 45 entegrasyon = 101 kontrol PASS.** Ortak build 250 nesne (246 C++ / 4 C), 295 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 850 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 856 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 494 adres, 498 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
