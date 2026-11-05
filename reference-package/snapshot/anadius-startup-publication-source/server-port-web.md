# Server portu yayını ve erken HTTP/HTTPS geçişi

Durum: **SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED**. Yalnız yazarın statik kaynak incelemesi yapıldı. 7 üretim CPP, 6 üretim header, 5 senaryo CPP ve 2 senaryo destek header'ı değişti. Son tam build ile farklı kaynak/header sayısı 101'dir; bu bir tamamlanma yüzdesi değildir.

## Adresli dayanak

Tarihsel REA/Ghidra server kanıtı `../anadius-server-source/evidence-map.json` içinde `0x180024480` için assembly `ev_4f9d3f62cce3b75d905b999b5f99a20ceb6c0fc1fb8597ea7bcba97cde3c61f6` olarak kayıtlıdır. `../anadius-all-functions.asm.txt` içindeki mevcut metin bu aşamada tekrar okundu:

| Adres | Gözlem |
|---|---|
| `0x18002466d`–`0x180024676` | Bound portun host dönüşümü ve `MOV word ptr [0x1800a3c90],AX`. |
| `0x180024690`–`0x1800246e4` | Port metni ve ortam değişkeni yolu; kaynak BOOL false sonucunu kontrol etmeden devam eder. |
| `0x180024725`–`0x18002472c` | İlk/Server bayrağı `0x1800a3c3d` adresiyle `0x180036820` çağrılır. |
| `0x180024758` | İlk async accept, port ve readiness yayımından sonra gelir. |

Önceki connect kanıtı `../anadius-host-platform-source/synchronous-hook-order.native.json`, assembly `ev_2e6fe3f4fb3ea3c8c61e9391e3000efc173a148ec843b7ec0d6deda38c2e2fb5`: `0x18000fd4f`–`0x18000fd5d` 443/80 bekleme istisnası; diğer tanınan port için `0x18000fd5f`–`0x18000fd64` içinde `036880(-1)`; original tail `0x18000fe02` ve original slot `0x1800a3838`.

Bunlar native sıralama kanıtıdır. Yeni C++ atomic payload/admission katmanı native makine kodu veya birebir ABI eşdeğeri değildir. Bu aşamada REA oturum/analiz sorgusu, binary açma veya özgün dosya erişimi yapılmadı; mevcut kanıtlar kullanıldı.

## Üretim bağlantısı

Server constructor, mevcut bound port yazımı ve ortam değişkeni çağrısından sonra yeni kaynak composition metodu `Execution::finish_server_startup` kullanır. Bu metot yalnız gerçek ilk/Server bayrağını kabul eder, port payload'ını gözlemciye bildirir ve aynı `finish_flag_036820` uygulamasını çağırır. Mutex/flag ve hata-prefix davranışı korunur. Bu yeni metot ayrı bir native fonksiyon kurtarıldığı anlamına gelmez. Payload bookkeeping LastError'ı korur; yeni bir bekleme veya flag seed'i yoktur.

`Milestones`, payload'ı aynı Execution kimliğiyle tek sefer kaydeder. Atomic hücrede geçerlilik biti ve 16 bit port birlikte tutulur; port sıfır da geçerli bir değerdir. `server_port()` yalnız payload ve gerçek Server finish olayı birlikte varsa bir kopya döndürür. Seed edilmiş ready baytları, yalnız payload, yalnız event, yanlış issuer, yinelenen payload veya eski event'ten sonra gelen geç payload uygun kanıt sayılmaz. İkinci payload geçersizlik durumunu kalıcı kılar. Gözlemci sözleşmesi bir Server construction içindir; arbitrary dış event yazıcıları desteklenmez.

Seçilen Host assembly, aynı Milestones sahibinden `published_port != requested_port` web kanıtı üretir. Foreign Domain yalnız requested port 80 veya 443 olduğunda bunu değerlendirir; successful-create original kanıtı da zorunludur. İki proof, mevcut aktif callback rezervasyonu korunarak Domain mutex'i dışında çalışır; dönüşte stop/poison yeniden denetlenir. Başarılı sonuç yalnız `OriginalOnly` izni açar. Genel Policy log/allocation/Execution-wait izni kapalı kalır. Bilinen false sonuç probe'u bırakılabilir tutar; belirsiz proof exception'ı poison oluşturur.

Native connect wrapper aileyi tanıyorsa dört baytlık prefix'ten requested portu okur. Yayımlanmış farklı bir server portuyla 80/443 çağrısı doğrudan SDK original'a, aynı socket/pointer/length ile aktarılır. Ham paylaşılan configuration.server_port hücresi okunmaz; port yazılmaz, log/sayaç/bekleme gövdesine girilmez. Tanınmayan uzunluk/aile için önceki aşamanın untouched yolu korunur.

Server portu 80 veya 443 ile aynıysa native yönlendirme gövdesi gerekebilir; bu dalın genel hazırlık ihtiyacı kaldırılmadı. Server payload/event henüz gelmemişse web geçişi de açılmaz. Dolayısıyla native'in çok erken HTTP/HTTPS davranışıyla tam eşitlik iddia edilmez. Stop yeni girişleri kapatır; önceden kabul edilmiş original/PTD/native lease drain koşulları önceki aşamayla aynıdır.

## Çalıştırılmamış senaryolar

- Server constructor senaryosu: gerçek source constructor + kontrollü backend/Publication, false environment sonucu sonrası payload → gerçek flag event → accept sırası.
- Milestones senaryoları: sıfır/80/443/65535 portları, foreign snapshot okuması, yanlış flag/issuer, payload/event eksikliği, duplicate/late payload ve unlock exception prefix'i.
- Network senaryosu: dört bayttan network-order port çıkarımı ve kısa prefix reddi.
- Foreign core matrisi: bound port 0/80/443/65535 ve requested 80/443/3216; yalnız no-redirect web durumunda original izni, hiçbirinde genel Policy izni verilmemesi.
- Native integration senaryosu: aynı gerçek Execution + Milestones, concrete Policy/native Owner/Publication ve kendi SDK stub'ı; hazır seed baytlarının event yerine geçmemesi, IPv4/HTTP ve IPv6/HTTPS backing'in değişmemesi, global readiness açılmadan original geçişi, mevcut completion/stop retention. Bu test tam seçilmiş Host assembly veya gerçek engine/socket çalıştırması değildir. Test fixture'ları gözlemciyi aynı Execution'a bağlayabilmek için optional parametre aldı.
- İki yeni child modu, bound portla eşleşen 80/443 çağrılarında Warming reddi için 185/186 çıkışını bekler. Bunlar gözlenmiş sonuçlar değildir; PASS yoktur.

## Sıradaki eksik

Diğer tanınan portlar için `036880(-1)` yolunu callback kabulüne bağlamak gerekiyor. Execution'ın flag/mutex/QPC/cache erişim sahipliği, bekleme sonrası sunucu portu/sayaç/log erişimi ve stop sırasında hâlâ bekleyen çağrının ömrü birlikte ele alınmalı. Sadece all-ready flag'ini zorlamak veya yeni bir DllMain beklemesi eklemek çözüm sayılmadı. Genel post-milestone activation bağlantısı, single-active callback kısıtı, loader attach/detach hook tanımları, rollback ve image/foreign/CRT finality açık kalır.

Execution Publication ve native Admission vtable'ları, Milestones/Foreign Domain yerleşimi ve ilgili kaynak imzaları değişti. Etkilenen tüketiciler sonraki açık rebuild talimatında birlikte derlenmelidir. SDK callback imzaları korunur. Derleme, proje testi, link, hedef/DLL çalıştırma ve ZIP paketleme yapılmadı. Tarihsel build kanıtları değişmedi. Yeni native gövde veya toplam DLL yüzdesi artışı iddia edilmez. Dosya hashleri ve satır bağlantıları `server-port-web.evidence.json` içindedir.
