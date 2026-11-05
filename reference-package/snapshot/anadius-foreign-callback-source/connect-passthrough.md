# Warming sırasında yalnız original connect geçişi

Durum: **SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED**. 5 üretim CPP, 6 üretim header ve 3 senaryo CPP dosyası değişti. Yalnız yazarın statik kaynak incelemesi yapıldı; senaryolar çalıştırılmadı. Son tam build ile farklı kaynak/header sayısı 92'dir; bu sayı tamamlanma yüzdesi değildir.

## Adresli dayanak ve sınır

Önceki `../anadius-host-platform-source/synchronous-hook-order.native.json` kaydında, `ev_2e6fe3f4fb3ea3c8c61e9391e3000efc173a148ec843b7ec0d6deda38c2e2fb5` assembly kanıtı:

| Adres | Native davranış ve bu aşamanın kapsamı |
|---|---|
| `0x18000fd1d`–`0x18000fd3c` | Uzunluk/aile tanınmıyorsa original kuyruğuna geçilir. Bu dalın önündeki blanket warming reddi seçilen kaynak yolunda ayrıldı. |
| `0x18000fd4f`–`0x18000fd5d` | 443/80 beklemeyi atlar; sunucu portunu okuyan/yönlendirebilen gövdeye gider. Bu yolların erken kabulü açılmadı. |
| `0x18000fd5f`–`0x18000fd64` | Diğer tanınan portlarda `036880(-1)` çağrılır. Bu yol ve başlangıç yayını hâlâ çözülmelidir. |
| `0x18000fe02` | `0x1800a3838` original slotundan indirect tail JMP. C++ SDK çağrısı aynı argüman ve sonuç aktarımını amaçlar; makine kodu/tail-call eşitliği iddiası değildir. |

REA `binary_session` ile package/server 6.8.0 ve kayıtlı komut yolu aligned olarak doğrulandı. Ghidra 12.1.4 kullanılabilir; aktif binary oturumu yok. Seçilmiş oturum alanları `connect-passthrough.session.json` içindedir. Bu aşamada binary açılmadı, yeni decompile/assembly sorgusu yapılmadı ve özgün dosyalar okunmadı/değiştirilmedi. Yukarıdaki native kayıt tarihsel kanıttır; yeni C++ davranışının çalıştığını göstermez.

## Üretim değişikliği

`network_hook_recovered::original_only_00fd00` yalnız seed uzunlukları ve family prefix üzerinden karar verir; port, sunucu portu, attempts, stream veya Execution'a erişmez. Core connect aynı sınıflandırmayı kullanır. Eşit IPv4/IPv6 uzunluklarında eski short-circuit alternatif korunur.

Native Admission'a iki kaynak metodu eklendi: connect probe sorgusu ve classification checkpoint. Native Context, construction sırasında uzunlukların const kopyasını tutar. Seçilen owning yolun sözleşmesi, bu seed uzunluklarının publish sonrasında değiştirilmemesidir. Erken wrapper yalnız bu kopyaları ve gerekiyorsa caller'ın iki baytlık family prefix'ini okur. Tanınmayan uzunlukta null pointer dahi incelenmeden original'a aktarılır. Caller backing üzerinde eşzamanlı dış yazım desteklenmez.

Foreign Domain için opt-in `ConnectProbe → Checking → OriginalOnly` yolu eklendi. Önce mevcut single-active/thread/fiber/PTD disiplini sağlanır. Probe sırasında hiçbir Policy veya original izni yoktur. Yalnız untouched sınıflandırma ve successful-create kanıtı, original SDK iznini açar. Genel callback Policy izni kapalıdır; log, allocation ve Execution beklemesi bu izinle açılamaz. Tanınan aile, eksik original yayını, overlap, reentry ve stop reddi sahte bir başarılı SDK sonucu üretmez; mevcut native error policy geçerlidir.

`callback_policy::Resources.original_scope`, original checkpoint iznini genel `callback_scope` izninden ayırır. Boşsa eski izin davranışına döner. Foreign factory iki izni kendi Domain'ine bağlar; önceden verilmiş izni sessizce değiştirmez. Seçilen Host assembly, erken original kanıtını önceki aşamadaki aynı Publication sahibinin connect bitinden üretir. Actual original dispatch yine aynı provider'ın exact-slot load ve SDK checkpoint denetiminden geçer. Kanıt callback'i Domain mutex'i tutulmadan çağrılır; beklenmeyen exception poison olarak tutulur.

Host setup tamamlanırken yalnız probe/original-only çağrı aktifse completion onu beklemez ve genel readiness kanıtı çağırmaz. Sonraki otomatik full activation bu aşamada eklenmedi. Stop yeni girişleri kapatır; aktif original/PTD/native lease boşalmadan cleanup kapanışı başarılı sayılmaz. Önceden kabul edilmiş original, pause sonrasında izin ve Context sahipliğini koruyarak tamamlanır.

## Çalıştırılmamış senaryolar

- Network senaryosu: sıfır/iki bayt backing, aile/uzunluk kombinasyonları, eşit uzunluklar, değişmeyen buffer ve 80/443'ün untouched sayılmaması.
- Foreign core senaryosu: false create proof, original-only izin, genel log/wait reddi, reentry, diğer callback'lerin warming reddi ve readiness'in yükselmemesi.
- Native integration senaryosu: concrete callback Policy + native Owner + shared Publication + kendi SDK stub'ı. Null uzunluk geçişi, iki bayt family, pointer/length/SOCKET ve gelen/giden LastError, geçici PTD temizliği ve blocked original sırasında completion/stop/close sırası tarif edilir. Tam seçilmiş Host Assembly veya gerçek hook engine çalıştırması değildir.
- Ayrı child senaryosu tanınan IPv4/HTTP yolunda Warming reddi için 184 çıkışını bekler. Bu gözlenmiş bir sonuç değildir.

## Açık kalanlar

Genel startup callback boşluğu kısmen açıktır: native tanınan connect yolları hâlâ all-ready kapısından geçer; Online/Libraries milestone'larından sonra otomatik controller activation bağlantısı yoktur. Sonraki hedef, 80/443 için sunucu portu yayınıyla diğer portların Execution beklemesini somut native/source yazım sırasına göre bağlamaktır. Global flag zorlaması, yeni Initialize API'si veya DllMain'e yeni bekleme eklenmedi.

Bu kaynak profili single-active kalır; arbitrary eşzamanlı callback/worker/SDK kullanımı çözülmedi. Kaynakta eklenen PTD/proof/admission işlemleri özgün native davranışın birebir eşdeğerliği veya loader-lock güvenliği kanıtı değildir. Gerçek engine/trampoline kod sahipliği, kaynak drain'i, loader attach/detach hook tanımları, rollback ve image/foreign/CRT finality hâlâ açıktır.

Native `Admission` vtable'ı, `Context`, callback Policy Resources ve bunları içeren sahiplerin yerleşimi değişti. Etkilenen tüketiciler sonraki açık rebuild talimatında birlikte derlenmelidir. Derleme, proje testi, link, hedef/DLL çalıştırma veya ZIP paketleme yapılmadı; eski build kanıtları korunur. Bu aşama yeni bir native gövde kurtarmadı; toplam DLL yüzdesi için yeni bir sayı verilmez. Satır/hash bağlantıları `connect-passthrough.evidence.json` içindedir.
