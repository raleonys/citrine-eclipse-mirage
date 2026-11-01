# REDFIELD RF29 — üretim/test sınırları ve refactor kayıt denetimi

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

Refactor bu tur kaynak/build kodunda uygulandı. Yalnız testlerde kullanılan
memory_allocator.cpp veptd_runtime.cpp tests/support/runtime altına taşındı;
iki header tests/support/include/preloader_l/runtime altında. İki Windows test
host'u tools yerine tests/native altında. Altı dosyanın byte'ları RF28 ile aynı;
262 uygulama dosyasının hiçbir implementation gövdesi değiştirilmedi.

Üretim include listesinde test-support yolu yok. Test support CMake target'ı
bu yolu kendi tüketicilerine aktarır;core target'a geri aktarmaz. Python builder
üretim/regresyon/test-support/native-entry/host rollerini aynı graph'tan türetir.
Objeler objects/<rol> altında ayrıdır. Native define yalnız kayıtlı DLL entry
varyantında kabul edilir. Üretim kaynak/header'larının test-model deklarasyonuna
bağlanması reddedilir. Derlenen üretim objelerinde generic allocator/PTD model
sembolleri bulunmadığı nm ile doğrulandı;test-support objelerinde gerçekten var.

Yeni dahili build_contract.py builder,source verifier veCMake preflight tarafından
ortak kullanılır. metadata/IMPORT_REFACTOR.json270 uygulama/build dosyasını exact
hash,source/lisans,owner/rol,ömür/bağımlılık,hata veadaptation planıyla kaydeder.
Dört vendor entegrasyonu ayrıca kaydedilir;upstream byte/pin/lisansları korunur.
Eksik/çift kayıt,stale hash,origin exemption,yanlış rol,kötü source,boş owner ve
kanıtsız/yalnız isim değişikliğiyle QUALIFIED iddiası reddedilir. QUALIFIED kayıt
gerçek subject/hash ve güncel dependency hash'lerine bağlı evidence gerektirir.
Bu denetim kayıt tutarlılığı ve build sınırıdır;anlamsal uzman incelemesinin veya
algoritma idealitesinin otomatik matematiksel ispatı değildir.

**270 dosya ve4 vendor entegrasyonu yeni kurala göre hâlâPENDING;QUALIFIED sayısı0.**
Önceki modül incelemeleri otomatik yeni-kural onayı sayılmadı. Normal yerel build
bu durumu açıkça raporlayarak çalışır. verify_source.cmd --require-refactor-complete
ise mevcut durumda exit1 veIMPORT_REFACTOR_PENDING verir;bu beklenen kapanış
reddidir,normal DLL derlemesini engelleyen yeni bir release kapısı değildir.

| Kontrol | Sonuç |
|---|---|
| Temiz GCC C++20 |58 production core+2 test support;157 object,97 linked çıktı PASS|
| Regresyon |96/96 PASS;RF28 stdout/exit96/96 aynı|
| Yeni boundary/import denetimleri |30 PASS veya beklenen ret|
| Gerçek header görünürlüğü |Production PTD header'ını bulamaz,test include ile derlenir|
| Gerçek object/symbol denetimi |Production model sembolleri yok,test-supportta var|
| Mimari/encoding/environment/POSIX runner |13/16/13/7 beklenen sonuç|
| Manifest/adoption/recipe/ZIP/deterministic repack/fresh extraction |PASS;2 paket bozma beklenen ret|
| GetConfig legacy diagnostic |FAIL_OBSERVED,exit14;96 pozitif testten ayrı|
| RF29 Windows/MSVC,Debug/CRT,native,Golden,oyun,CMake |NOT_RUN|

ASan/UBSan bu tur yeniden çalıştırılmadı;C++ gövdeleri değişmedi. CMake bu ortamda
yok;preflight wiring native configure/build çalışmış gibi sunulmaz. Mevcut
native fault/export varyantı DLL entry içinde macro ile guardlı kalır;yalnız
host kaynakları bu tur ayrıldı. Tam native composition ayrımı M14/M20'de açık.
Python Windows builder hâlâ /O2 /MT ve build/msvc_x64 kullanır. Ayrı Debug/CRT
operator akışları,dist finalization veURSYNEX→ARYRA→MUSTAVEX→CORVEXON→FALCION
aktarımı sonraki M20 işidir;boş klasör eklenerek tamamlanmış gösterilmedi.

Güncel governance2026-10-21.1 normative mirror'a bağlandı;Library-only debt
appendix taşınmadı. GPL-3.0-only,preloader_l,surge.cfg,üç export,35 binding,yedi
hook,vendor/MinHook pinleri veHF19/CURRENT korunur.1585 ordered recipe ve1327
adoption doğrulanır. Bütün refactor tamamlandı denmez;4 önceki kaynak incelemesi
kapalı,16 modül açık veimport qualification ayrıca bekliyor.

Windows'ta scripts\build_orchestration\build_verify.cmd çalıştırılır.
Başlık RF29 BUILD BOUNDARY REFACTOR 01 olmalıdır. Nihai developer DLL mevcut
build\msvc_x64\preloader_l.dll yolundadır;surge.cfg DLL'in yanında kalır.
Parent RF28 ZIP SHA-256:ca828327b6aeaf9a5e307bd51d1885e1c18efc0a94a15d52ca44c7016d521f07.
