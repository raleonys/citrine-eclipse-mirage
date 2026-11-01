# Native FLS/PTD yaşam döngüsü bağlantısı

Yeni `Binding`, mevcut kurtarılmış PTD/data/glue/heap/errno kaynaklarını gerçek Windows FLS slotuna ve yeniden yazılmış callback'e bağlar. Yeni çıkarılmış özgün binary gövdesi **0**; çalışma yeni bir kaynak bağlantısıdır. Tam DLL veya özgün global başlangıcı değildir.

`Binding`, yalnız kendi `state()` nesnesiyle kullanılır. İndeksi elle değiştirmeyin veya başka State/index ile kullanmayın. Bağımlılıkları önce kurun, error/PTD proxy döngüsünü bağlayın, ardından kurtarılmış `initialize_064e90` çağrısını yapın. Callback context taşımadığı için bu modül **aynı anda tek etkin domain** destekler; ikinci binding tahsis yapmadan reddedilir.

Kapanışta tüm kullanıcılar ve callback/initializer çağrıları durmuş olmalıdır. Kurtarılmış `uninitialize_064ecc`, `FlsFree` sonucunu dikkate almadan kendi indeksini geçersiz yapar. Yeni adapter gerçek slot indeksini ayrıca saklar: release başarısızsa bağlam ve bağımlılıklar yaşar; `retry_release()` ile gerçek bırakma tekrar denenir. Yeniden initialize, henüz bırakılmamış native slotu kaybettirmez. Callback ve bağımlılıklar, gerçek release başarıyla tamamlanana kadar korunur. Açık slotla binding yıkımı sözleşme ihlalidir ve `std::terminate` çağırır; otomatik veya sessiz unbind yapılmaz.

`WINAPI noexcept` callback sentinel pointer'ı dereference etmez; beklenmeyen C++ istisnasını Win32 sınırından çıkarmaz ve `std::terminate` yoluna gider. Bu korumalar yeni adapter sınırlarıdır; özgün exception/SEH davranışı olarak sunulmaz.

## Doğrulama

`build.ps1`, LLVM-MinGW/C++20/O2/Wall/Wextra/Werror ile kendi EXE'sini derler: **127 kontrol + 3 terminal senaryo PASS**. Bağımsız inceleme `review-native-fls.json` dosyasındadır. CMake yolu ayrıca çalıştırılmadı; ana PowerShell projesi ayrı ortak doğrulama kaydı üretir.

- Gerçek `FlsAlloc/GetValue/SetValue/Free`, gerçek process heap ve kurtarılmış PTD constructor/destructor kaynakları kullanılır. PTD cache hit, explicit free/reacquire, typed Context alanının bırakılması ve FLS callback temizliği sınandı.
- Allocation sırasında sentinel üzerinden reentry null döner; construct sırasında reentry önceden yayımlanmış PTD'yi görür. Acquire'ın LastError koruması ve errno bootstrap döngüsünün sentinel ile sonlanması doğrulandı.
- Slot/allocation/publication/release başarısızlıkları tipli SDK tablosunda enjekte edilir. Başarısız release sonrasında ayrı native indeks ve PTD belleği kalır; retry temizler. İkinci domain, açık slotla yeniden initialize, erken retry ve yanlış release kimliği reddedilir.
- Kendi `std::thread` iş parçacığında ayrı bir PTD/errno oluşturulur. Main, worker'ın verilerine `join()` öncesi erişmez. Thread çıkışındaki gerçek FLS callback sonrası refcount ve heap temizliği gözlenir; main PTD değişmez. Bu, **sıralı cross-thread yaşam döngüsü** testidir; eşzamanlı çoklu worker, lock veya atomic eşdeğerliği değildir.
- Üç ayrı own-process terminal modu açık binding yıkımı, sentinelli callback ve gerçek FlsFree callback'i içindeki injected throw durumunda terminate yolunu doğrular. Testin terminate handler'ı yalnız kendi test sürecini kod88 ile `TerminateProcess(GetCurrentProcess())` üzerinden bitirir. `_Exit/ExitProcess` rundown'ının kasıtlı geçersiz callback ömrüne tekrar girmesini engeller; genel program kapanış eşdeğerliğini kanıtlamaz.

Locale referansları, lock/atomic ve başlangıç/global kimlikleri kontrollü fixture'lardır. Ptd/Context tip yaşamı mevcut glue kurucularıyla, ham byte depoları önceki native heap adapter'ıyla kurulur. Genel DLL loader/startup/shutdown, birden fazla domain, paralel worker senkronizasyonu, MSVC ABI ve SEH/unwind açıktır. Orijinal veya yeniden derlenen DLL/oyun yüklenmedi ya da çalıştırılmadı; orijinal girdiler değiştirilmedi.

## Adresli dayanaklar

`0x180064b4c/064b90/064c64/064d08/064dd0/064e90/064ecc` PTD akışları; `0x1800648fc/0649ec/0649cc` constructor/destructor/callback ve ilgili locale/multibyte yardımcıları. Statik assembly'de `064bd1..064c38` sentinel→calloc→publish→construct, `064edb..064ee0` release sonucu yok sayma ve invalidate sırasını destekler. Hash bağlı REA kayıtları `evidence.json` içindedir; tercih edilen VA'lar callback olarak çalıştırılmaz.

Microsoft'un [FlsFree sözleşmesi](https://learn.microsoft.com/en-us/windows/win32/api/fibersapi/nf-fibersapi-flsfree) slot bırakıldığında dolu fiber kayıtlarının callback'lerinin çağrılmasını açıklar. Bu bağlayıcının bağımlılık ve quiescent kapanış sözleşmesi bu sınıra göre düzenlenmiştir; özgün DLL'in runtime eşdeğerliği kurulmuş değildir.
