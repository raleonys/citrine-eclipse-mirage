# Hook kapanışı, timer containment ve socket-close incelemesi

**Donor emekliliği BLOCKED kalır.** Bu devam turu bir doğrulanmış üretim
düzeltmesi, üç kalıcı test executable'ı ve Brownfield'in kendi Asio snapshot'ıyla
yeniden üretilen bir sahiplik açığı içerir. Socket-close bulgusu çözülmüş değildir.
Önceki rapor [DONOR_RECOVERY.md](DONOR_RECOVERY.md) tarihsel kapsamıyla korunur.

## Uygulanan hook cleanup düzeltmesi

`Application::Impl::stop`, hook quiesce başarısızlığını kendi içinde sonsuz
`while`/Sleep döngüsünde yeniden deniyordu. MinHook disable veya release sürekli
başarısızsa worker bu çağrıdan çıkamıyor; DLL worker'ının mevcut `ERROR_BUSY` ve
sonraki shutdown isteğiyle retry protokolüne ulaşamıyordu. Çağıranın
`DllShutdown(timeout)` beklemesi süre sonunda dönebilse de içerideki cleanup
bitmeyen denemeyi sürdürüyordu.

Artık her stop isteği tek quiesce denemesi yapar. Tamamlanmazsa exception aynı
Application/Hooks/Binding sahibini canlı tutan DLL worker'ına ulaşır. Worker
cleanup başarısızlığını kaydeder ve sonraki stop isteğini bekler. Başarılı disable
tekrar yapılmaz; drain veya engine release kaldığı yerden tamamlanır. Owner ancak
binding ve trampoline sahipliği bırakıldıktan sonra yok edilir. Üç C export,
ordinal'lar ve mevcut0–4 shutdown kodları değişmedi. Bu durumda eski kodun pending
timeout sonucunun yerini, worker'ın raporladığı `DLL_SHUTDOWN_CLEANUP_FAILED`
alabilir; yeni durum kodu eklenmedi.

100 ms yalnız mevcut hook drain denemesinin bütçesidir. Bu değişiklik native
API'leri, provider/handler join'lerini veya bütün Application shutdown işlemini
preempt eden bir toplam süre garantisi değildir. Caller timeout'u ve sahipliği
koruma sözleşmesi devam eder. Harici audit'in tüm shutdown liveness konusu
tamamlanmış sayılmaz.

## Gerçek owner zincirinde önce/sonra kanıtı

`application_shutdown_failure_contracts` gerçek `dll_entry.cpp` worker'ını,
`DllShutdown`, `Application`, hook transaction ve binding'i çalıştırır. Fixture
wrapper'ı yalnız izole Options ve sayaçlar sağlar; lifecycle işlemlerini gerçek
Application'a devreder. MinHook API ve external-thread scan sınırları kontrollüdür:
bu fixture gerçek hook patching veya thread suspension yapmaz. Native call drain
senaryosunda gerçek callback `Lease` sahibi açık tutulur; sayaç değeri elle
uydurulmaz.

| Hata | Eski kaynak | Düzeltilmiş kaynakta beklenen ve görülen |
|---|---|---|
| Hook disable başarısız | İç retry döngüsünden çıkmıyor. | Tek denemeden sonra cleanup failure; aynı binding/owner saklanıyor. |
| Gerçek in-flight Lease sürüyor | İçeride drain tekrarları sürüyor. | Lease ve engine canlı kalıyor; lease bırakıldıktan sonraki istekte tamamlanıyor. |
| Engine uninitialize başarısız | İç cleanup döngüsü tekrar ediyor. | Önceden tamamlanmış disable tekrarlanmadan yalnız release yeniden deneniyor. |
| External unload scan reddediyor | İç quiesce döngüsü sürüyor. | Owner korunuyor; scan izin verince sonraki istekte aynı owner kapanıyor. |

Aynı son test kaynaklarıyla dört senaryo önceki kaynakta FAIL, düzeltmede PASS.
Başarısız fixture testleri de fault'u kaldırıp DLL worker'ını kapatır; çalışan
trampoline/owner üzerinde kör FreeLibrary yapılmaz. Önceki kaynak, teslim edilmiş
arşivin manifesti doğrulanmış temiz açılımından derlenir; değişen çalışma ağacı
baseline yerine kullanılmaz. Build kayıtları harness ve application hash'lerini
ve her iki varyantın kullandığı aynı eski core archive hash'ini tutar.
[PINNED_RESULTS.json](../metadata/evidence/shutdown/PINNED_RESULTS.json).

## Timer hipotezinin sonucu: test edilen çağıranlar fail-closed

`refresh_deadline` yeni bekleyişi başlatmadan cached deadline'ı günceller.
Tek başına metoda bakınca exception sonrası cached state şüpheli görünür.
Ancak canlı oturuma etkisi, exception'ı çağıranın nasıl ele aldığına bağlıdır.

Yeni `timer_failure_contracts`, gerçek Asio socket/IOCP timer ve üretim Client
callback'leri üzerinde üç yolu sınar: receive callback sırasında wait initiation
hatası, receive sırasında cancellation sınırında exception ve timer callback'i
içinde reschedule hatası. Her üçünde session kapanır, cached deadline temizlenir
ve peer kapanışı görür. Eski kaynak da bu testleri geçirir. Bu yüzden timer runtime
politikasına veya vendor'a gereksiz bir düzeltme uygulanmadı.

Hata injection'ı yalnız test executable'ındaki timer sınırındadır. Başarılı
çağrılar gerçek Asio timer'a gider; kontrollü exception'lar operasyondan önce
üretilir. Bu, gerçek Windows'ta `cancel()`ın belirli bir hatayı ne sıklıkta
ürettiği veya bütün olası kısmi OS arızalarının kapandığı iddiası değildir.
Örneğin IOCP completion post başarısızlığında Asio'nun kendi completed queue
fallback'i vardır; bunun varlığı otomatik olarak C++ cancel exception'ı sayılmaz.

## Socket close: donor aktarımıyla çözülemeyen açık bulgu

Windows IOCP `win_iocp_socket_service_base::close`, `socket_ops::close` sonucunu
döndürürken implementation socket'ini **hata halinde de invalid_socket yapar**.
Bu nedenle yalnız `is_open()` ve ikinci Asio `close()` çağrısıyla native handle'ın
retirement'ı kanıtlanamaz. İkinci çağrı artık Win32 close'u denemeyebilir.

`socket_close_semantics_contracts` test-only Win32 sınırında native close'u
kontrollü olarak yapmadan hata döndürür. Sonraki Asio kodu ve socket gerçektir.
Normal close, ilk would-block sonrasında başarılı iç retry, iki kez withheld
would-block ve withheld network-down olmak üzere dört senaryo vardır. Son iki
senaryoda raw socket hâlâ geçerli, Asio `is_open()` false ve ikinci Asio close yeni
bir native deneme yapmıyor. Test native descriptor'ı sonunda kendisi kapatır.
Bu karakterizasyon testinin PASS olması ürünün bu hatadan kurtulduğu anlamına
gelmez. Kontrollü kombinasyonların gerçek Windows'taki oluşma sıklığı ölçülmedi.

Brownfield'in `AsioListenerOwner::apply_stop` kodu close hatasında return eder;
ancak sonraki kontrol veya cancelled accept completion, Asio kaydını kapalı
görüp quiescent bildirimine ilerleyebilir. Gerçek donor kaynak ve donor arşivinin
kendi Asio header'larıyla probe sonucu:

```text
raw_socket_alive=1 listening=1 quiesced=1 failed=1
```

Donor ve mevcut vendor snapshot'ı ikisi de1.38.2 bildirir ama dosyaları birebir
aynı değildir. Bu nedenle yalnız mevcut vendor ile çalıştırılan ilk probe'a
dayanılmadı: donor'un arşivden çıkarılmış kendi include ağacıyla yeniden derlendi
ve aynı sonuç alındı. [Donor build kimliği](../metadata/evidence/shutdown/DONOR_CLOSE_BUILD.json),
[sonuç](../metadata/evidence/shutdown/ACTUAL_DONOR_CLOSE_RESULT.log),
[snapshot farkı](../metadata/evidence/shutdown/ASIO_CLOSE_COMPARISON.json),
[probe kaynağı](../metadata/evidence/shutdown/listener_close_probe.cpp.txt).

Brownfield'in ayrı raw `SocketHandle` adaptörü başarısız close'da kendi numeric
owner'ını korur. `test_socket_close_retry` bu adaptörü fake `SocketApi` ile sınar;
Asio close davranışını sınamaz. Bu iki mimarinin test sonuçları birbirinin yerine
kullanılamaz. Benzer şekilde `test_shipping_shutdown_contract` enum static_assert
testidir; gerçek worker/cleanup hata enjeksiyonu değildir.

Mevcut ürünün native-close hata sahipliği de **açıktır**. Asio'nun kaydı sildikten
sonra raw sayısal handle'ı körlemesine yeniden kapatmak handle reuse ve pending
IO/callback sahipliğini çözmez. Retry sözleşmesi, native descriptor'ın durumunu,
cancel completion'larını, buffer ömrünü ve DLL unload iznini birlikte ele almalı.
Vendor baytlarını değiştirme veya donor socket runtime'ını toptan ithal etme
yapılmadı. Bu açık konu emekliliği engellemeye devam eder.

## Kapsam, doğrulama ve kalan işler

9 donor dosyasında ek branch karşılaştırmasıyla manuel kayıt sayısı76 oldu;
1729 first-party dosyada henüz manuel kayıt yok. 76 tam dosyanın bütün semantik
kapanışı iddia edilmiyor. [Ledger](../metadata/evidence/shutdown/MANUAL_REVIEW_LEDGER.json),
[bütün açık dosyalar](../metadata/evidence/shutdown/ALL_DONOR_FILES.csv).

Yerel Release201/201 ve etkilenen Debug13/13 geçti. Üretim C export'ları üç ve
ordinal'ları aynı; geliştirme DLL'i3350→3350, eklenen/çıkarılan sembol yok.
Public header, dört vendor, PE/resource/avatar, CFG/SDK/port ve Golden linker
politikaları değişmedi. Üretim64 ve korunan referans74 CPP ayrımı korunur.
Son source QA ve paket doğrulama kaydı
[VALIDATION.json](../metadata/evidence/shutdown/VALIDATION.json) içindedir.

Bu sonuçlar LLVM-MinGW/Wine sınıfındadır; gerçek MSVC/Windows IOCP arızası,
Golden veya oyun kabulü değildir. Native Windows/Golden ve oyun tekrarı bekliyor.
Socket close ownership dışında registry provenance, aggregate memory budgets,
provider/handler shutdown liveness, environment/console rollback ve kalan
source/test/tool ailelerinin incelemesi sürmelidir. Donor'lar obsolete ilan edilmedi.

Bu turun exact parent kaynak ZIP SHA256'sı:
`ef4572d32076e0ace03039b17e0fb6103b084a395ccdd27fd59b76886f2d9d1c`.
İncelenen dosya farkları [REVIEWED_CHANGES.json](../metadata/evidence/shutdown/REVIEWED_CHANGES.json)
içindedir; kabul edilmiş eski binary/log paketi parent yerine kullanılmaz.
