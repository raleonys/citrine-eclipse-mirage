# FIX18 — Kaynak başlangıç aşamalarını bağlama

FIX16 tam kaynak paketi üzerinde, FIX17de kanıtlanan D01/D12 başlangıç bağımlılıkları uygulandı. NON_RELEASE. Karşılaştırma tam RE kaynak paketidir; özgün DLL runtime eşdeğerliği iddiası yoktur.

## Ne değişti ve neden?

Önceki uygulama providerı senkron bitiriyor, IOyu sonra başlatıyor ve her iki ek DLL listesini aynı runtime workerda yüklüyordu. Network beklemesi yalnız servis ready bayrağına bakıyor, hazırlayan thread beklemeyi atlıyordu. Bunlar kaynak bootstrap/Execution akışından farklı entegrasyon kararlarıydı. İlk tarihsel gerekçeleri kaydedilmemiştir; Asio upstream bunları gerektirmez.

Kaynakta server işçisi bağımsızdır; provider ilk server bayrağını bekler. True ek-DLL seçicisi bootstrap çağrı zincirinde senkrondur. False seçicisi ayrı işçide ilk iki aşamayı bekler ve üçüncü bayrağı yayınlar. Network count=-1 üç bayrağı bekler. FIX18 bu bağımlılıkları module-owned, join edilen işçilere bağlar.

| Sorumluluk | FIX18 davranışı |
|---|---|
| Config + server | Owned bootstrap `prepare` içinde config/bind yapar, birinci bayrağı yazar ve gerçek Asio accept/IOyu başlatır |
| Provider | `activate_async` ayrı owned işçi başlatır; ilk aşamadan sonra kaynak worker çalışır, normal tamamlanmada ikinci aşama yayınlanır |
| Main seçicili ek DLL | Aynı bootstrap çağrısında `load(true)`; yeni wait veya finish eklenmez |
| Worker ek DLL | Ayrı owned işçi ilk iki aşamayı bekler, `load(false)` bitince üçüncü aşamayı yayınlar |
| Network | Kaynaktaki count shared StartupSteps predicateine geçer; init-thread bypass yok |
| Shutdown/hata | Cancellation phase başarısı değildir. Hata ownera exception olarak taşınır; ready false kalır. İşçiler bitmeden state/modüller yok edilmez |

Server birinci bayrağı, IO callbacklerinin çalıştığına dair ek bir garanti değildir; kaynakta da bayrak accept/run öncesindedir. `from_main` proses ana-thread garantisi olarak yorumlanmaz. Ağır kurulum DllMain/loader lock içine taşınmadı.

## Profilin atomik yayımı

Provider native çağrılarının tamamı boyunca request state mutexini tutmak IOyu ayırmayı işlevsiz bırakabilirdi. `OnlineSession` yalnız providerın değiştirdiği identity/avatar/online alanlarını yerel sonuçta toplar. Kaynak `worker_035b60` algoritması ve cache alanı aynı kalır. Normal tamamlanınca profil tek state kilidi altında yayımlanır; allocation yapan profil oluşturma committen önce yapılır. Hata yarım provider profilini görünür kılmaz.

Hazırlanmış uygulama provider devam ederken yapılandırılmış yerel profil/greeting sunabilir. Tamamlanmış provider profili sonraki istekte görünür. Bu, kullanıcının mevcut failure-atomic/ownership isteğiyle açıkça belgelenmiş entegrasyondur; kaynak global alanların getterlar arasında kısmen değişmesi birebir taklit edilmez. Resmî/custom avatar önceliği korunur. Yeni profile validation, boyut/kimlik limiti veya fallback eklenmedi.

## İptal ve sahiplik

Kaynağın flag predicatei korunur; eski 20ms polling ve iptal edilemeyen bekleme taşınmaz. `StartupSteps::wait` iptalde false döner. Genel ready yalnız üç aşama normal tamamlandığında verilir. Public C export/ordinal değişmedi.

Native DllShutdown stop eventine ek olarak lifetime korumalı aktif Service üzerinden phase beklemelerini iptal eder. Runtime owner, Servicei silmeden önce bu işaretçiyi exclusive SRW lock altında temizler. Socketler/handlerlar durdurulur; provider ve ek DLL işçileri join edilir; sonra provider/extra modüllerinin mevcut cleanupı çalışır. Native provider/DLL içindeki keyfi bir bloklanan çağrı zorla sonlandırılmaz; mevcut timeout sonucu modülü pinned durumda tutabilir.

## Kapsam ve kalan borç

Yedi mevcut runtime dosyası ve bir yeni header değişti. Diğer 113 mevcut src dosyası ve 1.060 vendor dosyası FIX16 ile byte-identical kalır. Handler worker sayısı, session FIFO/backpressure, hook batch politikası, registry serialization ve kaynak license/process-exit kararı değiştirilmedi. Golden araçları ve test beklentileri korunur.

D01 kaynak phase kapsamı içinde düzeltildi; raw detached CRT yaşamı yerine kullanıcının istediği owned bootstrap/join düzeni korunur. D12 **kısmi**: network/worker wait countları geri bağlandı; registry ClientValue wait hâlâ yalnız port kontrolüdür. Bunu mevcut global callback mutexi altında körlemesine üç-aşama beklemesine çevirmek provider/callback bağımlılığı oluşturabilir. Registry wait ve reentrancy için ayrı düzeltme gerekir. Toplam açık inceleme/kısmi kayıt **17**; hepsi doğrulanmış bug sayılmaz.

D11deki OutOfEvidenceInput özgün DLL’in aynı C++ exceptionı attığına kanıt değildir; reconstructed null-dereference kanıt sınırıdır. Null-factory politikası bu turda değiştirilmedi. D06 kapsamında request serialization, callback mutexi ve attempt locku açık kalır; yalnız provider native initializationın application state lockunu tutması kaldırıldı.

## Testin neyi kanıtladığı

Mevcut test beklentileri değiştirilmeden yeni integration fixture eklendi. Kaynak count predicatei 40 flag/count birleşiminde denetlenir. Gerçek TCP ile provider başlamadan greeting/local profile, true seçicinin synchronous callerda ve false seçicinin owned başka threadde çalışması, ikinci/üçüncü aşama beklemeleri, provider/extra hatası ve shutdown cancellation sınanır.

Linux test executableı yalnız libc `stat` sınırında gerçek provider-file sorgusunu geciktirir; sonra çağrıyı değişmeden kernel syscallına yollar. Böylece gerçek OnlineSession/Application/Service kodu bekletilir. Production selector, provider-root override veya upstream patch yoktur. Provider yok dalı gerçektir; provider hata vakası Linuxun mevcut native-provider-requires-Windows dalıdır. Bu test Windows provider/IOCP/hook testi değildir.

Ham QA sonuçları evidence/FIX18 ve dış FINAL_QA dosyasındadır. Portable/synthetic/Windows NOT_RUN ayrımı korunur; önceki sonuçlar taze sonuç diye aktarılmaz. Kaynak/hash scope kontrolü tek başına anlamsal doğruluk veya onay üretmez.
