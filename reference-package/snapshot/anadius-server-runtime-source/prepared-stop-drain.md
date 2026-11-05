# Erken stop öncesi prepared iş bildirimlerinin tüketilmesi

Sonraki kaynak aşaması: [prepared işler ile SessionIo'nun birlikte kapanışı](prepared-session-drain.md).
Aşağıdaki ayrı SessionIo eksikliği bu belgenin önceki snapshot'ına aittir.

Durum: **kaynak uygulandı; derlenmedi, test/link/DLL çalıştırılmadı**.
Runtime ve Profile başlık/uygulamaları ile mevcut Graph integration test
kaynağı değişti. Yalnız yazarın statik incelemesi yapıldı.

Önceki kaynaktaki bir hata düzeltildi: erken-stop195 senaryosu, canlı iş varken
Policy.worker_returned ve sonraki external-quiescence aşamasına ulaşacağını
varsayıyordu. Oysa Profile.Application.worker_returned, canlı iş kontrolünde
daha önce hata atıyor. Güncel default195 beklentisi bu gerçek terminal retained
prefix'i denetler. Önceki unexecuted incelemeler kendi eski hash'lerine aittir.

Yeni `Runtime.Dependencies.prepared_stop_timeout_ms` açık HOST seçimidir;
default0 ile kapalıdır, INFINITE reddedilir. Bağımsız `PreparedJobSourceDrain`
arayüzü, orijinal Parent Operation altında seçili PreparedJobs'un source
işlerinin kalıp kalmadığını bildirir. Bu boolean completion veya fiziksel/IO
finality izni vermez. InlineJobs ile bu seçenek sessizce etkinleşmez.

Stop null paketi geldiğinde canlı prepared source işi varsa Runtime Router,
yalnız kendi seçili stop okumasını erteler. Gerçek scheduler stop biti ve
generic dispatcher/native stop gövdeleri değiştirilmez. Aynı run_worker ve
Policy initialize/finish kapsamı açık kalır; bitmiş worker yeniden başlatılmaz.
Gerçek HOST receipt, mevcut NativeJobs tek-sefer doğrulaması ve original021770
continuation yolundan geçer. Son source işi tüketilince tek boş stop wake
yayımlanır. İş sayacı azaltma hâlâ dispatcher'ın mevcut tek transferidir.

İlk ertelemeden itibaren sonlu bütçe fiziksel GQCS beklemelerini sınırlar.
SDK error değeri deadline/proof çağrılarından önce hemen alınır. Bildirim yoksa
süre aşımı sahipleri korur. Resume post başarısızsa tamamlanan gövde tekrar
çalıştırılmaz; henüz dispatcher'a aktarılmamış work pin de korunur. Bu hatalar
normal kapanış veya retryable cleanup sayılmaz.

Hazırlanan ama çalıştırılmayan senaryolar:

| Mod | Kaynak beklentisi |
|---|---|
| `--prepared-native-early-stop` | Default politika: Profile, live job nedeniyle Policy returned hook'undan önce reddeder. Unconsumed sahiplik tutulur. Düzeltilmiş exit195. |
| `--prepared-native-early-drain` | Ready packet öncesindeki stop ertelenir; bir gerçek receipt, child/original cleanup ve tek work azaltımı; stop yeniden görülür. Kalan session IO için external proof false, sahiplik tutulur. Exit197. |
| `--prepared-native-early-drain-timeout` | Kontrollü proxy ready packet'i yayınlamaz; sonlu bekleme sonunda source work/handle/control korunur. Exit198. Bu, kernel post'un paket kaybettiği iddiası değildir. |
| `--prepared-native-early-drain-resume-fail` | Bir gövde tüketildikten sonra resume post false; dispatcher pin aktarımı ve worker finish uydurulmaz. Exit199. |

197 tam sunucu kapanışı değildir. Canlı receive/send/accept/session IO'su
ayrıca gözlenen paketler ve cancellation/drain sözleşmesiyle bitmelidir. Bu
aşama yalnız source-job receipt engelini çözer; sonraki hedef selected SessionIo
stop ile geç oluşan response IO'sunu aynı run içinde birlikte tamamlamaktır.

Runtime Dependencies alanı ve Profile.Application secondary base yerleşimi
değişti; bütün tüketiciler yetkili derlemede yeniden derlenmelidir. Eski base
virtual slot veya export eklenmedi. Orijinal dosyalar, DLL/ZIP ve eski build
kanıtları değişmedi. Loader hook'ları ve CRT/image/foreign finality açıktır;
bu kaynak aşamasından güvenilir toplam DLL yüzdesi hesaplanamaz.

Satır/hash kanıtı `prepared-stop-drain.evidence.json`, inceleme
`prepared-stop-drain-review.json` içindedir.
