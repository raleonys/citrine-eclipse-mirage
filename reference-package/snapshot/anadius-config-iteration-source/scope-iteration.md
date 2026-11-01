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
