# FIX22 — Windows lexical effect çağrılarının kaynak ownerına bağlanması

NON_RELEASE tam kaynak devamı; exact FIX21 ebeveyn ZIPinden türetildi. Karşılaştırma kullanıcının tam RE paketidir. Yeni bir Windows çalışma hatası veya özgün DLL binary/runtime eşdeğerliği iddia edilmez.

## Bulgu, neden ve karar

Pakette `parent_path_019820`, `filename_00e120`, `join_016b00`, `append_016b50` ve `is_absolute_0195b0` gövdeleri mevcuttur. BLUEFIELD bu gövdeleri koruduğu halde Configuration, License, Content ve OnlineSession adapterları lexical çağrıları ayrı ayrı std::filesystem üzerinden kuruyordu. Bu, kaynak algoritmasının aktif uygulama çağrısına bağlanmadığı bir ownership farkıdır. Sadece farklı API kullanılması ayrıca kanıtlanmış runtime bug sayılmaz.

Bu ilk tercihin bağımsız tarihsel gerekçesi kaydedilmemiştir. Kodda Linux native_path projectionı görülür; bu teknik gözlem Windows çağrılarını da bypass etme kararına sonradan kullanıcı onayı üretmez. Upstream bağımlılık böyle bir bypassı zorunlu kılmaz. FIX22, kaynak gövdeleri yeniden yazmak veya filesystem backendini topluca değiştirmek yerine onların aktif Windows lexical çağrılarının ortak ownerı olmasını sağlar.

## Bağlantı tablosu

| Aktif çağrı | Windows owner | Kaynak gövde |
|---|---|---|
| Configuration parent/join/filename | paths::WindowsEffects |019820 /016b00 /00e120|
| Configuration resolve | Aynı owner; UTF-8 conversion callerda |0192d0 composition:0195b0 →019820 →016b00|
| License parent/join/filename | Aynı owner |019820 /016b00 /00e120|
| Content path_append | Aynı owner; copy-return join sınırı korunur |016b00 →016b50|
| OnlineSession parent/join | Aynı owner |019820 /016b00|

`PathEffects` Windowsda compile-time `paths::WindowsEffects` aliasıdır. Runtime selector, CFG anahtarı veya fallback eklenmez. `paths.cpp` gövdeleri byte-identicaldır; yalnız paths.hppye forwarding/composition ownerı eklenir. Kaynak join, soldan kopya oluşturup append çağırır; Content adapterının mevcut copy-return imzası mutating/self-alias APIye dönüştürülmez.

Windows resolve compositionı önce conversion sonrası Path alır. Relative bayrağı açık ve kaynak is_absolute false ise executable parentına source join uygular; aksi halde Pathi değiştirmeden döndürür. Bu dalın raw0192d0 pseudo/assembly bağlantısı FIX21de kaydedilmiştir; normalization geri eklenmez. `0145d0` UTF-8 dönüşümü bu scope içinde yeniden uygulanmaz.

Linuxda `HostPathEffects` önceki native_path/model_path/std::filesystem projectionını taşır. POSIX `/absolute` pathinin Windows drive-root semantiğine zorlanması testleri bozardı; bu projection açık bir platform uyarlamasıdır. Windows bodylerinin Linuxda test edilmesi ile Linuxdaki actual adapterların host bodylerini kullanması ayrı kanıt sınıflarıdır. Yeni Host ownerın dört işlemi eski adapter ifadelerinin ortaklaştırılmasıdır; normalizasyon, encoding veya hata fallbackı eklenmez.

## Korunan sınırlar ve açık kalanlar

Altı mevcut runtime dosyası değişir: paths.hpp, environment.hpp/cpp, providers.hpp/cpp, online_session.hpp. Yeni runtime TU yoktur.116 diğer src ve1.060 vendor dosyası aynı kalır. Core/Windows production kaynak listeleri aynı; yalnız yeni integration testi graph/CMake hedefine eklenir.

CFG/profil/SDK/default/avatar, module sibling, SURGE veri kökü, dosya sahipliği/reparse policy, failure-atomic persistence, provider lifecycle, handler/session/thread/hook ve vendor pinleri değiştirilmez. Yeni kabul şartı, input validation, kota veya cleanup davranışı yoktur. gulrak/filesystem kapsamı genişletilmez; DEP05 ayrı kayıttır.

D10 lexical source-binding alt kapsamı düzelmiştir. D10 tamamen kapatılmaz: UTF-8/native-path dönüşümü, IO/ownership ve native hata/exception/ABI sınırları ile actual Windows testleri ayrı kalır. Bunların hepsine yalnız Linux PASSinden eşdeğerlik sonucu çıkarılmaz. Toplam15 review/partial kayıt korunur;15 doğrulanmış bug demek değildir.

## Testlerin anlamı

Yeni fixture Linuxda71 kontrol içerir:60 literal Windows lexical ve11 actual platform-selected caller kontrolü. Windows literal bölümü empty/drive-relative/drive-case/UNC/device/rooted/mixed-separator/dot/trailing-separator, unpaired UTF-16 ve embedded NUL değerlerini OS IO yapmadan işler. Bu inputlar source kod birimi sözleşmesidir; Windows APIlerinin hepsinin bunları kabul ettiği iddia edilmez. Actual bölüm Configuration/License/Content/OnlineSession nesnelerindeki dokuz lexical çağrıyı, resolverı ve Linuxa özel POSIX absolute ayrımını sınar. Windows derlemesinde aynı compile-time alias statik assertionla WindowsEffects olmalıdır; bu ortamda native yürütmesi NOT_RUNdır.

Önceki tüm C++ testleri ve FIX21deki26 spelling/link kontrolü byte-identicaldır. Eski normalization policy negatifinin mutation hedefi taşınan return çağrısına uyarlanır; aynı ret beklentisi korunur. İki yeni source-policy negatifi Windows owner aliasının ve application join delegationının bypassını reddeder. Yeni ownerın parent/filename/absolute seçimlerini bozan hedefli mutantlar değişmemiş fixturela reddedilmelidir. Test beklentileri implementasyonu geçirecek şekilde gevşetilmez.

Debug, CMake Release/CTest, ASan-UBSan, exact ZIP temiz build replay ve source/package/PE/Golden sonuçları final source digest ve ZIP SHA ile evidence/FIX22 ve dış FINAL_QAda bağlanır. Geçmiş stage PASSleri güncel kabul sayılmaz. Golden araçları ve upstream dosyaları değişmez; synthetic Golden/PE testleri gerçek DLL kabulü değildir.

Linux GCCnin licensing/license.cpp54 ve106daki iki mevcut noreturn uyarısı korunur; susturmak için yeni abort/terminate/exit davranışı eklenmez. ASan/UBSan leak denetimi kapalı çalıştırılır; LSan leak-free iddiası yoktur. Windows/MSVC/DLL/native IO/IOCP/provider/hook/LastError/Lease/CRT/game ve gerçek DLL Golden NOT_RUNdır. Current master version9 revision2026-10-25.1 yeniden kontrol edildi. AGENTS/metadata/QA kendi başına davranış yetkisi üretmez; canonical pointer ve diğer FIELD projeleri değişmedi.
