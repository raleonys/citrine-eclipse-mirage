# FIX16 — Başlangıç ve ek DLL yükleme sırası incelemesi

Karşılaştırma tabanı tam kullanıcı RE paketidir. Bu dosya kod değişikliği veya sırayı geri alma yetkisi üretmez. D01/D12 açık kalır.

| Aşama | RE paketinde | FIX15/FIX16 uygulamasında |
|---|---|---|
| Giriş | application_dll_main(reason1) doğrudan initialize çağırır | DllMain CRT-aware worker başlatır; ağır kurulum workerda |
| Config | initialize içinde environment/console/config senkron | Service.prepare → Application.initialize |
| Server | first_029ea0 detached worker başlatılır | prepare socket bind/listen yapar; IO loop activate sonrasında |
| Hook ara aşaması | between_workers_00e310 server launch sonrası | prepare sonrası hooks.install |
| Provider | second_035b60 ayrı detached worker | activate içindeki initialize_online senkron |
| ExtraDllsFromMain | boolean_worker(true) bootstrap çağrı zincirinde senkron; bekleme yok | Her iki liste activate sonrası aynı runtime_worker içinde |
| Diğer ExtraDlls | boolean_worker(false) ayrı detached işçi; load_phase önce wait_steps(2), sonra load ve finish flag | İkinci liste ilkinden hemen sonra; source phase helper kullanılmıyor |

`from_main` burada kaynak fonksiyonun boolean phase seçicisidir. Bu ad, DllMainin her zaman oyun prosesinin ana threadinde çağrıldığının kanıtı değildir. Kaynakta true kolu initialize çağıran thread üzerinde senkron çalışır. Native executableın threadini varsayarak kod taşınmaz.

## Neden yalnız çağrıyı taşımak doğru bir düzeltme değil?

Aday Application.connect ancak initialize_online tamamlandıktan sonra ready olur. Asio accept loop da provider initialize bittikten sonra başlar. Bu yüzden from_main listesini activate öncesine taşımak, kaynakta zaten ayrı server workerı çalışırken gerçekleşen yükleme ile aynı akışı kurmaz. Yüklenen DLL LSX bağlantısı beklerse mevcut hazır olmayan servisle bekleme sorunu doğabilir; bu bir risk çıkarımıdır, test edilmiş game sonucu değildir.

Yalnız kaynak load_phase_035850 fonksiyonunu çağırmak da yeterli değildir: wait_steps(2)nin gerçek readiness sahipleri, worker finish flagi ve cancellation/shutdown yaşamı bağlanmalıdır. Mevcut manuel insert-failure cleanupı kaldırıp kaynak allocator-failure yan etkilerini geri getirmek, kullanıcının açık resource ownership talebiyle ayrıca değerlendirilmelidir.

## Somut sonraki tasarım işi

1. Config, bound-listener, running IO, provider-ready ve extra-worker-ready aşamalarının tek sorumlularını çıkar.
2. Source true/false phase yüklemelerini bu aşamalara bağla; bütün akışı loader lock içine geri taşıma.
3. Async provider sürerken LSX connection ve profile state sözleşmesini açıklaştır; thread sayısı veya concurrency varsayımı ekleme.
4. Extra DLL LSX çağrısı, provider başlangıç hatası ve shutdown-during-wait senaryolarını somut testlerle kur.

Bu dört iş tamamlanmadan D01/D12 kapanmış sayılmaz. FIX16da başlangıç, provider, wait ve extra DLL kaynak dosyaları FIX15 ile byte-identical kaldı.

## Kaynak atıfları

- anadius-source/bootstrap.cpp: Bootstrap::application_dll_main / initialize / start_detached
- anadius-libraries-source/libraries.cpp: load_phase_035850
- anadius-server-source/server.cpp: server_start_029ea0 / construct_server_024480
- src/windows/dll_entry.cpp: runtime_worker / ExtraModules::load
- src/transport/service.cpp: prepare / activate / wait_ready_or_stopping
- src/application/application.cpp: initialize_online / Application::connect
