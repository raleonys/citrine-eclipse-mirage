<!-- PENDING-SOURCE-CHANGES -->
2026-11-05: Kullanıcının açık isteğiyle güncel tam ortak derleme ve testler geçti. Üretim DLL linki başarılı; DLL yüklenmedi/çalıştırılmadı. 296 nesne, 356 test grubu ve 19 framework terminal senaryosu doğrulandı. Güncel kaynakların son tam derlemeye göre bekleyen farkı 0. Bu sonuç tam davranış/loader/native ABI eşdeğerliği kanıtı değildir. Kanıt: `dll-critical-path/current-rebuild.evidence.json`. Önceki aşama metinleri tarihseldir; ZIP sonucu ayrı paket kaydındadır.
<!-- END-PENDING-SOURCE-CHANGES -->

# B3 — Somut kaynak backend'i ve beklenebilir worker ömrü

**Güncel kaynak notu, henüz derlenmedi:** `stop_and_join`, wait/close
sınırından gelen istisnada kalan kayıtları kalıcı hata durumunda tutar; aynı
sayısal HANDLE yeniden denenmez. Normal timeout/WAIT_FAILED/close FALSE
sonuçlarının mevcut retry davranışı korunur. Dört test senaryosu yalnız kaynak
olarak eklendi; çalıştırılmadı. Önceki derleme sonuçları aşağıda tarihsel olarak
korunur. Ayrıntı: `../anadius-owned-jobs-source/native-job-gap.md`.

Bu paket, B2'nin `RuntimeOwner` sınıfına verilebilen somut `SourceBackend` ile thread etkilerini saran somut `TrackingThreads` sınıfını ekler. Mevcut uygulama gövdeleri yeniden kullanılır. Yeni özgün native gövde veya native sağlayıcı kapanışı **0**; üretim DLL hazırlığı henüz tamamlanmış değildir.

## Mevcut kaynakların bağlanması

`SourceBackend`, mevcut `ProviderOwner` ve `Services` arayüzlerini uygular. Yeni bir zorunlu sanal arayüz eklemez. `Resources`, seçilmiş somut sağlayıcı nesnelerini `unique_ptr` ile aynı backend ömründe tutar; isteğe bağlı backing-state sahibi, ona bağımlı sağlayıcılardan sonra yıkılır. Bu nesnelerin gerçek OS/harici sağlayıcı uygulamaları çağıran tarafından seçilmelidir. `RuntimeSeed` ve B2 başlangıç config durumu açık girdidir; özgün global başlangıç değerleri tahmin edilmez.

Altı `Services` metodu şu yedi mevcut uygulama algoritmasına bağlanır:

| Kaynak yolu | Mevcut gövde |
|---|---|
| Birinci worker | `server_start_029ea0` |
| İkinci worker | `worker_035b60` |
| Ana/worker DLL yükleme aşaması | `load_phase_035850` |
| Worker'lar arası host kurulumu | `initialize_host_00e310` |
| Yüklenmiş DLL temizliği | `unload_035ab0` |
| Online nesne/modül temizliği | `cleanup_0366f0` |
| Hook temizliği | `cleanup_hooks_0102b0` |

Geri kalan **11 Services politikası** zorunlu alt sağlayıcıya yönlendirilir. Bu yönlendirmeler, özgün native uygulamanın kurtarıldığı veya sağlayıcıların tamamlandığı anlamına gelmez.

Server, online ve library aşamaları aynı `Execution` nesnesinin üç flag'ini kullanır. Online bekleme/finish ve library bekleme/finish çağrıları alt mock/global varsayımlarına bırakılmaz; mevcut Execution kaynak gövdelerine bağlanır. Config DLL listeleri `bind_state` sırasında kopyalanmaz. Ana liste gerçek main-load aşamasında, worker listesi ise kaynak `wait_steps(2)` tamamlandıktan sonra güncel config üzerinden hazırlanır. Yüklenmiş modüller vektörü aşamalar arasında korunur ve unload gövdesinin gözlediği davranış gereği temizlenmez. Typed container/path köprüsü özgün MSVC nesne ABI'si veya eşzamanlı vector mutasyonu için eşdeğerlik iddiası değildir.

Worker fonksiyon pointer'ları, tek image için üretim kodunda tanımlı route binding ve kısa kilit altında alınan lease ile backend'e ulaşır. Router `bind_state` sırasında yayımlanmaz; B2 native Binding kurulduktan sonra `register_exit` yolunda etkinleşir. Uygulama çağrısı boyunca router kilidi tutulmaz. Başarısız kayıt sonrası kaynaklar ve route kontrollü kapanışa kadar canlı kalır. Router kaldırıldıktan sonra saklanmış geç worker pointer'ı, eski backend'i okumadan terminal sözleşmeye gider.

## Ayrılmış thread'leri bekleme

`TrackingThreads`, mevcut `threads_recovered::Effects` uygulamasını kullanır. Başlatma algoritması tekrar yazılmaz. Kayıt, alt create çağrısından **önce** ayrılır; başarılı create sonrasında `Context` alanları okunmaz, çünkü worker onu çoktan serbest bırakmış olabilir.

Başarılı create'in döndürdüğü HANDLE için SDK `DuplicateHandle` ile ayrı bir bekleme handle'ı alınır. Bootstrap'ın özgün handle'ı kapatması bu kopyayı geçersiz kılmaz. Kayıtlar özgün HANDLE değeriyle tekilleştirilmez; Windows bu değeri yeniden kullanabilir. Bookkeeping, duplicate ve kapanış çağrıları etrafında LastError korunur.

Duplicate başarısızlığı veya create'in bir worker başlatıp başlatmadığı bilinmeden exception çıkması, takibi kalıcı olarak zehirler. Çalışan worker için uydurma rollback yapılmaz; backend bu durumda quiescence için `true` döndüremez. Başarısız normal create dönüşü ise kayıt rezervasyonunu kaldırır. Bekleyen create, timeout, başarısız wait veya join-copy close hatası, henüz tamamlanmayan kayıtları korur. Başarıyla bitmiş wait/close ön eki korunarak yeniden denenebilir. Bekleme sırasında registry kilidi tutulmaz.

Canlı veya zehirlenmiş kayıtlarla örtülü tracker yıkımı terminal sözleşme ihlalidir. Bu yeni kaynak ömür politikasıdır; özgün thread fonksiyonunun yeni bir native gövdesi sayılmaz.

## Kapanışın açık sınırı

`stop_and_join`, önce zorunlu `stop_sources` çağrısını yapar. Bu callback; yeni uygulama/worker/hook girişlerini ve geç exit/terminate kayıtlarını durdurmalı, önceden oluşturulmuş worker'ların tamamlanmasına izin vermelidir. Kullanımdaki sağlayıcıları worker'lar bitmeden unload edemez. Yalnız `std::atexit` kullanmak iptal edilebilir bir kayıt sistemi sağlamaz; gerçek registrar ve shutdown politikası ayrı açık iştir.

Kaynaklar durduktan sonra tracker gerçek duplicate handle'ları bekler. Router ancak kayıtlar boş ve aktif lease sayısı sıfırken aynı kilit altında kaldırılır. `active_calls()==0` tek başına yeterli değildir. False, exception, pending veya timeout owner/backend/route ömrünü korur. Kontrol çağrıları tek thread'e aittir ve reentry reddedilir; uygulama sağlayıcılarının kendi senkronizasyonu zorunludur.

`stop_and_join`, özgün exit cleanup'ını kendiliğinden çağırmaz. Mevcut `Bootstrap::on_exit` kaynak sırası library unload → online cleanup → hook cleanup olarak korunur. Gerçek geç kayıt iptali ve bu cleanup'ın doğru zamanda işletilmesi üretim politikası tarafından ayrıca kurulmalıdır.

## CRT ve PE giriş kararı

Yeni C++ sahiplik/bookkeeping nesneleri derlendikleri araç zincirinin CRT'sini kullanır. Gelecekteki compiler-CRT DLL girişinin yalnız yeniden kurulan uygulama katmanını çağırması planlanır; kurtarılmış dış CRT Entry ikinci kez otomatik çağrılmaz. Bu paket PE girişini kurmaz veya DLL yüklemez. Recovered thread sağlayıcılarının aynı açık runtime/PTD durumunu kullanması, harici C++ ABI ve normal/exception cleanup eşdeğerliği ayrıca doğrulanmalıdır.

## Doğrulama ve sayım

Yetkili gruplar `../source-workspace/build.ps1` içindeki `bootstrap_backend` ve `bootstrap_backend_integration` hedefleridir. Ara hızlı kayıtlar `work/b3-tracking-quick-results.json` ve `work/b3-integration-quick-results.json` içindedir; son gerçek sayılar ortak build'e bağlı validation kaydına yazılır. Module standalone build çalıştırılmış sayılmaz. Çekirdek testler yalnız kendi EXE alt süreçleri ve bize ait bir Win32 worker kullanır; gerçek uygulama socket/hook/harici DLL işlemleri kontrollü sağlayıcılarla sınanır.

B3 iki üretim nesnesi ve yedi yeni kaynak/test dosyası ekler. Beklenen ortak toplam 213 nesne (209 C++/4 C), 237 normal test grubu ve 19 framework terminal senaryosudur; bunlar ortak build PASS olmadan tamamlanmış sonuç olarak kullanılmaz. Önceki 696 kaynak korunur. Yeni composition-only dosyalar, `composition-only.json` içinde adres son ekli yönlendirmeleri görünür tutar; native mapped487/eligible compiled491/normal-claim124 ve önceki52native closure artırılmaz. Tam raw-all sembol listesi ayrıca korunur.

Sıra **1 DLL kritik yol → 2 mesaj zinciri → 3 XML işlevsel paketleri → 5 seçici CRT → 4 version preloader**. Genel CRT leaf kuyruğu beklemededir; numeric0033c0 XML sırasına ertelenmiştir. Orijinaller çalıştırılmaz/değiştirilmez, gerçek CFG/token/achievement içerikleri okunmaz, engelli version girdisi veya eski üretilmiş DLL yeniden denenmez. Tam DLL tamamlanma yüzdesi ölçülmedi.

## B3 ortak doğrulama

**59 çekirdek +170 entegrasyon =229 kontrol PASS**. Ortak build213nesne(209C++/4C),237normalgrup+19frameworkterminalsenaryosu içerir. Önceki696kaynak aynı;toplam703kaynak. Yeni native gövde/kapanış0;487adres/491eligible tag/52nativeclosure korunur. Bileşim ve raw-all defterleri ayrıdır. Gerçek sağlayıcı factory/registrar/PEentry sınırları açıktır.
