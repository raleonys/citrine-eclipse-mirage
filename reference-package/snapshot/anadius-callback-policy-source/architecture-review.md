# B16 bağımsız tasarım incelemesi

**PASS.** Reviewer: `/root/b16_policy_audit`; üretim kodunun yazarı değildir. Beş dondurulmuş kaynak/test dosyasını ve test sonuçlarını okudum. Root tarafından çalıştırılan **52 çekirdek +34 entegrasyon =86 kontrol** için kaynak ve iki EXE SHA-256 değerleri doğrulandı; testleri yeniden çalıştırmadım. Core sayısına iki kendi terminal alt süreci dahildir.

ByteWide, caller wchar_t/char16_t/unaligned backing üzerinde yalnız memcpy kullanır; compiler CRT'ye gerçek wchar_t staging verir. Overlap ve INT_MAX sınırları, invalid-handler öncesi gerçek output prefix'i, NUL/kapasite/bayt boyutu davranışı testlerle kapsanır. Bu compiler CRT seçimi özgün statik CRT/locale/invalid-handler/EH eşdeğerliği değildir.

Policy gerçek final Runtime original dizisini ve aynı-domain ApplicationMemory'yi kullanır. Storage/base yıkım sırası borrowed referansları korur. Dört baytlık handle ownership, yabancı/çift free reddi, known free failure retention ve açık owner retry test edilir. Core erase-before-free prefix'i korunur; retry eksik source tracking temizliğini uydurmaz. Canlı handle ile yıkım terminaldir.

İki ön inceleme ifadesi düzeltildi: shared_ptr<ostream> tek başına ayrı streambuf ömrünü garanti etmez; caller korumalıdır. Other-thread rejection yalnız Policy override'ları için geçerlidir; inherited final original/wide yöntemler kontrolü bypass eder. Tüm callback/state/stream kullanımı tek dış controller tarafından, reentry olmadan serialize edilmelidir. Bu sınıf whole-callback admission guard değildir; foreign host-thread PTD, genel eşzamanlı callback durumu ve otomatik dış quiescence açık kalır.

Beş registry whitelist kaydı retained dosya kanıtında 137 bayt ve tam NUL terminator'larla doğrulandı; reviewer özgün dosyayı okumadı. Root'un kısmi 144 bayt REA görünümü ayrı kapsamla kaydedilmelidir. Bilinmeyen VA dereference edilmez. Önceki B15 ZIP'indeki 797 kaynak uzantılı girdi byte-identical doğrulandı; önceki 780 takipli kaynağı kapsar.

Testler kendi process heap/FLS/CRT/SDK metadata ve kendi shim/fake trampoline'larını kullanır. Orijinal/rebuilt DLL, gerçek hook/registry/socket/process/UI/foreignmodule çalıştırılmadı. Tam DLL hazır değildir; yeni native gövde/kapanış sayısı 0. B10 review borcu ve tam ABI/CRT/provider/unload/PE sınırları açık kalır.
