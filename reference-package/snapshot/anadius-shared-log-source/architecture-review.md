# B20 bağımsız mimari incelemesi

**PASS — sınırlı kaynak sözleşmesi kapsamında blocker bulunmadı.** İnceleyen `/root/b18_ptd_audit`, hem üretim yazarı `/root` hem bağımsız integration testi yazarından ayrıdır. Beş yeni ve altı değişmiş kaynak okundu; 11 kaynak hash'i iki son quick kaydıyla, iki own EXE hash'i kendi kayıtlarıyla doğrulandı. Testler yeniden çalıştırılmadı. Root kayıtları: **30 core + 39 integration = 69 PASS**. Ortak tam derleme/delivery doğrulaması ayrıca yapılmalıdır.

Stream ve Lease, seçilmiş altı writer'ın **aynı managed Stream nesnesi** üzerindeki işlemlerini tek gate altında toplar. Böylece yalnız buffer değil, sentry/formatlama, width, flag, locale kullanımı, error state ve flush sırasında erişilen compiler ostream durumu da korunur. B10 önce ortak gate'i sonra kendi private gate'ini alır; private gate önce bırakılır. Configuration prefix'i conversion öncesinde görünür olur ve gate bırakılır. İki geçişli conversion sürerken başka thread'in library output'unu tamamladığı integration testi bu sınırı doğrudan kontrol eder.

`stream.hpp:9–28` ve `stream.cpp:26–30`: BufferOwner ilk **virtual** base'tir. Ostream'in virtual basic_ios/ios_base erase callback'leri tamamlanana kadar buffer sahibi yaşamaya devam eder. Lease, başarılı girişte güçlü Stream sahibini tutar; gate'i açtıktan sonra son sahibi bırakır. Null buffer ve shared sahibi bulunmayan seçilmiş stack nesnesi reddedilir. Core erase-event testi önceki nonvirtual-owner ömür hatasını kapsar.

`stream.cpp:31–55`: TLS active kontrolü legacy no-op dalından önce çalışır. Managed scope içindeki aynı-stream, başka managed-stream ve legacy-adapter girişleri başka gate veya ostream state'ine erişmeden reddedilir. Bu, managed→legacy B10 private-gate sıra terslenmesini kapatır. Tie ve actual rdbuf/owned-buffer kimliği gate altında denetlenir. Constructor hatasında TLS/lock temizlenir; same-thread immovable Lease yanlış thread veya bozuk TLS ile bırakılırsa terminate eder. Bu kontrol genel fiber/reentry desteği anlamına gelmez.

Acquire ve release ayrı WinLastError snapshot'ları kullanır. Acquire giriş değerini gövde başlamadan geri koyar; release gövdenin çıkış değerini mutex/TLS ve son sahip destruction işlemlerinden sonra korur. Gövdenin gerçek LastError etkisi gizlenmez. Kısmi output, counted NUL/zero-tail, width/flags, badbit/failbit ve seçilmiş exception maskesi compiler ostream davranışını korur. Flush hatası önce yazılmış baytları geri almaz; guard retry, clear veya rollback üretmez. Bir kayıt birden çok source çağrısından oluşuyorsa parçaları başka writer ile karışabilir.

## Kanıt yolları

- `stream_tests.cpp`: ownership/LastError, nested managed ve legacy retleri, tie/rdbuf retleri, unwind, streambuf badbit, persistent formatting/failure, altı thread output ve erase-event buffer ömrü.
- `stream_integration_tests.cpp:113–156`: altı seçilmiş writer aynı Stream, her counted/C-string insertion ve sekiz finish route; gerçek native callback çağrısı yoktur. Policy kendi tek controller thread'inde doğrudan log çağırır.
- `stream_integration_tests.cpp:158–246`: persistent format/error state, embedded NUL, first-pass zero tail, conversion dışında gate, query/fill/partial-write/flush hata prefix'leri ve açık caller recovery.
- Altı değişmiş eski CPP'nin ve beş yeni dosyanın tam hash'leri JSON raporundadır. Quick kayıtları tüm altı eski CPP'yi ve yeni Stream production unit'ini taze derler.

## Kalan koşullar

Tüm seçilmiş writer'lar aynı managed Stream'i kullanmalı; writer ve provider sahipleri çağrı boyunca yaşamalıdır. Ham ostream/buffer erişimi, aynı buffer üzerinde başka frontend, global locale değişikliği, fiber migration ve callback'in başka writer'ı çağırması/beklemesi desteklenmez. Format/error state Lease altında ayarlanabilir; **rdbuf/tie değişiklikleri Lease içinde de desteklenmez**. Girişteki kontrol, gövde içinde bunları değiştirmeye izin vermez. Tek başına legacy stream için yeni concurrency garantisi verilmez.

Compiler CRT errno/locale/invalid-parameter state'i ve process File-API mode/codepage; recovered PTD errno/DOS fallback, initial-locale/FLS/heap state'i ve dış worker hata yollarından ayrı kalır. Bu gate bunları korumaz. B18 sequential admission/PTD retention ve B19 aynı-Execution publication sözleşmeleri, kalan provider proof'u, native callback/ABI/hook/unload/final PE sınırları devam eder. Host normal dönüş publication'ı engine sağlığı veya worker quiescence değildir. B10 tarihsel review borcu ve genel CRT kuyruğu kapatılmamıştır.

Bu aşama yeni native body/closure üretmez: **0/0**. B19→B20 envanteri parent'a göre 800 eski kaynaktan 794 aynı + 6 açık değişiklik + 5 yeni = 805; tam kaynak koruma/common build ayrı delivery audit'inin sorumluluğundadır. Tam DLL durumu **BLOCKED** olarak kalır. İnceleyen yalnız bu iki raporu yazdı; original/AV artefaktını okumadı/değiştirmedi, original/native DLL çalıştırmadı ve gerçek kullanıcı cfg/token/profile/achievement içeriği kullanmadı.
