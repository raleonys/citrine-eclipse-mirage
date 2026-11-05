# Yönlendirmesiz diğer portlar için bekle–original geçişi

Durum: **SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED**. Yalnız yazarın statik kaynak incelemesi yapıldı. 4 üretim CPP, 4 üretim header ve 2 senaryo CPP değişti. Toplam 101 kaynak/header son tam build'den farklıdır; bu aşamadaki 10 dosyanın tamamı zaten bekleyenler arasındaydı. Sayının değişmemesi, bu aşamada kaynak değişmediği anlamına gelmez. Bu sayı tamamlanma yüzdesi değildir.

## Native dayanak

Önceki REA/Ghidra connect assembly kaydı `ev_2e6fe3f4fb3ea3c8c61e9391e3000efc173a148ec843b7ec0d6deda38c2e2fb5` ve mevcut `../anadius-all-functions.asm.txt` kullanıldı:

| Adres | Gözlem |
|---|---|
| `0x18000fd5f`–`0x18000fd64` | 80/443 dışındaki tanınan yol `036880(-1)` çağırır. |
| `0x18000fd69`–`0x18000fd7a` | Bekleme sonrası requested port 3216 ile, ardından global server portuyla karşılaştırılır. Hiçbiri eşleşmiyorsa log/yönlendirme gövdesi atlanır. |
| `0x18000fe02` | Original connect slotu `0x1800a3838` üzerinden tail JMP. |
| `0x180036958`–`0x180036971` | Flag taraması sonrası mutex unlock edilir; eksik başlangıç durumunda zaman/uyku yoluna geçilir. |

Execution'ın mevcut source gate'i flag/mutex ve QPC/cache erişimini seri hale getirir; native flag mutex'i ve source gate Sleep boyunca tutulmaz. Bu aşama Execution algoritmasını, polling aralığını veya native flag başlangıçlarını değiştirmedi. Yeni REA sorgusu/binary açma/özgün dosya erişimi yapılmadı. Mevcut assembly, yeni C++ kodunun çalıştığını kanıtlamaz.

## Üretim bağlantısı

Foreign Domain'e `Waiting` izni eklendi. Önkoşullar: callback mevcut PTD/native lease altında olmalı, caller controller thread'i olmamalı, requested port 80/443/3216 olmamalı, başarılı connect create kaydı geçerli olmalı ve aynı Execution'dan yayımlanan bound server portu requested porttan farklı olmalıdır. Server payload/event henüz yoksa bekleme başlatılmaz. Bu, mevcut native davranışın daha dar bir kaynak profilidir.

Seçilen Host assembly, önceki aşamanın aynı Milestones port kopyasını ve aynı original Publication sahibini kullanır. Native wrapper izin verilirse somut network Effects üzerinden mevcut `wait_steps_036880(-1)` çağrısını yapar. Policy, yeni `connect_wait_scope` ile yalnız bu network metoduna ve yalnız count -1'e izin verir. Registry beklemesi, log, allocation, original SDK ve genel Policy izni Waiting sırasında kapalıdır. Permission bookkeeping LastError'ı korur; gerçek Execution çağrılarının sonuç/error davranışı değiştirilmez.

Bekleme normal döndükten sonra `connect_wait_complete` aynı thread/fiber/aktif callback rezervasyonunu, original kaydını ve no-redirect port kanıtını yeniden denetler. Başarılıysa wait izni kapanır ve yalnız OriginalOnly izni açılır. Wrapper original SDK'ya aynı socket/pointer/length ile bir kez gider; sockaddr, attempts, log ve yönlendirme gövdesi kullanılmaz. Bekleme exception ile çıkarsa completion çağrılmaz. False post-wait proof SDK izni açmaz; belirsiz proof exception'ı poison oluşturur. Genel readiness otomatik açılmaz.

Host setup completion, yalnız kısıtlı Waiting çağrısı aktifken onu beklemeden tamamlanabilir. Pause yeni girişleri kapatır; önceden kabul edilmiş waiter PTD, Context/native lease ve kod sahipliğini tutar. Gerçek native bekleme döndüğünde, geçerli kayıtları yeniden denetlenerek aynı kabul edilmiş çağrı original üzerinden tamamlanabilir. Bu, yeni bir callback kabulü değildir.

**Stop native beklemeyi iptal etmez veya başarılı tamamlanmış saymaz.** İlgili flag'ler tamamlanmazsa bekleme süresiz kalabilir ve kapanış eksik kalır; provider/code teardown yapılamaz. Source stopper'ın bekleyen çağrının ihtiyaç duyduğu worker ilerlemesini kesmemesi, hâlâ somut üretim stop sözleşmesinin bir parçasıdır. Controller'ın warming başlangıcına bu blocking yol verilmedi; yeni DllMain beklemesi veya Initialize API'si eklenmedi.

## Çalıştırılmamış senaryolar

Core senaryosu controller reddini; web/3216/matching server-port dışlamalarını; eksik original kaydını; wait-only PTD iznini; log, registry wait ve farklı count reddini; gerçek Execution'ın normal dönüşünü; izinlerin wait → original değişimini; completion sonrası tekrar bekleme reddini ve geri çekilen post-wait original proof'un SDK izni vermemesini tarif eder.

Native integration senaryosu aynı gerçek Execution/Milestones, concrete Policy/native Owner/Publication ve kendi SDK stub'ını kullanacak şekilde eklendi. Kontrollü seed'de Online/Libraries başlangıçta sıfırdır. Server payload/event yayımlandıktan sonra IPv6 port4000 çağrısı mevcut polling/Sleep yolunda tutulur. Controller host completion ve pause yaparken Context'in tutulması, gerçek finish_flag çağrılarıyla ilerleme, değişmeyen sockaddr, tek SDK çağrısı ve drain sonrası kapanış beklenir. Platform/SDK kontrollüdür; tam seçilmiş Host Assembly, gerçek worker grafiği, engine veya socket davranışı gözlenmedi.

Yeni child modu port3216 için Warming reddi ve 187 çıkışını bekler. Bu gözlenmiş sonuç değildir. Hiçbir senaryo derlenmedi veya çalıştırılmadı; PASS yoktur.

## Açık kalanlar

3216 ve bound server portuyla eşleşen yönlendirme çağrılarının log/sayaç/UI/exit ve port yazımı bağımlılıkları açık kalır. Genel post-milestone activation bağlantısı yoktur. Single-active callback kısıtı devam eder: beklerken bir worker'ın başka hooked callback'e ihtiyaç duyması hâlâ overlap reddine yol açabilir. Genel callback/worker reentry ve concurrency eşdeğerliği bu aşamada çözülmedi. Pre-Server ve controller erken bekleme profilleri de native ile eşdeğer ilan edilmez.

Admission vtable'ı, callback Policy Resources ve Foreign Domain/ilgili sahiplerin yerleşimi veya imzaları değişti; etkilenen tüketiciler birlikte rebuild gerektirir. SDK callback imzaları korunur. Loader attach/detach hook tanımları, rollback ve image/foreign/CRT finality hâlâ açıktır. Derleme, proje testi, link, DLL çalıştırma veya ZIP güncellemesi yapılmadı; tarihsel build kanıtları korunur. Yeni native gövde veya toplam DLL yüzdesi artışı iddia edilmez. Hash/satır kayıtları `connect-wait-passthrough.evidence.json` içindedir.
