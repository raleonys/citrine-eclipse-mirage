# B2 — Tek bootstrap owner ve somut facade

Bu modül yeni kaynak bileşimidir. Yeni özgün native gövde veya native sağlayıcı kapanışı **0**; bir somut `BootstrapFacade` ve bir `RuntimeOwner` ekler. Önceki config, thread ve bootstrap gövdeleri yeniden kullanılır. Genel CRT üretimi beklemededir; gerçek DLL giriş stratejisi ve üretim backend'i henüz tamamlanmamıştır.

`BootstrapFacade`, yalnız `BootstrapConfiguration` üzerinden tek `BootstrapDependencies` tabanı alır. Üç config başlangıç metodu mevcut kaynaktadır. Thread başlangıcı mevcut `begin_thread_05e8e0` ve native `Resolver` üzerinden gider; ikinci `ThreadBridge` tabanı yoktur. Exit ve terminate kayıtları yeniden derlenmiş callback adreslerine çevrilir. Thread close işlemi aynı thread Effects sağlayıcısını kullanır. Geri kalan `Services` politikaları zorunlu kalır; forwarding bu native algoritmaları kurtarmış sayılmaz.

`RuntimeOwner`, çağıranın verdiği başlangıç config durumunu ve üç console flag'ini, tek `ProviderOwner` backend'ini, facade'ı, Bootstrap nesnesini ve sonradan yayımlanan native Binding'i aynı ömürde tutar. Config/flag üyeleri backend'den önce kurulur ve ondan sonra yıkılır. `providers()` bir kez sorgulanır. `bind_state` callback yayımlayamaz veya worker oluşturamaz. Owner kopyalanamaz ve taşınamaz; adresleri kararlı kalır.

Yaşam döngüsü `Constructed → Bound → Stopping → Closed` şeklindedir. Owner kontrol metotları tek kontrol thread'ine aittir ve reentry reddedilir; worker/config senkronizasyonu backend sorumluluğudur. Bütün üyeler kurulmadan native Binding yayımlanmaz. `application_entry()` yalnız Bound durumunda verilir. `close()`, önce gerçek callback kabulünün durmasını ve pending/suspended worker'ların tamamlanmasını zorunlu `stop_and_join` sağlayıcısından ister. False veya exception, owner ve Binding'i canlı tutar; yeniden deneme mümkündür. Native `active_calls()==0` tek başına yeterli değildir. Bağlı/Stopping owner'ın örtülü yıkımı terminal sözleşme ihlalidir; gizli close veya join yapılmaz.

**Hata politikası owner'dan uzun yaşamalıdır.** Eski native katman `last_errors` işaretçisini Binding kapanışından sonra da tutar; bu nedenle dışarıdan verilen Errors politikası kapanış/yıkımdan sonraki olası giriş denemeleri boyunca, pratikte süreç ömründe, geçerli olmalıdır.

Bu paket uygulama katmanı bileşimidir. PE giriş noktası kurmaz; compiler CRT veya kurtarılmış CRT başlangıcını kendiliğinden çağırmaz ve çift CRT başlangıcı üretmez. Worker fonksiyon pointer'larının, backend/global/OS/CRT uygulamalarının ve gerçek quiescence sağlayıcısının tamamlandığı iddia edilmez. Normal bootstrap çağrı sırası değiştirilmez; owner durum makinesi yeni kaynak politikasıdır, özgün entry/ABI/EH gövdesi değildir.

## Derleme ve sayım

Yetkili doğrulama yolu `../source-workspace/build.ps1` içindeki `bootstrap_owner` ve `bootstrap_owner_integration` gruplarıdır. Kaynaklar sabitlendikten sonra ortak derleme bir kez çalıştırılır. Ara hızlı kontroller önceki kaynak arşivi ve yeni owner.cpp ile yapılabilir; modül standalone build çalıştırılmış sayılmaz. Gerçek test sayıları B2 validation kaydına yazılır.

Adres son ekli facade metotları `reconstruction-inventory/generate_inventory.py` içindeki açık composition-only politikasıyla ayrı `composition-only.json` defterinde tutulur. Native kaynak eşlemesini veya native gövde/kapanış sayısını artırmazlar. Tam derlenmiş sembol listesi korunur; object probe ham tüm-tag sayısını ve composition hariç karşılaştırılabilir sayıyı ayrı raporlar. Test/fixture sağlayıcıları üretim implementasyonu sayılmaz.

Yeni sıra **1 DLL kritik yol → 2 mesaj zinciri → 3 XML işlevsel paketleri → 5 seçici CRT → 4 version preloader**. Numeric0033c0 XML sırasına ertelenmiştir. Orijinal DLL/oyun yürütülmez veya değiştirilmez; gerçek CFG/token/achievement içerikleri okunmaz; engelli version girdisi veya eski üretilmiş DLL yeniden denenmez.

## B2 ortak doğrulama

Gerçek yeni paket sayımı: **58 çekirdek +146 entegrasyon =204 kontrol PASS**. Ortak build211üretim nesnesi(207C++/4C),235normalgrup ve19terminal framework senaryosu içerir. Core grubu ayrıca yalnız kendi EXE'siyle üç terminal ömür durumunu sınar. B2 yeni1üretim nesnesi/5kaynak dosyası ekler; önceki691kaynak değişmedi. Orijinal native gövde/kapanış sayısı0; adres eşleme487,eligible derlenmiş tag491 ve önceki52native closure korunur. Ayrı raw-all/composition ledger facade yönlendirmelerini görünür tutar. Somut backend/worker/global/CRT-PEentry bağlantıları sonraki pakettedir.
