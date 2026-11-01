# FIX20 — Tek configuration orchestration sahibi

NON_RELEASE tam kaynak devamı; mühürlü FIX19 ebeveyninden türetilmiştir. Karşılaştırma tam RE kaynak paketidir; özgün DLL binary/runtime eşdeğerliği değildir.

## D18deki çoğaltma

RE `configuration.cpp::initialize_configuration` base/override yüklemeleri, runtime path/avatar ayarları, AVAILABLE_VERSION, SDK dönüşümü, achievement storage ve environment yayımını bir yerde toplar. BLUEFIELDda adopted `state.cpp` içinde bu body bulunuyordu; aktif Application aynı adımları kendi içinde tekrar yazmıştı. İlk tarihsel gerekçe kaydedilmemiştir. Bu çoğaltma bir upstream zorunluluğu değildir.

FIX20 mevcut bodynin iki açık aşamasını ortaklaştırır:

- `apply_configuration_layers`: base → override birer kez; runtime pathler ve logical avatar; installed→available version; SDK nokta/virgül dönüşümü; achievement storage.
- `publish_configuration_environment`: ContentId → EAGameLocale aynı ad/değer/sırayla yayımlanır.

Standalone `initialize_configuration` mevcut executable atamasını yapıp iki ortak fonksiyonu çağırır. Application mevcut erken executable/native startup yerini korur; kendi optional CFG passini ortak layer fonksiyonuna verir, profile/router oluşturur, ardından ortak environment fonksiyonunu çağırır. İki kopya post-configuration algoritması artık bulunmaz. TU taşınmadı veya birleştirilmedi.

## Neden tüm standalone fonksiyonu doğrudan Applicationa çağırmadık?

Standalone default DocumentPass ile Applicationın isteğe bağlı dosya/parsing/diagnostic passi farklı sınırlardır. Default pass zorunlu file readerına bağlanabilir; bunu Applicationa takmak surge.cfgsiz başlangıcı yeniden bozabilirdi. Applicationın `apply` body/parsers/status/optional-file davranışı aynen kalır; ConfigurationPass yalnız mevcut iki çağrıyı aynı yerden yönetir.

Standalone fonksiyon environmentı storage sonrasında yayımlar. Applicationın mevcut publication noktası ise profile/router constructiondan sonradır. Bütün fonksiyonu erkenden çağırmak failure sırasını değiştirirdi. Yayım bu yüzden ayrı ortak aşamadır. Bu separation yeni bir CFG seçeneği/ürün politikası değil, mevcut caller sınırını koruyan internal compositiondır.

## Değişmeyen davranış

| Sınır | Korunan davranış |
|---|---|
| Executable/native startup | Application executable ataması native startup öncesindedir; ortak preparation bunu tekrar etmez |
| CFG | Base sonra override, birer kez; mevcut optional/malformed/unreadable ayrımı ve Config2 parserı aynı |
| Profil/defaultlar | SURGE adı/kimlikler ve mevcut config override semantiği aynı |
| Path/avatar | Aynı RuntimePaths pathleri; AvatarStore logical_default ile aynı default-avatar pathi; custom avatar önceliği ve lazy materialization korunur |
| SDK/version | Aynı replace/copy, storage çağrısından önce |
| Achievement | Aynı Storage nesnesi aynı update fonksiyonuna gider; startup INI zamanlaması ertelenmez |
| Environment | Aynı iki çağrı ve order; Applicationda profile/router hazır olmadan yayın yok |
| Hata | Base hatası overrideı, override hatası sonraki storageı durdurur; publication çalışmaz. Yeni fallback veya transaction eklenmez |

Yeni input/cache/path kısıtı, eager cleanup, avatar hash şartı veya retry değeri yoktur. Threads/provider/registry/hooks/session davranışı ve vendor pinleri değiştirilmez. Allocation/ABI/lifetime düzeyinde özgün binary eşdeğerliği iddia edilmez.

## Test ve kanıt

Mevcut41 test grubu ve beklentileri korunur. Yeni integration fixture31 kontrolle ortak source orderı, caller-owned publication boundaryyi ve actual Application davranışını sınar. Mock composition kısmı kaynak virtual effect sınırlarını denetler; actual bölüm gerçek CFG, filesystem ve process environment kullanır.

Actual test CFGsiz başlangıç, base/override profil/SDK/environment, hatalı base/override ve gerçek achievement-directory storage hatasını çalıştırır. Hata halinde eski ContentId/EAGameLocale değerleri korunur; normalizationın persistence öncesi olduğu ve mevcut engelleyici dosyanın değişmediği doğrulanır. Test ortam değerlerini sonunda geri yükler. Bu test router allocation OOMunun veya orijinal DLLin native yürütmesi olarak sunulmaz; profile/router-before-publication call sırası ayrıca statik bağlanmıştır.

Source kontrolü Applicationın ortak ownerı kullanmasını ve publicationın router sonrasında kalmasını denetler. Targeted negatifler geçici üretim kopyalarında publicationı preparationa taşımayı ve override yerine base replayini sınar. Source/hash guardı anlamsal doğruluk veya kullanıcı onayı yerine geçmez.

## Kapsam ve borç

Yalnız üç mevcut runtime dosyası değişti: application.cpp, configuration/state.cpp ve state.hpp.119 diğer src ve1.060 vendor dosyası aynı kalır. D18 kaynak ownership kapsamında kapandı.15 inceleme/kısmi kayıt açık; bunların hepsi doğrulanmış bug değildir. Önceden düzeltilmiş encoding/read-write/diagnostic satırlarının geçmişten kalan next-action metinleri current raporda da düzeltildi; immutable ebeveyn paketleri değişmedi.

Güncel master MD/JSON version9 revision2026-10-25.1 yeniden kontrol edildi. AGENTS/metadata/QA yeni davranış yetkisi üretmez. NON_RELEASE; canonical pointer veya BROWNFIELD/diğer FIELD projeleri değişmedi. Windows/MSVC/DLL/IOCP/native provider/hook/CRT/game, gerçek DLL Golden ve CMake/CTest koşuları çalıştırılmadıysa NOT_RUN olarak kalır. Portable/synthetic sonuçlar onların yerine geçmez.
