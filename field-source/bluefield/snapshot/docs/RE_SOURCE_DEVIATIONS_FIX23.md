# FIX23 — Gerçek guard düzeltmesi ve açık kayıtların kapanışı

NON_RELEASE tam kaynak devamı. Exact FIX22 ebeveyninden türetilmiştir. Karşılaştırma kullanıcının tam RE paketidir; özgün DLL binary eşdeğerliği değildir.

## 15 sayısı neden düşmüyordu?

Önceki rapor, tamamlanan bir kaynak düzeltmesini native test yapılmadığı için açık tutuyor; gerekçeli korunabilecek entegrasyon farklarını da henüz değerlendirilmemiş kaynak hatalarıyla aynı sayaçta taşıyordu. FIX21/FIX22 gerçek lexical düzeltme yaptı fakat D10a başka sınırların belirsizlikleri eklenince ana kayıt kapanmadı. Bu kapsam yönetimi hatasıydı. Ayrıca D11de gerçek bir kaynak/test sapması hâlâ bulunuyordu. Native test eksikliği kaynak düzeltmesinin yerini tutmaz; tamamlanan kaynak düzeltmesini de sonsuza kadar açık bırakmaz.

| Kayıt | FIX23 sonucu | Yapılan iş |
|---|---|---|
| D11 | Kaynak guardı düzeltildi | Null factory sessiz return yerine packet OutOfEvidenceInput; orijinal bağımsız test bloğu literal geri alındı |
| D10 | Tamamlanmış kaynak kapsamı kapandı | FIX21/FIX22 lexical düzeltmeleri kapatıldı; native kanıt ayrı NOT_RUN defterinde |
| D05 | Review tamamlandı, fark korunuyor | Queued activationın table/context staging sonrasında yapılması gerekçeli entegrasyon tercihi; source veya upstream zorunluluğu denmiyor |
| D17 | Review tamamlandı, fark korunuyor | Sahipli CRT environment copy warning/lifetime uyarlaması; ek allocation/error farkı açıkça korunuyor |

Açık kaynak incelemeleri15→11dir. Bu dört bug düzeltmesi değildir:1 yeni runtime guard düzeltmesi,1 önceki kaynak düzeltmesinin kapanışı,2 incelemesi tamamlanan korunmuş farktır. Kaynakla farklılıkları D05/D17 satırlarından silinmez. Native testler hâlâ NOT_RUNdır; yeni bir native PASS veya tam eşdeğerlik sonucu çıkarılmaz. Bütün15 satırın before/after gerekçesi AUDIT_DISPOSITIONS_FIX23.csv/json içindedir.

## D11 — Hata nasıl ortaya çıktı ve ne düzeldi?

`anadius-online-source/online.cpp`, null create_online sonucunda `OutOfEvidenceInput` atar. Bu sentinel çözümlenmiş paketin bilinmeyen input-domain guardıdır; özgün DLLin aynı C++ exceptionı attığının kanıtı değildir. BLUEFIELDda bu dal `log_line(...,"provider factory rejected"); return;` olmuştu. Bu özel tercihin bağımsız ilk gerekçesi kaydedilmemiştir ve upstream bunu gerektirmez.

Üstelik unit test de `state.module==0` ve `finish` bekleyerek bu sessiz tamamlanmayı doğru sayacak biçimde değiştirilmişti. Bu test kaynak sözleşmesini korumuyordu. FIX23te önce orijinal RE testinin null-factory bloğu literal geri alındı; hâlâ FIX22 olan runtime o testte `caught` assertionıyla reddedildi. Runtime daha sonra packet guardına döndürüldü. Aynı test artık geçer; implementasyonu geçirmek için yeni bir beklenen davranış icat edilmedi. Bu, Q02nin oracle bağımsızlığı alt probleminde de somut düzeltmedir; Q02nin tamamı kapanmaz.

Present provider null nesne döndürürse artık status/getter çağrılmaz ve worker normal `finish` yayınlamaz. Module41 synthetic handleı kaynak testindeki gibi caller-owned kalır. Üretimde mevcut Service catch/fail_startup cancellationı ve OnlineSession.close cleanup ownerı korunur; yeni bir cleanup/fallback/timeout eklenmez. Absent providerın offline returnü, missing export/version branchleri ve normal profil/kimlik/avatar davranışı byte-identicaldır.

Yalnız src/profiles/online.cpp değişir.121 diğer src ve1.060 vendor dosyası aynıdır. Testlerde yalnız online unit dosyasındaki tek null-factory bloğu kaynak paketinden literal geri alınır; diğer önceki C++ testleri aynıdır. Runtime graphı, test hedefleri ve Golden araçları değişmez.

## D10 — Kaynak kapanışı, native kabulü değil

D10un asıl konusu lexical adapterın kaynak algoritmalarını bypass etmesi ve ek normalizasyondur. FIX21 ek normalizasyonu kaldırdı; FIX22 Windows parent/join/filename/resolve çağrılarını mevcut kaynak bodylerine bağladı. Bu kaynak kapsamı tamamlandı. IO/backend kapsamı DEP05te, statusun packet-unknown domaini D07de; encoding/native yürütme ise mevcut ayrı doğrulama defterindedir. Bunları D10a ekleyip tamamlanmış işi partial tutmak bırakıldı. Windows path/native IO/ABI test yapılmış sayılmaz.

## D05 — Neden batch korunuyor?

Native owner callback Context ve original slotlarını oluşturup gerekli hookları queueya alır; eksik stagingde activation öncesi hata verir. Gerekli son slot da eklendikten sonra tek MH_ApplyQueued çağrısı gelir; callback serialization kilidi o çağrı sırasında tutulmaz. Mevcut stage-only fixture normal/optional yüzeyleri, partial create/queue ve missing target vakalarını yürütür.

Bu sıra, callbackin henüz hazırlanmamış table/context görmesini önleyen mevcut owned integration tercihidir. Tek tek enablea geri çevirmek bu özelliği kaldırır. Bu nedenle mevcut batch korunur. MinHookun zorunlu davranışı veya özgün kaynak eşdeğerliği denmez; batch atomic activation garantisi de verilmez. Native ApplyQueued/partial rollback burada çalıştırılmadı; cleanup/fault-domain politikası D13te ve native acceptance defterinde açık kalır. Bu review kararı geçmişte kullanıcı tek tek bu sırayı onayladı şeklinde sunulmaz.

## D17 — Neden sahipli CRT kopyası korunuyor?

Microsoftun `_dupenv_s` belgesi bu APInin CRT `_environ` kaynağından allocation/copy yaptığını, bulunmayan değişkende başarı+null döndüğünü, allocation hatasını ENOMEM ile bildirdiğini ve bufferın free ile bırakılmasını belirtir. Mevcut adapter matching CRT free kullanan unique_ptr ile allocationı tutar, kendi stringine kopyalar; caller get_environment hemen kendi stringini oluşturur. Win32 live environment APIye geçilmez. Bu değerlendirme native CRT yürütmesi değildir.

Warning/borrowed-pointer lifetime amacıyla bu uyarlama korunur. Ek allocation ve system_error/bad_alloc davranışı packet getenv ile birebir değildir; kaynak farkı kaydında açıkça kalır. Caller input fixtureı mevcut bodyyle yeniden çalıştırılır; actual Windows CRT/OOM vakası NOT_RUNdır. Runtime dosyası bu karar için değiştirilmez.

Birincil API kaynağı: [Microsoft Learn — _dupenv_s](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/dupenv-s-wdupenv-s?view=msvc-170),2026-10-27 kontrolü. Bu belgeden native ABI veya game sonucu türetilmez.

## Kalan11 kayıt

DEP05 backend kapsamı; D02 thread modeli; D03 session pipelining/sıra; D06 kilit kapsamı; D07 packet-unknown status domaini; D13 unload/fault policy; D14 process/service/session hata kapsamı; D15 XML parse/depth sınırı; D16 avatar seçimi ile provider yan etkileri; Q02 diğer oracle/diagnostic kapsamı; D21 iç algoritma boundları.

Bunlar tek tür değildir ve11 doğrulanmış bug sayısı değildir. D02/D03 gibi açık davranış farkları önceliklidir. Hiçbirine yalnız sayı azalsın diye doğru denmez veya toplu rollback uygulanmaz. Native olmayan kaynak kapanışı ile native validation ayrı tutulur; eski QA ve paketler değiştirilmez.

## Doğrulama sınırları

Runtime guardının bağımsız kaynak fixtureı focused Debug ve ASan/UBSan ile; bütün mevcut44 grup CMake Release/CTest ile; exact teslim ZIP de tek temiz yeni-nesne CMake replay ile doğrulanır.61 source-policy,6 paket ve8 synthetic PE negatifleri kayıtlıdır. Source digest/ZIP SHA ve actual sonuçlar evidence/FIX23 ile dış FINAL_QAda bulunur; bu belge tek başına PASS receiptidir denmez.

İki mevcut GCC noreturn uyarısı bu kapsamda değiştirilmez. Windows/MSVC/DLL/provider/native hook/CRT/IOCP/IO/ABI/game ve gerçek DLL Golden NOT_RUN. LSan leak-free iddiası yoktur. NON_RELEASE çalışma resmî pointer veya başka FIELD projesini değiştirmez.
