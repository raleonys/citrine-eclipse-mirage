# Profile çocuk mesajı ve hazırlanan yanıtın devamı

Durum: **yalnız kaynak; derleme, proje testleri, link veya DLL çalıştırılmadı**.
Bu seçili HOST politikası033d70'i çocuk işte, yanıt scheduling ve021770
closure temizliğini asıl worker'da çalıştıracak şekilde bağlar. Orijinal native
ayrık-thread zamanlamasının birebir rekonstrüksiyonu veya üretim DLL kapanışı
iddiası değildir. Varsayılan InlineJobs aynı worker'da çalışmaya devam eder.

`Application::JobHandoff::process_message()` acquired bellek, exact job/session
generation, closure/owner/function/control eşleşmesi ve başarılı start/detach
gerektirir. Tek bütün Parent Operation altında canlı JobPermit alır; Profile
lifecycle/başka çalışma ile çakışmayı reddeder. Kaynak checkpoint'i her erişimde
dispatch tombstone'u pinleyip Application'a sonra erişir; kopyalanmış izin
Operation bitince geçersizleşir. Metadata kilitleri Parent/provider çağrıları
boyunca tutulmaz.

Exact session ile eşleşen sağlıklı `native_profile::Logic` ve `JobLogic`
birlikte zorunludur. Graph'ın önceki JobLogic bağlantısı böyle seçilir; generic
SessionLogic fallback yoktur. Somut Policy'nin çocuk04a47c temizliği de gerekir.
Asıl istek boşsa033d70 çalıştırılmaz. Doluysa033d70 bir kez çağrılır, mutasyona
uğrayan istek mevcut closure'da, yanıt shared claim içindeki stabil alanda
tutulur. Bilinmeyen033d70 prefix'i stack sonuçlarına bağlanmaz. İşlemeden önce
yakalanan `had_message`, işleme sonrasında isteğin boşalmasını ayırt eder.

Akış `Fresh → Processing → Prepared → Consuming → Consumed` olarak tek yöndedir.
Hata mevcut claim/Profile'ı poisoned tutar; tekrar işlem veya tahmini rollback
yapılmaz. `release_job` yalnız Fresh işlerde eski runner'ı yeniden uygun hale
getirebilir. İşlenmiş bir mesajın bu yolla ikinci kez çalıştırılması reddedilir.

Çocuk `return_memory()` çağrısını bitirdikten ve owning handoff erişimleri
serialize edildikten sonra asıl worker bütün normal Parent Operation altında
`complete_job(handoff)` çağırabilir. `after_completion()` da Returned+Prepared
ve busy olmayan claim'leri Policy'nin opaque lifecycle callback'ine girmeden
tüketir. Somut Policy nonreentrant olduğu için bu sıra önemlidir. Bir scheduler
asıl IOCP worker'ına dönüş bildirimini ayrıca sağlamalıdır; bu API onu başlatmaz.

Devam yolu mevcut021770'in zaten var olan FrameMessageInvoker parametresini
kullanır. Closure ve hazırlanmış istek header'ı yeniden eşleştirilir. Asıl istek
dolu idiyse `dispatch_prepared_response` mevcut028d30'un033d70 sonrası gövdesini
çalıştırır: shared acquisition, iki Environment kopyası,021420 response dispatch,
ters sırada environment destruction, residual control/string temizliği.
İşleme sırasında istek sıfır uzunluğa inse bile bu yanıt gönderilir; empty-request
kısayolu veya ikinci033d70 çağrısı yapılmaz. Asıl istek zaten boş idiyse mevcut
empty-message yolu seçilir.

Sonra021770 asıl worker'ın04a47c kaydını temizler ve closure48'i bırakır. Profile
raw-job strong pin'ini bırakır, detached/returned kaydı retire eder, claim Released
olur. Çocuk04a47c ve asıl worker04a47c ayrı aktörlerin kayıtlarıdır. Yeni export,
base virtual slot veya021770 imza değişikliği yoktur. Yeni kaynak helper'ı native
VA'dan yürütülen bir fonksiyon değildir. Gelecekte yetkilendirilecek derlemede
değişen header'ların ilgili tüketicileri de yeniden derlenmelidir.

Kapanışta belirsizlik olursa source job Unknown/poisoned kalır. Native response
tail'e transferden önce yanıt claim'dedir; tail başladıktan sonraki hatalar
mevcut ham buffer/control sahiplik prefix'lerini korur, kurtarılabilir bir
rollback veya tekrar scheduling garantisi vermez. Logical bellek dönüşü,
fiziksel native thread/FLS çıkışı ve backing/code finality ayrı yükümlülüklerdir.

`message_dispatch_tests.cpp` içine çalıştırılmamış18 durumluk hazırlanmış yanıt
matrisi, erken alias reddi ve021770 hazırlanmış yanıt senaryosu eklendi.
Inline/heap/aligned heap, boşalmış heap isteği, boş veya NUL içeren yanıt,
iki kopya/tek scheduling, doğru free sayıları ve closure48 sırası tarif edilir.
**Bu kaynaklar tam prepared Graph/Profile fixture değildir; hiç çalıştırılmadı.**

Kalan ana bağlantılar: native Jobs'un owning handoff transferi, start/detach
sonrası acquire, logical dönüş/IOCP bildirimi, fiziksel thread/FLS join/reap ve
uçtan uca prepared Graph/Profile başarı/belirsiz-prefix senaryosu. Çocukta native
response scheduling yetkisi açılmadı; bu aşama açıkça asıl-worker devamını seçer.
İki üretim loader hook'u ve daha geniş CRT/image/foreign-code ömür sözleşmeleri
ayrıca açıktır. Güvenilir toplam DLL tamamlanma yüzdesi çıkarılmamıştır.
