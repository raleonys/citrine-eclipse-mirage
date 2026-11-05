# Ağ etkilerinin dönüşünde izin/PTD denetimi

Durum: **SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED**. Yalnız yazarın statik kaynak incelemesi yapıldı. 1 üretim CPP, 1 üretim header ve 2 senaryo CPP değişti. Son tam build ile farklı toplam kaynak/header sayısı 102'dir. Native fonksiyon kurtarma veya toplam DLL yüzdesi için yeni kredi yoktur.

## Somut eksik

Önceki Policy network log/flush/dialog/exit metotları yalnız girişte `check_connect_body()` çağırıyordu. Son flush döndüğünde izin/PTD bozulmuşsa, `redirect_body_00fd00` önce attempts sayacını artırıp caller portunu yazabiliyor; daha sonraki original checkpoint hatayı ancak bu prefix'ten sonra buluyordu. Ayrı bir scope grant'i false olmuş fakat gerçek PTD sağlıklı kalmışsa body completion original kanıtıyla devam edebiliyor, SDK çağrısı bile gerçekleşebiliyordu. Bu, kaynak izin protokolündeki somut boşluktur; hedef DLL'de gözlenmiş bir crash değildir.

Native sıralama dayanağı önceki assembly `ev_2e6fe3f4fb3ea3c8c61e9391e3000efc173a148ec843b7ec0d6deda38c2e2fb5`: `0x18000fdbb` flush çağrısını, `0x18000fdc0`–`0x18000fdc8` sayaç yükleme/artırma/yazımını, `0x18000fdd3`–`0x18000fde0` port yazımını gösterir. Original tail `0x18000fe02`'dedir. Yeni denetim native algoritma değişikliği veya yeni kurtarılmış fonksiyon değildir; yeniden kurulmuş PTD/izin protokolünün tutarlılık kontrolüdür. Bu aşamada yeni REA sorgusu veya özgün dosya erişimi yapılmadı.

## Düzeltme

Private `connect_body_call` giriş, normal dönüş ve exception dönüşünde aynı izin/PTD denetimini yapar. Beş network Effect metodu bu yolu kullanır: literal log, port log, flush, dialog ve exit. Geçerli exception dönüşünde aynı alt exception `throw;` ile aktarılır; izin geçersizse postcheck hatası önceliklidir. Metadata denetimi mevcut LastError korumasını kullanır; alt etkinin outgoing hata değeri silinmez.

Managed stream Lease, lambda gövdesinde oluşturulup bitirilir; postcheck çalışmadan önce kilit ve stream-operation sahipliği bırakılır. Probe boyunca stream kilidi tutulmaz. Tamamlanmış log veya dialog etkisi geri alınmış sayılmaz. Gerçek süreç exit'i dönmediğinden sonrasında denetim yapılabilmesi beklenmez; throw eden veya beklenmedik şekilde dönen kaynak exit sağlayıcıları mevcut hata/terminate sözleşmesini korur.

Son flush postcheck'i başarısızsa sayaç ve port yazımına artık geçilmez. Dialog postcheck'i başarısızsa sonraki exit/port akışı yürütülmez; bu aşamaya kadar gerçekleşmiş counter/UI prefix'i korunur. Public metot imzaları, Resources alanları, vtable ve object layout değişmedi. Scope callback'leri daha sık sorgulanır; saf metadata/PTD kontrolü sözleşmesine uymaları gerekir.

## İç içe callback haritası

| Sınır | Kaynakta görülen durum | Açık ihtiyaç |
|---|---|---|
| Connect wait | Execution uyku sırasında kendi kilitlerini bırakır; Native Admission ve tek CallbackThreads sahibi aktif kalır. | Worker'dan yeni hook çağrısı mevcut Domain.enter içinde overlap/reentry reddine gider. Ayrı çağrı sahipliği gerekir. |
| Original SDK | `Boundary::invoke` pre/postcheck yapar; native `invoke` Admission'ı SDK dönüşüne kadar tutar. | İç içe çağrı için parent PTD ve original/code sahipliğini koruyan ayrı kapsam henüz yok. |
| Log | Managed Stream thread-local active işareti ve kendi Lease'i ile reentry'yi reddeder. | Domain kapısını kaldırmak log kapsamının yeniden giriş sözleşmesini çözmez. |
| Dialog | Policy içinden owned Dialog/SDK sınırına gidilir; artık dönüşte izin tekrar kontrol edilir. | SDK'nın gerçekten hangi hooked callback'leri çağırdığı statik caller'dan kanıtlanmış değildir. |
| Online/library worker | Source Online `load_copy_0450d0`, factory/virtual çağrıları; library source `LoadLibraryW` sınırı içerir. | Bu opaque çağrıların dinamik callback ilişkileri bu aşamada gözlenmedi. |

Dolayısıyla single-active kuralı bu aşamada gevşetilmedi. İki SDK sınırı arasındaki izin sürekliliği düzeltildi; eşzamanlı veya iç içe callback desteğinin tamamlandığı iddia edilmez. Sonraki odak, wait/original sırasında yalnız original'a geçen connect çağrılarının ayrı per-invocation PTD/lease ile desteklenebilirliğidir; registry/redirect mutable state ve log reentry ayrıca değerlendirilmelidir.

## Çalıştırılmamış senaryolar

Core Policy senaryosu, managed stream flush'ının explicit body grant'ini geri çekmesini tarif eder. Log prefix'i kalırken attempts=0, sockaddr değişmemiş ve original çağrı sayısı=0 beklenir. Permission probe aynı stream'i yeniden Lease eder; bu, pre/postcheck'in effect Lease'i dışında olması gerektiğini görünür kılar. Normal lower LastError ve geçerli/bozuk izinle throwing exit exception önceliği de kaynakta kapsanır.

Native child senaryosu concrete Policy/native Owner/Domain/Publication/Milestones ve gerçek same-domain PTD kullanır. Controlled buffer yalnız ek scope grant'ini geri çeker; FLS fiziksel olarak bozulmaz. Bu seçim, normal native exception yolundan sonra gerçek private CallbackSeed counter prefix'inin incelenmesine izin verir. 192 çıkışı için counter=0, değişmemiş caller portu, original=0, tamamlanmış log prefix'i, boş native lease/PTD scope ve hâlâ tutulan code owner beklenir. Bu, fiziksel FLS bozulma testi veya tam seçilmiş Host Assembly/worker/engine/socket testi değildir. Hiçbir senaryo çalıştırılmadı; PASS yoktur.

Derleme, proje testi, link, DLL çalıştırma ve ZIP güncellemesi yapılmadı. Tarihsel build kanıtları korunur. General activation, controller/pre-Server profili, nested/overlap callback, loader hook tanımları, rollback ve image/foreign/CRT finality hâlâ açıktır. Kesin hash/satır bağlantıları `network-effect-postcheck.evidence.json` içindedir.
