# Kaynak varlığı ve bağımlılık envanteri

Kullanıcı sırası **1 → 2 → 3 → 5 → 4**; üst yol haritası [YOL-HARITASI.md](../YOL-HARITASI.md). Bu ilk envanter bir tamamlanma yüzdesi veya native eşdeğerlik kanıtı değildir. Üç ayrı görüntüden 2.381 yerel giriş içerir; görüntü SHA-256/adres anahtarları çakışmaz.

- `functions.csv`: Excel/metin editöründe filtrelenebilir işlev/adres/kaynak satırları.
- `functions.json`: tüm tanımlar, kaynak satırları, hashler, kanıt kayıtları, derlenmiş sembol adayları ve sınırlı çağrı ilişkileri.
- `providers.json`: pure-virtual bildirimleri ve aynı isimli kaynak override adayları. Bildirim sayısı açık engel sayısı değildir; derived zinciri ayrıca incelenir. Güncel adet summary.json içindedir.
- `dependencies.json`: son kaynak zincirinde kapatılmış 7 eski sağlayıcı, R1/R2/R3 normal gövdeleri ve native/CRT/preloader bitiş ölçütleri; 49 seçili çağrı kenarı ve 30 kanıt dosyası hash'i.
- `outside-canonical.json`: yerel giriş listesi dışındaki 24 adres etiketi; data/helper ve raw-gap ayrımı yapılmadan yerel işlev paydasına eklenmez.
- `object-symbols.txt` / `current-object-probe.json`: güncel 180 üretim nesnesinden aynı ajan tarafından yenilenen defined symbol gözlemi; bağımsız son inceleme değildir. Kaynak varlığı gözlemi tamamlanmış işlev sayısı sayılmaz.
- `baseline-object-symbols.txt` / `independent-probe.json`: ilk bağımsız önsayımın 177 nesne, 814 etiketli symbol satırı ve 435 yerel giriş adresi korelasyonu korunmuştur; tarihsel kayıttır.
- `mapping-audit.md/json`: bağımsız giriş/build/kapsam kontrolü; son üretilmiş tablo incelemesi beklemede olarak korunmuştur.
- `dependency-audit.md`: sonraki aşamaların okunabilir engel listesi.
- `inputs.json`: generator'ın okuduğu outputs dosyalarının tam hash/boyut manifesti; gerçek Downloads CFG veya diğer özel dosyalar okunmaz.
- `rea-session.json`: doğrulanan REA sunucu kimliği ve Ghidra sağlayıcısının oturum gözlemi; bu aşamada yeni native analiz başlatılmadı.
- `validation.json`: offline veri bütünlüğü ve parser fixture kontrolleri; bunlar yeni C++ davranış testleri değildir.

## Durum alanları

`source_definition_mapped`: adres etiketli veya açık belge eşlemesiyle C++ tanımı bulundu. Eski semantic model ile ayrıntılı gövde aynı satırda ayrı tanımlar olarak bulunabilir. Tam native gövde veya bağımsız DLL demek değildir.

`selected_upstream_adapter_only`: bilinen orijinal adrese yönelik seçilmiş upstream motor adapter'i var; özgün motor gövdesi kurtarıldı denmez.

`assembly_wrapper_mapped`: version dış export sarmalayıcısının MASM tanımı bulundu; C++ kaynağıyla karıştırılmaz. version DLL ayrı build/static PE doğrulamasına sahiptir; common object korelasyonunun false olması bu ayrı build'i geçersiz kılmaz.

`module_record_only`: kaynak modülünün kanıt dosyasında işlev kaydı var; kesin tanım eşlemesi kurulmadı. `no_definition_mapping`: bu mapper kesin tanım veya uygun modül kaydı göstermedi. Bu iki durumun hiçbiri kodun hiç yazılmadığını kanıtlamaz.

`compiled_symbol_correlated`: tagged kaynak tanımı aynı modülün mevcut defined object symbol'üyle eşleşti; inline gövde ayrı symbol üretmeyebilir. Object symbol'ü de semantic eşdeğerlik kanıtı değildir.

`binding_status=requires_explicit_dependency_review` ve `abi_equivalence=not_established` otomatik olarak yeşile çevrilmez. Kaynakla bağlanan mevcut send sağlayıcıları dependencies.json'da adresleriyle ayrı kayıtlıdır.

## Yeniden üretme

Workspace kökünden mevcut Python 3 ile `python outputs/reconstruction-inventory/generate_inventory.py`, sonra `python outputs/reconstruction-inventory/validate_inventory.py` çalıştırılır. Generator yalnız yerel outputs verisini okur ve bu klasöre JSON/CSV yazar; DLL'yi veya ağ API'sini çalıştırmaz. Girdi snapshot kayıtları yenilendiyse önce generator, sonra validator yeniden çalıştırılmalıdır.

Eski yaklaşık %80 tarihsel tahmini bu verinin ölçüsü değildir. İlk eşleme anadius'ta 434 source-definition, 6 adapter, 130 module-only ve 1.506 unmapped bulmuştu. R2 `021d10`, R1 `010040/008350`, R3 `025c40` kaynak gövdelerini ekledi; güncel sayılar summary.json içindedir. Seçilmiş byte adapter'leri genel CRT gövdeleri olarak sayılmaz. Version'da 197 MASM + 1 C++; preloader'da 5 explicit source + 102 unmapped eşlemesi sürer. Bu oranlardan tamamlanma veya bitiş tarihi hesaplanmaz.
