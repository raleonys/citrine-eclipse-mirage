# Prepared işler ile takip edilen SessionIo'nun birlikte kapanışı

Sonraki kaynak aşaması: [fiziksel child çıkışı ve concrete Worker retry](../anadius-application-graph-source/worker-physical-close.md).

Durum: **kaynak uygulandı; derleme/test/link/DLL çalıştırılmadı**. Stop ve
Runtime başlık/uygulamaları ile mevcut Graph integration test kaynağı değişti.
Yazarın statik incelemesi yapıldı; bağımsız inceleme veya runtime PASS yoktur.

Önceki gate yalnız prepared source işlerinin receipt'lerini bekliyor, kalan
session IO'su için kapanışı erteliyordu. Yeni açık `Scope::SessionIoPrepared`,
aynı Runtime'ın managed prepared işlerine izin verirken receive/send/accept ve
Posted yanıt işlerini de stop öncesinde tamamlatır. Mevcut AcceptOnly/SessionIo
scope'larının proof gereklilikleri ve default seçim korunur. Yeni scope'un
`other_work_drained` kararı hâlâ yeni external admission'ı kapatmalı ve bütün
unmanaged raw/deferred/foreign işleri bitirmelidir.

Runtime, yeni scope için sonlu prepared-stop bütçesini ve exact Profile
PreparedJobs bağlantısını doğrular. Stop, açık managed binding olmadan bu scope'a
bağlanmaz. İptal ve kaynak yönlendirmesi orijinal worker/Parent üzerinde kalır.

`session_io_pending`, kabul edilmiş stop kararı, pending loan'lar, aktif source
packet binding'leri ve acknowledgement bekleyen route'ları denetler. Cache'teki
inert accept alias'ı ancak gerçek OwnedAcceptMemory kimliğiyle eşleşirse atlanır.
Geç oluşan response loan'ı pump ile iptal edilir veya hazır Posted callback'i
gözlenerek düşürülür. CancelIoEx true/false/ERROR_NOT_FOUND tek başına loan bitirmez.

Stop gate hem iş receipt'inden hem her IO receipt acknowledgement'ından sonra
yeniden değerlendirilir. Private controller wake tüketildiğinde de kontrol
edilir. Managed source ve takip edilen IO bitmeden boş stop wake yayımlanmaz.
İş sayacı azaltma yetkisi mevcut dispatcher'da kalır; worker yeniden başlatılmaz.
Controller post metadata'sı, Parent dışında close_worker'ın mevcut sınırlı
posts_done beklemesinde tamamlanır; yalnız wake zaten tüketilmişken bu metadata'yı
Parent içinde bekleyip döngüyü kilitleme riski eklenmedi.

Hazırlanan ama çalıştırılmayan senaryolar:

| Mod | Kaynak beklentisi |
|---|---|
| `--prepared-native-session-drain` | Gerçek Domain/native child/own IOCP kaynak yolu; controller managed scope'u isterken bir hazır job vardır. Bir job receipt, pending accept/receive ve oluşursa geç send/Posted response tamamlanır. Graph/Profile/NativeJobs/Stop Closed; ardından gerçek thread/FLS join ve controller backing release beklenir. Exit0. |
| `--prepared-native-session-cancel-fail` | İlk iptal reddedilir; hiçbir gözlenmemiş loan veya iş claim'i bırakılmaz, worker finish/port close uydurulmaz. Exit201. |
| `--prepared-native-session-missing-io` | Kontrollü proxy bir receive completion'ını yayımlamaz. Prepared job bitse de IO/control tutulur ve sonlu bütçe biter. Exit202. |

Bu fixture'da socket ve CancelIoEx sağlayıcıları sentetiktir. Kendi IOCP'sindeki
packet kimliği korunur, fakat995 SDK sonucu scripted olarak verilir; bu, gerçek
kernel iptalinin gözlendiği anlamına gelmez. External proof'un true seçilmesi de
yalnız unmanaged işi olmayan kapalı fixture'ın açık varsayımıdır. Hiçbir senaryo
bu aşamada yürütülmedi.

`Stop::bind` kaynak imzasına defaulted managed parametresi eklendi; bütün çağrı
noktaları yetkili derlemede yeni başlıklarla rebuild/link gerektirir. Yeni scope
enum sonuna eklendi; eski değerler, base virtual slot'lar ve export'lar korunur.

Sıradaki hedef bu birleşik akışı gecikmeli fiziksel/FLS child çıkışı ve
Parent dışı close-deferral/controller retry ile birlikte ele almaktır. Mevcut
fixture child'ı bağımsız proof HANDLE ile önceden join ederek zamanlamayı
kontrol eder; bütün olası scheduling'i kanıtlamaz. Production loader hook'ları
ve CRT/image/foreign finality ayrıca açıktır.

Orijinal dosyalar, DLL/ZIP ve eski build kanıtları değişmedi. Güvenilir yeni bir
toplam DLL tamamlanma yüzdesi yoktur. Satır/hash kanıtı
`prepared-session-drain.evidence.json`, inceleme `prepared-session-drain-review.json`
içindedir.
