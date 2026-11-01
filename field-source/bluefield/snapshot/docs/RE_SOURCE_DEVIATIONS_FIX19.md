# FIX19 — Registry kaynak beklemesi ve callback kilit sahipliği

NON_RELEASE tam kaynak devamı; mühürlü FIX18 ebeveyninden türetilmiştir. Karşılaştırma kullanıcının tam RE kaynak paketidir. Özgün DLL binary/runtime eşdeğerliği iddiası değildir.

## D12de kalan sapma

RE `anadius-registry-source/registry.cpp::query_impl` önce orijinal queryyi çağırır, tracked ClientValue eşleşmesini seçer, `wait_036880(-1)` bekler ve ardından ClientPathi yazar. LanguageValue ve ilgisiz sorgular bu beklemeye girmez.

FIX18de native `Effects::wait_036880` yalnız portun atanmış olmasına bakıyordu. Bu provider ve ek DLL işçisinin tamamlanmasını denetlemiyordu. Bu sadeleştirmenin ilk tarihsel gerekçesi kayıtlı değildir; Asio veya MinHook upstream gereği değildir.

FIX19 native adapterı aynı count ile `Service::wait_steps`e bağlar. FIX18in shared source predicatei, provider/server/extra aşamaları ve owned cancellation düzeni korunur. Provider leaf adapterının port kontrolünden önce gerçek Service işçisi phase1 beklemesini yapar; network ve registry count=-1 ilk üç normal tamamlanmayı bekler. İptal phase başarı bayrağı üretmez.

## Neden yalnız wait çağrısını değiştirmedik?

Bizim eklediğimiz native callback recursive mutexi querynin tamamında tutuluyordu. Sorgu provider/ek DLL aşamasını beklerken bu kilidi tutarsa, başlangıcın ilerlemesi için gerekli başka bir registry callbackini engelleyebilir. Kaynağın registry algoritmasında bu ek mutex yoktur.

Yeni küçük `callback_lock::Guard`, her callbackin mevcut mutex acquisitionını thread-local bir frame olarak izler. Yalnız registry wait süresince `Pause` o threadde callbacklerin aldığı tüm recursive acquisitionları bırakır. Bekleme dönüşünde acquisitionlar ilk alınma sırasıyla geri alınır. Normal dönüş, exception ve nested pause/reentry aynı RAII sahipliğini korur. Frame listesi allocation veya yeni derinlik/adet sınırı eklemez.

Tek recursive acquisition bırakmak yeterli değildir: query başka bir callback üzerinden yeniden girilmişse dış frame kilitleri hâlâ tutulabilir. Test bu nedenle üç callback frameiyle başka bir threadin ilerlemesini denetler. Haricî/untracked kilitlerin açıldığı iddia edilmez; sadece native invokenin kendi Guardları izlenir.

Native `Lease` mutex askıya alınırken aktif kalır. `invoke` içindeki lease scopeu, active-call sayacı ve binding close/unload koşulu değiştirilmedi. Context/modül query bitmeden yok edilemez. Registry wait içindeki senkronizasyon normal dönüşte önceki LastError değerini geri koyar; native result/outgoing LastError capture akışı korunur. Bunların gerçek Win32 yürütmesi bu ortamda yapılmadı.

## Korunan kaynak algoritması

`src/hooks/registry.cpp` FIX18 ile byte-identicaldır. Tam RE kaynağına göre yalnız include ve namespace relocationı çıkarıldığında da eşleşir. Orijinal-call, match, wait, write ve type sırası değişmedi; caller capacity capture, null/size davranışı ve source exception yolu korunur. Her queryye önceden genel bir wait eklenmedi; yeni Windows status kodu veya sahte başarı üretilmedi.

Bekleme öncesinde selected branch kararı verilmiştir; source fonksiyon bu wait üzerinden bir set/map iteratorı taşımaz. Shared Values map native setup sonrası salt okunurdur. Tracking setleri callback kilidi altında kullanılmaya devam eder. Testte bir peer tracking girdisini mock-close ile değiştirebilir; bu, gerçek Windows handleını eşzamanlı kapatmanın API garantisi olarak yorumlanmaz.

## Test yüzeyi

1. RE paketinin `registry_tests.cpp` dosyası include/namespace relocationıyla doğrudan alındı. Bütün mevcut assertionlar korunur. Gerçek SDK DWORD/HKEY ABI fonksiyonu yalnız Windows altında derlenir; Linuxa sahte SDK typedefi eklenmedi. Portable gövdede957 kontrol çalışır.
2. Yeni51 kontrollü fixture gerçek, değiştirilmemiş registry algoritmasını, production Guard/Pause kodunu, gerçek recursive mutexleri ve gerçek StartupSteps CVyi birlikte çalıştırır. Original Windows registry API effectleri mocktur.
3. ANSI ve UTF-16 ClientValue dalları üç recursive frame altında bekletilir. Bir peer ilgisiz query ve mock-close yapıp provider aşamasını bitirir. Sorgu üçüncü extra-worker aşamasını da bekler; sonra kaynak buffer/type/size cevabını verir. Bekleme boyunca ayrı lifetime sahibi aktif kalır. Bu fixture native Lease yürütmesi diye sunulmaz.
4. Language/unmatched/untracked dalların beklememesi, nested pause/reentry, exception sonrası lock sahipliği, wait hatasının source outputları ve cancellation denetlenir.

İki yeni test hedefi mevcut production registry.cppyi test-support source olarak derler. CMake ve portable runner açıkça bağlanmıştır. Production TU graphı veya uygulama kaynak sahipliği taşınmadı. Eski C++ test dosyaları ve mevcut test beklentileri aynı kaldı; source-policy testine üç yeni negatif vaka eklendi.

## Kapsam ve açık kayıtlar

İki mevcut runtime cpp ve bir yeni header değişti. Önceki119 src dosyası ile1.060 vendor dosyası aynı kalır. FIX18 startup/provider/profile/thread/extra kaynakları, session/handler worker sayısı, diğer callback kilitleri, hook batch, input/cache kısıtları ve Golden araçları değişmedi.

D12 kaynak wait kapsamı içinde kapandı. D06 genel serialization/request-state/attempt-lock kararları ayrı açık kalır. Toplam16 inceleme/kısmi kayıt sürer; bunlar16 kesin bug değildir. Cumulative raporun düzeltilmiş bulgularındaki `difference/impact` artık güncel çözümü gösterir; ilk gözlemler historical snapshot alanına ayrılmıştır. Ebeveyn paket/tarihsel receiptler yeniden yazılmaz.

Güncel bağımsız master MD/JSON version9, revision2026-10-25.1 kontrol edildi. AGENTS/kapsam/QA başarılı olması kendi başına yeni kullanıcı onayı üretmez. Kod değişiklikleri ve kaynak karşılıkları CHANGE_AUTHORITY_FIX19.json içinde sınırlıdır. Windows/MSVC/DLL/IOCP/native hook/LastError/provider/CRT/game sonuçları NOT_RUN ise portable/synthetic sonuçlarla doldurulmaz.
