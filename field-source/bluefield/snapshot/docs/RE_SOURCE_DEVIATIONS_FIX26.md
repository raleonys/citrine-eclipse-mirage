# FIX26 — İki derleyici uyarısı giderildi; D14 hata/exit farkı açık

Tam kaynak/build/QA devam paketi, NON_RELEASE; exact FIX25 ebeveynine dayanır. Karşılaştırma tam kullanıcı RE kaynak paketidir. Runtime davranışı değiştiren bir exit/catch politikası eklenmedi.

## Giderilen uyarı ve değişikliğin sınırı

GCC13 mevcut licensing fatal/ticket_required fonksiyonlarının sonunda virtual noreturn Effect çağrısından sonra iki “noreturn function does return” uyarısı üretiyordu. Aynı TU parentta -Werror ile reddedilir. FIX26 tek private nonvirtual noreturn helperda aynı virtual Effecti aynı kodla çağırır. GNU/Clang builtin unreachable yalnız mevcut noreturn sözleşmesini compiler control-flowya taşır; MSVCde builtin derlenmez. Geri dönen override zaten önceki sözleşmeyi ihlal eder. Yeni abort/terminate/throw veya fallback eklenmedi. Exceptionlar mevcut callerına aynı şekilde yayılır.

Helper ve iki çağrı dönüşümü geri çevrilince dosya sealed FIX25 ile byte-identicaldır. Dialoglar, exit kodları, license blocked state, token denemeleri ve branch sırası değişmez. Bütün önceki C++ fixtureları, vendor1060 dosya, production graph, Golden araçları ve CMD/PS1 korunur. Paket içinde CRT emülasyonu veya ayrı termination implementationı eklenmedi.

## Tam paketten netleşen D14 bulguları

628 C++/header dosyasında exit/no-response/invalid-message sembolleri tarandı; kaynak gövdeleri ayrıca okundu. `anadius-crt-termination-source/termination.cpp` pakette vardır: exit_05cdc0 normal mode0/onexit/initterm ve process termination effects zincirine gider. Bu kanıt yokmuş gibi davranılmadı. Callerlar ve bugünkü adapterlar metadata/D14_ERROR_BOUNDARY_FIX26.json ile docs/D14_ERROR_BOUNDARY_FIX26.csv içinde18 satırla bağlandı.

- License -4/-5/-6/-2/-3/-7 ve ticket -1 dalları bugünkü LicenseEffects içinde SessionFailure olur. TCP session kapanır; service ready normal SessionFailureda temizlenmez. License blocked state paylaşılır; sonraki license istekleri başka connectionlarda da NoResponsea girebilir. Bu source process-termination intentiyle eşdeğer değildir ve bu turda düzeltilmiş sayılmaz.
- Network15. bağlantı dalı hâlâ direct Win32 ExitProcess kullanır. “Bütün exitler session exceptionına dönüştü” genellemesi yanlıştır. CRT normal-exit cleanup zinciriyle direct ExitProcess aynı implementation değildir; native kanıt NOT_RUNdır.
- Yeni açıkça kaydedilen alt fark: source ticket write false dönerse fallback dialoguna, sonra exit(-1)e gider. Mevcut atomic adapter true döner veya exception atar; write exceptionında bu false-result dialog/exit dalı atlanır. Bu önceki mevcut sapmadır; FIX26 üretmedi. Failure-atomic/recovery gereği ile boolean sonuç sözleşmesi ayrı çözümlenmelidir; blanket catch eklenmedi.
- NoResponse, malformed XML/decrypt empty response, invalid-message session exceptionı, persistence recovery service stopu ve native callback failfast birbirinden ayrıldı. Hepsi process exit veya aynı failure scope olarak sunulamaz.

Tarihsel host-exit dönüşümünün bağımsız gerekçesi kaydedilmemiştir. Compiler fixi ya da metadata yeni davranış yetkisi üretmez. Kaynak farkını görünür kılma ve compiler uyarısı giderme işi tamamlanır; process/service/session tercihi açık kalır.

## Durum

Açık9→9: DEP05, D02, D06, D07, D13, D14, D15, Q02, D21. Bu tur semantic closure sayısı0, compiler warning repair sayısı2dir. Kapanmamış D14 sırf sayıyı azaltmak için kapatılmadı. Final test/build/ZIP sonuçları QA dosyasında; Windows/MSVC/DLL/provider/game ve actual DLL Golden NOT_RUN.
