# B9 — Canlı dizin işleme ve thread hata tablosu

B8'de bildirilen configuration dizin sırası farkı somut kaynak zincirinde kapatıldı: her girdinin filename/narrow/filter/remove işlemleri **sonraki FindNext'ten önce** çalışır. Geç enumeration hatasından önce yapılmış silme korunur; callback hatası sonraki increment'i engeller ve sahip olunan handle kapanır.

Services thread_error yedi gerçek code/context çiftiyle bağlandı. Services11/17'den12/17'ye çıktı; beş politika açık. Configuration legacy bağlantıları18/21, üç açık politika olarak kaldı. Bu oranlar DLL tamamlanma yüzdesi değildir. Yeni native gövde/kapanış0; mapped487/eligible491/normalclaims124/kapanış52 korunur; tam DLL **BLOCKED**.

## Canlı kaynak zinciri

initialize_configuration → B5 Configuration → B7 ConfigurationPlatform → B8 ConfigurationPersistence → B8 ConfigurationFilesystem → Filesystem::visit_directory_entries

DirectoryVisitor iki pointer içeren senkron bir kaynak callback'idir. Callable allocation yapılmaz. Optional virtual eski21 metottan sonra eklenmiştir; native ABI iddiası yoktur ve tüm kaynaklar yeniden derlenir. Dört wrapper çağrıyı iletir. Somut Filesystem aktif find handle'ı korur, girdiyi verir, sonra ilerler. Callback LastError'ı sonraki SDK çağrısına aktarılır; cleanup close kendi SDK sonucunu korur.

Eski vector API collecting adapter olarak kalır; eski sağlayıcılar default vector fallback kullanabilir. Bu yollar eager davranışını korur. Kapanan fark somut configuration visitor zincirindeki normal akış ve test edilmiş hata prefix'leridir. Kaynak Path ve context çağrı boyunca kararlı kalmalı; entry referansı saklanmamalıdır. Genel native iterator storage/EH/SEH, eşzamanlı mutation ve ABI eşdeğerliği açık kalır.

## Thread hata kanıtı

| İndeks | Kod | Korunan context |
|---:|---:|---|
| 0 | 16 | device or resource busy |
| 1 | 22 | invalid argument |
| 2 | 3 | no such process |
| 3 | 12 | not enough memory |
| 4 | 1 | operation not permitted |
| 5 | 36 | resource deadlock would occur |
| 6 | 11 | resource unavailable try again |

Tablolar0x180087df0/0x180087db8 korunan REA rdata kaydından doğrulandı. Root statik olarak yalnız singleton0x1800a14e0 alanındaki8baytı okudu:0802088001000000 → vtable0x180080208. Korunan vtable name slotu00fe60'a gider; bu fonksiyon generic döndürür. Evidence:ev_108796666f1e134d971f55a1f575e33ff47f704541695016fd846180008ca5a9. HedefSHA9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c doğrulandı, target kapatıldı.

Compiler std::generic_category ve std::system_error seçilen yeniden derleme temsilidir. Native category nesne kimliği, message/what() biçimi, locale, MSVC exception nesnesi/EH ABI'si birebir kanıtlanmış sayılmaz. Geçersiz indeksler özgün OOB davranışı taklit edilmeden destek alanı sınırıyla out_of_range olur. Kaynak Bootstrap'ın6/1/1 hata seçimi sınanmıştır.

## Doğrulama kapsamı

110 çekirdek +64 entegrasyon =174 kontrol PASS. Reentry ayrı senkron handle durumlarıyla sınanır; concurrency iddiası yoktur. Core callback→FindNext sırasını, dot atlamayı, LastError'ı, callback hata/kapanış prefix'lerini ve default fallback'i sınar. Entegrasyon dört wrapper, gerçek FilePass/NativeFactory ve kendi sentetik dosyalarını kullanır.

Başarısız senaryoda gerçek Windows enumeration içinden bilinen erken_AVATAR girdisi ilk mantıksal kayıt olarak sunulur; ardından kontrollü FindNext hatası enjekte edilir. Gerçek dosya silmesi ve canlı gerçek handle vardır; hata/sıra kontrollüdür. Başarı senaryosu normal OS sırasını kullanır. Filename/narrow/remove hataları increment'i engeller. Thread kodları SourceServices/NativeExecution ve kaynak Bootstrap üzerinden sınanır.

13 önceki kaynak dosyası açık before/after SHA kaydıyla güncellendi; diğer728 kaynak aynıdır. Beş yeni dosyayla toplam746 kaynak. Tarihsel B7/B8 kayıtları eski source hash'lerine bağlıdır; güncel bütün kaynak ortak B9 build'iyle doğrulanır.

Orijinaller veya yeniden üretilmiş DLL çalıştırılmadı/değiştirilmedi. Gerçek kullanıcı CFG/token/achievement içeriği ve engellenmiş version girdileri okunmadı. Yeni orijinal veri sorgusu yukarıdaki statik8baytla sınırlıdır. Testler yalnız workspace build/b9-fixtures altında yeni dizinler kullanır. Öncelik1→2→3→5→4; generic CRT leaf/numeric0033c0 beklemededir.

---

# B9 — Canlı dizin visitor bağlantısı

B8'deki toplama sınırı üretim configuration yolunda kaldırıldı. initialize_configuration tek mevcut gövdesini kullanır; her mantıksal girdinin filename → narrow → _AVATAR_ filtresi → koşullu remove zinciri, sonraki SDK FindNextFileW çağrısından **önce** tamamlanır. Geç enumeration hatası önceki girdilerin etkilerini geri almaz. İkinci bir startup gövdesi veya tahmini başarı stub'ı eklenmedi.

## Kaynak arayüzü

configuration::DirectoryVisitor, {void* context, void (*visit)(void*, const Path&)} taşır. Senkron borrowed callback'tir; callable allocation yapmaz. Context bütün çağrı boyunca canlı kalır; Path referansı callback dönüşünde sona erer. Null visit, ilk SDK işlemi öncesi invalid_argument ile reddedilir. Null context, callback bunu kullanmıyorsa geçerlidir.

Dependencies::visit_directory_entries, yeni **opsiyonel kaynak metodu** olarak eski sanal metotların sonuna eklenir. Önceki sağlayıcılar için default implementation eski vector sonucunu ziyaret eder. Yeni saf sanal metotla eski sağlayıcılar kırılmaz. Bu extension özgün binary vtable'sı değildir; bütün interface kullanıcıları yeniden derlenmelidir.

Gerçek bileşimde dört wrapper çağrıyı kaybetmez:

- B5 bootstrap_providers::Configuration → lower visitor.
- B7 bootstrap_policies::ConfigurationPlatform → lower visitor.
- B8 ConfigurationPersistence → lower visitor.
- B8 ConfigurationFilesystem → ortak Filesystem::visit_directory_entries.

Callback context'i dış Dependencies nesnesini gösterir. Filename/narrow/remove dış bileşimin seçilmiş politikalarından geçmeye devam eder. Eski Filesystem::directory_entries API'si aynı canlı stream'i vector içine toplayan adaptördür. Default eski sağlayıcı fallback'i ve doğrudan vector API eager kalır; bütün olası sağlayıcılara native iterator eşdeğerliği iddia edilmez.

## Handle, LastError ve reentry

Find handle tek çağrının stack sahibinde yaşar. Callback dönerse iterator ilerler; callback exception atarsa handle bir kez kapatılır ve exception yayılır. Önceki başarılı yan etkiler geri alınmaz. B8 tam BOOL kontrolü, terminal owned-handle close ve daha güçlü status-fallback close politikası değiştirilmedi.

Her stream ayrı Boundary, data ve handle tutar; Filesystem üzerinde değişebilir cursor/global route yoktur. İç içe çağrı ayrı kaynaklara sahiptir. Dış filesystem yarışları yine çağıranın sorumluluğudur.

Callback öncesi son SDK LastError yeniden kurulur. Callback döndüğünde veya exception attığında güncel LastError yakalanır; sonraki SDK advancement callback sonucunu alır. Böylece pre-callback değer, remove/conversion yan etkilerinin üstüne yazılmaz. Compiler bookkeeping ve exception nesnelerinin özgün MSVC yan etkileriyle tam eşdeğerlik iddiası yoktur.

İlk mantıksal girişten önce hata directory_iterator::directory_iterator, en az bir callback döndükten sonraki advancement hatası directory_iterator::operator++ bağlamını taşır. Dot girdileri mantıksal ziyaret sayılmaz. Callback exception'ında iterator ilerletilmez.

## Değişen kaynak sınırı

Yalnız on onaylı önceki üretim dosyası değişti:

1. anadius-config-source/configuration.hpp ve .cpp
2. anadius-bootstrap-providers-source/providers.hpp ve .cpp
3. anadius-bootstrap-policies-source/config_platform.hpp ve .cpp
4. anadius-config-storage-source/filesystem.hpp ve .cpp
5. anadius-config-storage-source/persistence.hpp ve .cpp

Yeni production CPP veya yeni provider interface sınıfı yoktur. Eski 21 configuration politika paydası artmaz. Opsiyonel visitor ayrı kaynak capability'sidir; yeni native fonksiyon/kapanış değildir. Diğer önceki kaynaklar byte düzeyinde korunmalıdır.

Beş değişen CPP, C++20/O2/Wall/Wextra/Werror ile ayrı ayrı derlendi. Hızlı link bu beş güncel object/source'u eski partial archive'den **önce** seçmelidir. Son ortak build bütün interface kullanıcılarını yeniden derler. Bağımsız test/ortak build sonucu ayrı kaydedilir.

## Korunan statik kanıt

Hedef SHA-256: 9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c. Yeni target sorgusu veya orijinal dosya okuması gerekmedi; mevcut kaynak ve korunan Ghidra çıktısı kullanıldı.

| Adres | Kanıt |
|---|---|
| 0x18001c870 caller | pseudo ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e; asm ev_42beaf13c42bb51716fcf407bd74587506fbb6acf17bfba9e16c31bdd31f8fba |
| 0x180011a40 iterator açılışı | pseudo ev_ce0c5dec45c662c9f2baac77b5e61e47cb04a6ad1950c85f881ed16e13b2a781 |
| 0x180017a40 SDK doğrulama/açılış | pseudo ev_43d76c5092901a642bc14870de0f639acd0a7988b96aa1b64e002477da21b4a7 |
| 0x180017110 advancement | pseudo ev_b73cfdf1cde3954d962ee0f307723dae4ede3cb6ce29cbe6c2092083fd65cb47 |
| 0x18004948c remove | pseudo ev_e72194d20aa35a443ed071ea2568e1157254a50f348d17fb16fc5638fff78b0d |

Korunan anadius-all-functions.pseudo.c caller gövdesinde açılış satır23411, _AVATAR_ kontrolü23689, remove23726 ve advancement23733 sırasındadır. Visitor-before-next tasarımı buna dayanır. Tam pseudo/asm SHA'ları ve block hash'leri B8 source-evidence.json içinde korunur.

Yeni native gövde/kapanış yoktur. Native iterator nesne yerleşimi, MSVC exception/allocator ABI, genel concurrency, loader lock ve tam DLL hazırlığı kapanmış sayılmaz. Orijinaller/oyun/yeniden üretilmiş DLL çalıştırılmadı veya değiştirilmedi. Kullanıcı CFG/token/achievement içerikleri ve engellenmiş version girdileri okunmadı.

---

# B9 — Indexed thread error route

The existing SourceServices::thread_error no longer delegates to an unresolved
lower provider. It calls throw_thread_error_04a648, which selects the exact
retained code/context pair and throws a rebuilt compiler std::system_error.
The helper adds one selected Services route: 12/17 bound and five remaining.
No new provider interface, native function-body claim or native closure is added.

| Index | Retained code | Exact context |
|---:|---:|---|
| 0 | 16 | device or resource busy |
| 1 | 22 | invalid argument |
| 2 | 3 | no such process |
| 3 | 12 | not enough memory |
| 4 | 1 | operation not permitted |
| 5 | 36 | resource deadlock would occur |
| 6 | 11 | resource unavailable try again |

04a648 indexes DWORD cells at 0x180087df0 and the pointer table at
0x180087db8, passes both to04a528, then invokes the native C++ throw helper.
All seven pairs were decoded from retained REA rdata
ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082.
The retained bundle and decoded bytes/addresses are hash-bound in
thread-error-evidence.json.

Category choice was checked independently rather than inferred from errno
numbers. Parent performed a static eight-byte REA query at0x1800a14e0:
0802088001000000 points to vtable0x180080208. Its retained name slot points
to0x18000fe60, whose pseudocode returns "generic". Evidence:
ev_108796666f1e134d971f55a1f575e33ff47f704541695016fd846180008ca5a9
and ev_a4d693e411afb626e5e7c3c3dd130c68e02975e25ba85568d15281ee8de1aa47.
Message slot0x18000f240 calls0x1800480d8; the complete pointer/name/message
chain is recorded. Target SHA-256:
9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

This supports selecting the rebuilt compiler's std::generic_category()
family. It does not establish original singleton pointer identity, native
std::system_error object layout/RTTI, translated category messages, what()
punctuation/locale, errno/LastError side effects or MSVC throw/unwind ABI.
Those remain compatibility boundaries. No original VA is called.

The supported input domain is0..6. Other indices throw std::out_of_range
before reading the table; the native unchecked out-of-range access is not
reproduced. The code and explicit context are preserved for valid inputs.
Compiler exception allocation may throw its own allocation exception.

Only three approved prior source files changed for this scope:
services.hpp, services.cpp, and the B7 policies_tests.cpp assertion.
The historical B7 test now checks retained code11, selected generic category
and explicit context instead of claiming the thread error remains lower-owned.
Other Services methods still delegate or use their previous concrete routes.

Strict C++20/O2/Wall/Wextra/Werror/static compile of the helper and Services
translation units passed. Independent B9 tests and the common build are
recorded by the test/delivery owners. This author did not read an original input,
execute/load a target or rebuilt DLL, modify original bytes, inspect actual
user CFG/token/achievement contents, or access a blocked version artifact.

## B9 ortak doğrulama

**110 çekirdek + 64 entegrasyon = 174 kontrol PASS.** Ortak build 225 nesne (221 C++ / 4 C), 249 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 728 kaynak aynı; 13 önceki dosya açık hash kaydıyla güncellendi; toplam 746 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
