# Sunucu oturumu → XML / AES / dispatcher bağlantısı

Bu modül, mevcut sunucu `Session` kaynağını gerçek mesaj çekirdeği ve handler router'a bağlar. **Yeni kurtarılmış özgün yordam sayısı 0'dır.** `MessageHandler` ve `RoutedEffects`, daha önce kaynakta olan gövdeler için yeni üretim bağlantılarıdır. Yeni ASIO/socket backend'i, yerel port, network listener veya özgün DLL çalıştırması eklenmez.

`MessageHandler`, `server_recovered::Handler` arayüzünü uygular:

- `reset_02bc70`: mevcut gerçek cipher reset ve iki byte alanının sıfırlanmasını çağırır. Adapter constructor'ı reset eklemez. Önceki Session constructor ve accepted-start yolu iki mantıksal reset çağrısını zaten yapar.
- `initial_message_02d540`: gerçek XML yazıcısı ve challenge üreteci üzerinden greeting üretir. XMLPrinter BufferSize son NUL'ü içerir; string bu uzunluğu korur. Version, caller'ın canlı string hücresinden her çağrıda okunur.
- `process_message_033d70`: borrowed input view'ı açık uzunlukla kendi string'ine kopyalar; gerçek mesaj çekirdeği bu kopyayı hex decode/retry sırasında değiştirebilir. Encrypted/show/cipher/event hücreleri aynı canlı referanslardır. Boş girdi için yeni bir filtre eklenmez; Session'ın mevcut 028d30 normal akışı boş frame'i bu çağrıdan önce atlar.

`RoutedEffects`, mesaj Effects arayüzünün yalnız `registered_handler` metodunu gerçek `dispatch_recovered::Router` kaynağına bağlar. Diğer atoi/log/error/thread davranışları abstract kalır; başarılı no-op üretim policy'si eklenmez. Router'ın kullandığı gerçek handler/state/provider kaynakları caller tarafından hazırlanmalıdır. Mevcut router, uygun `MemoryPrinter` çıktısı ve doğru NodeAccess/request ömrü ister; bu modül farklı XML nesne ABI'si icat etmez.

Statik çağrı dayanakları: 023000 ilk reset, 025720 accepted reset → greeting write-all → read; 025480 delimiter dışındaki string'i worker'a verir, launch/detach sonrası buffer tüketir ve read'i yeniden kurar; 028d30 boş mesajı atlar, diğerinde 033d70 ardından response boş olsa da dispatch-write çağırır. 02d540 canlı version globalini okur. Bağlantı bu gövdeleri değiştirmez. Native accepted reset'in geçici nesne oluşturup byte kopyalaması ile mevcut kaynağın mantıksal in-place reset çağrısı aynı allocator/nesne ömrü/ABI olarak sunulmaz.

Her oturum ayrı encrypted/unknown/cipher hücreleri kullanmalıdır; version/show/event gibi uygulama hücreleri paylaşılabilir. Bu ayrımı caller kurar; adapter kendi başına Session state'i veya native global oluşturmaz.

`StateReferences`, XML backend, crypto factory, random provider, Effects, Router ve tüm alt state/provider nesneleri caller'a aittir. Adapter ve Session callback'leri boyunca yaşamalıdırlar. Session'ın raw callback capture'ı ve detached worker sözleşmesi değişmez; native shared_ptr, MSVC string/allocator, socket move/cancellation, gerçek concurrency, ASIO/IOCP ve SEH/ABI eşitliği açık kalır. Scheduling/Transport arayüzleri bu modül tarafından uygulanmaz.

Testler bellek içi transport/scheduler ve sentetik state/time/provider'lar kullanır; uygun akışlarda gerçek Session, XML reader/MemoryPrinter, AES/challenge, Router, işleyiciler ve ortak olay kuyruğunu birleştirir. Entegrasyonda gerçek Router üzerinden QueryContent, ShowIGO ve GrantAchievement çalışır; diğer handler/provider/lisans yolları bu yeni testle yürütülmüş sayılmaz. Sentetik cipher veya etkiler kullanılan ayrı sözleşme testleri native runtime kanıtı sayılmaz. Dosya I/O, gerçek kullanıcı CFG/XML/INI/token okuma, socket veya özgün DLL/oyun yürütmesi yoktur.

Sonuçlar `validation.json` ve `common-validation.json`; statik dayanaklar `evidence.json`, adresli `evidence.asm.txt` / `evidence.pseudo.c.txt`, bağımsız `flow-audit.md/json` ve `review.md/json` içindedir. `rea-session.json` sunucu/sağlayıcı kimliğini korur. `build.ps1` bağımsız, `../source-workspace/build.ps1` ortak LLVM-MinGW C++20 `-O2 -Wall -Wextra -Werror` yoludur. CMake tanımı ayrıca verilir; ayrıca çalıştırılmış sayılmaz. Çıktı tam bağımsız DLL değildir; CRT kaynakları beklemede kalır.


Ortak doğrulama sonucu: **19 Handler + 1154 Session entegrasyonu = 1173 yeni kontrol PASS**. Ortak proje142 üretim nesnesi,127 normal test grubu ve19 terminal senaryosuyla geçti. Yeni özgün yordam0; socket/ASIO veya tam DLL/runtime eşitliği kanıtı değildir.
