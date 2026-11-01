# Düşük seviye dosya tablosu yaşam döngüsü

Bu modül üç özgün normal akış gövdesini C++ olarak kurar: `initialize_lowio 0x180067890`, `uninitialize_lowio 0x1800678cc` ve `destroy_handle_array 0x180073258`. `ArrayEffects` mevcut destructor kaynağını tablo walker'ına bağlayan yeni bir adapter'dir; dördüncü özgün gövde sayılmaz.

Başlatıcı lock7 alır ve `ensure_fh_exists_0732a8(0)` çağrısının **tüm EAX** değerini sınar. Düşük32bit sıfırsa inherited `067698`, ardından standard-handle `067788` çağrılır; her normal dal unlock7 ile biter. Üst RAX bitleri karar için kullanılmaz. Örneğin0x100 başarısızlıktır,0x100000000 düşük32bit sıfır olduğu için başarı dalına girer. Alt ensure tekrar lock7 alabildiği için sağlayıcının kilidi recursive olmalıdır.

Üç alt rutin bu modülde saf sanal ve zorunlu sağlayıcılardır: gerçek array oluşturma/kapasite artırma, inherited handle çözümü ve stdin/out/err API davranışı henüz bağlanmış değildir. Testte bu sağlayıcılar yalnız kendi array tahsislerini ve kontrollü çağrı sırasını kullanır; gerçek dosya handle'ı edinmez.

Kapanış, `A54A0` tablosunun128hücresini ulaştığında okur. Nonnull değer için destructor çağrılır, sonra aynı hücre sıfırlanır. Callback'in current hücreye yaptığı yazma bu son sıfırlamayla üzerine yazılır; ilerideki hücre değişiklikleri ulaşıldığında görülür. Tablo0x400bayttır. Ardındaki ayrı `A58A0` kapasite DWORD'ü sıfırlanmaz; buradan genel reinitialization güvenliliği sonucu çıkarılmaz. Kapanış kendi başına lock7 almaz.

Array destructor NULL'da işlem yapmaz. Diğer durumda64kritik bölgeyi0x48stride ile siler ve ancak sonrasında array'i `0668c0` kaynağına verir. Extent0x1200bayttır. CloseHandle çağrısı yoktur. Sağlayıcı her adresi yaşayan gerçek typed kritik bölgeye çözmelidir; üretim çekirdeği keyfi byte array'i native CS olarak cast etmez.

`build.ps1` önceden ortak proje ile derlenmiş `../source-workspace/build/anadius_core_partial.a` kullanır. Kendi fixture EXE'sindeki639kontrol:
- Tam EAX/düşük AL ayrımı ve başarı/başarısızlık çağrı sırası.
- İlk/orta/son128slot,ileriki hücre mutasyonu ve post-call current sıfırlama; kapasite korunması.
- NULL array ve64×0x48 destructor sınırları.
- Kendi native process heap'imizde üç typed array, toplam192gerçek SDK DeleteCriticalSection işlemi ve üç HeapFree.
- Testte gerçek recursive CS7 ve typed Record ömrü. Record yalnız40bayt CS ve32bayt opaque kalan depodur; özgün kaydın tüketilmeyen alanları için tam tip/ABI iddiası yoktur.

Native test array'lerini kendisi oluşturur ve `std::construct_at` ile nesne ömrü başlatır. Her CS fixture tarafından başlatılır; kapanış sırasında silinir, array ömrü bitirilir ve heap bloğu bırakılır. Registry/array/table yaşamı dışarıdan quiescent'tir; concurrency veya MSVC SEH/finally davranışı eklenmez. Fixture alt sağlayıcıları, özgün array constructor/inherited/std handle kodunun tamamlanması değildir.

`node collect-evidence.cjs` özgün DLL hash'i ve retained REA listing hash'leriyle203rawgövde baytını,iki pair8 callback kimliğini ve üç gövdenin gerçek EvidenceID'lerini doğrular. `evidence.json`, `lowio.selected.asm.txt/.pseudo.c` ve `review-lowio-lifecycle.json` kanıt/bağımsız inceleme sonuçlarıdır. Aynı script kardeş pair-leaves modülündeki eksik iki leaf için yeni REA18bayt okumalarını özgün PE ile karşılaştırıp `../source-workspace/pair-leaves-rea-comparison.json` üretir.

`pair-bindings-audit.md/.json` 16ACRTpair'in hâlen gereken ortak state kimliklerini ve sağlayıcılarını listeler. Bu ledger, üretim dispatcher'ı değildir. Özellikle runtime/heap/onexit cookie'leri,EncodedGlobals alanları,Entry/termination application tablosu ve A4EF0 pointer/nonzero görünümü tutarlı paylaşılmalıdır. Bütün16callback'in tek global graph ile birlikte çalışması,lower lowio sağlayıcıları,SEH/unwind/export/DllMain ve tam DLL eşdeğerliği açık kalır. Özgün DLL/oyun çalıştırılmadı veya değiştirilmedi.
