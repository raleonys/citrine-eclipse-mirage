# B27 bağımsız mimari inceleme

Sonuç: **PASS**. İnceleyen `/root/b22_provider_audit`, üretim ve iki test grubunun yazarlarından bağımsızdır.

Status +08 ve getter +10/+18/+20, retained Win64 caller şekilleriyle bytewise slot çözümüne bağlanır. Status yalnız düşük32 biti kullanır; getter RAX byte pointer’ı NUL’a kadar kopyalanır. QueryUse exact adopted object kontrolünden sonra vptr okur ve aynı module/code reservation’ını SDK ile gerçek string scan/allocation/copy sonuna kadar tutar. Tam foreign prototip/class ABI veya pointer okunabilirliği kanıtı oluşmaz.

Owned-copy çağrıları core, OnlineExecution, RecoveredEffects ve OnlinePlatform üzerinden açıkça forward edilir. Gerçek SourceBackend worker ve focused RecoveredEffects proxy testleri her uzun getter kopyasının gerçek heap allocation’ında aynı active object/code owner’ı ve destroy/free reddini doğrular. Üç forward’ı kaldırılmış work kopyasıyla aynı test, normal cleanup sonrasında precise active-copy predicate’inde beklenen exit1 üretir; gerçek backend değişmez ve negatif kontrol PASS sayılmaz. Observer, korunmayan negatif object’i yok etmez; recursion ve LastError korunur.

Query exception/null/gerçek bad_alloc, ownership değiştirmeyen query sözleşmesi altında known object’i korur; önceki field atamaları kalır, başarısız/sonraki alanlar eski kalır. Invalid vptr/slot poison+unknown retention oluşturur; PAGE_NOACCESS yapılan kendi object/table sayfasında sonraki query/destroy/free hiçbir yeni okumaya ulaşmaz. Raw getter pointer’ı dönüşten sonra yine caller serialization gerektirir.

B26 cookie düzeltmesi korunur: factory sonucu object’e atanır, exact heap cookie bırakılır, sonra status çağrılır. Flag/log ve username→persona→user-id sırası korunur. Üst RAX bitlerini kasıtlı nonzero yapan kendi status stub’ıyla0/1/2/-1 düşük32 branch davranışı actual worker’da doğrulanır. Tüm yedi seçili foreign source çağrısı typed bağa ulaşır; lower foreign counter sıfırdır. Bu **7/7 source binding**, tam DLL veya yeni original native kapanış değildir.

Root standard quick run’ları **35 core +38 integration =73 PASS kontrolü** üretti; integration yazarı `/root/b22_integration`. İnceleyen12 kaynak, iki EXE, negatif JSON/log/EXE/diff ve retained ASM/pseudocode hash’lerini doğruladı; test çalıştırmadı. B26 arşivi ve830 önceki kaynak: **822 aynı,8 açık değişiklik,4 yeni kaynak; toplam834**. Ortak build başlatıldı;245 nesne/241CPP/285 grup+19terminal sonucu pending/unreviewed’dur. Yeni Effects virtual/source ABI için bütün tüketiciler yeniden derlenmelidir.

Non-null getter, copy boyunca okunabilir NUL-sonlandırılmış veri sağlamalıdır. CodeLease gerçek module/vtable kod ömrünü sağlamalıdır; token bunu veya module pin/provenance’ı kanıtlamaz. Dış/asynchronous teardown, raw ABA, external mutation ve arbitrary reentry desteklenmez. B18–B26 PTD/ownership/publication/stream ve dış quiescence sözleşmeleri sürer.

**Yeni native gövde/kapanış:0; DLL bütünü hazır değildir.** Configuration/license/legacy document, B10 borcu, generic CRT ve native ABI/PE/hooks/unload açıktır. Orijinal/rebuilt target DLL veya oyun çalıştırılmadı; kullanıcı içeriği okunmadı. Ayrıntılar `architecture-review.json` içindedir.
