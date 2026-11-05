# Erken connect yönlendirme gövdesi

Durum: **SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED**. Yazarın statik kaynak incelemesi yapıldı. 5 üretim CPP, 6 üretim header ve 3 senaryo CPP değişti. Tamamı zaten pending olduğundan son tam build ile farklı dosya sayısı 101 olarak kaldı; 14 dosyanın hashleri değişti. Tamamlanma yüzdesine veya yeni native gövde sayısına kredi verilmedi.

## Adresli dayanak

Mevcut REA/Ghidra assembly `ev_2e6fe3f4fb3ea3c8c61e9391e3000efc173a148ec843b7ec0d6deda38c2e2fb5` ve `../anadius-all-functions.asm.txt` kullanıldı:

| Konum | Gözlem |
|---|---|
| `0x18000fd5f`–`0x18000fd7a` | Non-web bekleme, ardından 3216 veya server port eşleşmesi. 80/443 beklemeyi atlar. |
| `0x18000fd7c`–`0x18000fdbb` | Requested ve server portları loglanır, sonra flush yapılır. |
| `0x18000fdc0`–`0x18000fdd1` | `0x1800a3888` sayacı 32 bit artırılır, sonuç 15 ile karşılaştırılır. |
| `0x18000fdd3`–`0x18000fde0` | Server port tekrar okunur, network-order dönüşümle caller sockaddr +2 alanına yazılır. |
| `0x18000fe02` | Original connect slotu `0x1800a3838` üzerinden tail JMP. |
| `0x18000fe09` sonrası | 15. deneme uyarı/çıkış dalı; normal port yazımı/original yolu atlanır. |

Yeni REA sorgusu, binary açma veya özgün dosya erişimi yapılmadı. Bu native kanıt, yeni kaynak bağlantısının çalıştığını veya loader güvenliğini kanıtlamaz.

## Kaynak bağlantısı

Mevcut log → counter → terminal15 → port store dizisi `redirect_body_00fd00` kaynak yardımcısına ayrıldı. Yardımcı bekleme veya original çağrı yapmaz. Tam `connect_00fd00` gövdesi aynı yardımcıya canlı server-port referansını verir; böylece mevcut effect'ler sonrası portu yeniden okuma davranışı korunur. Erken wrapper yayımlanmış portun yerel kopyasını verir ve aynı native Context sahibinin attempts hücresini kullanır. Callback seed sayacına dışarıdan tutulan writer'lar desteklenmez; bu bir source sahiplik sözleşmesidir.

Foreign Domain, opt-in redirect port proof ile yalnız foreign thread'de, successful-create original kaydı ve aynı Execution'dan Server payload/event varsa 3216 veya matching-port yönlendirmeyi kabul eder. Non-web yol `RedirectWaiting` ile mevcut `wait_steps(-1)` iznini alır; 80/443 `RedirectPending` ile yeni bekleme eklemez. Gerekli bekleme normal döndükten sonra portun değişmediği ve original kaydının geçerli olduğu tekrar denetlenir.

`RedirectBody` yalnız network literal/port/flush, dialog ve exit Policy metodlarına izin verir. Registry, allocation, genel callback, wait ve original SDK izinleri bu sırada kapalıdır. Counter artışı ve caller port yazımı bu izole body içinde yapılır. Body completion checkpoint'i kaydı tekrar doğrular; sonra body izni kapanır, yalnız original SDK izni açılır. Doğrulama başarısızsa önceki log/counter/port prefix'i geri alınmış sayılmaz ve başarılı SDK sonucu üretilmez.

Erken redirect factory, `shared_log::Stream` sahibi olmayan plain ostream'leri reddeder. Seçilen Host assembly zaten aynı managed stream'i paylaşır. Her formatted write/flush kendi mevcut stream Lease'ini kullanır; tüm mesajın tek atomik log işlemi olduğu iddia edilmez. Kilit wait, dialog veya original çağrısına taşınmaz. SDK dialog/exit sağlayıcılarının ömrü ve nonthrowing/native sözleşmeleri kaynak sahibinin sorumluluğudur. Hedef içindeki uyarı metni analiz verisi olarak korundu; operasyonel talimat olarak izlenmedi.

Mevcut active PTD/native lease ve kod sahipliği body boyunca tutulur. Pause yeni girişleri kapatır; önceden kabul edilmiş çağrı, proof'ları geçerliyse tamamlanabilir. Stop bitmeyen native wait'i veya modal çağrıyı iptal etmez. Controller warming, pre-Server ve henüz original kaydı olmayan yollar bu yeni izinle açılmadı. Genel `ready()` veya post-milestone full activation değiştirilmedi.

## Çalıştırılmamış senaryolar

- Network core: çıkarılan body'nin bekleme/original çağırmaması, canlı portu flush sonrasında tekrar okuması, short-prefix reddi ve terminal15'in port/original öncesinde kalması. Mevcut full-core karar, wrap ve exception senaryoları aynı yardımcının tüketicisi olarak korunur.
- Foreign core: unmanaged stream reddi, controller reddi, eksik/değişen snapshot, non-web wait/body geçişi; body'de allocation/registry flush/original reddi; yalnız izinli network log/counter/port store ve completion sonrası original izni.
- Native integration: concrete Policy/native Owner/Publication/Milestones, managed log ve kendi SDK stub'ı ile 3216→4660, 4660→4660, 80→80, 443→443 kaynak senaryoları. Scope SDK original'a geçmeden önce kapanır; IPv6 tail ve SDK sonuç/error aktarımı tarif edilir. Flag'ler kontrollü Execution finish çağrılarıyla tamamlanır. Gerçek worker grafiği/engine/socket veya tam seçilmiş Host assembly çalıştırılmadı.
- Terminal child, attempts seed14 ile kaynak UI mock'unu ve exit gözlemcisini kullanır. 191 çıkışı için log, error başlığı/0x10 bayrağı, henüz değişmemiş sockaddr ve sıfır original çağrı beklenir. Bu gözlenmiş sonuç değildir. Eski opt-in olmayan profillerin Warming ret senaryoları korunur.

Hiçbir C++ kaynak derlenmedi, hiçbir proje testi çalıştırılmadı; PASS yoktur.

## Açık kalanlar

Single-active kısıtı hâlâ vardır: worker hook'ları veya dialog/log/original içinden gelen iç içe callback'ler overlap/reentry reddine gidebilir. Bu nedenle erken yönlendirme source bağlantısı, tam üretim callback uyumluluğunun tamamlandığı anlamına gelmez. Sonraki somut hedef bu çağrı ilişkilerini ve bekleme sırasında worker ilerlemesini çözmektir. Genel activation, controller/pre-Server profili, loader attach/detach hook tanımları, rollback ve image/foreign/CRT finality de açıktır.

Admission vtable'ı, Policy Resources ve Foreign Domain/ilgili kaynak sahiplerinin yerleşimi veya imzaları değişti. Etkilenen tüketiciler açık rebuild talimatında birlikte derlenmelidir. SDK callback imzaları korunur. Derleme, test, link, DLL çalıştırma ve ZIP güncellemesi yapılmadı; tarihsel build kanıtları korunur. Kesin hash/satır bağlantıları `connect-redirect-body.evidence.json` içindedir.
