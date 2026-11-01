# B25 bağımsız mimari inceleme

Sonuç: **PASS**. İnceleyen `/root/b22_provider_audit`, üretim ve iki test grubunun yazarlarından bağımsızdır.

B25 yalnız seçili `version_check` çağrısını somut typed source binding’e bağlar. Retained ASM’de `0x180035e8c` açık argüman hazırlamadan çözülen adrese çağrı yapar; `0x180035e8e` EAX’ı 2 ile karşılaştırır. Seçili çağrı şekli sıfır girdili, düşük 32-bit sonuçtur; tam foreign prototipi/ABI/kod kökeni kanıtı değildir. Retained ASM/pseudocode hash’leri mevcut evidence ile doğrulandı; orijinal dosya yeniden açılmadı.

Exact yüklü module ve nonzero procedure eşleştirilir. Lookup öncesi eski callable kayıt emekliye ayrılır; başarılı lookup ve açık code lease olmadan çağrı yayımlanmaz. Yanlış veya stale pointer SDK’ya ulaşmaz. Gate; lower, lease callback, typed SDK veya lease deleter boyunca tutulmaz. Rezervasyon code ve factory-capture yok edilirken sürer; snapshot çalışabilir, dört managed operasyonun yeniden girişi lower etkilerden önce reddedilir. Bu davranış ilk incelemede bulunan retirement boşluğunu düzeltir.

Geçerli synchronous version exception’ı aynı kimliği, LastError’ı, prefix’i ve binding’i korur; otomatik retry yoktur. Lookup/lease başarısızlığında module kalır, callable kayıt açılmaz; lookup LastError lease callback/destructor bookkeeping’inden korunur. Free öncesi procedure geçersizleşir, code owner lower free boyunca tutulur. Normal TRUE/FALSE module-zero prefix’ini korur; FALSE unload başarısı sayılmaz. Load/free exception’ı unknown outcome için poison oluşturur ve kör retry engellenir. Implicit raw module/object cleanup eklenmez.

`create_online`, status, üç getter ve destroy olmak üzere **altı foreign yol lower’da kalır**. Version seçilmezse eski yedi forward korunur. Gerçek kaynak worker/load/cleanup testleri kendi typed stub, module/export/UI uçları ve bellek dosyalarıyla başarı, missing/mismatch, lease, version/destroy/load/free hata prefix’lerini doğrular. Object token/vtable ABI’si bu aşamada çözülmüş sayılmaz.

Root’un **50 core**, `/root/b22_integration` ajanının **44 integration** PASS kaydı; altı kaynak ve iki EXE hash’iyle doğrulandı; testler yeniden çalıştırılmadı. B24 arşiv kimliği ve 822 eski kaynak doğrulandı: **820 aynı**, **2 açık değişiklik**, **4 yeni kaynak**, toplam **826**. Ortak build sürüyordu; planlanan 243 nesne / 239 CPP / 281 grup + 19 terminal senaryosu bu raporda PASS sayılmadı.

CodeLease gerçek kod/module ömrünü sağlamalıdır; nonnull token kendi başına module pin, provenance veya native ABI kanıtı değildir. Kalan altı foreign yol, dış module işlemleri ve teardown bu guard’ın dışında kendi serialization/lifetime sözleşmelerini gerektirir. B18–B24 PTD/admission/publication/stream sözleşmeleri sürer; yeni version guard PTD veya unload/quiescence kanıtı eklemez.

**Yeni native gövde/kapanış: 0; DLL bütünü hazır değildir.** Kalan altı online foreign yol, configuration/license/legacy document, B10 borcu, generic CRT ve native ABI/PE/hooks/unload açıktır. Orijinal/rebuilt target DLL veya oyun çalıştırılmadı; gerçek kullanıcı cookie/profile/token içeriği okunmadı. Kanıt ve hash ayrıntıları `architecture-review.json` içindedir.
