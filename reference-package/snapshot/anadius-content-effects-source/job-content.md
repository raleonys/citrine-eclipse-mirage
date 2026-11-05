# Content çocuk iş kapsamı

Durum: **yalnız kaynak kodu; derleme/test/link/DLL çalıştırılmadı**. Dört dosya
bağımsız statik incelemeden geçti. Yeni test kaynakları çalışma zamanı kanıtı
değildir; önceki PASS/ZIP kayıtları tarihsel sürümlerine aittir.

`Content::with_job` canlı Parent JobPermit, senkron POD CallCheckpoint ve
çağıranın eylemini alır. Eylem referansla çağrılır; sahip olunan std::function
aktarımı yoktur. Void ve değer sonuçları ayrı tutulur; referans sonuçlar
reddedilir, değer yerel optional'dan değer olarak taşınır. Facade, eylem,
checkpoint context'i ve tüm backing'ler çağrı sonuna kadar çağıran tarafından
korunur. Kopyalanmış izin, Parent Operation ömrünü uzatmaz.

Stack JobCall, ilk opaque callback öncesinde binding rezervasyonu altında
yayımlanır. Kapsam içinde yalnız aynı çocuk thread/fiber normal Content
metotlarına girebilir. Her metot kendi busy/checking rezervasyonunu admission
öncesinde kurar. Callback, Parent/proof, dosya ve SDK çağrıları metadata kilidi
dışında çalışır. Yutulan yeniden giriş poisoned kalır; sonraki alt çağrıya
ilerlenmez. Normal ve hata çıkışında stack descriptor yok olmadan raw kayıt
kaldırılır. Kapsam dışında normal metotlar asıl worker/fiber gerektirir;
bind/clear ve son destruction da asıl worker'da kalır.

Normal kapsam sonunda GuardedFiles hem known hem de pending-owner sıfır
olmalıdır. File-open veya initial_state sonrasında belirsizlik varsa mevcut
GuardedFiles ham dosya sahibini tutar. Kapsam kapanışı tahmini close/delete
yapmaz ve başarısız işlemi yeniden çalıştırmaz. Bekleyen dosyanın callback'i
stack JobCall'ı tutmaz: mevcut stable Content Impl callback'i kalır ve poisoned
durumda eski caller checkpoint'ine ulaşmadan reddedilir.

Graph Content adapter'ı mevcut opaque zinciri içinde yalnız seçili somut
Content'i shared olarak tutup aynı Graph JobCall Parent/permit ve Impl.admit
checkpoint'ini geçirir. Çocuk legacy/custom fallback yoktur. Printer sonucu,
kapsamın son kontrolünden önce Graph writer envanterine alınır. Dönen somut
MemoryDocument kendi XML belleğini tutar; sonraki erişimler mevcut Graph Writer
kontrolleriyle korunur. Legacy tek-path avatar çağrısına owner/size uydurulmaz;
Graph mevcut tam AvatarRequest yolunu kullanır.

Dosya/filesystem/encoding algoritmaları değiştirilmedi. Filesystem çağrıları
per-call state kullanır; encoding rebuilt CRT/Win32 sözleşmesini korur. Online
normal avatar çağrısının creator affinity'si yoktur; controller-only cleanup ve
modül/code ömrü kuralları korunur. Parent izni, ödünç Runtime/Paths/Online veya
lower-code ömrünü ve dış global mutasyonların serialize edildiğini tek başına
kanıtlamaz; bunlar çağıranın mevcut sözleşmeleridir.

`effects_tests.cpp` içine gerçek Domain/NativeThreads/prepared iki Parent
altyapısını kullanan alt katman senaryosu yazıldı. Kontrollü clock, memory XML/
INI, enjekte directory/CopyFile/encoding ve test EXE'sindeki Online slot28
üzerinden pozitif eylem; yanlış Parent/actor, default-child, expired/fresh seal,
değer/void sonuç, scope dışına yetki sızmaması, yeniden giriş ve original reuse
tarif edilir. İki yeni izole süreç modu, çocuk scope içinde open/initial-state
denial sonrası dosya sahibi/close/destructor sayıları değişmeden süreci bitirir.
**Hiçbiri çalıştırılmadı. Tam prepared Graph/Profile çocuk testi değildir.**

Kalanlar: Licensing alt sağlayıcısı, Profile'ın JobLogic seçimi ve tam request/
response/closure/021770 son temizleme zinciri; native Jobs/gerçek thread-FLS
çıkışı bağlantıları ve prepared uçtan uca fixture. İki üretim loader hook'u
ayrıca açıktır. Yeni native gövde/ABI kapanışı veya toplam DLL yüzdesi yoktur.
