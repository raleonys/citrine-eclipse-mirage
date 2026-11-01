> Tarihsel aşama kaydı. Güncel FIX13 davranışı ve geri alınan sınırlar için ORIGINAL_DEVIATION_AUDIT_FIX13.md geçerlidir.

# FIX11 — gulrak path ve Asio dispatch failure düzeltmesi

## Kapsam

HF19'daki pristine gulrak/filesystem pin'i 3812f8a6f31ed67d32272ac33fe2b85c6b2c947e,
header sürümü 1.5.17. Altı include dosyası, MIT lisansı ve pin kaydı byte-identical
taşındı. Yerel vendor yaması, network fetch ve BROWNFIELD application C++ kopyası yok.
HF19 CMake/adapter kapsamı path-only'dir; tüm filesystem IO'sunu değiştirmez.
Özgün paketin Mac ghc sembolleri Windows'un exact ghc sürümünü veya tüm Windows
IO'sunun aynı backend'i kullandığını kanıtlamaz. Windows std::filesystem tipleri
ve kurtarılmış UTF-16 lexical yordamları ayrı kanıttır.

## Gerçek path bağlantısı

Bootstrap GetModuleFileNameW(owned_module) -> surge.cfg ve Application config ->
surge_override.cfg yolları yeni owned module_sibling üzerinden geçer. Gerçek
pristine ghc::filesystem::path parent_path/join çağrılır. Native raw sonuç FIX10'ın
module.parent_path()/leaf sonucu ile kıyaslanır; aynıysa gulrak sonucu kullanılır.
Gulrak separator normalization ile yazımı değiştiriyorsa mevcut native sonuç
korunur. Path operator== yerine native string karşılaştırılır. HF19 mismatch'te
fail-closed yaparken bu adapter çalışan BLUEFIELD native yolunu korur; bu fark
bilinçlidir. Otomatik extended prefix ekleme HF19 gibi kapalıdır.

Adapter CWD/absolute/canonical lookup veya IO yapmaz, module native units'ini
encode etmez. Windows'ta native wide, POSIX'te native byte storage kullanır.
Leaf tek filename olmalıdır: empty/dot/dot-dot/NUL/slash/backslash/colon dahili
API'de reddedilir. Üretim caller'ları sabit surge.cfg/surge_override.cfg kullanır;
kullanıcı config anahtarlarına yeni kural eklenmez. Empty/relative base FIX10
native sonucunu korur. Resource hataları propagate olur; bad_alloc sessiz fallback
üretmez. Her allocation/native davranışın özgünle eşdeğer olduğu iddia edilmez.

Native status/fstream/IO/error-code, directory iterator/create/remove/copy,
Windows code-page/UTF-16 conversion, kaynak lexical algoritmaları, cache/avatar/
provider yolu ve original numeric identity korunur. Optional base/override,
SURGE varsayılanı, SDK ve config uygulama sırası değişmez. Yeni config seçicisi yok.

## QA'da bulunan Asio hata yolu

İlk Debug koşusunda global one-shot allocation fault bazen owned Session yerine
Asio dispatch içine denk geldi. Log IO-thread exception boundary'yi gösterdi;
sonraki client EOF/timeout ile başarısız oldu. Eski catch run()'a devam ediyor,
ready'yi temizlemiyor ve exception accept callback/rearm'dan önce oluşursa kabul
beklemeyen ama ready görünen listener kalabiliyordu. Bu bulgu saklandı; ilk koşunun
ham dosyaları erişilebilir olmadığından evidence kaydı açıkça transcript-only'dir.

FIX11 catch, stopping=true/ready=false yayımlar, listener'ı kapatır, io'yu durdurur
ve waiter'ları bildirir. Host normal stop/join ownership'ini korur. Hata loglanır;
servisin otomatik toparlandığı iddia edilmez. Internal is_ready getter'ı atomic
mevcut durumu gözlemler; public DLL C ABI değişmez. Windows IOCP/unload kabulü yok.

Testte iki farklı hata sözleşmesi ayrılır:
- Sabit 64 KiB receive buffer içeren owned Session allocation'ı boyut aralığıyla
  hedeflenir; kaynak cap/layout değişirse test injection assertion'ı fail eder.
  Sonraki client profile almalı ve service ready kalmalıdır.
- En fazla altı arbitrary background fault denemesi, ya yeni istemciye gerçek
  response ya da explicit ready=false ister. False-ready cevap veremeyen listener
  fail olur. Terminal dispatch ile accept-rearm terminal failure ayrı sayılır.
  Global allocator override yalnız bu test executable'ındadır, üretimde yoktur.

Önceki ilk tanılama yalnız dispatch logunu terminal sayıyordu; rearm failure ve log
zamanlaması nedeniyle bu kontrol yetersizdi. Yeni assertion gerçek atomic readiness
ile yapılır. Timeout artık EOF ile karıştırılmaz: fixture receive error kodunu
raporlar, 33. client cap testi gerçek EOF ister. Malformed request testi yalnız
no-response/sonraki client ilerlemesini kanıtlar; session close iddiası kaldırıldı.

## Test kapsamı ve sınırlar

Yeni unit: Linux'ta 2137 kontrol, 1062 native comparison (554 gulrak, 508 spelling
preservation), fixed fixtures, Unicode/raw byte, empty/relative/long/NUL, invalid
leaf, repeated separator. Windows-only drive/UNC/NT/extended/>260/unpaired UTF-16
case'leri native çalıştırılmadan PASS sayılmaz.
Yeni integration: 47 kontrol; Unicode module directory, farklı cwd'de decoy base
ve override, no-config SURGE, base/override sırası, absent base+override, malformed/
directory config hatası, failure sonrası recovery, native binary IO ve missing
required read. Yalnız sentetik temp dosyaları kullanılır ve cwd geri alınır.

8766 frozen Config2, 115 parser, 53 full-profile ve gerçek TCP testleri kalır.
Asio test check sayısı fault campaign attempt sayısına bağlıdır; her receipt kendi
attempt/terminal/dispatch/rearm/recovery sayısını yazar. Toplam 30 C++ + 3 tool =
33 grup. Tam Debug, ASan/UBSan ve temiz ZIP replay ayrı kayıtlanır. Leak detection
sanitizer koşusunda kapalıdır. MSVC/Windows Native/CRT, gerçek DLL Golden/oyun,
özgün filesystem version/allocator/IO/scheduler parity ayrı ve NOT_RUN/UNPROVED'dur.
