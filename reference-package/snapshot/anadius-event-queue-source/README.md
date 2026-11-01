# Ortak olay kuyruğu: achievement, IGO ve mesaj tüketimi

`mutex_02c620` ve `queue_02c400`, iki uygulama getter'ının normal kontrol akışıdır. İkisi de signed `thread_epoch < guard` koşulunu, header çağrısını ve ardından `guard == -1` denetimini korur. Başlatan dal mutex için flags2 constructor; kuyruk için başlangıç DOWN olayının kopyayla eklenmesini ister. Ardından atexit callback kaydı ve footer çağrılır. Atexit dönüş değeri yoksayılır. Her yol caller'ın aynı mutex veya storage nesnesini döndürür. İki global guard ve TLS epoch sıfırdan tahmin edilmez; caller tarafından sağlanır.

Kuyruğun ilk olayı tam 68 bayttır:

```xml
<LSX><Event sender="EbisuSDK"><IGOEvent State="DOWN"/></Event></LSX>
```

`Initialization`, TLS/header/footer, gerçek mutex constructor ve cleanup kaydının zorunlu sınırıdır. Callback enum değerleri yalnız kimliktir; özgün VA'lar yürütülmez. Bu modül gerçek CRT thread-static guard, `_Init_thread_abort`, atexit tablosu veya OS mutex yaşam süresini yeniden yazmaz. Native exception/unwind/abort sırasında tamamlanmamış kurulumun temizlenmesi açık kalır. Provider istisnaları yutulmaz; normal kaynakta sonraki adımlar çalışmaz. Bundan native SEH eşitliği veya tamamlanmış retry/teardown sonucu çıkmaz. CRT kaynakları beklemede kalır.

`Storage`, native deque/proxy/string geçici nesnesi, allocation ve enqueue/pop işlemlerinin sınırıdır. `construct_seed_copy`, özgün temporary-string/deque/proxy/map kurma, seed copy ve temporary destruction işlerini kapsayan bir sözleşmedir. Alt 026670 growth, 034660 native move/SSO ve allocator gövdeleri bu aşamada tam kaynaklaştırılmış sayılmaz.

`OwnedStorage`, çalışan **seçilmiş FIFO uyumluluk backend'idir**: std::deque/std::string kullanır, açık uzunluklu baytları ve NUL'ü korur, push sonrası kaynak string'i mantıksal olarak boşaltır. Native ring-map kapasitesi, proxy, SSO, tahsis sayısı ve başarısız tahsiste kısmi state eşitliği iddia edilmez. Construct öncesi/tekrar construct/boş front-pop için açık kaynak kullanım hataları verir; bunlar özgün hata yordamları değildir. Yeni atomic size yayını, relaxed nonempty sorgusunun deque'ye dokunmadan yapılmasını sağlar; native bellek modeli rekonstrüksiyonu değildir. Diğer storage işlemleri caller kilidi veya tek yürütücülü kullanım gerektirir. Atomik count, genel eşzamanlı object lifetime/initialization çözümü sayılmaz.

Yeni `ContentQueue`, `IgoQueue` ve `MessageQueue` adapter'ları aynı Context/Storage/Mutex'i paylaşır. Ayrıca keşfedilmiş özgün yordamlar sayılmaz; önceki kaynak gövdelerine bağlantıdır. Kendileri kilit eklemez:

- Achievement tamamlanması, mevcut event lock altında lazy queue getter'ını çağırır ve achievement XML olayını ekler. `initialized_locked()` getter'ı çalıştırır; başarılı dönüşten sonra true verir. Bu, önceki content arayüzündeki sorguyu native lazy erişime bağlar ve çift seed üretmez.
- IGO, lock → lazy getter → açık DOWN enqueue sırasını kullanır. Soğuk kuyrukta ilk seed ile birlikte iki DOWN olayı oluşur; bunlardan biri silinmez.
- Mesaj işleyicisi, şifrelenmiş response başarıyla üretildikten sonra queue getter'ını **mutex'ten önce** çağırır. Soğuk tüketim dahi ilk DOWN olayını yaratabilir. Her uygun response en çok bir olayı tüketir; event encryption false dönerse de bir olay pop edilir. Şifresiz response, başarısız response encryption veya daha erken hata kuyruğu tüketmez. Bunlar mevcut 033d70 gövdesinin davranışıdır; bu modül o gövdeyi değiştirmez.

İki lazy nesnenin cleanup kayıt sırası erişene bağlıdır: soğuk tüketici kuyruk kaydını mutex'ten önce; soğuk IGO/achievement üreticisi mutex kaydını kuyruktan önce oluşturur. Sabit bir global cleanup sırası varsayılmaz; bu kayıtların gerçekten yürütülmesi ayrıca bağlanmalıdır.

Caller, Context/Guards/Initialization/Mutex/Storage ve adapter nesnelerinin ömrünü korumalı, gerçek native provider'ların guard/locking/cleanup davranışını ayrıca bağlamalıdır. Burada tam uygulama/global/OS/CRT/allocator/concurrency/ABI/SEH eşitliği kurulmuş sayılmaz. Testler sentetik guard/mutex/cleanup kayıtlarını, seçilen OwnedStorage'ı ve mevcut gerçek grant/IGO/message/XML kaynaklarını kullanır. Kullanıcı CFG/XML/INI/token dosyaları okunmaz, özgün DLL/oyun çalıştırılmaz.

Kanıtlar `evidence.json`, adresli `evidence.asm.txt` / `evidence.pseudo.c.txt`, bağımsız `flow-audit.md/json` ve `review.md/json` içindedir. Sunucu/sağlayıcı kimliği `rea-session.json`; testler `validation.json` ve `common-validation.json` ile bağlıdır. `build.ps1` bağımsız, `../source-workspace/build.ps1` ortak LLVM-MinGW C++20 doğrulama yoludur. CMake tarifi ayrıca verilir; ayrıca çalıştırıldığı iddia edilmez. Çıktı tam DLL değildir.


Ortak doğrulama sonucu: **2054 kuyruk + 133 entegrasyon = 2187 yeni kontrol PASS**. Ortak proje140 üretim nesnesi,125 normal test grubu ve19 terminal senaryosuyla geçti. Testler dosya I/O yapmaz; cipher/guard/mutex/cleanup kayıtları kontrollüdür. Tam DLL/native runtime eşitliği oranı değildir.
