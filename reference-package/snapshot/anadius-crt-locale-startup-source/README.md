# Locale başlangıç ve kapanış kaynakları

Bu modül üç özgün normal-akış gövdesini C++ olarak yeniden kurar: ilk current-locale ataması `0x180064468`, kapanış yardımcısı `0x180065104` ve `__acrt_uninitialize_locale` `0x180065338`. `SourceEffects` yeni bir bağlayıcıdır; mevcut `0709d8` kaynak kodunu çağırır ve ek özgün gövde sayılmaz.

İlk atama 16 çiftli ACRT başlangıç tablosunun ilk callback'idir: `0x18008aa60` → `0x180064468`; karşılık gelen uninitializer NULL'dır. Gövde, `0x1800a17e0` ilk locale adresini `0x1800a52e0` current hücresine düz MOV ile yazar ve AL=1 döndürür. Referans artırmaz. İlk locale sayacı zaten1 olduğu için bu adımda `0709d8` kullanılması sayacı yanlışlıkla2 yapardı. Üst dönüş register bitleri hakkında eşdeğerlik iddiası yoktur.

Bu17baytlık leaf önceki2076fonksiyonlu Ghidra envanterinde yoktur. `initializer-byte-proof.json` 17gövdebaytını,3paddingbaytını,PEoffset0x63868'i,iki RIP-relative hedefi,ilk tablo kimliğini ve LLVM disassembly hash'ini kaydeder. Yeni REA Evidence `ev_5e35d3129089836990868a249972c775d324e7f7cfd11a9b99b60e24b9b481ff`20baytlık tam okumayı doğrular. Bu yeni bir Ghidra fonksiyon sınıflandırması değildir; ham bayt kanıtıyla desteklenen yerel statik gövde çözümüdür. `node verify-initializer.cjs` hash'e bağlı özgün dosyayı salt okunur doğrular; hedef kodu çalıştırmaz.

Kapanış yardımcısı lock token'ını önce okur, kilidi alır, tek current-locale hücresini ilk locale ile karşılaştırır. Farklıysa gerçek `0709d8` updater'ını çağırır ve dönüşünü hücreye ayrıca yazar. Bu son yazma `0x180065152` assembly işlemi olduğu için updater'ın daha önce yazmış olması nedeniyle kaldırılmaz. Unlock token'ı işlemler bittikten sonra yeniden okunur; C++ arayüzü iki canlı DWORD lvalue'sini ayrı zamanlarda okur. `065338` iki ayrı yerel DWORD'ü4 yapar ve yardımcıyı çağırır. Kapanış callback'i preterminator tablosunda `0x18007f730` hücresindedir. Lconv/ctype/mb_cur_max alias'ları bu gövdede yeniden yayımlanmaz.

Retained `065104` pseudo Evidence `ev_b183b7650c91500655f4a95389def832d0a79d893a8f688009716a8b7698b7f2`, assembly Evidence `ev_c0bb591d054c51baae2bf8cbae8bbf908c43e85d699a0b730f9dc7f0aa91fd4a`; `065338` pseudo Evidence `ev_2d33ae0db58262e8ed051e3ea09c28573b21fad376f11efef03f2bdcfda90cbf`. `static-startup-audit.json/.md` seçilmiş gövdeleri,16pair sırasını,DIR64/dosya offset kanıtlarını ve retained kaynak hash'lerini içerir.

`build.ps1`, önce `../source-workspace/build.ps1` ile oluşturulmuş kısmi arşivi kullanır; yeni kaynağı ayrıca derler ve kendi fixture EXE'sini çalıştırır.117kontrol:
- Düz başlangıç yazması,aynı/different/null current,lock/unlock token zamanı ve ayrı son yazma.
- Gerçek updater ile dinamik-eski-locale modelinin sonref/free yolu ve ikinci okuma sırasında değişen hücre.
- Özgün başlangıç veri grafiği,3typed LONG,gerçek locale-ref/PTD/FLS/heap kaynakları ve4kendi iş parçacığı.
- İlk locale ref1→main2→workers6→join2→FLSkapanış1;MBref0→1→5→1→0. Tüm5PTD heap tahsisi serbest kalır.

Native testte kritik bölümler fixture sağlayıcısına aittir; exception-table kimliği sentetiktir. Bu testte multibyte codepage initializer `06f5f8` çalıştırılmaz. MB ref akışı yalnız PTD retain/release içindir; MB startup son durumu olarak sunulmaz. `multibyte-startup-audit.md/.json` gerçek MB initializer'ın ek setter/publish/systemCP işlemlerini ve kalan entegrasyon boşluklarını ayrı belgeler.

Bütün bağlı nesneler, FLS callback'leri ve iş parçacıkları bitene kadar hayatta kalır; current-locale değişimleri quiescent yapılır. MSVC SEH funcletleri/exception unwind,tüm CRT başlatıcılarının birleşik çalışması,orijinal exception table,export/DllMain ABI ve tam DLL davranış eşdeğerliği açık kalır. Özgün DLL veya oyun çalıştırılmadı,değiştirilmedi. Bağımsız inceleme `review-locale-startup.json`:PASS ve kaynak hash'leri.
