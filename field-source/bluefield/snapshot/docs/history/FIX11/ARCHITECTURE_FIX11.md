# Uygulama mimarisi

`Application::Impl` tek config, profil, online durum, achievement yolları,
event kuyruğu, mutex'ler ve dispatcher sahibidir. Native dosya/clock/config/
licensing/persistence etkilerinin somut uygulamaları aynı ömre bağlıdır.
Her `Connection`, kendi cipher, challenge ve message handler nesnesini taşır.
Global değişen uygulama verisi app mutex'i altında, bağlantı cipher'ı bağlantı
mutex'i altında kullanılır. C++ nesnesi veya STL/allocator public DLL ABI'sini geçmez.

`Service::prepare` config'i uygular, loopback socket'i bind/listen eder ve gerçek
`EALsxPort` değerini yayımlar. Native owner hook'ları kurar. `Service::activate`
version-2 online sağlayıcının gerçek mevcut/yok dalını çalıştırır, final profili
bağlar ve accept worker'ını başlatır. Doğrudan API ile ek thread başlatma,
hook kurma, config okuma veya bekleme DllMain içinde yapılmaz.

FIX09 transport'u pinned upstream Asio async accept/read/write yoluna geçirir.
Windows IOCP zorunludur. Tek IO thread ve iki worker pool thread'i bounded session
sahipliğiyle çalışır; istemci başına thread yolu aktif değildir. Kaynak receive-window
korunur. Maksimum 32 canlı istemci, 1 MiB frame/config ve 128 config nesting sınırı
sürer. Stop, IO/pool ve pending callback ömrünü Application'dan önce sonlandırır.
Ayrıntılı sıralama ve özgün eşdeğerlik sınırı ASIO_TRANSPORT_FIX09.md içindedir.

FIX10 config document sınırı pinned PEGTL VDF gramerini çağırır. Character/key/
value/object action'ları mevcut State sahipliğinde kalır; cursor rewind action
rollback değildir. Owned ParseError adapter'ı çağıran sözleşmesini korur. Hash
sırası ve Application nesting/size guard'ları değişmez. Tasarım ve sınırlar
CONFIG2_PEGTL_FIX10.md içinde; filesystem backend'i bu adımın dışındadır.

Native callback'ler SDK imzalarını korur. Registry/host map ve handle kümeleri
serialize edilir. Connect callback'in readiness beklemesi registry kilidini
tutmaz; sayaç artışı ayrı kilitlidir. Bootstrap thread kendi başlangıcını
beklemez. Config/transport hazırken registry etkileri profil beklemesine bağlanmaz.

Shutdown socket işlerini durdurur, bağlantıları join eder, hook'ları disable
eder ve leased callback'leri boşaltır. Haricî thread register/stack'lerinde modül
adresleri için konservatif kontrol yapılır; false positive unload'u erteler.
Sonra trampoline/binding/handle sahipliği bırakılır. CRT-aware bootstrap thread
tamamen çıktıktan sonra host shutdown çağrısı ortamı geri yükler ve self-pin'i
bırakır. Nonzero sonuç unload izni vermez. Host kendi LoadLibrary referansını
DllShutdown dönüşüne kadar tutmalı ve public çağrılarını quiesce etmelidir.

## Kaynak uyarlamalarının sınırı

- Özel CRT/STL modelleri üretime alınmadı; normal MSVC /MT, debug /MTd kullanılır.
- KeyForLicense normal gövdesi input pseudo/ASM kanıtından uygulandı; final config/
  profil ataması sonrası commit edilir. Mevcut regular license dosyası korunur.
- Online version_check=2 ve create_online/native vtable uçları source çağrı
  sözleşmelerine bağlandı. Avatar adapter'inde kaybolmuş owner/size argümanları
  restore edildi. Gerçek orijinal online DLL çalıştırılmadı; ABI kabulü beklemededir.
- Online DLL bir bağımlılık olarak paketlenmez. Yoksa gerçek source offline dalı
  çalışır. Cached profile dizilimi persona_id;user_id;username şeklindedir.
- Online aramasında executable dizini, ardından config dizini kullanılır.
- LoadExtraDLLsFromMain ve LoadExtraDLLs sıralı olarak post-loader worker'da yüklenir.
- FIX04 özgün iki embedded 208x208 PNG'yi input'un adreslediği pinned donor
  örneğinden geri getirir. Config avatar yoluna yeni dosya yazılmaz.
- Fatal licensing durumları yeni owner'da session failure olarak sınırlandırılır;
  sahte başarı verilmez. Kod/binary/Consumer eşdeğerliği bu değişikliklerden çıkarılmaz.

Windows/MSVC, gerçek Windows hook/unload, orijinal online-provider ABI'si ve
Consumer davranışı henüz doğrulanmadı. Runtime kaynakları bu sınırları açık tutar.

## FIX02 CryptQueryObject bağlantısı

Yeni selector yalnız aynı input paketin `preloader-source/preloader_core.cpp`
içindeki `Core::crypt_query` gövdesinden türetilir. BROWNFIELD C++ kopyalanmaz.
File object türünde null pointer veya case-sensitive `preloader_l.dll` suffix
dalı kaynak companion filename'ına gider; diğer object türleri/yollar orijinal
pointer ile geçer. Geçerli UTF-16'da ASCII suffix karşılaştırması source UTF-8
final-byte karşılaştırmasıyla aynı kararı verir. Native adapter maksimum32768
UTF-16 birimini ReadProcessMemory ile kendi sürecinden okur. Okunamayan veya
bozuk UTF-16 işaretçiler orijinal API'ye değiştirilmeden aktarılır. Bu güvenli
sınır, kaynak modelin malformed-string trap yoluyla eşdeğerlik iddiası değildir.

Callback enum'unun sonuna yeni slot eklenir; önceki on indeks korunur.
MinHook create/enable ve tam callback/original çağrı süresini kapsayan lease,
mevcut NativeHooks sahibi içindedir. Ayrı patch engine yoktur. Gerçek CRYPT32
import'u loader dependency'sini korur. Crypt callback registry map kilidini
tutmaz. Gerçek API'nin dönüşü/çıktıları değiştirilmez; gelen LastError original
öncesi, çıkan LastError sınırlı logging sonrası geri konur. Normal disable/drain/
external-quiescence/uninitialize yolu yeni trampoline'ı da kapsar.

`HOOKS.json` zorunlu/koşullu sayımı kaydeder: 7+2+2, maksimum11. Kullanıcı
tarafından kaldırılmış kapsam bu değişiklikle yeniden eklenmez.

## FIX03 hook aktivasyon sırası

Önceki NativeHooks sahibi Context serialization kilidi altında host initializer
çağırıyor, initializer her başarılı hook'u hemen enable ediyordu. Böylece oyun
thread'i yeni callback'te kilidi beklerken installer sonraki loader/API hedefine
girebiliyordu. Bu kilit sırası startup için gerçek bir deadlock riskiydi; kullanıcı
logu tek başına bu döngünün yaşandığını ispatlamaz.

Host initializer şimdi yalnız resolve/create/queue yapar; sahte enable başarısı
üretmez, Effects API açıkça queue_hook_enable adını taşır. Önceki on hook'un
sırası ve source'daki opsiyon koşulları korunur. Native owner crypt hook'unu da
stage eder. Eksik target veya başarısız queue durumunda commit yoktur. Context
kilidi bırakıldıktan sonra tek MH_ApplyQueued, ardından başarıya bağlı installed
yayını vardır. Partial apply başarısızlığı normal cleanup'a gider; rollback
Windows'ta doğrulanmış veya kusursuz atomik olduğu iddia edilmez.

Pinned MinHook vendor kaynağı değiştirilmedi. Bu işlem, BROWNFIELD'daki
SAFE-FREEZE vendor değişikliğinin taşındığı anlamına gelmez. Batch etkinleşmesi
sonrası online-provider/loader zamanlaması ayrıca Windows doğrulaması ister.

Logging DllMain'e eklenmez. Post-loader worker service-prepare/hook/activate/
extra-module fazlarını, Effects her target için resolve/create/queue sınırlarını
yazar. Environment logger UTC milisaniye/PID/TID ekleyip flush eder. Böylece
eski append kayıtları yeni FIX03 süreciyle karıştırılmadan incelenebilir.

## FIX04 başlangıç durumu ve config yokluğu

Application önce configuration::initial_state() ile input 036d20 constructor
mantıksal durumunu kurar. CLI ön hazırlığı, isteğe bağlı base ve override sırasıyla
uygulanır. Yok dosya boş belge gibi uygulanarak derived license/default DLL
path'lerinin de kurulması sağlanır. Eksik Config2 kökü kaynakta olduğu gibi boş
node'dur. Mevcut dosya için I/O/syntax hataları bastırılmaz. Profil handler'ı
son online aşamadan sonra aynı Application state'inden snapshot alır.
Environment.profile_root boşsa data_root kullanılır. Native worker profile_root'u
özgün anadius/LSX emu cache konumuna bağlar; log/achievement root'u değişmez.
Cache formatı ve auth başarısızlığına bağlı kullanım koşulları değiştirilmez.

## FIX05 code identity and scoped QA

Owned C++ namespaces and CMake core/engine targets use preloader_l. This leaves
public C exports/ordinals unchanged. Online is a verified SURGE/HF19 CFG dialect
gap; it produces an explicit unsupported diagnostic, without a guessed auth mapping.
HF19 field ownership/nesting/comment fixtures exercise the Application adapter.
Protocol tool selftests operate on synthetic PE bytes in a separate tools/golden
policy context and do not accept a new DLL. See CFG_QA_AND_GOLDEN_AUDIT.md.

## FIX06 Golden build ownership

Production links its pinned Golden resource and production.def; public ordinals
and preloader_l namespace remain unchanged. The NMake build driver delegates all
staging and publication to scripts/qa/run_golden.py. The first four unchanged HF19
tools enforce their original policy pins. The FALCION adapter enforces this source
layout's sealed manifest and LOCAL_BUILD payload rules. Gate order, ordinal/link
policy, failure propagation, raw-input immutability and read-only gates have
separate synthetic negative tests. No application config/hook semantics changed.

## FIX07 güncel durum

Kullanıcı son sürümde surge.cfg olmadan başlangıç ve profilin çalıştığını bildirdi.
Önceki no-config OPEN bulgusu USER_CONFIRMED düzeyinde kapanır; exact DLL SHA ve
Golden receipt sağlanmadı. FIX07 varsayılan adı SURGE olur, sayısal ID'ler korunur.
Eski CFG'nin exact etkisi ve düzeltmenin 14 alanlı profile eşitliği yeni entegrasyon
testindedir. Kapsam 26 C++ + 3 tool grubudur. FIX07 native/Windows/DLL Golden
NOT_RUN; eski CFG ile tüketicide anadius görülmesinin exact nedeni açık kalır.
Ayrıntı PROFILE_CONFIG_FIX07.md içindedir.

## FIX08 dependency notu

FIX07 kullanıcı tarafından çalışır olarak doğrulandı. FIX08 exact HF19 pristine
MinHook/TinyXML2 pinlerine geçer. Asio/IOCP ve PEGTL/VDF özgün kanıtları ile
BLUEFIELD backend seçimleri ayrı şeylerdir; DEPENDENCIES_FIX08.md güncel açıklamadır.
Önceki XML depth=100 vendor uyarlaması owned reader'a taşınır. Derin bozuk XML
hata sırası/footprint ve Windows native eşdeğerliği açık sınırdır.

## FIX11

module_sibling uses pristine gulrak parent/join at bootstrap config and override
paths, with raw native-spelling compatibility. IO/status/UTF-16 lexical code is
unchanged. An unexpected Asio IO dispatch exception now clears ready, sets stopping,
closes listener and stops io; ordinary host stop still joins IO/pool before app
destruction. is_ready only reads existing atomic state; C ABI remains unchanged.
FILESYSTEM_FIX11.md specifies both changes and native limits.
