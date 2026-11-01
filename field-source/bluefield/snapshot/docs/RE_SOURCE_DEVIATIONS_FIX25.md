# FIX25 — Avatar seçimi provider/cache işlemlerini engellemez

NON_RELEASE tam kaynak paketi, exact FIX24 ebeveyninden türetildi. Karşılaştırma kullanıcının tam RE C++ paketidir; özgün DLL binary eşdeğerliği talebi veya iddiası değildir.

## Somut fark ve neden düzeltildi

FIX24 QueryImage online dalında `&& !own` koşulu vardı. Kendi profil avatarında provider çağrısı, cache indirmesi, daha büyük boyutun `last_avatar` dosyasına kopyası ve saved-size/log işlemleri tümüyle atlanıyordu. Görünür SURGE/custom avatar seçimi kullanıcı talebidir; bu bütün side effectleri kaldırma yetkisi değildir. Tarihsel gerekçe bağımsız kaydedilmedi; Asio veya başka upstream gerektiriyormuş gibi sonradan açıklanamaz.

Kaynak `anadius-content-handlers-source/content_handlers.cpp::query_image_0314f0` online own/other için provider çağrısı yapar. Başarıda own ve saved_size < atoi(width)/208 ise kopyayı dener; kopya başarılıysa boyutu günceller ve loglar. FIX25 bu sırayı geri getirir. Own ResourcePath kullanıcı seçimi olarak kalır; other başarılı provider cacheini seçer. Raw Width yazımı korunur; yeni kota, isim sınırlaması, alt klasör, CFG veya vendor değişikliği yoktur.

## Raporun düzeltilen ayrımı ve korunan farklar

Önceki D16 açıklaması initialize_online avatar restoreundan söz ediyordu; mevcut FIX24 yayınlaması avatarı zaten değiştirmiyordu. Paket online.cpp dalında ise download/copy yoktur: cookie/last-avatar statından sonra avatar yolunu seçer. Bu dalın kaldırılması görünür seçim gereğinin parçası olarak korunur; sırf kaynağa benzeyecek diye yeni stat hataları üretilmez. identity/profile cache okuma-yazması değişmez.

Copy adapterı existing failure-atomic writerı kullanır; hata native CopyFileW false yerine exception olabilir. Bu davranış bu aşamada değiştirilmedi ve native eşdeğerlik diye sunulmaz. last_avatar kaynakta writable cachetir; kullanıcı custom yolunu aynı cache dosyasına alias ederse cache yazımı dosyayı etkileyebilir. Yeni alias kısıtı, alternatif layout veya sidecar hash politikası eklenmedi. Oracle packetin path-only avatar effectini mevcut explicit owner/size interfaceine testte uyarlar; Windows vtable/wchar ABI kanıtı değildir.

## Bağımsız denetim ve kalan iş

Literal kaynak function/helper bodyları test-only oraclea alınır; origin/archive/slice hashleri korunur. Matris provider/copy/log sırasını ve signed-size durumunu karşılaştırır. Başarısız kopyada boyut/log değişmemesi, eşit/küçük boyutta kopya olmaması, provider başarısızlığı ve hata sırası sınanır. Gerçek dosya testinde provider cachei/last_avatar oluşurken normal resmî avatar, sidecar ve custom dosya seçimi/byte korunması ayrı denetlenir. Parent FIX24 aynı testi geçemez. Content unit fixtureının iki eski beklentisi tam suppressionı kodladığı için açıkça güncellendi; diğer fixturelar ve Golden araçları korunur.

D16 `CORRECTED_AVATAR_EFFECTS_USER_SELECTION_RETAINED`; açık10→9. Kalanlar DEP05, D02, D06, D07, D13, D14, D15, Q02, D21. Source closure native provider, Windows build, oyun veya gerçek DLL Golden PASS değildir. Final çalıştırma sonuçları ayrı QA dosyasında verilmiştir.
