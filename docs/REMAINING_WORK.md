# Güncel açık işler

Güncel değişiklikler [CODEBASE_OPTIMIZATION.md](CODEBASE_OPTIMIZATION.md), yerel
test sonuçları [doğrulama kaydı](evidence/optimization/VALIDATION.json) içindedir.

Önceki yüklenen DLL için beş Windows profili ve Native193/193 PASS loglarla
doğrulandı; oyun kabulünü kullanıcı bildirdi. Bu sonuçlar
[sabit kabul kaydında](evidence/OWNER_NATIVE_ACCEPTANCE.json) korunuyor.
Önceki DLL'in kabulü yeni XML/Release kaynak değişikliklerini kapsamaz.

| Açık iş | Sonraki işlem | Kanıt sınırı |
|---|---|---|
| Yeni Windows/MSVC kabulü | Beş normal build profili ve Native Acceptance'ı temiz kaynakta çalıştır. | Yeni kaynak için LLVM-MinGW/Wine ve sanitizer kanıtı var; MSVC çalıştırılmadı. |
| Yeni oyun kabulü | Production dist/preloader_l.dll ile kullanılan oyun senaryolarını ve kapanışı doğrula. | Önceki oyun PASS yeni kaynak için yeniden test yerine geçmez. |
| Native boyut katkısı | Aynı MSVC/SDK/Release ayarlarıyla önce/sonra derle; collect_size_evidence.py ile DLL/MAP/OBJ kanıtlarını topla. | Önceki native DLL/log mevcut; aynı build'in native OBJ/MAP'i yüklenmedi. |
| Olağanüstü Asio teardown | stop() post/join/resource failure yollarını sahiplik korunarak native hata enjeksiyonuyla incele. | Destructor exception uyarısı takip olarak açık; güvenli düzeltme kanıtlanmadığı için exception yutma eklenmedi. |

AST'de başvuru görülmemesi silme kanıtı değildir; virtual/callback/lifetime ve
geliştirme API kökleri korunur. Aşağıdaki önceki notlar tarihsel kayıttır ve
güncel yerel sonuç/kalan iş listesi yerine kullanılmamalıdır.

## Tarihsel kabul notları

Güncel not (YF-BLUEFIELD-ALIGN-20261029): Bluefield 11. hook/lifecycle ve tüm
beyan edilmiş Brownfield metadata hedefleri kaynakta bağlıdır. Güncel doğrulama
[BLUEFIELD_ALIGNMENT_STATUS.json](evidence/HOOK_ALIGNMENT_STATUS.json)
içindedir. Yeni kaynakla gerçek Windows/MSVC Native/Production ve oyun kabulü
hâlâ gereklidir. Aşağıdaki önceki kabul ayrıntıları tarihsel kayıttır.

Güncel not (YF-GOLDEN-AVATAR-20261029): Dört Golden ürün hedefi artık tanımlıdır;
önceki hedef-eksik BLOCKED durumu giderildi. Kalan iş yeni kaynakla gerçek
Windows Native/Golden kabulüdür. [Güncel kayıt](GOLDEN_AVATAR_FIX.md).
Aşağıdaki liste önceki kabul durumunun tarihsel ayrıntılarını içerir.

# Açık kalan kabul işleri

Bu liste uygulama kodu eksikliği ile çalıştırılmamış kabul testini ayırır.
Güncel kaynak, sabit C arayüzü ve Wine doğrulaması
[PUBLIC_ABI_STATUS.json](evidence/PUBLIC_ABI_STATUS.json) içindedir.

| İş | Somut sonraki işlem | Mevcut sınır |
|---|---|---|
| Windows/MSVC | Native profilini namespace çakışması düzeltmesiyle tekrar çalıştır; tam CTest sonucunu sakla. | Son kullanıcı loglarında Compile/Debug runtime ve CRT6/6 PASS. Tam Native test grubu async receive testindeki C2668 nedeniyle henüz çalışmadı. |
| Windows komut arayüzü | Compile/Debug/Native/CRT girişlerinde hata kodu, son sonuç ve console pause davranışını gerçek PowerShell/CMD ile çalıştır. | Kaynak ve negatif sözleşme denetimleri geçti; Windows penceresi davranışı çalıştırılmadı. |
| Gerçek online provider | Doğru hash/sürümü belli `preloader_l_online.dll` ile version2/factory/virtual-slot ve hata/cleanup yollarını çalıştır. | Mevcut test kontrollü fixture DLL'i kullanır; özgün harici provider verilmiş ve kabul edilmiş sayılmaz. |
| Adı/sürümü belli Consumer/oyun | Hangi host'un bu branded DLL'i nasıl yükleyeceğini pinle; gerçek başlangıç, LSX istekleri, hook koşulları ve kapanışı izle. | Sağlanan özgün EXE'nin statik importu `anadius64!anadius`; bu kaynak paket EXE'yi değiştirmez ve farklı isimlere otomatik uyumluluk iddia etmez. |
| Tam Golden zinciri | Gerçek MSVC linkinden kalan section/header/import/CRT hedeflerini kanıtla; ardından altı aşamayı gerçek DLL üzerinde çalıştır. | Resource profili ve beş C export sabit. Artık tüm ürün hedefleri tanımlıdır; yeni kaynakla gerçek MSVC altı kapı + final isim runtime doğrulaması çalıştırılmalıdır. Fixture sonuçları MSVC Golden yerine geçmez. |

548-byte legacy ABI, kanıtlanmış açık iş listesinden çıkarılmıştır. Sağlanan
RE kaynaklarında bu boyutta yetkili bir arayüz tanımı bulunmadı. SURGE marker'ı
ve gerçek DllShutdown uygulanmıştır; ayrıntı [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
Library senkronizasyonu, yayın ve resmî release bu çalışmanın parçası değildir.
