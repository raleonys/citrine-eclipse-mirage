# CRT başlangıç ve kapanış callback'leri

`callbacks.cpp` iki özgün gövdenin normal akışını yeniden kurar: `0x18006447c` onexit tablolarının başlatılması ve `0x1800644f8` global multibyte kaydının kapanışı. Environment tarafındaki dört gövde ayrı `../anadius-crt-environment-cleanup-source` modülündedir. Bu aşama toplam altı yeni normal akış gövdesidir; bütün DLL veya tüm CRT kurulumunun tamamlandığı iddiası değildir.

İlk callback, ACRT pair tablosunda indeks15'in initializer'ıdır (`0x18008ab50`). Paylaşılan `termination::State.onexit.crt_0a52a8` ve `termination::State.quick_table_a52c0` tablolarını bu sırayla gerçek `initialize_onexit_table_0643f8` kaynağıyla başlatır. Gölge bir State veya tablo kopyası oluşturulmaz. Her tablo begin==capacity_end ise üç alanını mevcut cookie ile encoded-NULL durumuna getirir; bu koşul sağlanmayan canlı tablolar korunur. İki alt çağrının sonucu boolean başarıya dönüştürülmez; özgün AL=1 normal dönüşü korunur. Üst dönüş register bitleri için eşdeğerlik iddiası yoktur.

Multibyte callback, pair indeks13'ün uninitializer'ıdır (`0x18008ab38`):
1. Current pointer'ı yükleyip ilk DWORD'üne atomik -1 uygular.
2. Önceki değer1 değilse başka işlem yapmadan döner;0→FFFFFFFF wrap bu dala dahildir.
3. Önceki değer1 ise current'ı **ikinci kez** okur ve initial kimliğiyle karşılaştırır.
4. İkinci değer initial değilse bu yakalanmış değeri free'ye verir; free normal döndükten sonra current'ı initial'a yazar.

Free argümanı için üçüncü current okuması yoktur. Atomik sağlayıcısının current'ı değiştirmesi free adayını değiştirebilir; free sırasında başka current yazılması, sonraki sabit initial yazımıyla üzerine yazılır. Initial kimliğinde önceki ref1 olsa bile reset yazımı yoktur. Initialized/type/case/global-pair/flag alanları değiştirilmez ve lock5 eklenmez. Girişte current'ın geçerli, yaşayan bir counter olması gereklidir; özgün kaynakta null veya initialized guard yoktur. Yaşam döngüsü sırasını caller sağlamalıdır.

`collect-evidence.cjs` özgün DLL hash'ini, retained assembly/pseudo hash'lerini, 99 gövde baytını ve iki callback slotunun DIR64 entry'lerini doğrular. `evidence.json` dosya offset'lerini, bayt/hash'leri ve gerçek retained REA ID'lerini saklar; `callbacks.selected.asm.txt/.pseudo.c` tam seçilmiş gövdeleri içerir. Bu turda MCP araçları kısa süre görünmezdi, sonra yeniden geldi. `rea-session.json` sunucu6.5.0 kimliğinin ve mevcut Ghidra12.1.4 adayının salt-okunur doğrulamasını saklar. Yeni native decompilation sorgusu yapılmadı; işlev kanıtları önceki tam kayıtlardan tekrar kullanıldı.

`build.ps1` önce ortak `../source-workspace/build.ps1` ile üretilmiş kısmi arşivi kullanır; yeni gövdeyi ayrıca derler ve kendi fixture EXE'sini çalıştırır.105 kontrol:
- Farklı cookie'ler ve boş/canlı normal/quick tablolarının ayrı koşulları.
- Gerçek onexit register/execute/recalloc kaynakları, kendi native heap tahsisleri, native fixture kritik bölge ve callback sırasında yeniden kayıt:12→13→11; quick callback21 ayrı kalır.
- Atomik öncesi/sonrası current okumaları, ikinci free adayı ve son reset sırası.
- Özgün başlangıç MB verisinde ref0 wrap ve ref1 initial skip; typed LONG dinamik kayıtta ref2→1→0 ve gerçek HeapFree öncesi quiescent unbind.
- Enjekte edilmiş normal HeapFree başarısızlığında kaynak hata eşlemesi çalışır, current yine initial'a döner; fixture kalan kendi bloğunu ayrıca serbest bırakır.

Testin kritik bölge sağlayıcısı lock0 ve lock2'yi aynı recursive CS'ye bağlar; bu bütün özgün lock-bank topolojisini kanıtlamaz. Callback kimlikleri kendi kontrollü binding'leridir. Registry/heap/byte görünümleri için eşzamanlı iş parçacığı yoktur; lifetime değişimleri quiescent'tir. MB startup `06f5f8`, gerçek kullanıcı environment'ı, tüm CRT callback sırası, MSVC SEH/unwind ve tam DLL/ABI eşdeğerliği açık kalır. Özgün DLL/oyun çalıştırılmadı veya değiştirilmedi.

`multibyte-shutdown-audit.md/.json` eski/yeni pointer sınırlarını, pair13'ün normal reverse walker'da çağrılmasını, initialization rollback'te init=NULL olduğu için atlanmasını ve terminating yolunda callback walker'ın kullanılmamasını ayrıca belgeler. Bağımsız inceleme `review-lifecycle-callbacks.json`:PASS; kaynak hash'leri kayıtlıdır.
