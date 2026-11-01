# Bağımsız Config2 uygulama ve hash modeli incelemesi

**PASS — bildirilen normal kaynak akışı kapsamı içinde engelleyici bulgu yok.** Üretim kaynakları değiştirilmedi, bağımsız test tekrarı yapılmadı. Root'un kayıtlı95 application kontrolü ve hash modülünün kayıtlı4929 kontrolü okundu; son ortak derleme sonucu ayrıca common-validation/build-record ile belgelenir.

İnceleme, `work/config-consumer-audit.md/json` içindeki çağrı yerleri ve retained `019ad0`, `0162f0`, `0165c0`, `016ad0`, `017440`, `017570`, `011ba0`, `012820`, parser scalar/object move pasajlarına karşı yapıldı. Yeni application kaynağı eski configuration helper semantiğini kullanırken mutable lookup ve kopya geçmişini yeniden kurar.

Doğrulananlar:

- Raw byte okuma, tek BOM işlemi, fresh parser ve yalnız seçili Config2 ilk kopyası. Owned parser/input yaşamı log'dan önce biter. Eksik Config2 boş mantıksal değer verir. File/parser hataları log ve ayarlardan önce yayılır.
- Aynı section destination Game → Emulator → User → Content → Entitlements → Achievements boyunca yeniden kullanılır. Base/override ve language flag guard'ları gerçek yürütme noktasında değerlendirilir. Helper/provider çağrıları lookup'larla iç içedir; önceden bir sıra planı yürütülmez.
- Beş global attribute `operator[]` ifadesinde section text lookup'u ayrı statement ile önce bağlanmıştır. Böylece C++ argument evaluation sırasının global map insertion'ı mutable lookup'tan önceye taşıması önlenir. Ön incelemede bildirilen bu nokta düzeltildi; açık bulgu kalmadı.
- Content record Name/Version/State ve Entitlements record Group/Version/Type/EntitlementTag sırası tamdır. Ayrı record destination iki döngüde yeniden kullanılır; section node'a geri yazılmaz. Native record copy de name/value string kopyasından sonra çocuk tablosunu assign ettiği için logical full Node assignment burada normal değer/sıra semantiğine uygundur.
- AchievementIDs lookup'u AchievementsSet'ten önce ayrı statement ile yapılır. Names/Totals/Overrides child list'leri section copy sonrası doğrudan gezilir; fazladan copy eklenmez. SteamIdPrefix guard'ı güncel/korunmuş state.steam_app_id değerini kullanır. Totals/Overrides bulunmayan global achievement oluşturmaz.
- Hash modeli unsigned byte FNV, binary32 factor1 büyümesi, sekiz kat küçük-kapasite yolu, power-of-two rehash, duplicate stable entry, source-capacity clone ve destination-capacity assignment'ı ayırır. Parent rehash öncesi eski destination traversal konumlarının recursive payload reuse'u korunur. Yeni kayıtlar clone, fazla tail kayıtları silmedir; self-copy rehash yapmaz.
- Parser duplicate scalar fresh table kurar; object move/swap replacement subtree container'ını korur ve eski geçici nesne yok edilir. Final first-created unique slot'lardan parse tablosunu yeniden kurma bu gözlenen normal action yolları için uygundur.
- rounded_buckets çağrıları gözlenen [8,2^59] domainiyle sınırlandırılmıştır. Daha küçük raw017570 argümanları için model guard'ı özgün hata davranışı diye sunulmaz. Ön incelemede fark edilen requested0 kapsam belirsizliği giderildi.

Test kaynakları güçlü ayrımlar içerir:64/128 geçmiş kapasitesi aynı Content anahtarlarını farklı sıraya sokar;65 child alanından kalmış512 bucket tablosu Totals conversion hatasından önce hangi güncellemenin tamamlanacağını değiştirir; registry widen callback'i daha sonraki language flag kararını değiştirir; override dolu koleksiyonları kabul ederek User sonrasında durur. Hash testlerinin ayrı std::list::splice oracle'ı ve eski sekiz-key modelle720 permütasyon karşılaştırması üretim regroup algoritmasından ayrı bir sınama sağlar.

README ve CMake tarifi incelendi. Yeni giriş açıkça `config_application::load_and_apply`'dır; eski initialize_configuration çağrı grafiğinin otomatik yükseltildiği iddia edilmez. CMake ayrıca çalıştırılmış sayılmaz. Trace yalnız kaynak tanılamasıdır ve kendi allocation etkileri özgün davranış değildir.

Sınırlar: unique-key/factor1 normal akış; ayrık owned source/destination veya self-copy. Native MSVC layout/allocator/SEH ve fault zamanlaması, concurrent/overlapping subtree mutation, tüm path/locale/license/provider işlemleri, global startup ve bütün DLL eşdeğerliği doğrulanmadı. NativeFactory yalnız uygulamanın kendi oluşturduğu fixture dosyasında kullanılır. Gerçek kullanıcı CFG/token içeriği okunmadı; özgün DLL/oyun çalıştırılmadı veya değiştirilmedi.

İncelenen dosyaların hash'leri ve kanıt kimlikleri `review.json` içindedir.
