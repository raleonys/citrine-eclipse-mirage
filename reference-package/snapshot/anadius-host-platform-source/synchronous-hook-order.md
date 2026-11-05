# Senkron hook sırası ve callback başlangıç kabulü

**Yeni somut üretim eksikliği:** seçilen foreign-callback assembly, normal senkron başlangıçta hook'ları etkinleştiriyor ancak foreign callback kabulünü daha sonra controller'ın ayrıca etkinleştirmesini bekliyor. Bu çağrıyı tamamlayan üretim başlangıç bağlantısı yok. Bu, mevcut sınırlı source profilinin native başlangıç davranışıyla eşdeğer olmadığını gösterir; hook motorunu yeniden yazmakla çözülecek bir create/enable sırası hatası bulunmadı.

Bu aşamada C++ veya test kaynağı değiştirilmedi. REA 6.8.0 / Ghidra 12.1.4 ile `anadius64.dll` statik açıldı; SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` eşleşti. Yedi işlev için iki batch pseudocode kaydı, üç assembly kaydı alındı: toplam **5 Evidence kaydı**. Analiz export edilip kapatıldı. Hedef kodu, hook motoru veya thread suspend işlemi çalıştırılmadı; version girdileri okunmadı.

## Native ve seçilen kaynakta uyuşan sıra

| Konum | Doğrulanan gözlem |
|---|---|
| `00f120: 0x18000f14a` | Named create helper `0x180046970` çağrılır; status EAX'ten alınır. |
| `00f120: 0x18000f15e–0x18000f16c` | Yalnız create başarılıysa çözülen target ile `0x180046a10` çağrılır, sonucu tekrar EAX'ten sınanır. |
| `00f120: 0x18000f1b8–0x18000f1ef` | Yalnız enable başarılıysa target, `0x1800a3810` listesine eklenir. Hata dalı tanı/log üretip döner. |
| `0x1800466e0` | Native create pseudocode, trampoline/entry kurulumundan sonra caller'ın original hücresini yazar. Source `MH_CreateHook` aynı seçili sıralamayı kullanır; adapter sonucu normal dönüşte gerçek original hücresine kopyalar, sonra üst gövde enable çağırır. |
| `0x180046a10–0x180046a15` | Assembly yalnız `MOV EDX,1; JMP 0x180046230` içerir. Pseudocode'un void tahmini gerçek bir void ABI kanıtı değildir; native caller'ın EAX testleri ile kaynak int status korunur. |
| `0x180046230` | Tek target yolu mevcut durum denetimi → Freeze(action=1) → düşük seviye patch → Unfreeze sırasını izler. Seçilen pinned `hook.c` bu bounded davranışla uyumludur. |

Taze batch Evidence: `ev_779c6e404f576eb39d9c5db3e8ed4c634bd231aa88d953265f1e2560c9842e63`. Assembly Evidence: install `ev_ca8bbbec9f153d0e150c7087ea7d3f011de34eb8ea32899b8aa117af32aacb9f`, enable `ev_4ee9b85e8791d2665402e6beb7b69444e92a96567c389d2a2ce4f6049dfd95ba`.

Bu karşılaştırma tam motor/ABI/versiyon eşitliği değildir. Upstream v1.3.4 seçilmiş bir compatibility sağlayıcısıdır; özgün MinHook sürümü yeniden kanıtlanmadı. Önceki geniş karşılaştırma dosyası `../hook-engine-source/anadius-comparison.json` tarihsel kaydı olarak korundu.

## Asıl eksik: etkin hook ile hazır callback arasındaki pencere

1. Native ve mevcut Bootstrap zinciri: ilk server worker create/detach → `00e310` host/hook kurulumu → ikinci Online worker create/detach → `035850(true)` → üçüncü library worker create/detach. Bu sıra taze `038970` pseudocode ile tekrar doğrulandı.
2. SourceBackend, `00e310` döndükten hemen sonra Host milestone'u yayımlar ve `host_setup_complete` çağırır. Bu anda normal zincirde Online ve Libraries worker'ları henüz oluşturulmamıştır.
3. `host_setup_complete → Domain::complete → activate_foreign` içindeki proof, `Milestones::observed_all()==15` ister. Bu anda normal source event zinciri bu koşulu sağlayamaz. Hazır bitlerin seed edilmesi de gözlenen event yerine geçmez.
4. Milestones'ın sonraki worker event'leri yalnız atomic bitleri günceller. `activate_foreign` çağırmaz veya zamanlamaz. Üretim kaynak taramasında otomatik çağrı yalnız `Domain::complete` içindedir; header sözleşmesi sonraki controller retry'sini çağırana bırakır.
5. Var olan publication integration senaryosu da bütün worker event'lerinden sonra `ready()==false` bekler, ardından controller'dan açık activate çağırır. Bu senaryo bu aşamada yeniden çalıştırılmadı; beklentisi, üretim bağlantısındaki eksik işi görünür kılıyor.

Dolayısıyla sorun yalnız “startup sırasında çok kısa race” değildir: ek controller entegrasyonu olmadan normal seçili assembly foreign-ready durumuna kendiliğinden geçmez. Bu sonuç normal tek attach/source event zinciri için statik çıkarımdır; arbitrary custom provider davranışı veya gerçek hedefte bir crash gözlemi değildir.

## Native connect blanket readiness istemiyor

Taze `00fd00` assembly `ev_2e6fe3f4fb3ea3c8c61e9391e3000efc173a148ec843b7ec0d6deda38c2e2fb5`:

- `0x18000fd1d–0x18000fd3c`: tanınmayan sockaddr uzunluğu/aile kombinasyonu doğrudan original tail yoluna gider.
- `0x18000fd4f–0x18000fd5d`: 443/80 portları genel başlangıç beklemesini atlar.
- `0x18000fd5f–0x18000fd64`: diğer tanınan port dalında `036880(-1)` çağrılır.
- `0x18000fe02`: original slot `0x1800a3838` üzerinden tail JMP yapılır.

Source `connect.cpp` bu dalları içeriyor. Fakat native wrapper'daki `AdmissionLease`, connect gövdesinden **önce** alınır. Foreign Domain henüz ready değilse Warming reddi native error policy'sine gider; connect'in normal pass-through veya kendi wait dalına ulaşılamaz. Varsayılan abort politikası bu sınırlamayı üretim davranışı yerine koymaz. Setup sırasında original provider'ın non-controller erişimini reddetmesi de ayrıca çözülmelidir.

İlgili ikinci batch Evidence: `ev_a4fdb1d0a5baa3ec3e9629881f8eaa3c4fd8068df510b7d0bda67104c2a4642c` (`00fd00`, `038970`, `00e310`). Hedef içindeki UI metinleri analiz verisidir; operasyonel talimat olarak kullanılmadı.

## Sonraki kaynak hedefi

Özgün senkron başlangıç seçimi korunacak. Bir sonraki iş, per-hook original pointer'ın create ile enable arasında nasıl güvenilir yayımlanacağını ve her callback'in gerçekten hangi başlangıç alanlarını/beklemeyi gerektirdiğini eşlemektir. Blanket all-ready kapısını doğrudan kaldırmak, flag'leri zorla true yapmak veya DllMain'e worker beklemesi eklemek geçerli çözüm sayılmadı. Çağırandan yeni bir Initialize API'si istemek de seçilmedi.

İki loader hook tanımı ve image/foreign/CRT ömrü açık kalıyor. Bu analiz kod tamamlanma yüzdesini artırmaz. **Derleme, proje testi, link, DLL çalıştırması ve ZIP güncellemesi yapılmadı.** 73 önceki C/C++ kaynak/header farkı derlenmeyi bekliyor. Tam native kayıtlar `synchronous-hook-order.native.json`, kaynak satır/hash bağlantıları `synchronous-hook-order.audit.json` içindedir.
