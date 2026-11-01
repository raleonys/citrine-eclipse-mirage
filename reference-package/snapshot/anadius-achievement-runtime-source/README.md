# Achievement grant: zaman ve kalıcılık bağlantısı

Bu modül, daha önce kaynakta olan `content_handlers_recovered::grant_achievement_030010` işleyicisinin UTC zamanı, XML belgesi ve Watcher INI kayıt çağrılarını gerçek kaynak modüllerine bağlar. `utc.cpp` içindeki `utc_time_041b60`, yeni kurtarılan bir normal uygulama yordamıdır. `PersistenceEffects` ve `bind_state`, bu rekonstrüksiyon için yazılmış bağlantılardır; ayrıca keşfedilmiş özgün fonksiyonlar sayılmaz.

`041b60` önce `time(nullptr)`, ardından sonuç -1 olsa da gmtime çağırır. Gmtime başarısızsa varsa epoch çıktısını 0 yapar ve boş string döndürür; formatlayıcı çağrılmaz. Başarıda dokuz tm alanı kopyalanır. Epoch pointer varsa, ham saat sonucu yerine bu kopyadan gerçek `041ff0` takvim hesabı yeniden çalıştırılır ve çıktı **formatlamadan önce** yazılır. Formatlama sonradan istisna üretirse epoch yazımı geri alınmaz. Mode baytı 0 ise `%Y-%m-%dT%H:%M:%S`, herhangi başka bayt ise `%Y-%m-%d_%H-%M-%S` seçilir. Sonuç, formatlayıcının mevcut uzunluklu tamponudur; kısmi/boş/NUL içeren sonuçlar için ek başarı denetimi yapılmaz.

`Clock` ve `Formatter` zorunlu sınırlardır. `HostClock`, mevcut CRT `std::time` / `_gmtime64_s`; `HostFormatter`, mevcut global C++ locale ile `std::put_time` / string stream uyumluluk seçimidir. Bunlar özgün 05eeb8/gmtime64/time_put gövdelerinin kaynak kurtarımı veya tüm locale/hata davranışlarının eşitliği değildir. Native STL/CRT iterator, allocation, exception ABI ve SEH eşitliği açık kalır. Önceki CRT kaynakları değiştirilmez.

`PersistenceEffects` dört metodu bağlar:

- UTC: yeni 041b60 normal akışı ve gerçek takvim hesabı.
- XML printer: mevcut `MemoryPrinter` ve `HandlerWriter`; belgeden CStr kopyası ilk NUL'de kesilir ve sonlandırıcı hariçtir. Bu, UTC stringbuf dönüşünün explicit-length davranışından farklıdır. Printer salt çıktı içindir; request attribute okuma kullanımı açık hata üretir.
- XML dosyası: gerçek `write_binary_03c1d0`; özgün grant çağrısı gibi BOOL sonucu yoksayılır.
- Watcher INI: gerçek `write_ini_01f9a0`; canlı `Paths::ini_path` ve aynı `State::achievements` kullanılır. Başka map verilirse yan etkiden önce açık domain hatası üretilir. Sessiz map/state kopyası yoktur.

`bind_state`, aynı configuration state, storage paths, mutex ve event kuyruğu üzerinden content handler görünümü kurar. String/map hücreleri canlı referanstır. Content/entitlement vektörleri span ile ödünç alınır; görünüm kullanılırken yeniden tahsis edilmemelidir. State, paths, clock, formatter, dosya sağlayıcısı, mutex ve event nesnelerinin ömrü caller sorumluluğudur. Diğer content handler etkileri abstract kalır.

Mevcut grant kaynağı ilerlemeyi kilit altında günceller; tamamlanmada event üretir, XML yolu doluysa Progress != 0 kayıtlarını yazar, sonra INI kaydını çağırır ve response'u üretir. Achievement mutex bu işlemler boyunca tutulur. Tamamlanmış kayıtta tekrar çağrı yan etkileri atlar. Hatalar önceki state değişikliklerini geri almaz. Bu modül bu gövdeyi değiştirmez; önceden zorunlu olan dört çağrıyı bağlar. Diğer achievement yolları, genel event/mutex başlangıcı, socket/online/global/DLL/ABI eşitliği tamamlanmış sayılmaz.

Testler kendi üretilen XML/INI dosyalarını ve sentetik clock/event/lock kayıtlarını kullanır. Özgün DLL veya oyun çalıştırılmaz; gerçek kullanıcı CFG/XML/INI/token dosyaları okunmaz. Test kapsamı ve sayıları `validation.json` ve ortak `common-validation.json`; komutlar `build.ps1`; statik kanıtlar `evidence.json`, adresli `evidence.asm.txt` / `evidence.pseudo.c.txt`, bağımsız `flow-audit.md/json` ve `review.md/json` içindedir. REA sunucu/sağlayıcı doğrulaması `rea-session.json` içindedir.

C++20 / LLVM-MinGW ile `-O2 -Wall -Wextra -Werror` kullanılır. `build.ps1` bağımsız test girişidir; `../source-workspace/build.ps1` ortak arşiv ve testleri üretir. CMake tanımı ayrıca verilir; burada ayrıca çalıştırıldığı iddia edilmez. Çıktı tam bir replacement DLL değildir.


Ortak doğrulama sonucu: **76 UTC + 163 grant/kalıcılık kontrolü = 239 yeni kontrol PASS**. Ortak proje138 üretim nesnesi,123 normal test grubu ve19 terminal senaryosuyla geçti. Bunlar tam DLL veya ölçülmüş davranış eşdeğerliği oranı değildir.
